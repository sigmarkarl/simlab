/*
 * func.cc
 *
 *  Created on: Feb 3, 2009
 *      Author: root
 */

#include "simlab.h"
#include <math.h>
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

template <typename T, typename K> void t_func( T t, int tlen, double (*func)(double) ) {
	for( int i = 0; i < tlen; i++ ) {
		t[i] = (double)0;
		func( t[i] );
	}
}

void dfunc( double (*func)( double ) ) {
	if( data.length == -1 ) {
		if( data.type == -66 ) t_func< c_simlab<double&>&,double >( *(c_simlab<double&>*)data.buffer, data.length, func );
		//else if( data.type == -34 ) t_func< c_sl<float>&,float >( *((c_sl<float>*)data.buffer), data.length, func );
		//else if( data.type == -32 ) t_func< c_sl<int>&,int >( *((c_sl<int>*)data.buffer), data.length, func );
		//else if( data.type == -16 ) t_func< c_sl<short>&,short >( *((c_sl<short>*)data.buffer), data.length, func );
	} else {
		//if( data.type == 66 ) t_func< double*,double >( (double*)data.buffer, data.length, func );
		//else if( data.type == 34 ) t_func< float*,float >( (float*)data.buffer, data.length, func );
		//else if( data.type == 32 ) t_func< int*,int >( (int*)data.buffer, data.length, func );
		//else if( data.type == 16 ) t_func< short*,short >( (short*)data.buffer, data.length, func );
	}
}

JNIEXPORT int arcsin() {
	dfunc( asin );
	return 0;
}

JNIEXPORT int arccos() {
	dfunc( acos );
	return 0;
}

JNIEXPORT int arctan() {
	dfunc( atan );
	return 0;
}
