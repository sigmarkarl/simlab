/*
 * conv.cc
 *
 *  Created on: Feb 4, 2009
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

JNIEXPORT int zero();
extern simlab 	data;
extern c_const<int>		iconst;
extern c_const<float>	fconst;

template <typename T,typename K> T* t_deconvolve( T* buffer, int length, K* c_buffer, int c_length, T* ret ) {
	for( int k = length-1; k >= c_length; k-- ) {
		int i = k-c_length;
		T val = (buffer[k]-ret[i])/c_buffer[c_length-1];
		int start = i-c_length-1 > 0 ? i-c_length-1 : 0;
		for( int t = start; t < i; t++ ) {
			ret[t] += val*c_buffer[t-start];
		}
		ret[i] = val;
	}
}

template <typename T,typename K> T* t_convolve( T* buffer, int length, K* c_buffer, int c_length, T* ret ) {
	for( int k = 0; k < length; k++ ) {
		for( int t = k; t < k+c_length; t++ ) {
			ret[t] += buffer[k]*c_buffer[t-k];
		}
	}
}

template <typename T,typename K> void t_conv( T* buffer, int length, int chunk, K* c_buffer, int c_length, int c_chunk ) {
	int retchunk = (chunk+c_chunk-1);
	int retlen = retchunk*(length/chunk);
	T* ret = new T[ retlen ];
	data.buffer = (int)ret;
	data.length = retlen;
	zero();
	//int r = 0;
	for( int i = 0; i < length; i+=chunk ) {
		int w = i/chunk;
		for( int k = i; k < i+chunk; k++ ) {
			for( int t = k; t < k+c_chunk; t++ ) {
				ret[w*retchunk+(t-i)] += (T)(buffer[k]*c_buffer[(w*c_chunk+(t-k))%c_length]);
			}
		}
	}
}

template <typename T,typename K> void t_deconv( T* buffer, int length, int chunk, K* c_buffer, int c_length, int c_chunk ) {
	int retchunk = chunk-c_chunk+1;
	int retlen = retchunk*(length/chunk);
	T* ret = new T[ retlen ];
	data.buffer = (int)ret;
	data.length = retlen;
	zero();
	int r = 0;
	for( int i = 0; i < length; i+=chunk ) {
		int w = i/chunk;
		for( int k = i; k < i+retchunk; k++ ) {
			ret[r] = (T)(buffer[k]/c_buffer[(w*c_chunk)%c_length]);
			for( int t = k+1; t < k+c_chunk; t++ ) {
				buffer[t] -= (T)(ret[r]*c_buffer[(w*c_chunk+(t-k))%c_length]);
			}
			r++;
		}
	}
}

template <typename T,typename K,typename V> class c_conv {
public:
	c_conv( T tbuf, int tlen, K kbuf, int klen, int rlen ) : t(tbuf), tl(tlen), k(kbuf), kl(klen), rl(rlen) {
		rchunk = tl+kl-1;
	}
	virtual V & operator[]( int i ) {
		r = 0;
		int w = i/rchunk;
		int v = i%rchunk;

		int wc = w*tl;
		int wcc = w*kl;

		int start = v-kl < 0 ? 0: v-kl;
		int stop = v > tl ? tl : v;
		for( int u = start; u <= stop; u++ ) {
			r += t[wc + u]*k[wcc + kl-u-1];
		}
		return r;
	}
	T 	t;
	int tl;
	K	k;
	int kl;
	int rl;

	V	r;
	int rchunk;
};

template <typename T> void t_conver( T buffer, int chunk, int c_chunk, int rlen ) {
	if( data.type < 0 ) {
		if( data.type == -66 ) {
			data.buffer = (int)new c_conv<c_simlab<double&>&,T,double>( *(c_simlab<double&>*)data.buffer, chunk, buffer, c_chunk, rlen );
			data.length = rlen;
		}
	} else {
		if( data.type == 66 ) {
			data.buffer = (int)new c_conv<double*,T,double>( (double*)data.buffer, chunk, buffer, c_chunk, rlen );
			data.type = -66;
			data.length = rlen;
		}
	}
}

JNIEXPORT int convolver( simlab convee, simlab chnk, simlab c_chnk ) {
	int ret = 3;
	if( c_chnk.buffer == 0 ) {
		c_chnk.buffer = convee.length;
		ret = 2;
	}
	if( chnk.buffer == 0 ) {
		chnk.buffer = data.length;
		ret = 1;
	}
	int chunk = chnk.buffer;
	int c_chunk = c_chnk.buffer;

	int retchunk = chunk+c_chunk-1;
	int retlen = retchunk*(data.length/chunk);

	if( convee.type < 0 ) {
		if( convee.type == -66 ) t_conver<c_simlab<double&>&>( *(c_simlab<double&>*)convee.buffer, chunk, c_chunk, retlen );
	} else {
		if( convee.type == 66 ) t_conver<double*>( (double*)convee.buffer, chunk, c_chunk, retlen );
	}

	/*if( data.type == 66 ) {
		if( convee.type == 66 ) t_conver( (double*)data.buffer, data.length, chunk, (double*)convee.buffer, convee.length, c_chunk );
		else if( convee.type == 32 ) t_conv( (double*)data.buffer, data.length, chunk, (int*)convee.buffer, convee.length, c_chunk );
		else if( convee.type == 8 ) t_conv( (double*)data.buffer, data.length, chunk, (unsigned char*)convee.buffer, convee.length, c_chunk );
	} else if( data.type == 32 ) {
		if( convee.type == 66 ) t_conv( (int*)data.buffer, data.length, chunk, (double*)convee.buffer, convee.length, c_chunk );
		else if( convee.type == 32 ) t_conv( (int*)data.buffer, data.length, chunk, (int*)convee.buffer, convee.length, c_chunk );
		else if( convee.type == 8 ) t_conv( (int*)data.buffer, data.length, chunk, (unsigned char*)convee.buffer, convee.length, c_chunk );
	} else if( data.type == 8 ) {
		if( convee.type == 66 ) t_conv( (unsigned char*)data.buffer, data.length, chunk, (double*)convee.buffer, convee.length, c_chunk );
		else if( convee.type == 32 ) t_conv( (unsigned char*)data.buffer, data.length, chunk, (int*)convee.buffer, convee.length, c_chunk );
		else if( convee.type == 8 ) t_conv( (unsigned char*)data.buffer, data.length, chunk, (unsigned char*)convee.buffer, convee.length, c_chunk );
	}*/

	return ret;
}

JNIEXPORT int conv( simlab convee, simlab chnk, simlab c_chnk ) {
	if( chnk.buffer == 0 ) chnk.buffer = data.length;
	if( c_chnk.buffer == 0 ) c_chnk.buffer = convee.length;
	int chunk = chnk.buffer;
	int c_chunk = c_chnk.buffer;
	if( data.type == 66 ) {
		if( convee.type == 66 ) t_conv( (double*)data.buffer, data.length, chunk, (double*)convee.buffer, convee.length, c_chunk );
		//(chunk+c_chunk-1)*(data.length/chunk) )
		else if( convee.type == 32 ) t_conv( (double*)data.buffer, data.length, chunk, (int*)convee.buffer, convee.length, c_chunk );
		else if( convee.type == 8 ) t_conv( (double*)data.buffer, data.length, chunk, (unsigned char*)convee.buffer, convee.length, c_chunk );
	} else if( data.type == 32 ) {
		if( convee.type == 66 ) t_conv( (int*)data.buffer, data.length, chunk, (double*)convee.buffer, convee.length, c_chunk );
		else if( convee.type == 32 ) t_conv( (int*)data.buffer, data.length, chunk, (int*)convee.buffer, convee.length, c_chunk );
		else if( convee.type == 8 ) t_conv( (int*)data.buffer, data.length, chunk, (unsigned char*)convee.buffer, convee.length, c_chunk );
	} else if( data.type == 8 ) {
		if( convee.type == 66 ) t_conv( (unsigned char*)data.buffer, data.length, chunk, (double*)convee.buffer, convee.length, c_chunk );
		else if( convee.type == 32 ) t_conv( (unsigned char*)data.buffer, data.length, chunk, (int*)convee.buffer, convee.length, c_chunk );
		else if( convee.type == 8 ) t_conv( (unsigned char*)data.buffer, data.length, chunk, (unsigned char*)convee.buffer, convee.length, c_chunk );
	}
	return 3;
}

JNIEXPORT int deconv( simlab convee, simlab chnk, simlab c_chnk ) {
	if( chnk.buffer == 0 ) chnk.buffer = data.length;
	if( c_chnk.buffer == 0 ) c_chnk.buffer = convee.length;
	int chunk = chnk.buffer;
	int c_chunk = c_chnk.buffer;
	if( data.type == 66 ) {
		if( convee.type == 66 ) {
			t_deconv( (double*)data.buffer, data.length, chunk, (double*)convee.buffer, convee.length, c_chunk );
		}
		else if( convee.type == 32 ) t_deconv( (double*)data.buffer, data.length, chunk, (int*)convee.buffer, convee.length, c_chunk );
		else if( convee.type == 8 ) t_deconv( (double*)data.buffer, data.length, chunk, (unsigned char*)convee.buffer, convee.length, c_chunk );
	} else if( data.type == 32 ) {
		if( convee.type == 66 ) t_deconv( (int*)data.buffer, data.length, chunk, (double*)convee.buffer, convee.length, c_chunk );
		else if( convee.type == 32 ) t_deconv( (int*)data.buffer, data.length, chunk, (int*)convee.buffer, convee.length, c_chunk );
		else if( convee.type == 8 ) t_deconv( (int*)data.buffer, data.length, chunk, (unsigned char*)convee.buffer, convee.length, c_chunk );
	} else if( data.type == 8 ) {
		if( convee.type == 66 ) t_deconv( (unsigned char*)data.buffer, data.length, chunk, (double*)convee.buffer, convee.length, c_chunk );
		else if( convee.type == 32 ) t_deconv( (unsigned char*)data.buffer, data.length, chunk, (int*)convee.buffer, convee.length, c_chunk );
		else if( convee.type == 8 ) t_deconv( (unsigned char*)data.buffer, data.length, chunk, (unsigned char*)convee.buffer, convee.length, c_chunk );
	}
	return 3;
}
