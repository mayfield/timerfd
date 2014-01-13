#include <Python.h>
#include <datetime.h>
#include <sys/timerfd.h>


static PyObject *ErrorObject;

typedef struct {
    PyObject_HEAD
    PyObject *x_attr;
} TimerfdObject;

static PyTypeObject Timerfd_Type; 

#define TimerfdObject_Check(v)  (Py_TYPE(v) == &Timerfd_Type)

static TimerfdObject *newTimerfdObject(PyObject *arg)
{
    TimerfdObject *self; 
    self = PyObject_New(TimerfdObject, &Timerfd_Type);
    if (self == NULL) {
        return NULL;
    }

    self->x_attr = PyDict_New();
    if (self->x_attr == NULL)
        return NULL;

    return self;
}//newTimerfdObject()


static void Timerfd_dealloc(TimerfdObject *self)
{
    Py_XDECREF(self->x_attr);
    PyObject_Del(self);
}//Timerfd_dealloc()

static PyObject *Timerfd_demo(TimerfdObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":demo")) {
        return NULL;
    }

    Py_RETURN_NONE;
}//*Timerfd_demo()

static PyMethodDef Timerfd_methods[] = {
    {"demo",            (PyCFunction)Timerfd_demo,  METH_VARARGS,
        PyDoc_STR("demo() -> None")},
    {NULL,              NULL}           /* sentinel */
};

static PyObject *
Timerfd_getattro(TimerfdObject *self, PyObject *name)
{
    if (self->x_attr != NULL) {
        PyObject *v = PyDict_GetItem(self->x_attr, name);
        if (v != NULL) {
            Py_INCREF(v);
            return v;
        }
    }
    return PyObject_GenericGetAttr((PyObject *)self, name);
}

static int
Timerfd_setattr(TimerfdObject *self, char *name, PyObject *v)
{
    if (self->x_attr == NULL) {
        self->x_attr = PyDict_New();
        if (self->x_attr == NULL)
            return -1;
    }
    if (v == NULL) {
        int rv = PyDict_DelItemString(self->x_attr, name);
        if (rv < 0)
            PyErr_SetString(PyExc_AttributeError,
                "delete non-existing Xxo attribute");
        return rv;
    }
    else
        return PyDict_SetItemString(self->x_attr, name, v);
}

static PyTypeObject Timerfd_Type = {
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyVarObject_HEAD_INIT(NULL, 0)
    "timerfdmodule.o",             /*tp_name*/
    sizeof(TimerfdObject),          /*tp_basicsize*/
    0,                          /*tp_itemsize*/
    /* methods */
    (destructor)Timerfd_dealloc,    /*tp_dealloc*/
    0,                          /*tp_print*/
    (getattrfunc)0,             /*tp_getattr*/
    (setattrfunc)Timerfd_setattr,   /*tp_setattr*/
    0,                          /*tp_reserved*/
    0,                          /*tp_repr*/
    0,                          /*tp_as_number*/
    0,                          /*tp_as_sequence*/
    0,                          /*tp_as_mapping*/
    0,                          /*tp_hash*/
    0,                          /*tp_call*/
    0,                          /*tp_str*/
    (getattrofunc)Timerfd_getattro, /*tp_getattro*/
    0,                          /*tp_setattro*/
    0,                          /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,         /*tp_flags*/
    0,                          /*tp_doc*/
    0,                          /*tp_traverse*/
    0,                          /*tp_clear*/
    0,                          /*tp_richcompare*/
    0,                          /*tp_weaklistoffset*/
    0,                          /*tp_iter*/
    0,                          /*tp_iternext*/
    Timerfd_methods,                /*tp_methods*/
    0,                          /*tp_members*/
    0,                          /*tp_getset*/
    0,                          /*tp_base*/
    0,                          /*tp_dict*/
    0,                          /*tp_descr_get*/
    0,                          /*tp_descr_set*/
    0,                          /*tp_dictoffset*/
    0,                          /*tp_init*/
    0,                          /*tp_alloc*/
    0,                          /*tp_new*/
    0,                          /*tp_free*/
    0,                          /*tp_is_gc*/
};



       /* int timerfd_gettime(int fd, struct itimerspec *curr_value); */


static PyObject * m_timerfd_create(PyObject *self, PyObject *args, PyObject *kwargs)
{
    char *kw[] = {"flags", NULL};
    long clockid = 0, flags = 0;
    int t_res;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|ll", kw, &clockid, &flags))
        return NULL;

    if (clockid == 0) {
        clockid = CLOCK_MONOTONIC;
    }

    t_res = timerfd_create(clockid, flags);
    if (t_res < 1) {
        return PyErr_SetFromErrno(NULL);
        /* PyErr_SetString(ErrorObject, "timerfd_create system call faild."); */
        /* return NULL; */
    }

    return (PyObject *)PyLong_FromLong(t_res);
}//m_timerfd_create()

static PyObject *m_timerfd_settime(PyObject *self, PyObject *args, PyObject *kwargs)
{
    printf("m_timerfd_settime\n");

    char *kw[] = {"fd", "deadline", "interval", "flags", NULL};
    long fd = 0;
    long  flags = 0;
    long msec = 0;
    int t_res;
    PyObject *deadline = NULL;
    PyObject *interval = NULL;
    PyObject *it_val = NULL;
    PyObject *it_inter = NULL;

    struct itimerspec new_val;
    struct itimerspec old_val;
    memset(&new_val, 0, sizeof(struct itimerspec));
    memset(&old_val, 0, sizeof(struct itimerspec));


    if (!PyArg_ParseTupleAndKeywords(
                args, kwargs, "l|OOl:timerfd_settime", kw,
                &fd, &deadline, &interval, &flags)) {
        return NULL;
    }
    printf("fd %ld, flags %ld deadline %p interval %p\n", fd, flags, deadline, interval);

    if (deadline != NULL) {
        printf("deadline is set\n");
        Py_INCREF(deadline);
        if (PyDelta_Check(deadline)) {
                printf("deadline is Delta\n");
                msec = PyDateTime_DELTA_GET_MICROSECONDS(deadline);
        } else if (PyLong_Check(deadline)) {
                printf("deadline is long\n");
                msec = PyLong_AsLong(deadline);
        } else {
            printf("deadline is UK\n");
            Py_DECREF(deadline);
            PyErr_SetString(
                PyExc_TypeError,
                "deadline is not a valid type. Must be a timedelta object or integer");
            return NULL;
        }
        Py_DECREF(deadline);
    }
    printf("deadline is %ld\n", msec);
    if (msec > 0) {
        new_val.it_value.tv_sec = msec / 1000;
        new_val.it_value.tv_nsec = (msec % 1000) * 1000000;
    }

    msec = 0;
    if (interval != NULL) {
        printf("interval is set\n");
        Py_INCREF(interval);
        if (PyDelta_Check(interval)) {
                printf("interval is Delta\n");
                msec = PyDateTime_DELTA_GET_MICROSECONDS(interval);
        } else if (PyLong_Check(interval)) {
                printf("interval is long\n");
                msec = PyLong_AsLong(interval);
        } else {
            Py_DECREF(interval);
            PyErr_SetString(
                PyExc_TypeError,
                "interval is not a valid type. Must be a timedelta object or integer");
            return NULL;
        }
        Py_DECREF(interval);
    }
    printf("interval is %ld\n", msec);
    if (msec > 0) {
        new_val.it_interval.tv_sec = msec / 1000;
        new_val.it_interval.tv_nsec = (msec % 1000) * 1000000;
    }

    printf("settime fd %ld, flags %ld new_val %p old_val %p\n", fd, flags, &new_val, &old_val);
    t_res = timerfd_settime(fd, flags, &new_val, &old_val);
    printf("timerfd_settime response %d\n", t_res);

    if (t_res != 0) {
        printf("error out\n");
        return PyErr_SetFromErrno(NULL);
    }

    printf("create tuple\n");
    PyObject *resp = PyTuple_New(2);
    if (resp == NULL) {
        printf("bad tuple\n");
        return NULL;
    }

    printf("get val delta\n");
    if (old_val.it_value.tv_sec > 0 || old_val.it_value.tv_nsec > 0) {
        it_val = PyDelta_FromDSU(0, old_val.it_value.tv_sec, old_val.it_value.tv_nsec);
        PyTuple_SetItem(resp, 0, it_val);
    } else {
        Py_INCREF(Py_None);
        PyTuple_SetItem(resp, 0, Py_None);
    }

    printf("get interval delta\n");
    if (old_val.it_interval.tv_sec > 0 || old_val.it_interval.tv_nsec > 0) {
        it_inter = PyDelta_FromDSU(0, old_val.it_interval.tv_sec, old_val.it_interval.tv_nsec);
        PyTuple_SetItem(resp, 0, it_inter);
    } else {
        Py_INCREF(Py_None);
        PyTuple_SetItem(resp, 1, Py_None);
    }

    printf("return %p\n", resp);
    return (PyObject *)resp;
}//m_timerfd_settime()


static PyObject * m_timerfd_gettime(PyObject *self, PyObject *args)
{
    long fd = 0;
    int t_res;
    struct itimerspec g_time;
    PyObject* resp;
    PyObject* it_val;
    PyObject* it_inter;

    if (!PyArg_ParseTuple(args, "l", &fd)) {
        PyErr_SetString(
            PyExc_TypeError,
            "fd is not a valid type or the argument is missing");
        return NULL;
    }

    t_res = timerfd_gettime(fd, &g_time);
    if (t_res == -1) {
        return PyErr_SetFromErrno(NULL);
    }

    resp = PyTuple_New(2);

    if (g_time.it_value.tv_sec > 0 || g_time.it_value.tv_nsec > 0) {
        it_val = PyDelta_FromDSU(0, g_time.it_value.tv_sec, g_time.it_value.tv_nsec);
        PyTuple_SetItem(resp, 0, it_val);
    } else {
        Py_INCREF(Py_None);
        PyTuple_SetItem(resp, 0, Py_None);
    }

    if (g_time.it_interval.tv_sec > 0 || g_time.it_interval.tv_nsec > 0) {
        it_inter = PyDelta_FromDSU(0, g_time.it_interval.tv_sec, g_time.it_interval.tv_nsec);
        PyTuple_SetItem(resp, 1, it_inter);
    } else {
        Py_INCREF(Py_None);
        PyTuple_SetItem(resp, 1, Py_None);
    }

    return resp;
}//m_timerfd_gettime()

static PyObject *
timerfd_new(PyObject *self, PyObject *args)
{
    TimerfdObject *rv;

    if (!PyArg_ParseTuple(args, ":new"))
        return NULL;
    rv = newTimerfdObject(args);
    if (rv == NULL)
        return NULL;
    return (PyObject *)rv;
}

static PyTypeObject Str_Type = {
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyVarObject_HEAD_INIT(NULL, 0)
    "timerfdmodule.Str",             /*tp_name*/
    0,                          /*tp_basicsize*/
    0,                          /*tp_itemsize*/
    /* methods */
    0,                          /*tp_dealloc*/
    0,                          /*tp_print*/
    0,                          /*tp_getattr*/
    0,                          /*tp_setattr*/
    0,                          /*tp_reserved*/
    0,                          /*tp_repr*/
    0,                          /*tp_as_number*/
    0,                          /*tp_as_sequence*/
    0,                          /*tp_as_mapping*/
    0,                          /*tp_hash*/
    0,                          /*tp_call*/
    0,                          /*tp_str*/
    0,                          /*tp_getattro*/
    0,                          /*tp_setattro*/
    0,                          /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    0,                          /*tp_doc*/
    0,                          /*tp_traverse*/
    0,                          /*tp_clear*/
    0,                          /*tp_richcompare*/
    0,                          /*tp_weaklistoffset*/
    0,                          /*tp_iter*/
    0,                          /*tp_iternext*/
    0,                          /*tp_methods*/
    0,                          /*tp_members*/
    0,                          /*tp_getset*/
    0, /* see PyInit_xx */      /*tp_base*/
    0,                          /*tp_dict*/
    0,                          /*tp_descr_get*/
    0,                          /*tp_descr_set*/
    0,                          /*tp_dictoffset*/
    0,                          /*tp_init*/
    0,                          /*tp_alloc*/
    0,                          /*tp_new*/
    0,                          /*tp_free*/
    0,                          /*tp_is_gc*/
};

static PyObject *
null_richcompare(PyObject *self, PyObject *other, int op)
{
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
}

static PyTypeObject Null_Type = {
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyVarObject_HEAD_INIT(NULL, 0)
    "timerfdmodule.Null",            /*tp_name*/
    0,                          /*tp_basicsize*/
    0,                          /*tp_itemsize*/
    /* methods */
    0,                          /*tp_dealloc*/
    0,                          /*tp_print*/
    0,                          /*tp_getattr*/
    0,                          /*tp_setattr*/
    0,                          /*tp_reserved*/
    0,                          /*tp_repr*/
    0,                          /*tp_as_number*/
    0,                          /*tp_as_sequence*/
    0,                          /*tp_as_mapping*/
    0,                          /*tp_hash*/
    0,                          /*tp_call*/
    0,                          /*tp_str*/
    0,                          /*tp_getattro*/
    0,                          /*tp_setattro*/
    0,                          /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    0,                          /*tp_doc*/
    0,                          /*tp_traverse*/
    0,                          /*tp_clear*/
    null_richcompare,           /*tp_richcompare*/
    0,                          /*tp_weaklistoffset*/
    0,                          /*tp_iter*/
    0,                          /*tp_iternext*/
    0,                          /*tp_methods*/
    0,                          /*tp_members*/
    0,                          /*tp_getset*/
    0, /* see PyInit_xx */      /*tp_base*/
    0,                          /*tp_dict*/
    0,                          /*tp_descr_get*/
    0,                          /*tp_descr_set*/
    0,                          /*tp_dictoffset*/
    0,                          /*tp_init*/
    0,                          /*tp_alloc*/
    0, /* see PyInit_xx */      /*tp_new*/
    0,                          /*tp_free*/
    0,                          /*tp_is_gc*/
};

static PyMethodDef timerfd_methods[] = {
    {"create",
        (PyCFunction)m_timerfd_create,
        METH_VARARGS | METH_KEYWORDS,
        PyDoc_STR("Add doc for create")},
    {"settime",
        (PyCFunction)m_timerfd_settime,
        METH_VARARGS | METH_KEYWORDS,
        PyDoc_STR("Add doc for create")},
    {"gettime",
        (PyCFunction)m_timerfd_gettime,
        METH_VARARGS,
        PyDoc_STR("Add doc for create")},
    {"new", timerfd_new, METH_VARARGS, PyDoc_STR("new() -> new Timerfd object")},
    {NULL, NULL}           /* sentinel */
};

PyDoc_STRVAR(module_doc,
"This is a template module just for instruction.");

static PyModuleDef timerfdmodule = {
    PyModuleDef_HEAD_INIT,
    "timerfd",
    module_doc,
    -1,
    timerfd_methods,
    NULL,
    NULL,
    NULL,
    NULL
};

PyMODINIT_FUNC
PyInit_timerfd(void)
{
    PyObject *m = NULL;
    PyDateTime_IMPORT;

    /* Due to cross platform compiler issues the slots must be filled
     * here. It's required for portability to Windows without requiring
     * C++. */
    Null_Type.tp_base = &PyBaseObject_Type;
    Null_Type.tp_new = PyType_GenericNew;
    Str_Type.tp_base = &PyUnicode_Type;

    /* Finalize the type object including setting type of the new type
     * object; doing it here is required for portability, too. */
    if (PyType_Ready(&Timerfd_Type) < 0)
        goto fail;

    /* Create the module and add the functions */
    m = PyModule_Create(&timerfdmodule);
    if (m == NULL)
        goto fail;

    /* Add some symbolic constants to the module */
    if (ErrorObject == NULL) {
        ErrorObject = PyErr_NewException("timerfd.error", NULL, NULL);
        if (ErrorObject == NULL)
            goto fail;
    }
    Py_INCREF(ErrorObject);
    PyModule_AddObject(m, "error", ErrorObject);


    PyModule_AddObject(m, "CLOCK_MONOTONIC", PyLong_FromLong((long)CLOCK_MONOTONIC));
    PyModule_AddObject(m, "CLOCK_REALTIME", PyLong_FromLong((long)CLOCK_REALTIME));
    PyModule_AddObject(m, "TFD_NONBLOCK", PyLong_FromLong((long)TFD_NONBLOCK));
    PyModule_AddObject(m, "TFD_CLOEXEC", PyLong_FromLong((long)TFD_CLOEXEC));

    /* Add Str */
    if (PyType_Ready(&Str_Type) < 0)
        goto fail;
    PyModule_AddObject(m, "Str", (PyObject *)&Str_Type);

    /* Add Null */
    if (PyType_Ready(&Null_Type) < 0)
        goto fail;
    PyModule_AddObject(m, "Null", (PyObject *)&Null_Type);
    return m;
 fail:
    Py_XDECREF(m);
    return NULL;
}
