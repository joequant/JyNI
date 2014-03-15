/* This File is based on cobject.c from CPython 2.7.5 release.
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


/* Wrap void* pointers to be passed between C modules */

#include "JyNI.h"


/* Declarations for objects of type PyCObject */

typedef void (*destructor1)(void *);
typedef void (*destructor2)(void *, void*);

static int cobject_deprecation_warning(void)
{
    return PyErr_WarnPy3k("CObject type is not supported in 3.x. "
        "Please use capsule objects instead.", 1);
}


PyObject *
PyCObject_FromVoidPtr(void *cobj, void (*destr)(void *))
{
    PyCObject *self;

    if (cobject_deprecation_warning()) {
        return NULL;
    }

    self = PyObject_NEW(PyCObject, &PyCObject_Type);
    if (self == NULL)
        return NULL;
    self->cobject=cobj;
    self->destructor=destr;
    self->desc=NULL;

    return (PyObject *)self;
}

PyObject *
PyCObject_FromVoidPtrAndDesc(void *cobj, void *desc,
                             void (*destr)(void *, void *))
{
    PyCObject *self;

    if (cobject_deprecation_warning()) {
        return NULL;
    }

    if (!desc) {
        PyErr_SetString(PyExc_TypeError,
                        "PyCObject_FromVoidPtrAndDesc called with null"
                        " description");
        return NULL;
    }
    self = PyObject_NEW(PyCObject, &PyCObject_Type);
    if (self == NULL)
        return NULL;
    self->cobject = cobj;
    self->destructor = (destructor1)destr;
    self->desc = desc;

    return (PyObject *)self;
}

void *
PyCObject_AsVoidPtr(PyObject *self)
{
    if (self) {
        if (PyCapsule_CheckExact(self)) {
            const char *name = PyCapsule_GetName(self);
            return (void *)PyCapsule_GetPointer(self, name);
        }
        if (self->ob_type == &PyCObject_Type)
            return ((PyCObject *)self)->cobject;
        PyErr_SetString(PyExc_TypeError,
                        "PyCObject_AsVoidPtr with non-C-object");
    }
    if (!PyErr_Occurred())
        PyErr_SetString(PyExc_TypeError,
                        "PyCObject_AsVoidPtr called with null pointer");
    return NULL;
}

void *
PyCObject_GetDesc(PyObject *self)
{
    if (self) {
        if (self->ob_type == &PyCObject_Type)
            return ((PyCObject *)self)->desc;
        PyErr_SetString(PyExc_TypeError,
                        "PyCObject_GetDesc with non-C-object");
    }
    if (!PyErr_Occurred())
        PyErr_SetString(PyExc_TypeError,
                        "PyCObject_GetDesc called with null pointer");
    return NULL;
}

void *
PyCObject_Import(char *module_name, char *name)
{
    PyObject *m, *c;
    void *r = NULL;

    if ((m = PyImport_ImportModule(module_name))) {
        if ((c = PyObject_GetAttrString(m,name))) {
            r = PyCObject_AsVoidPtr(c);
            Py_DECREF(c);
	}
        Py_DECREF(m);
    }
    return r;
}

int
PyCObject_SetVoidPtr(PyObject *self, void *cobj)
{
    PyCObject* cself = (PyCObject*)self;
    if (cself == NULL || !PyCObject_Check(cself) ||
	cself->destructor != NULL) {
	PyErr_SetString(PyExc_TypeError, 
			"Invalid call to PyCObject_SetVoidPtr");
	return 0;
    }
    cself->cobject = cobj;
    return 1;
}

static void
PyCObject_dealloc(PyCObject *self)
{
    if (self->destructor) {
        if(self->desc)
            ((destructor2)(self->destructor))(self->cobject, self->desc);
        else
            (self->destructor)(self->cobject);
    }
    PyObject_DEL(self);
}


PyDoc_STRVAR(PyCObject_Type__doc__,
"C objects to be exported from one extension module to another\n\
\n\
C objects are used for communication between extension modules.  They\n\
provide a way for an extension module to export a C interface to other\n\
extension modules, so that extension modules can use the Python import\n\
mechanism to link to one another.");

PyTypeObject PyCObject_Type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "PyCObject",		/*tp_name*/
    sizeof(PyCObject),		/*tp_basicsize*/
    0,				/*tp_itemsize*/
    /* methods */
    (destructor)PyCObject_dealloc, /*tp_dealloc*/
    0,				/*tp_print*/
    0,				/*tp_getattr*/
    0,				/*tp_setattr*/
    0,				/*tp_compare*/
    0,				/*tp_repr*/
    0,				/*tp_as_number*/
    0,				/*tp_as_sequence*/
    0,				/*tp_as_mapping*/
    0,				/*tp_hash*/
    0,				/*tp_call*/
    0,				/*tp_str*/
    0,				/*tp_getattro*/
    0,				/*tp_setattro*/
    0,				/*tp_as_buffer*/
    0,				/*tp_flags*/
    PyCObject_Type__doc__	/*tp_doc*/
};
