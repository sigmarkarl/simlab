#include "simlab.h"

#include <stdio.h>
#include <stdarg.h>
#include <vector>

#ifdef JAVA
#include <jni.h>
#else
#ifdef WIN
#define JNIEXPORT	__declspec(dllexport)
#else
#define JNIEXPORT	extern "C"
#endif
#endif

extern simlab 		data;
extern passa<4> 	passnext;
int (*prnt)( const char*, ... );

template <typename T, typename K> class c_printer {
public:
	c_printer( const T nv, int size ) : v(nv), s(size) { /*v = (int*)*new int[1]; *v = (int)nv; */ };
	virtual K operator[]( int i ) { return (K)v[i]; };
	const T 	v;
	int	s;
};

template <> int c_printer<int*,int>::operator []( int i ) {
	if( i % s == s-1 ) printf("%d\n", v[i] );
	else printf("%d ", v[i] );
	return v[i];
};

template <> float c_printer<float*,float>::operator []( int i ) {
	if( i % s == s-1 ) printf("%f\n", v[i] );
	else printf("%f ", v[i] );
	return v[i];
};

template <> double c_printer<double*,double>::operator []( int i ) {
	if( i % s == s-1 ) printf("%e \n", (float)v[i] );
	else printf("%e ", (float)v[i] );
	return v[i];
};

template <typename T, typename K> void t_printrecursive( const char* format, const char* end, T & buffer, std::vector<int> & shape, int index, int here ) {
	if( index > 0 ) {
		long len = 1;
		for( int k = 0; k < index; k++ ) {
			len *= shape[k];
		}
		for( int i = 0; i < shape[index]; i++ ) {
			t_printrecursive<T,K>( format, end, buffer, shape, index-1, here+i*len );
		}
	} else {
		int length = shape[0];
		for( int i = here; i < here+length; i++ ) {
			prnt( format, (K)buffer[i] );
		}
	}
	prnt( end );
}

template <typename T,typename K> void t_print( const char* format, const char* end, T & buffer, int length, ... ) {
	va_list args;
	va_start(args, length);

	std::vector<int> vi;
	simlab val = va_arg( args, simlab );
	int mul = 1;
	while( val.buffer != 0 && vi.size() < 8 ) {
		vi.push_back( val.buffer );
		mul *= val.buffer;
		val = va_arg( args, simlab );
	}
	va_end( args );
	if( length == -1 ) mul = length;
	else if( mul != length || vi.size() == 0 ) vi.push_back( length/mul );
	t_printrecursive<T,K>( format, end, buffer, vi, vi.size()-1, 0 );
}

extern "C" JNIEXPORT int printer( simlab size ) {
	if( data.type == 32 ) {
		c_printer<int*,int> * printer = new c_printer<int*,int>( (int*)data.buffer, size.buffer );
		data.buffer = (int)printer;
		data.type = -data.type;
	} else if( data.type == 34 ) {
		//float* f = (float*)data.buffer;
		c_printer<float*,float> * printer = new c_printer<float*,float>( (float*)data.buffer, size.buffer );
		data.buffer = (int)printer;
		data.type = -data.type;

		/*c_printer<float*,float> & pf = *printer;
		for( int i = 0; i < data.length; i++ ) {
			printf( "%f co%f\n", f[i], pf[i] );
		}*/
	} else if( data.type == 66 ) {
		c_printer<double*,double> * printer = new c_printer<double*,double>( (double*)data.buffer, size.buffer );
		data.buffer = (int)printer;
		data.type = -data.type;
	}

	return 0;
}

extern "C" JNIEXPORT int touch( simlab val ) {
	if( val.type == 0 ) {
		if( data.type == -32 ) {
			c_simlab<int> & cslab = *(c_simlab<int>*)data.buffer;
			for( int i = 0; i < data.length; i++ ) {
				cslab[i];
			}
		} else if( data.type == -34 ) {
			c_simlab<float> & cslab = *(c_simlab<float>*)data.buffer;
			for( int i = 0; i < data.length; i++ ) {
				cslab[i];
			}
		} else if( data.type == -66 ) {
			c_simlab<double> & cslab = *(c_simlab<double>*)data.buffer;
			for( int i = 0; i < data.length; i++ ) {
				cslab[i];
			}
		}
	} else {
		if( data.type == -32 ) {
			c_simlab<int> & cslab = *(c_simlab<int>*)data.buffer;
			cslab[val.buffer];
		} else if( data.type == -34 ) {
			c_simlab<float> & cslab = *(c_simlab<float>*)data.buffer;
			cslab[val.buffer];
		} else if( data.type == -66 ) {
			c_simlab<double> & cslab = *(c_simlab<double>*)data.buffer;
			cslab[val.buffer];
		}
	}

	return 1;
}

JNIEXPORT int print( const char* format, const char* end, ... ) {
	if( format == NULL ) {
		char* c = (char*)data.buffer;
		for( int i = 0; i < data.length; i++ ) {
			putchar( c[i] );
		}
		putchar( '\n' );
	} else if( data.type > 0 ) {
		if( end == NULL ) end = "\n";
		if( data.length == -1 ) {
			if( data.type == 66 ) {
				//PseudoBuffer<double>*	buffer = (PseudoBuffer<double>*)data.buffer;
				//if( format[1] == 'e' ) t_print<PseudoBuffer<double>,double>( format, end, *buffer, buffer->length(), passnext );
				//else if( format[1] == 'd' ) t_print<PseudoBuffer<double>,int>( format, end, *buffer, buffer->length(), passnext );
			} else if( data.type == 32 ) {
				c_simlab<int>*	buffer = (c_simlab<int>*)data.buffer;
				//if( format[1] == 'e' ) t_print<c_simlab<int>,double>( format, end, *buffer, buffer->length(), passnext );
				//else if( format[1] == 'd' ) t_print<c_simlab<int>,int>( format, end, *buffer, buffer->length(), passnext );
			} else if( data.type == 34 ) {
				//PseudoBuffer<float>*	buffer = (PseudoBuffer<float>*)data.buffer;
				//if( format[1] == 'e' ) t_print<PseudoBuffer<float>,double>( format, end, *buffer, buffer->length(), passnext );
				//else if( format[1] == 'd' ) t_print<PseudoBuffer<float>,int>( format, end, *buffer, buffer->length(), passnext );
			}
		} else if( data.length == 0 ) {
			/*if( data.type == 32 ) {
				if( format[1] == 'e' ) t_print<unsigned*,double>( format, end, (unsigned int*)&data.buffer, 1, passnext );
				else if( format[1] == 'd' ) t_print<unsigned int*,int>( format, end, (unsigned int*)&data.buffer, 1, passnext );
			} else if( data.type == 34 ) {
				if( format[1] == 'e' ) t_print<float*,double>( format, end, (float*)&data.buffer, 1, passnext );
				else if( format[1] == 'd' ) t_print<float*,int>( format, end, (float*)&data.buffer, 1, passnext	 );
			}*/
		} else {
			if( data.type == 66 ) {
				double* db = (double*)data.buffer;
				if( format[1] == 'e' ) {
					printf("ok\n");
					t_print<double*,double>( format, end, db, data.length, passnext );
				} else if( format[1] == 'd' ) {
					t_print<double*,int>( format, end, db, data.length, passnext );
				}
			} else if( data.type == 32 ) {
				int* ib = (int*)data.buffer;
				if( format[1] == 'e' ) t_print<int*,double>( format, end, ib, data.length, passnext );
				else if( format[1] == 'd' ) t_print<int*,int>( format, end, ib, data.length, passnext );
			} else if( data.type == 16 ) {
				short* sb = (short*)data.buffer;
				if( format[1] == 'e' ) t_print<short*,double>( format, end, sb, data.length, passnext );
				else if( format[1] == 'd' ) t_print<short*,int>( format, end, sb, data.length, passnext );
			} else if( data.type == 34 ) {
				float* fb = (float*)data.buffer;
				if( format[1] == 'e' ) t_print<float*,double>( format, end, fb, data.length, passnext );
				else if( format[1] == 'd' ) t_print<float*,int>( format, end, fb, data.length, passnext	 );
			} else if( data.type == 8 ) {
				char*	cb = (char*)data.buffer;
				if( format[1] == 'e' ) t_print<char*,double>( format, end, cb, data.length, passnext );
				else if( format[1] == 'd' ) t_print<char*,int>( format, end, cb, data.length, passnext	 );
			}
		}
	} else {
		if( data.type == -66 ) {
			c_simlab<double&> & sdb = *(c_simlab<double&>*)data.buffer;
			if( format[1] == 'e' ) {
				printf("okok %d\n", sdb[2] );
				t_print<c_simlab<double&>,double>( format, end, sdb, data.length, passnext );
			} else if( format[1] == 'd' ) {
				t_print<c_simlab<double&>,int>( format, end, sdb, data.length, passnext );
			}
		} else if( data.type == -64 ) {
			c_simlab<long long> & slb = *(c_simlab<long long>*)data.buffer;
			if( format[1] == 'e' ) t_print<c_simlab<long long>,double>( format, end, slb, data.length, passnext );
			else if( format[1] == 'd' ) t_print<c_simlab<long long>,int>( format, end, slb, data.length, passnext );
			else if( format[1] == 'l' ) t_print<c_simlab<long long>,long long>( format, end, slb, data.length, passnext );
		} else if( data.type == -32 ) {
			c_simlab<int> & sib = *(c_simlab<int>*)data.buffer;
			if( format[1] == 'e' ) t_print<c_simlab<int>,double>( format, end, sib, data.length, passnext );
			else if( format[1] == 'd' ) t_print<c_simlab<int>,int>( format, end, sib, data.length, passnext );
		} else if( data.type == -16 ) {
			short* sb = (short*)data.buffer;
			if( format[1] == 'e' ) t_print<short*,double>( format, end, sb, data.length, passnext );
			else if( format[1] == 'd' ) t_print<short*,int>( format, end, sb, data.length, passnext );
		} else if( data.type == -34 ) {
			c_simlab<float> & sfb = *(c_simlab<float>*)data.buffer;
			if( format[1] == 'e' ) t_print<c_simlab<float>,double>( format, end, sfb, data.length, passnext );
			else if( format[1] == 'd' ) t_print<c_simlab<float>,int>( format, end, sfb, data.length, passnext	);
		} else if( data.type == -8 ) {
			char*	cb = (char*)data.buffer;
			if( format[1] == 'e' ) t_print<char*,double>( format, end, cb, data.length, passnext );
			else if( format[1] == 'd' ) t_print<char*,int>( format, end, cb, data.length, passnext );
		}
	}

	return 2;
}

JNIEXPORT int printihtml( ... ) {
	return print( "%d\t", "<br>\n", passnext );
}

JNIEXPORT int printdhtml( ... ) {
	return print( "%e\t", "<br>\n", passnext );
}

JNIEXPORT int printi( ... ) {
	return print( "%d\t", "\n", passnext );
}

JNIEXPORT int printd( ... ) {
	return print( "%e\t", "\n", passnext );
}

JNIEXPORT int printl( ... ) {
	return print( "%lld\t", "\n", passnext );
}
