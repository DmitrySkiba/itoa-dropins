/*
 * Copyright (C) 2011 Dmitry Skiba
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _PTHREADPP_INCLUDED_
#define _PTHREADPP_INCLUDED_

#include <pthread.h>
#include <errno.h>
#include <exception>

/*
 Various C++ wrappers and utilities for pthread.
 Currently defined (see comments in this file for help):
 
 Wrappers (init, destroy, operator&):
 - mutexattr_wrapper
 - mutex_wrapper
 - condattr_wrapper
 - cond_wrapper
 
 Objects (all methods, check & throw errors):
 - mutex
 
 Utilities:
 - mutex_wrapper_guard
 - mutex_guard

*/

namespace pthreadpp {


///////////////////////////////////////////////////////////////////// wrapper classes

/*
 Wrappers are simple classes which encapsulate pthread object and
  provide functions to init, destroy and get it.
 Use wrappers where the only thing you want is automatic destruction.
 Wrappers don't throw exceptions, they return error codes or just ignore
  them in destructors.
 
 If you want exceptions on errors and even more strict encapsulation use 
 'objects classes' described later in this file.
*/


/*
 Base wrapper class, encapsulates pthread object and knows how to
  destroy it.
*/
template<
    class ObjectType,
    int(*DestroyFn)(ObjectType*)
>
class wrapper_base {
public:
    const ObjectType* operator&() const throw() {
        return m_valid?&m_object:0;
    }
    ObjectType* operator&() throw() {
        return m_valid?&m_object:0;
    }
    
    bool is_valid() const throw() {
        return m_valid;
    }

    int destroy() throw() {
        if (!m_valid) {
            return EINVAL;
        }
        int error=DestroyFn(&m_object);
        m_valid=!!error;
        return error;
    }
    
    /*
     Handy when you need to attach externally created object
      or detach contained object to escape scope.
    */
    void attach(const ObjectType& object) throw() {
        destroy();
        m_object=object;
        m_valid=true;
    }
    ObjectType detach() throw() {
        m_valid=false;
        return m_object;
    }
protected:
    wrapper_base() throw():
        m_valid(false)
    {
    }
    wrapper_base(const ObjectType& initializer) throw():
        m_valid(true),
	    m_object(initializer)
    {
    }
    
    int init_done(int init_error) throw() {
        m_valid=!init_error;
        return init_error;
    }
    ObjectType* handle() {
        return &m_object;
    }
private:
    wrapper_base(const wrapper_base&);
    wrapper_base& operator=(const wrapper_base&);
private:
    bool m_valid;
    ObjectType m_object;
};


/*
 Attribute wrapper class, has init() function with no extra parameters.
*/
template <
    class ObjectType,
    int (*InitFn)(ObjectType*),
    int (*DestroyFn)(ObjectType*)
>
class attr_wrapper: public wrapper_base<ObjectType,DestroyFn> {
    typedef wrapper_base<ObjectType,DestroyFn> base;
public:
    attr_wrapper() throw() {
    }
    explicit attr_wrapper(const ObjectType& initializer) throw():
        base(initializer)
    {
    }
    
    int init() throw() {
        base::destroy();
        return base::init_done(InitFn(&base::handle()));
    }    
};


/*
 Object wrapper class, init() function takes optional attribute as an argument.
*/
template <
    class ObjectType,
    class AttributeType,
    int (*InitFn)(ObjectType*,const AttributeType*),
    int (*DestroyFn)(ObjectType*)
>
class wrapper: public wrapper_base<ObjectType,DestroyFn> {
    typedef wrapper_base<ObjectType,DestroyFn> base;
public:
    wrapper() throw() {
    }
    explicit wrapper(const ObjectType& initializer) throw():
        base(initializer)
    {
    }
    
    int init(const AttributeType* attrs=0) throw() {
        base::destroy();
        return base::init_done(InitFn(base::handle(),attrs));
    }
};


/*
 Typedefs for mutexattr_wrapper and mutex_wrapper.
*/
typedef attr_wrapper<
    pthread_mutexattr_t,
    pthread_mutexattr_init,
    pthread_mutexattr_destroy
> mutexattr_wrapper;
typedef wrapper<
    pthread_mutex_t,
    pthread_mutexattr_t,
    pthread_mutex_init,
    pthread_mutex_destroy
> mutex_wrapper;


/*
 Typedefs for condattr_wrapper and cond_wrapper.
*/
typedef attr_wrapper<
     pthread_condattr_t,
     pthread_condattr_init,
     pthread_condattr_destroy
> condattr_wrapper;
typedef wrapper<
    pthread_cond_t,
    pthread_condattr_t,
    pthread_cond_init,
    pthread_cond_destroy
> cond_wrapper;


///////////////////////////////////////////////////////////////////// object classes

/*
 Object classes provide true incapsulation and also check for errors.
 They throw pthreadpp::fatal_error on all errors except for destroy() call
  in destructor. This is deliberate choice as I prefer rather to leak 
  object than to have terminate() called in case of double exception.
*/


/*
 Fatal error exception.
 Thrown when pthreadpp object (e.g. mutex) gets a nonzero error code 
  from pthread function.
 The best thing to do when you get this exception is to log and die.
*/
class fatal_error: public std::exception {
public:
    fatal_error(int error_code):
        m_error_code(error_code)
    {
    }
    int error_code() const throw() {
        return m_error_code;
    }
    virtual const char* what() const throw() {
        return "pthread function returned an unexpected error.";
    }
private:
    int m_error_code;    
};

/*
 Mutex object.
*/
class mutex {
public:
    explicit mutex(const pthread_mutexattr_t* attrs=0) {
        check_error(m_mutex.init(attrs));
    }
    explicit mutex(const pthread_mutex_t& initializer) throw():
        m_mutex(initializer)
    {
    }
    
    ~mutex() throw() {
        m_mutex.destroy();
    }
    
    void lock() {
        check_error(pthread_mutex_lock(&m_mutex));
    }
    bool trylock() {
        int error=pthread_mutex_trylock(&m_mutex);
        if (error==EBUSY) {
            return false;
        }
        check_error(error);
        return true;
    }
    void unlock() {
        check_error(pthread_mutex_unlock(&m_mutex));
    }

    // Use with care, don't destroy.    
    const pthread_mutex_t* handle() const {
        return &m_mutex;
    }
    pthread_mutex_t* handle() {
        return &m_mutex;
    }
    
    /** C++ compatible variant of PTHREAD_MUTEX_INITIALIZER.
     * Some implementations define PTHREAD_MUTEX_INITIALIZER as a struct initializer
     *  which can't be used to initialize mutex object.
     */
    static pthread_mutex_t initializer() {
        pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
        return mutex;
    }
private:
    static void check_error(int error_code) {
        if (error_code) {
            throw fatal_error(error_code);
        }
    }
private:
    mutex_wrapper m_mutex;
};

///////////////////////////////////////////////////////////////////// utilities

/*
 Automatic guard for mutex_wrapper.
 Can be created with raw pthread_mutex_t* too.
 Ignores errors from lock/unlock functions.
*/
class mutex_wrapper_guard {
public:
    explicit mutex_wrapper_guard(mutex_wrapper& m) throw():
        m_mutex(&m)
    {
        pthread_mutex_lock(m_mutex);
    }
    mutex_wrapper_guard(pthread_mutex_t* m) throw():
        m_mutex(m)
    {
        pthread_mutex_lock(m_mutex);
    }
    ~mutex_wrapper_guard() throw() {
        pthread_mutex_unlock(m_mutex);
    }
private:
    mutex_wrapper_guard(const mutex_wrapper_guard&);
    mutex_wrapper_guard& operator=(const mutex_wrapper_guard&);
private:
    pthread_mutex_t* m_mutex;
};


/*
 Automatic mutex guard. Can throw exception from ctor/dtor.
*/
class mutex_guard {
public:
    explicit mutex_guard(mutex& m):
        m_mutex(m)
    {
        m_mutex.lock();
    }
    ~mutex_guard() {
        m_mutex.unlock();
    }
private:
    mutex_guard(const mutex_guard&);
    mutex_guard& operator=(const mutex_guard&);
private:
    mutex& m_mutex;
};


/////////////////////////////////////////////////////////////////////

} // namespace pthreadpp

#endif // _PTHREADPP_INCLUDED_
