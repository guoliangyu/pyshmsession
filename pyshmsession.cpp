
#include <sys/ipc.h>
#include <sys/shm.h>
#include <map>
#include <string>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "sharemap.h"
#include <Python.h>

namespace PyShmSession
{

static Map* instance = NULL;

bool init(const char* filepath, int blocksize, int maxitem)
{
    int MEM_SIZE = (sizeof(MemBlock) + blocksize - 1) * maxitem + sizeof(int) * 2 + sizeof(int) * maxitem * 2 + 32; 
    key_t mkey = ftok(filepath, (int)'a');
    int shmid = shmget(mkey, MEM_SIZE, IPC_CREAT | IPC_EXCL | 0666);
    if(shmid == -1){
        if(errno == EEXIST){
            //session file already exsit:
            shmid = shmget(mkey, MEM_SIZE, 0666);
            void* buf = shmat(shmid, NULL, 0);
            if((long)buf == -1){
                return false;
            }
            std::string flag((const char*)buf, 32);
            if(flag != "f424d712334841b7b3d7a380fe1ecad8"){
                PyShmSession::instance = new Map(buf+32, blocksize, maxitem);
                if(PyShmSession::instance->init())
                    memcpy(buf, "f424d712334841b7b3d7a380fe1ecad8", 32);
                else{
                    shmdt(buf);
                    return false;
                }
            }else {
                PyShmSession::instance = new Map(buf+32, blocksize, maxitem);
            }
        }
        else
            return false;
    } else {
        //success to create:
        void* buf = shmat(shmid, NULL, 0);
        if((long)buf == -1){
            return false;
        }
        PyShmSession::instance = new Map(buf+32, blocksize, maxitem);
        if(PyShmSession::instance->init())
            memcpy(buf, "f424d712334841b7b3d7a380fe1ecad8", 32);
        else{
            shmdt(buf);
            return false;
        }

    }
    return true;
}

bool get(const char* key, int klen, unsigned char* data, int len)
{
    if(PyShmSession::instance)
        return PyShmSession::instance->get(key, klen, data, len);
    else
        return false;
}

bool add(const char* key, int klen, unsigned char* data, int dlen)
{
    if(PyShmSession::instance)
        return PyShmSession::instance->add(key, klen, data, dlen);
    return false;
}

bool update(const char* key, int klen, unsigned char* data, int dlen)
{
    if(PyShmSession::instance)
        return PyShmSession::instance->update(key, klen, data, dlen);
    return false;
}

void remove(const char* key, int klen)
{
    if(PyShmSession::instance)
        PyShmSession::instance->del(key, klen);
}

bool touch(const char* key, int klen)
{
    if(PyShmSession::instance)
        return PyShmSession::instance->touch(key, klen);
    return false;
}

void recycle(int ncycles)
{
    if(PyShmSession::instance)
        PyShmSession::instance->recycle(ncycles);
}

int size()
{
    if(PyShmSession::instance)
        return PyShmSession::instance->size();
    return 0;
}


}

static PyObject* addwraper(PyObject* self, PyObject* args)
{
    char* key;
    char* value;
    int klen=0,vlen=0;
    PyArg_ParseTuple(args, "s#s#", &key, &klen, &value, &vlen);
    if(PyShmSession::add(key, klen, (unsigned char*) value, vlen))
    {
        Py_INCREF(Py_True);
        return Py_True;
    }
    else
    {
        Py_INCREF(Py_False);
        return Py_False;
    }

}

static PyObject* delwraper(PyObject* self, PyObject* args)
{
    char *key;
    int klen = 0;
    PyArg_ParseTuple(args, "s#", &key, &klen);
    PyShmSession::remove(key, klen);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject* touchwraper(PyObject* self, PyObject* args)
{
    char *key;
    int klen = 0;
    PyArg_ParseTuple(args, "s#", &key, &klen);
    if(PyShmSession::touch(key, klen)){
        Py_INCREF(Py_True);
        return Py_True;
    } else {
        Py_INCREF(Py_False);
        return Py_False;
    }
}

static PyObject* recyclewraper(PyObject* self, PyObject* args)
{
    int ncycles = 0;
    PyArg_ParseTuple(args, "i", &ncycles);
    PyShmSession::recycle(ncycles);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject* updatewraper(PyObject* self, PyObject* args)
{
    char *key;
    char *value;
    int klen=0,vlen=0;
    PyArg_ParseTuple(args, "s#s#", &key, &klen, &value, &vlen);
    if(PyShmSession::update(key, klen, (unsigned char*)value, vlen))
    {
        Py_INCREF(Py_True);
        return Py_True;
    }
    else
    {
        Py_INCREF(Py_False);
        return Py_False;
    }

}

static PyObject* getwraper(PyObject* self, PyObject* args)
{
    char *key;
    unsigned char value[1024];
    int klen=0,vlen=0;
    PyArg_ParseTuple(args, "s#i", &key, &klen, &vlen);
    if(PyShmSession::get(key, klen, value, vlen))
    {
        return Py_BuildValue("s#", value, vlen);
    }
    else
    {
        Py_INCREF(Py_None);
        return Py_None;
    }
}

static PyObject* initwraper(PyObject* self, PyObject* args)
{
    char* filepath;
    int blocksize=0, maxitem=0;
    PyArg_ParseTuple(args, "sii", &filepath, &blocksize, &maxitem);
    if(PyShmSession::init(filepath, blocksize, maxitem))
    {
        Py_INCREF(Py_True);
        return Py_True;
    }
    else
    {
        Py_INCREF(Py_False);
        return Py_False;
    }
}

static PyObject* sizewraper(PyObject* self, PyObject* args)
{
    if(PyShmSession::instance)
        return Py_BuildValue("i", PyShmSession::instance->size());
    else
        return Py_BuildValue("i", 0);
}

static PyMethodDef methods[] = 
{
    {"add", addwraper, METH_VARARGS, "add key-value"},
    {"erase", delwraper, METH_VARARGS, "delete key-value"},
    {"update", updatewraper, METH_VARARGS, "update key-value"},
    {"get", getwraper, METH_VARARGS, "get key-value"},
    {"init", initwraper, METH_VARARGS, "init key-value"},
    {"touch", touchwraper, METH_VARARGS, "touch key"},
    {"recycle", recyclewraper, METH_VARARGS, "recycle key value"},
    {"size", sizewraper, METH_VARARGS, "size"},
    {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC initpyshmsession()
{
    Py_InitModule("pyshmsession", methods);
}


