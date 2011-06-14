//
//  qscriptable.h
//  qScript
//
//  Created by avansc on 6/7/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _qscriptable_h
#define _qscriptable_h

#include <new>

#ifdef QRENDER_EXPORTS
#define QSCRIPTEXPORT_API		__declspec(dllexport)
#define QSCRIPT_TEMPLATE
#else
#define QSCRIPTEXPORT_API		__declspec(dllimport)
#define QSCRIPT_TEMPLATE extern
#endif

class qscriptengine;

template<class T>
//class QSCRIPTEXPORT_API qscriptable
class qscriptable
{
public:
    qscriptable();
    qscriptable(const T &other);
    ~qscriptable();
    
    virtual void REGISTER_SCRIPTABLES(qscriptengine *engine) = 0;
    
    void addRef();
    void release();
    
    T &operator=(const T &other);
    
    static void constructor(void *mem);
    static void destructor(void *mem);
    static void *factory();
    static T *copyFactory(const T &other);
    
    static void regit(qscriptengine *engine);
    
protected:
    int refCount;
};

template<class T>
qscriptable<T>::qscriptable()
:   refCount(1)
{
}

template<class T>
qscriptable<T>::qscriptable(const T &other)
:   refCount(1)
{
    //this = &other;
}

template<class T>
qscriptable<T>::~qscriptable()
{
}

template<class T>
void qscriptable<T>::addRef()
{
    this->refCount++;
}

template<class T>
void qscriptable<T>::release()
{
    if(--this->refCount == 0)
        delete this;
}

template<class T>
T &qscriptable<T>::operator=(const T &other)
{
    // Copy only the data, not the reference counter
    // ex. buffer = other.buffer;
    
    // Return a reference to this object
    return *((T*)this);
	//return *dynamic_cast<T*>(this);
}


template<class T>
void qscriptable<T>::constructor(void *mem)
{
    new(mem) T();
}

template<class T>
void qscriptable<T>::destructor(void *mem)
{
    ((T*)mem)->~T();
}

template<class T>
void *qscriptable<T>::factory()
{
    return new T();
}

template<class T>
T *qscriptable<T>::copyFactory(const T &other)
{
    return new T(other);
}

template<typename T>
void qscriptable<T>::regit(qscriptengine *engine)
{
    //T::LIBRARY_REGISTER_SCRIPTABLES(engine);
}

#endif