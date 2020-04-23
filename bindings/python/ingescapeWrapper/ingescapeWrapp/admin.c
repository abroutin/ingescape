//
//  admin.h
//  ingescapeWrapp
//
//  Created by vaugien on 12/02/2018.
//  Copyright © 2018 ingenuity. All rights reserved.
//

#include "admin.h"
#include <stdio.h>
#include <ingescape/ingescape.h>

//wrapper for igs_setCommandLine
PyObject * setCommandLine_wrapper(PyObject * self, PyObject * args)
{
    char * line;
    PyObject * ret;
    
    // parse and cast the line argument given in python
    if (!PyArg_ParseTuple(args, "s", &line)) {
        return NULL;
    }
    
    igs_setCommandLine(line);
    // if everything is ok return 1
    ret = PyLong_FromLong(1);
    return ret;
}


//wrapper for igs_setVerbose
PyObject * setVerbose_wrapper(PyObject * self, PyObject * args)
{
    bool verbose;
    PyObject * ret;
    
    // parse and cast the verbose argument given in python
    if (!PyArg_ParseTuple(args, "b", &verbose)) {
        return NULL;
    }

    igs_setVerbose(verbose);

    ret = PyLong_FromLong(1);
    return ret;
}

//wrapper for igs_getVerbose
PyObject * getVerbose_wrapper(PyObject * self, PyObject * args)
{
    bool verbose;
    PyObject * ret;

    verbose = igs_isVerbose();
    
    // build the resulting bool into a Python boolean and return it
    if(verbose){
        ret = Py_True;
    }else{
        ret = Py_False;
    }
    return ret;
}

//wrapper for igs_setLogStream
PyObject * setLogStream_wrapper(PyObject * self, PyObject * args)
{
    bool stream;
    PyObject * ret;
    
    // parse and cast into a bool the stream argument given in python
    if (!PyArg_ParseTuple(args, "b", &stream)) {
        return NULL;
    }

    igs_setLogStream(stream);

    ret = PyLong_FromLong(1);
    return ret;
}

//wrapper for igs_getLogStream
PyObject * getLogStream_wrapper(PyObject * self, PyObject * args)
{
    bool stream;
    PyObject * ret;

    stream = igs_getLogStream();
    
    // build the resulting bool into a Python boolean and return it
    if(stream){
        ret = Py_True;
    }else{
        ret = Py_False;
    }
    return ret;
}

//wrapper for igs_setLogInFile
PyObject * setLogInFile_wrapper(PyObject * self, PyObject * args)
{
    bool useLogFile;
    PyObject * ret;
    
     // parse and cast into a bool the useLogFile argument given in python
    if (!PyArg_ParseTuple(args, "b", &useLogFile)) {
        return NULL;
    }

    igs_setLogInFile(useLogFile);

    ret = PyLong_FromLong(1);
    return ret;
}

//wrapper for igs_getLogInFile
PyObject * getLogInFile_wrapper(PyObject * self, PyObject * args)
{
    bool useLogFile;
    PyObject * ret;

    useLogFile = igs_getLogInFile();
    
    // build the resulting bool into a Python boolean and return it
    if(useLogFile){
        ret = Py_True;
    }else{
        ret = Py_False;
    }
    return ret;
}

//wrapper for igs_setUseColorVerbose
PyObject * setUseColorVerbose_wrapper(PyObject * self, PyObject * args)
{
    bool useColorVerbose;
    PyObject * ret;

    // parse and cast into a bool the useColorVerbose argument given in python
    if (!PyArg_ParseTuple(args, "b", &useColorVerbose)) {
        return NULL;
    }

    igs_setUseColorVerbose(useColorVerbose);

    ret = PyLong_FromLong(1);
    return ret;
}

//wrapper for igs_getUseColorVerbose
PyObject * getUseColorVerbose_wrapper(PyObject * self, PyObject * args)
{
    bool useColorVerbose;
    PyObject * ret;

    useColorVerbose = igs_getUseColorVerbose();
    
    // build the resulting bool into a Python boolean and return it
    if(useColorVerbose){
        ret = Py_True;
    }else{
        ret = Py_False;
    }
    return ret;
}

//wrapper for igs_setLogPath
PyObject * setLogPath_wrapper(PyObject * self, PyObject * args)
{
    char * path;
    PyObject * ret;

    
    // parse and cast into a bool the useColorVerbose argument given in python
    if (!PyArg_ParseTuple(args, "s", &path)) {
        return NULL;
    }

    igs_setLogPath(path);

    ret = PyLong_FromLong(1);
    return ret;
}

//wrapper for igs_getLogPath
PyObject * getLogPath_wrapper(PyObject * self, PyObject * args)
{
    char * path;
    PyObject * ret;

    path = igs_getLogPath();
    
    // build the resulting char* into a Python bytes and return it
    ret = PyBytes_FromString(path);
    return ret;
}


//wrapper for igs_interrupted
PyObject * interrupted_wrapper(PyObject * self, PyObject * args)
{
    if (igs_Interrupted){
        Py_RETURN_TRUE;
    }else{
        Py_RETURN_FALSE;
    }
}


//wrapper for igs_setDefinitionPath
PyObject * setDefinitionPath_wrapper(PyObject * self, PyObject * args)
{
    char * path;
    PyObject * ret;
    
    // parse and cast into a char* the string path given in python
    if (!PyArg_ParseTuple(args, "s", &path)) {
        return NULL;
    }
    
    igs_setDefinitionPath(path);

    ret = PyLong_FromLong(1);
    return ret;
}

//wrapper for igs_setMappingPath
PyObject * setMappingPath_wrapper(PyObject * self, PyObject * args)
{
    char * path;
    PyObject * ret;
    
    // parse and cast into a char* the string path given in python
    if (!PyArg_ParseTuple(args, "s", &path)) {
        return NULL;
    }

    igs_setMappingPath(path);

    ret = PyLong_FromLong(1);
    return ret;
}

//wrapper for igs_writeDefinitionToPath
PyObject * writeDefinitionToPath_wrapper(PyObject * self, PyObject * args)
{
    PyObject * ret;
    
    igs_writeDefinitionToPath();

    ret = PyLong_FromLong(1);
    return ret;
}


//wrapper for igs_writeMappingToPath
PyObject * writeMappingToPath_wrapper(PyObject * self, PyObject * args)
{
    PyObject * ret;

    igs_writeMappingToPath();

    ret = PyLong_FromLong(1);
    return ret;
}

//wrapper for igs_version
PyObject * version_wrapper(PyObject * self, PyObject * args)
{
    int version;
    PyObject* ret;

    version = igs_version();

    ret = PyLong_FromLong(version);
    return ret;
}

//wrapper for igs_trace
PyObject * trace_wrapper(PyObject * self, PyObject * args)
{
    char * log;
    PyObject * ret;
    
    // parse and cast into a bool the useColorVerbose argument given in python
    if (!PyArg_ParseTuple(args, "s", &log)) {
        return NULL;
    }
    
    PyFrameObject* frame = PyEval_GetFrame();
    Py_INCREF(frame);
    PyObject *function = frame->f_code->co_name;
    Py_INCREF(function);
    Py_DECREF(frame);
    
    PyObject* funcTuple = Py_BuildValue("(O)", function);
    Py_DECREF(function);
    if (!funcTuple) return 0;
    const char* functionStr = 0;
    if (!PyArg_ParseTuple(funcTuple, "s", &functionStr)) {
        Py_DECREF(args);
        return 0;
    }
    
    /* s now points to a const char* - use it, delete args when done */
    
    Py_DECREF(funcTuple);
    
    if(strcmp(functionStr, "<module>") == 0){
        igs_log(IGS_LOG_TRACE, "main", log);
    }else{
        igs_log(IGS_LOG_DEBUG, functionStr, log);
    }
    
    ret = PyLong_FromLong(1);
    return ret;
}

PyObject * debug_wrapper(PyObject * self, PyObject * args)
{
    char * log;
    PyObject * ret;
    
    // parse and cast into a bool the useColorVerbose argument given in python
    if (!PyArg_ParseTuple(args, "s", &log)) {
        return NULL;
    }
    
    PyFrameObject* frame = PyEval_GetFrame();
    Py_INCREF(frame);
    PyObject *function = frame->f_code->co_name;
    Py_INCREF(function);
    Py_DECREF(frame);
    
    PyObject* funcTuple = Py_BuildValue("(O)", function);
    Py_DECREF(function);
    if (!funcTuple) return 0;
    const char* functionStr = 0;
    if (!PyArg_ParseTuple(funcTuple, "s", &functionStr)) {
        Py_DECREF(args);
        return 0;
    }
    
    /* s now points to a const char* - use it, delete args when done */
    
    Py_DECREF(funcTuple);
    
    if(strcmp(functionStr, "<module>") == 0){
        igs_log(IGS_LOG_DEBUG, "main", log);
    }else{
        igs_log(IGS_LOG_DEBUG, functionStr, log);
    }
    
    ret = PyLong_FromLong(1);
    return ret;
}


PyObject * info_wrapper(PyObject * self, PyObject * args)
{
    char * log;
    PyObject * ret;
    
    // parse and cast into a bool the useColorVerbose argument given in python
    if (!PyArg_ParseTuple(args, "s", &log)) {
        return NULL;
    }
    
    PyFrameObject* frame = PyEval_GetFrame();
    Py_INCREF(frame);
    PyObject *function = frame->f_code->co_name;
    Py_INCREF(function);
    Py_DECREF(frame);
    
    PyObject* funcTuple = Py_BuildValue("(O)", function);
    Py_DECREF(function);
    if (!funcTuple) return 0;
    const char* functionStr = 0;
    if (!PyArg_ParseTuple(funcTuple, "s", &functionStr)) {
        Py_DECREF(args);
        return 0;
    }
    
    /* s now points to a const char* - use it, delete args when done */
    
    Py_DECREF(funcTuple);
    
    if(strcmp(functionStr, "<module>") == 0){
        igs_log(IGS_LOG_INFO, "main", log);
    }else{
        igs_log(IGS_LOG_INFO, functionStr, log);
    }
    
    ret = PyLong_FromLong(1);
    return ret;
}


PyObject * warn_wrapper(PyObject * self, PyObject * args)
{
    char * log;
    PyObject * ret;
    
    // parse and cast into a bool the useColorVerbose argument given in python
    if (!PyArg_ParseTuple(args, "s", &log)) {
        return NULL;
    }
    
    PyFrameObject* frame = PyEval_GetFrame();
    Py_INCREF(frame);
    PyObject *function = frame->f_code->co_name;
    Py_INCREF(function);
    Py_DECREF(frame);
    
    PyObject* funcTuple = Py_BuildValue("(O)", function);
    Py_DECREF(function);
    if (!funcTuple) return 0;
    const char* functionStr = 0;
    if (!PyArg_ParseTuple(funcTuple, "s", &functionStr)) {
        Py_DECREF(args);
        return 0;
    }
    
    /* s now points to a const char* - use it, delete args when done */
    
    Py_DECREF(funcTuple);
    
    if(strcmp(functionStr, "<module>") == 0){
        igs_log(IGS_LOG_WARN, "main", log);
    }else{
        igs_log(IGS_LOG_WARN, functionStr, log);
    }
    
    ret = PyLong_FromLong(1);
    return ret;
}

PyObject * error_wrapper(PyObject * self, PyObject * args)
{
    char * log;
    PyObject * ret;
    
    // parse and cast into a bool the useColorVerbose argument given in python
    if (!PyArg_ParseTuple(args, "s", &log)) {
        return NULL;
    }
    
    PyFrameObject* frame = PyEval_GetFrame();
    Py_INCREF(frame);
    PyObject *function = frame->f_code->co_name;
    Py_INCREF(function);
    Py_DECREF(frame);
    
    PyObject* funcTuple = Py_BuildValue("(O)", function);
    Py_DECREF(function);
    if (!funcTuple) return 0;
    const char* functionStr = 0;
    if (!PyArg_ParseTuple(funcTuple, "s", &functionStr)) {
        Py_DECREF(args);
        return 0;
    }
    
    /* s now points to a const char* - use it, delete args when done */
    
    Py_DECREF(funcTuple);
    
    if(strcmp(functionStr, "<module>") == 0){
        igs_log(IGS_LOG_ERROR, "main", log);
    }else{
        igs_log(IGS_LOG_ERROR, functionStr, log);
    }
    
    ret = PyLong_FromLong(1);
    return ret;
}

PyObject * fatal_wrapper(PyObject * self, PyObject * args)
{
    char * log;
    PyObject * ret;
    
    // parse and cast into a bool the useColorVerbose argument given in python
    if (!PyArg_ParseTuple(args, "s", &log)) {
        return NULL;
    }
    
    PyFrameObject* frame = PyEval_GetFrame();
    Py_INCREF(frame);
    PyObject *function = frame->f_code->co_name;
    Py_INCREF(function);
    Py_DECREF(frame);
    
    PyObject* funcTuple = Py_BuildValue("(O)", function);
    Py_DECREF(function);
    if (!funcTuple) return 0;
    const char* functionStr = 0;
    if (!PyArg_ParseTuple(funcTuple, "s", &functionStr)) {
        Py_DECREF(args);
        return 0;
    }
    
    /* s now points to a const char* - use it, delete args when done */
    
    Py_DECREF(funcTuple);
    if(strcmp(functionStr, "<module>") == 0){
        igs_log(IGS_LOG_FATAL, "main", log);
    }else{
        igs_log(IGS_LOG_FATAL, functionStr, log);
    }

    ret = PyLong_FromLong(1);
    return ret;
}

PyObject * setLogLevel_wrapper(PyObject * self, PyObject * args)
{
    int * logLevel;
    PyObject * ret;
    
    
    // parse and cast into a bool the useColorVerbose argument given in python
    if (!PyArg_ParseTuple(args, "i", &logLevel)) {
        return NULL;
    }
    igs_setLogLevel(logLevel);
    
    ret = PyLong_FromLong(1);
    return ret;

}

PyObject * getLogLevel_wrapper(PyObject * self, PyObject * args)
{
    PyObject * ret;
    
    
    int log = igs_getLogLevel();
    
    ret = PyLong_FromLong(log);
    return ret;
}

PyObject * setLicensePath_wrapper(PyObject * self, PyObject * args)
{
    char * licensePath;
    PyObject * ret;
    
    
    // parse and cast into a bool the useColorVerbose argument given in python
    if (!PyArg_ParseTuple(args, "s", &licensePath)) {
        return NULL;
    }
    igs_setLicensePath(licensePath);
    
    ret = PyLong_FromLong(1);
    return ret;
    
}

PyObject * igs_getNetdevicesList_wrapper(PyObject * self, PyObject * args)
{
    PyObject * ret;
    
    
    char **resultList;
    int nbList;
    igs_getNetdevicesList(&resultList, &nbList);
    
    // build the resulting list into a Python object.
    ret = PyList_New(nbList);
    int i ;
    for (i = 0; i < nbList; i++){
        //set items of the python list one by one
        PyList_SetItem(ret, i, PyBytes_FromString(resultList[i]));
    }
    igs_freeNetdevicesList(resultList, nbList);
    return ret;
}

PyObject * igs_getNetadressesList_wrapper(PyObject * self, PyObject * args)
{
    PyObject * ret;
    
    
    char **resultList;
    int nbList;
    igs_getNetaddressesList(&resultList, &nbList);
    
    // build the resulting list into a Python object.
    ret = PyList_New(nbList);
    int i ;
    for (i = 0; i < nbList; i++){
        //set items of the python list one by one
        PyList_SetItem(ret, i, PyBytes_FromString(resultList[i]));
    }
    igs_freeNetaddressesList(resultList, nbList);
    return ret;
}
