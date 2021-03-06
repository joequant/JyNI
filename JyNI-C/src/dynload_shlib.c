/* This File is based on dynload_shlib.c from CPython 2.7.3 release.
 * It has been modified to suit JyNI needs.
 *
 * Copyright of the original file:
 * Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010,
 * 2011, 2012, 2013, 2014 Python Software Foundation.  All rights reserved.
 *
 * Copyright of JyNI:
 * Copyright (c) 2013, 2014 Stefan Richthofer.  All rights reserved.
 *
 *
 * This file is part of JyNI.
 *
 * JyNI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * JyNI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with JyNI.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Linking this library statically or dynamically with other modules is
 * making a combined work based on this library.  Thus, the terms and
 * conditions of the GNU General Public License cover the whole
 * combination.
 *
 * As a special exception, the copyright holders of this library give you
 * permission to link this library with independent modules to produce an
 * executable, regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting executable under
 * terms of your choice, provided that you also meet, for each linked
 * independent module, the terms and conditions of the license of that
 * module.  An independent module is a module which is not derived from
 * or based on this library.  If you modify this library, you may extend
 * this exception to your version of the library, but you are not
 * obligated to do so.  If you do not wish to do so, delete this
 * exception statement from your version.
 */


/* Support for dynamic loading of extension modules */

#include "JyNI.h"
#include "importdl.h"

#include <sys/types.h>
#include <sys/stat.h>

#if defined(__NetBSD__)
#include <sys/param.h>
#if (NetBSD < 199712)
#include <nlist.h>
#include <link.h>
#define dlerror() "error in dynamic linking"
#endif
#endif /* NetBSD */

#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#else
#if defined(PYOS_OS2) && defined(PYCC_GCC)
#include "dlfcn.h"
#endif
#endif

#if (defined(__OpenBSD__) || defined(__NetBSD__)) && !defined(__ELF__)
#define LEAD_UNDERSCORE "_"
#else
#define LEAD_UNDERSCORE ""
#endif


const struct filedescr _PyImport_DynLoadFiletab[] = {
#ifdef __CYGWIN__
	{".dll", "rb", C_EXTENSION},
	{"module.dll", "rb", C_EXTENSION},
#else
#if defined(PYOS_OS2) && defined(PYCC_GCC)
	{".pyd", "rb", C_EXTENSION},
	{".dll", "rb", C_EXTENSION},
#else
#ifdef __VMS
	{".exe", "rb", C_EXTENSION},
	{".EXE", "rb", C_EXTENSION},
	{"module.exe", "rb", C_EXTENSION},
	{"MODULE.EXE", "rb", C_EXTENSION},
#else
	{".so", "rb", C_EXTENSION},
	{"module.so", "rb", C_EXTENSION},
#endif
#endif
#endif
	{0, 0}
};

static struct {
	dev_t dev;
#ifdef __VMS
	ino_t ino[3];
#else
	ino_t ino;
#endif
	void *handle;
} handles[128];
static int nhandles = 0;


dl_funcptr _PyImport_GetDynLoadFunc(const char *fqname, const char *shortname,
									const char *pathname, FILE *fp)
{
	// puts("_PyImport_GetDynLoadFunc");
	dl_funcptr p;
	void *handle;
	char funcname[258];
	char pathbuf[260];
	int dlopenflags=0;
	if (strchr(pathname, '/') == NULL) {
		/* Prefix bare filename with "./" */
		PyOS_snprintf(pathbuf, sizeof(pathbuf), "./%-.255s", pathname);
		pathname = pathbuf;
	}
	PyOS_snprintf(funcname, sizeof(funcname),
				  LEAD_UNDERSCORE "init%.200s", shortname);

	if (fp != NULL) {
		int i;
		struct stat statb;
		int fno = fileno(fp);
		fstat(fno, &statb);
		for (i = 0; i < nhandles; i++) {
			if (statb.st_dev == handles[i].dev &&
				statb.st_ino == handles[i].ino) {
				p = (dl_funcptr) dlsym(handles[i].handle, funcname);
				return p;
			}
		}
		if (nhandles < 128) {
			handles[nhandles].dev = statb.st_dev;
#ifdef __VMS
			handles[nhandles].ino[0] = statb.st_ino[0];
			handles[nhandles].ino[1] = statb.st_ino[1];
			handles[nhandles].ino[2] = statb.st_ino[2];
#else
			handles[nhandles].ino = statb.st_ino;
#endif
		}
	}

#if !(defined(PYOS_OS2) && defined(PYCC_GCC))
	//dlopenflags = PyThreadState_GET()->interp->dlopenflags;
	dlopenflags = JyNI_GetDLOpenFlags();
	//puts("dlopenflags obtained:");
	//printf("%i\n", (int) dlopenflags);
#endif

	//if (Py_VerboseFlag)
//	JNIEnv *env;
//	if ((*java)->GetEnv(java, (void **)&env, JNI_VERSION_1_2)) {
//		return NULL; // JNI version not supported
//	}
	env(NULL);
	//JyNI todo: fix verbose mode; it currently segfaults if active
	//if ((*env)->CallStaticIntMethod(env, JyNIClass, JyNIGetDLVerbose))
//	{
//		PySys_WriteStderr("dlopen(\"%s\", %x);\n", pathname, dlopenflags);
//	}
	//printf("dlopen(\"%s\", %x);\n", pathname, dlopenflags);
#ifdef __VMS
	/* VMS currently don't allow a pathname, use a logical name instead */
	/* Concatenate 'python_module_' and shortname */
	/* so "import vms.bar" will use the logical python_module_bar */
	/* As C module use only one name space this is probably not a */
	/* important limitation */
	PyOS_snprintf(pathbuf, sizeof(pathbuf), "python_module_%-.200s",
				  shortname);
	pathname = pathbuf;
#endif

	handle = dlopen(pathname, dlopenflags);
	if (handle == NULL) {
		const char *error = dlerror();
		if (error == NULL)
			error = "unknown dlopen() error";

		//todo: Check, why error is not correctly propagated on the designated way.
		//Until that is done, we print it out plainly with puts.
		puts("dlopen-error:");
		puts(error);
		//puts(pathname);

		PyErr_SetString(PyExc_ImportError, error);
		JyNI_JyErr_SetString((*env)->GetStaticObjectField(env, pyPyClass, pyPyImportError), error);
		return NULL;
	}
	if (fp != NULL && nhandles < 128)
		handles[nhandles++].handle = handle;
	p = (dl_funcptr) dlsym(handle, funcname);
	return p;
}
