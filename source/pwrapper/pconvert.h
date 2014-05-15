/******************************************************************************
 *
 * MantaFlow fluid solver framework
 * Copyright 2011 Tobias Pfaff, Nils Thuerey 
 *
 * This program is free software, distributed under the terms of the
 * GNU General Public License (GPL) 
 * http://www.gnu.org/licenses
 *
 * Python argument wrappers and conversion tools
 *
 ******************************************************************************/

// force to load pclass first
#ifdef _PTYPE_H
#ifndef _PCONVERT_H
#define _PCONVERT_H

#include <string>
#include <map>
#include <vector>
#include "general.h"
#include "vectorbase.h"
// forward decl.
// forward declaration to minimize Python.h includes
#ifndef PyObject_HEAD
#ifndef PyObject_Fake
struct _object;
typedef _object PyObject;
#define PyObject_Fake
#endif
#endif

namespace Manta { 
template<class T> class Grid; 
class PbClass;
class FluidSolver;


//! Locks the given PbClass Arguments until ArgLocker goes out of scope
struct ArgLocker {    
    void add(PbClass* p);
    ~ArgLocker();
    std::vector<PbClass*> locks;
};

PyObject* getPyNone();

// Conversion functions

// for PbClass-derived classes
template<class T> T fromPy(PyObject* obj) { 
    if (PbClass::isNullRef(obj)) 
        return 0; 
    PbClass* pbo = PbClass::fromPyObject(obj); 
    if (!pbo || !(pbo->canConvertTo("FluidSolver"))) 
        throw Error("can't convert argument"); 
    return (T)(pbo); 
}
template<class T> PyObject* toPy(const T& v) { 
    if (v.getPyObject()) 
        return v.getPyObject(); 
    T* co = new T (v); 
    return co->assignNewPyObject("FluidSolver"); 
}

// builtin types
template<> float fromPy<float>(PyObject* obj);
template<> double fromPy<double>(PyObject* obj);
template<> int fromPy<int>(PyObject *obj);
template<> PyObject* fromPy<PyObject*>(PyObject *obj);
template<> std::string fromPy<std::string>(PyObject *obj);
template<> const char* fromPy<const char*>(PyObject *obj);
template<> bool fromPy<bool>(PyObject *obj);
template<> Vec3 fromPy<Vec3>(PyObject* obj);
template<> Vec3i fromPy<Vec3i>(PyObject* obj);
template<> PbType fromPy<PbType>(PyObject* obj);

template<> PyObject* toPy<int>( const int& v);
template<> PyObject* toPy<std::string>( const std::string& val);
template<> PyObject* toPy<float>( const float& v);
template<> PyObject* toPy<double>( const double& v);
template<> PyObject* toPy<bool>( const bool& v);
template<> PyObject* toPy<Vec3i>( const Vec3i& v);
template<> PyObject* toPy<Vec3>( const Vec3& v);
typedef PbClass* PbClass_Ptr;
template<> PyObject* toPy<PbClass*>( const PbClass_Ptr & obj);

// additional indirection somehow needed to resolve specializations in ppreg.cpp
//template<class T> inline PyObject* d_toPy(T val) { return toPy<T>(val);}

//! Encapsulation of python arguments
class PbArgs {
public:
    PbArgs(PyObject *linargs = NULL, PyObject* dict = NULL);
    void setup(PyObject *linargs = NULL, PyObject* dict = NULL);
    
    void check();
    FluidSolver* obtainParent();
    
    inline int numLinArgs() { return mLinData.size(); }
    
    inline bool has(const std::string& key) {
        return getItem(key, false) != NULL;
    }
    
    inline PyObject* linArgs() { return mLinArgs; }
    inline PyObject* kwds() { return mKwds; }
    
    template<class T> inline void add(const std::string& key, T arg) {
        DataElement el = { toPy(arg), false };
        mData[key] = el;
    }    
    template<class T> inline T get(size_t number) { 
        return fromPy<T>(getItem(number, true));
    }
    template<class T> inline T getOpt(size_t number, T defarg) { 
        PyObject* o = getItem(number, false);
        return (o) ? fromPy<T>(o) : defarg;
    }
    template<class T> inline T get(size_t number, const std::string& key, ArgLocker *lk=NULL) {
        PyObject* o = getItem(key, false, lk);
        if (o) return fromPy<T>(o);
        o = getItem(number, false, lk);
        if (o) return fromPy<T>(o);
        errMsg ("Argument '" + key + "' is not defined.");        
    }
    template<class T> inline T getOpt(size_t number, const std::string& key, T defarg, ArgLocker *lk=NULL) { 
        PyObject* o = getItem(key, false, lk);
        return (o) ? fromPy<T>(o) : getOpt<T>(number, defarg);
    }
    template<class T> inline T get(const std::string& key) { 
        return fromPy<T>(getItem(key, true)); 
    }
    template<class T> inline T getOpt(const std::string& key, T defarg) { 
        PyObject* o = getItem(key, false);
        return (o) ? fromPy<T>(o) : defarg;
    }
    template<class T> Grid<T>* getGrid(const std::string& key) { 
        return get<Grid<T>*>(key);         
    }
    template<class T> Grid<T>* getGridOpt(const std::string& key, Grid<T>* defGrid) { 
        return getOpt<Grid<T>*>(key, defGrid);         
    }
    
    PbArgs& operator=(const PbArgs& a); // dummy
    void copy(PbArgs& a);
    void clear();
    
    static PbArgs EMPTY;
    
protected:
    PyObject* getItem(const std::string& key, bool strict, ArgLocker* lk = NULL);
    PyObject* getItem(size_t number, bool strict, ArgLocker* lk = NULL);    
    
    struct DataElement {
        PyObject *obj;
        bool visited;
    };
    std::map<std::string, DataElement> mData;
    std::vector<DataElement> mLinData;
    PyObject* mLinArgs, *mKwds;    
};

} // namespace
#endif
#endif
