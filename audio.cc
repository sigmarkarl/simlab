/*
 * audio.cc
 *
 *  Created on: Dec 28, 2008
 *      Author: root
 */

#include "simlab.h"

#include <stdio.h>
//#include <pulse/simple.h>

extern simlab data;

/*JNIEXPORT int play() {
	pa_simple *s;
	pa_sample_spec ss;

	ss.format = PA_SAMPLE_S16NE;
	ss.channels = 2;
	ss.rate = 44100;

	s = pa_simple_new(NULL, "Simlab", PA_STREAM_PLAYBACK, NULL, "Music", &ss, NULL, NULL, NULL );

	int error;
	pa_simple_write( s, (void*)data.buffer, bytelength( data.type, data.length ), &error );

	return 0;
}

JNIEXPORT int record() {
	printf( "record" );

	return 0;
}*/
