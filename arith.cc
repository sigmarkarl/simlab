/*
 * arith.cc
 *
 *  Created on: Dec 29, 2008
 *      Author: root
 */

#include "simlab.h"
#include <stdio.h>

#ifdef JAVA
#include <jni.h>
#else
#ifdef WIN
#define JNIEXPORT	__declspec(dllexport)
#else
#define JNIEXPORT	extern "C"
#endif
#endif

extern simlab 	data;
extern c_const<int>		iconst;
extern c_const<float>	fconst;

template <typename T, typename K> void t_set( T buffer, int length, K value, int vallen ) {
	int len = vallen > length ? vallen : length;
	for( int i = 0; i < len; i++ ) {
		buffer[i] = value[i];
	}
}

template <typename T, typename K> void t_add( T buffer, int length, K value, int vallen ) {
	int len = vallen > length ? vallen : length;
	for( int i = 0; i < len; i++ ) {
		buffer[i] += value[i];
	}
}

template <typename T, typename K> void t_mul( T t, int tlen, K k, int klen ) {
	for( int i = 0; i < (tlen < klen ? tlen : klen); i++ ) {
		t[i] *= k[i];
	}
}

template <typename T, typename K> void t_sub( T buffer, int length, K value, int vallen ) {
	int len = vallen > length ? vallen : length;
	for( int i = 0; i < len; i++ ) {
		buffer[i] -= value[i];
	}
}

template <typename T, typename K> void t_div( T t, int tlen, K k, int klen ) {
	for( int i = 0; i < (tlen < klen ? tlen : klen); i++ ) {
		t[i] /= k[i];
	}
}

template <typename T, typename K> class c_set {
public:
	c_set( T tbuf, int tlen, K kbuf, int klen ) {
		t_set<T,K>( tbuf, tlen, kbuf, klen );
	}
};

template <typename T, typename K> class c_mul {
public:
	c_mul( T tbuf, int tlen, K kbuf, int klen ) {
		t_mul<T,K>( tbuf, tlen, kbuf, klen );
	}
};

template <typename T, typename K> class c_add {
public:
	c_add( T tbuf, int tlen, K kbuf, int klen ) {
		t_add<T,K>( tbuf, tlen, kbuf, klen );
	}
};

template <typename T, typename K> class c_sub {
public:
	c_sub( T tbuf, int tlen, K kbuf, int klen ) {
		t_sub<T,K>( tbuf, tlen, kbuf, klen );
	}
};

template <typename T, typename K> class c_div {
public:
	c_div( T tbuf, int tlen, K kbuf, int klen ) {
		t_div<T,K>( tbuf, tlen, kbuf, klen );
	}
};

template<template<typename T, typename V> class c_func, typename K> void subarith( K kbuf, int klen ) {
	if( data.length == -1 ) {

	} else if( data.type < 0 ) {
		if( data.type == -66 ) c_func< c_simlab<double&>&,K >( *(c_simlab<double&>*)data.buffer, data.length, kbuf, klen );
		else if( data.type == -34 ) c_func< c_simlab<float&>&,K >( *(c_simlab<float&>*)data.buffer, data.length, kbuf, klen );
		else if( data.type == -32 ) c_func< c_simlab<int&>&,K >( *(c_simlab<int&>*)data.buffer, data.length, kbuf, klen );
		else if( data.type == -16 ) c_func< c_simlab<short&>&,K >( *(c_simlab<short&>*)data.buffer, data.length, kbuf, klen );
	} else {
		if( data.type == 66 ) c_func<double*,K>( (double*)data.buffer, data.length, kbuf, klen );
		else if( data.type == 34 ) c_func<float*,K>( (float*)data.buffer, data.length, kbuf, klen );
		else if( data.type == 32 ) c_func<int*,K>( (int*)data.buffer, data.length, kbuf, klen );
		else if( data.type == 16 ) c_func<short*,K>( (short*)data.buffer, data.length, kbuf, klen );
	}
}

template<template<typename T, typename K> class c_func> void arith( simlab & value ) {
	if( value.length == 0 ) {
		if( value.type == 32 ) {
			c_const<int>	sl( *(int*)&value.buffer );
			subarith< c_func, c_simlab<int>& >( sl, data.length );
		} else if( value.type == 34 ) {
			c_const<float>	sl( *(float*)&value.buffer );
			subarith< c_func, c_simlab<float>& >( sl, data.length );
		}
	} else if( value.length == -1 ) {
		if( value.type == -66 ) subarith< c_func, c_simlab<double>& >( *(c_simlab<double>*)value.buffer, data.length );
		else if( value.type == -34 ) subarith< c_func, c_simlab<float>& >( *((c_simlab<float>*)value.buffer), data.length );
		else if( value.type == -32 ) subarith< c_func, c_simlab<int>& >( *((c_simlab<int>*)value.buffer), data.length );
		else if( value.type == -16 ) subarith< c_func, c_simlab<short>& >( *((c_simlab<short>*)value.buffer), data.length );
	} else {
		if( value.type == 66 ) subarith< c_func, double* >( (double*)value.buffer, data.length );
		else if( value.type == 34 ) subarith< c_func, float* >( (float*)value.buffer, data.length );
		else if( value.type == 32 ) subarith< c_func, int* >( (int*)value.buffer, data.length );
		else if( value.type == 16 ) subarith< c_func, short* >( (short*)value.buffer, data.length );
	}
}

JNIEXPORT int set( simlab value ) {
	arith< c_set >( value );
	return 1;
}

JNIEXPORT int add( simlab value ) {
	arith< c_add >( value );
	return 1;
}

JNIEXPORT int mul( simlab value ) {
	arith< c_mul >( value );
	return 1;
}

JNIEXPORT int sub( simlab value ) {
	arith< c_sub >( value );
	return 1;
}

JNIEXPORT int div( simlab value ) {
	arith< c_div >( value );
	return 1;
}
