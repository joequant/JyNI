/* This File is based on ceval.h from CPython 2.7.3 release.
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

#ifndef Py_CEVAL_H
#define Py_CEVAL_H
#ifdef __cplusplus
extern "C" {
#endif


/* Interface to random parts in ceval.c */

PyAPI_FUNC(PyObject *) PyEval_CallObjectWithKeywords(
    PyObject *, PyObject *, PyObject *);

/* Inline this */
#define PyEval_CallObject(func,arg) \
    PyEval_CallObjectWithKeywords(func, arg, (PyObject *)NULL)

PyAPI_FUNC(PyObject *) PyEval_CallFunction(PyObject *obj,
                                           const char *format, ...);
PyAPI_FUNC(PyObject *) PyEval_CallMethod(PyObject *obj,
                                         const char *methodname,
                                         const char *format, ...);

//PyAPI_FUNC(void) PyEval_SetProfile(Py_tracefunc, PyObject *);
//PyAPI_FUNC(void) PyEval_SetTrace(Py_tracefunc, PyObject *);

struct _frame; /* Avoid including frameobject.h */

PyAPI_FUNC(PyObject *) PyEval_GetBuiltins(void);
PyAPI_FUNC(PyObject *) PyEval_GetGlobals(void);
PyAPI_FUNC(PyObject *) PyEval_GetLocals(void);
PyAPI_FUNC(struct _frame *) PyEval_GetFrame(void);
PyAPI_FUNC(int) PyEval_GetRestricted(void);

/* Look at the current frame's (if any) code's co_flags, and turn on
   the corresponding compiler flags in cf->cf_flags.  Return 1 if any
   flag was set, else return 0. */
//PyAPI_FUNC(int) PyEval_MergeCompilerFlags(PyCompilerFlags *cf);

PyAPI_FUNC(int) Py_FlushLine(void);

PyAPI_FUNC(int) Py_AddPendingCall(int (*func)(void *), void *arg);
PyAPI_FUNC(int) Py_MakePendingCalls(void);

/* Protection against deeply nested recursive calls */
PyAPI_FUNC(void) Py_SetRecursionLimit(int);
PyAPI_FUNC(int) Py_GetRecursionLimit(void);

#define Py_EnterRecursiveCall(where)                                    \
            (_Py_MakeRecCheck(PyThreadState_GET()->recursion_depth) &&  \
             _Py_CheckRecursiveCall(where))
#define Py_LeaveRecursiveCall()                         \
            (--PyThreadState_GET()->recursion_depth)
PyAPI_FUNC(int) _Py_CheckRecursiveCall(char *where);
PyAPI_DATA(int) _Py_CheckRecursionLimit;
#ifdef USE_STACKCHECK
#  define _Py_MakeRecCheck(x)  (++(x) > --_Py_CheckRecursionLimit)
#else
#  define _Py_MakeRecCheck(x)  (++(x) > _Py_CheckRecursionLimit)
#endif

PyAPI_FUNC(const char *) PyEval_GetFuncName(PyObject *);
PyAPI_FUNC(const char *) PyEval_GetFuncDesc(PyObject *);

PyAPI_FUNC(PyObject *) PyEval_GetCallStats(PyObject *);
PyAPI_FUNC(PyObject *) PyEval_EvalFrame(struct _frame *);
PyAPI_FUNC(PyObject *) PyEval_EvalFrameEx(struct _frame *f, int exc);

/* this used to be handled on a per-thread basis - now just two globals */
PyAPI_DATA(volatile int) _Py_Ticker;
PyAPI_DATA(int) _Py_CheckInterval;

/* Interface for threads.

   A module that plans to do a blocking system call (or something else
   that lasts a long time and doesn't touch Python data) can allow other
   threads to run as follows:

    ...preparations here...
    Py_BEGIN_ALLOW_THREADS
    ...blocking system call here...
    Py_END_ALLOW_THREADS
    ...interpret result here...

   The Py_BEGIN_ALLOW_THREADS/Py_END_ALLOW_THREADS pair expands to a
   {}-surrounded block.
   To leave the block in the middle (e.g., with return), you must insert
   a line containing Py_BLOCK_THREADS before the return, e.g.

    if (...premature_exit...) {
        Py_BLOCK_THREADS
        PyErr_SetFromErrno(PyExc_IOError);
        return NULL;
    }

   An alternative is:

    Py_BLOCK_THREADS
    if (...premature_exit...) {
        PyErr_SetFromErrno(PyExc_IOError);
        return NULL;
    }
    Py_UNBLOCK_THREADS

   For convenience, that the value of 'errno' is restored across
   Py_END_ALLOW_THREADS and Py_BLOCK_THREADS.

   WARNING: NEVER NEST CALLS TO Py_BEGIN_ALLOW_THREADS AND
   Py_END_ALLOW_THREADS!!!

   The function PyEval_InitThreads() should be called only from
   initthread() in "threadmodule.c".

   Note that not yet all candidates have been converted to use this
   mechanism!
*/

PyAPI_FUNC(PyThreadState *) PyEval_SaveThread(void);
PyAPI_FUNC(void) PyEval_RestoreThread(PyThreadState *);

#ifdef WITH_THREAD

PyAPI_FUNC(int)  PyEval_ThreadsInitialized(void);
PyAPI_FUNC(void) PyEval_InitThreads(void);
PyAPI_FUNC(void) PyEval_AcquireLock(void);
PyAPI_FUNC(void) PyEval_ReleaseLock(void);
//PyAPI_FUNC(void) PyEval_AcquireThread(PyThreadState *tstate);
//PyAPI_FUNC(void) PyEval_ReleaseThread(PyThreadState *tstate);
PyAPI_FUNC(void) PyEval_ReInitThreads(void);

//#define Py_BEGIN_ALLOW_THREADS { \
//						jputs("Py_BEGIN_ALLOW_THREADS"); \
//                        PyThreadState *_save; \
//                        _save = PyEval_SaveThread();
//#define Py_BLOCK_THREADS        jputs("Py_BLOCK_THREADS"); PyEval_RestoreThread(_save);
//#define Py_UNBLOCK_THREADS      jputs("Py_EUNBLOCK_THREADS"); _save = PyEval_SaveThread();
//#define Py_END_ALLOW_THREADS    jputs("Py_END_ALLOW_THREADS"); \
//								PyEval_RestoreThread(_save); \
//                 }

#define Py_BEGIN_ALLOW_THREADS { \
                        PyThreadState *_save; \
                        _save = PyEval_SaveThread();
#define Py_BLOCK_THREADS        PyEval_RestoreThread(_save);
#define Py_UNBLOCK_THREADS      _save = PyEval_SaveThread();
#define Py_END_ALLOW_THREADS    PyEval_RestoreThread(_save); \
                 }


#else /* !WITH_THREAD */

#define Py_BEGIN_ALLOW_THREADS {
#define Py_BLOCK_THREADS
#define Py_UNBLOCK_THREADS
#define Py_END_ALLOW_THREADS }

#endif /* !WITH_THREAD */

PyAPI_FUNC(int) _PyEval_SliceIndex(PyObject *, Py_ssize_t *);


#ifdef __cplusplus
}
#endif
#endif /* !Py_CEVAL_H */
