/*
 * map.cc
 *
 *  Created on: Feb 3, 2009
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

template <typename K, typename T, typename V> class c_map : public c_simlab<V> {
public:
	c_map( T tb, K kb ) : t(tb), k(kb) {}
	virtual V operator[]( int i ) {
		return t[ (int)k[i] ];
	}
	T	t;
	K	k;
};

template <typename T> void t_mapper( T t, int l ) {
	if( data.length == -1 ) {
		if( data.type == -66 ) {
			data.buffer = (long)new c_map< T, c_simlab<double&>&, const double& >( *(c_simlab<double&>*)data.buffer, t );
			data.length = l;
		}
		//else if( data.type == -32 ) c_vmap< c_simlab<int&>& >( *(c_simlab<double&>*)value.buffer, value.length );
	} else {
		if( data.type == 66 ) {
			data.buffer = (long)new c_map< T, double*, const double& >( (double*)data.buffer, t );
			data.type = -66;
			data.length = l;
		}
		//else if( data.type == 32 ) c_vmap< int* >( (int*)value.buffer, value.length );
	}
}

inline void mppr( simlab & value ) {
	if( value.length == -1 ) {
		if( value.type == -66 ) t_mapper< c_simlab<double&>& >( *(c_simlab<double&>*)value.buffer, value.length );
		else if( value.type == -32 ) t_mapper< c_simlab<int&>& >( *(c_simlab<int&>*)value.buffer, value.length );
	} else {
		if( value.type == 66 ) t_mapper< double* >( (double*)value.buffer, value.length );
		else if( value.type == 32 ) t_mapper< int* >( (int*)value.buffer, value.length );
	}
}

JNIEXPORT int mapper( simlab value ) {
	mppr( value );
	return 1;
}
