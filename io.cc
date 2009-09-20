/*
 * io.cc
 *
 *  Created on: Jan 3, 2009
 *      Author: root
 */

#include "simlab.h"
#include <stdio.h>
#include <string>
#include <string.h>

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

extern passa<4> 	passnext;

JNIEXPORT int load( simlab filename, simlab str ) {
	FILE*	f = fopen( (const char*)filename.buffer, "r" );
	char*	buffer = (char*)data.buffer;
	int len = data.type/32;
	//while( fscanf( f, (const char*)str.buffer, buffer, buffer+(len/4) ) ==  ) {

	//}
	fclose( f );

	return 2;
}

JNIEXPORT int oldload( simlab filename, ... ) {
	FILE* f = fopen( (char*)filename.buffer, "r" );
	std::string scan;

	int i = 0;
	simlab*	pLab = (simlab*)&passnext;
	simlab & lab = pLab[++i];
	if( lab.type == 8 ) scan.append("%s");
	else if( lab.type == 32 ) scan.append("%d");
	lab = pLab[++i];
	while( lab.type != 0 ) {
		if( lab.type == 8 ) scan.append("\t%s");
		else if( lab.type == 32 ) scan.append("\t%d");
		lab = pLab[++i];
	}
	scan.append("\n");

	int l = 0;

	char buffer[12];
	int t = 0;

	for( int k = 1; k < i; k++ ) {
		lab = pLab[k];
		if( lab.type == 32 ) {
			if( l)
			*(long*)(&buffer[t]) = (long)&lab.buffer;
			t += 32/8;
		}
	}

	while( fscanf( f, scan.c_str(), buffer ) == i-1 ) {

	}

	fclose( f );

	return i;
}

template <char count> void t_dump( FILE* f, const char* format ) {
	printf( "%d %s\n", count, format );

	passa<count>*	s = (passa<count>*)data.buffer;

	for( int i = 0; i < data.length; i+=count ) {
		fprintf( f, format, s+=sizeof(passa<count>) );
	}
}

JNIEXPORT int dump( simlab filename, simlab str ) {
	//printf( "hey %d", sizeof(double*) );
	//return 0;

	FILE* f = stdout;
	if( strcmp( (const char*)filename.buffer, "stdout" ) != 0 ) {
		f = fopen( (const char*)filename.buffer, "w" );
	}
	const char* format = (const char*)str.buffer;
	//printf( "%s %d %d\n", (char*)str.buffer, str.type, str.length );
	int c = 0;
	int v = 0;
	while( format[c] != 0 ) {
		if( format[c] == '%' ) v++;
		c++;
	}

	if( v == 1 ) t_dump<1>( f, format );
	else if( v == 2 ) t_dump<2>( f, format );
	else if( v == 3 ) t_dump<3>( f, format );
	else if( v == 4 ) t_dump<4>( f, format );
	else if( v == 5 ) t_dump<5>( f, format );
	else if( v == 6 ) t_dump<6>( f, format );
	else if( v == 7 ) t_dump<7>( f, format );
	else if( v == 8 ) t_dump<8>( f, format );
	else if( v == 9 ) t_dump<9>( f, format );
	else if( v == 10 ) t_dump<10>( f, format );
	else if( v == 11 ) t_dump<11>( f, format );
	else if( v == 12 ) t_dump<12>( f, format );

	fclose( f );

	return 2;
}

JNIEXPORT int loader( simlab filename, simlab str ) {

	return 2;
}
