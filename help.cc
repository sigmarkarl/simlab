/*
 * help.cc
 *
 *  Created on: Jan 3, 2009
 *      Author: root
 */

#include "simlab.h"
#include <stdio.h>
#include <map>
#include <string>

static std::map<int,std::string>								helplib;

JNIEXPORT int tut( simlab numr ) {
	int num = numr.buffer;

	if( num == 1 || num == 0 ) {
		printf( "1. Create a doublebuffer, length 10:\n\ttype 66\n\tresize 10\n" );
	} if( num == 2 || num == 0 ) {
		printf( "2. Run system commands:\n\tsys(\"ls\")\n\tsys(\"date\")\n" );
	}

	return 1;
}

JNIEXPORT int func() {
	std::map<int,std::string>::iterator it = helplib.begin();
	while( it != helplib.end() ) {
		int i = it->second.find( '\n' );
		printf( "%s\n", it->second.substr(0,i).c_str() );
		it++;
	}
	return 0;
}

JNIEXPORT int help( simlab fnct ) {
	int fnc = fnct.buffer;
	if( helplib.size() == 0 ) {
		helplib[(long)help] = "help( string functionname )\nShows help for the named function";
		helplib[(long)tut] = "tut( int tut_num )\nShow tutorials";
		/*helplib[(int)sys] = "sys( string cmd_name )\nRun system command";
		helplib[(int)prim] = "prim()\nFills the current buffer with prime numbers in ordered sequence";
		helplib[(int)fibo] = "fibo()\nFills the current buffer with fibonacci numbers in ordered sequence";
		helplib[(int)idx] = "idx()\nFills the current buffer with whole numbers in ordered sequence";
		helplib[(int)sine] = "sine()\nSine of each element of the array";
		helplib[(int)cosine] = "cosine()\nCosine of each element of the array";
		helplib[(int)last] = "prev()\nSelection previous object pointer";
		helplib[(int)poly] = "poly( array poly )\nCalculates the input polynomial for each element of the current array";
		helplib[(int)conv] = "conv( array, int current_columns, int input_columns )\nCreates a new array which is the convolution of the current array with the input array";
		helplib[(int)view] = "view( int start, int size )\nCreates a new array internal to the selected";*/
	}

	if( fnc == 0 ) {
		func();
	} else if( helplib.find(fnc) != helplib.end() ) {
		const char* val = helplib[fnc].c_str();
		if( val != NULL ) printf( "%s\n", val );
	}

	return 1;
}
