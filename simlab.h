/*
 * simlab.h
 *
 *  Created on: Dec 29, 2008
 *      Author: root
 */

#ifndef SIMLAB_H_
#define SIMLAB_H_

#ifdef JAVA
#include <jni.h>
#else
#ifdef WIN
#define JNIEXPORT	__declspec(dllexport)
#else
#define JNIEXPORT	extern "C"
#endif
#endif

#ifndef WIN
#include <dlfcn.h>
#else
#include <windows.h>
#endif

inline long dopen( char* name ) {
#ifndef WIN
	return (long)dlopen( name, RTLD_GLOBAL );
#else
	return (long)GetModuleHandle( name );
#endif
}

inline long dsym( int handle, char* symbol ) {
#ifndef WIN
	return (long)dlsym( (void*)handle, symbol );
#else
	return (long)GetProcAddress( (HINSTANCE)handle, symbol );
#endif
}

struct simlab {
	long		buffer;
	long		length;
	long		type;
};

inline int bytelength( int type, int length ) {
	if( type < 8 ) return (type*length)/8;
	return (type/8)*length;
}

template <typename T> class c_sl {
public:
	c_sl() {};
	virtual T operator[]( int i ) { return v; };
	T	v;
};

template <typename T> class c_simlab {
public:
	c_simlab() {};
	virtual T operator[]( int i ) { } ;//return v; };
};

template <typename T> class c_const : public c_simlab<T> {
public:
	c_const( T t ) : c(t) {};
	virtual T operator[]( int i ) { return c; }
	T c;
};

template<char count>
struct passa {
	simlab dw;
    passa<count-1> big;
};
template<> struct passa<0>{};

template<char count>
struct passb {
    passb<count-1> big;
    int dw;
};
template<> struct passb<0>{};

#endif /* SIMLAB_H_ */
