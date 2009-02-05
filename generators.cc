#include "simlab.h"

//#include <jni.h>
#include <stdio.h>

extern simlab data;

template<typename T> class c_buffer {
public:
	c_buffer( T* b, int l ) : buf(b), len(l) {};
	virtual T operator[](int i) { return buf[i%len]; };
	T*	buf;
	int len;
};

template<typename T,typename L> class c_inverter {
public:
	c_inverter( T & nt ) : t(nt) {}
	virtual L operator[]( int i ) { return t[(int)t[i]]; }
	T & t;
};

template<typename T,typename L> class c_transposer {
public:
	c_transposer( T & nt, int cols, int rows ) : t(nt), c(cols), r(rows) {}
	virtual L operator[]( int i ) { int ni = i%(c*r); return t[i+(ni%r)*c+ni/r-ni]; }
	T & t;
	int	c;
	int	r;
};

template<typename T,typename L> class c_flipper {
public:
	c_flipper( T & nt, int nv ) : t(nt), v(nv) {}
	virtual L operator[]( int i ) { return t[i+v-2*(i%v)-1]; }
	T & t;
	int v;
};

template<typename T,typename K,typename L> class c_shifter {
public:
	c_shifter( T & nt, K & nv, int ns ) : t(nt), v(nv), s(ns) { /*v %= s; if( v < 0 ) v = s + v;*/ }
	virtual L operator[]( int i ) { int k = i%s; return t[i-k+(k+v[0])%s]; }
	T & t;
	K & v;
	int s;
};

template<typename T,typename K,typename L> class c_adder {
public:
	c_adder( T nt, K nk ) : t(nt), k(nk) {}
	virtual L operator[]( int i ) {
		//printf("%s", "hello\n");
		return t[i]+k[i];
	}
	T t;
	K k;
};

template<typename T,typename K> class c_caster {
public:
	c_caster( K & nv ) : v(nv) {};
	virtual T operator[](int i) { return (T)v[i]; };
	K & v;
};

template <typename T, typename K> class c_funcer {
public:
	c_funcer( T (*nf)( T ), K & nv ) : f(nf), v(nv) {}
	virtual T operator[]( int i ) { return f(v[i]); }
	T (*f)( T );
	K & v;
};

template<typename T, typename K, typename L> class c_matmuler {
public:
	c_matmuler( T & nt, K & nk, int ntl, int nkl ) : t(nt), k(nk), tl(ntl), kl(nkl) {}
	virtual L operator[](int i) {
		//int retc = (kl/v);
		//int retr = (tl/val);
		//int c = i%retc;
		//int r = i/retc;
		//int rval = (i*v*v)/kl;
		int r = tl*(i/kl);
		int c = (i%kl);

		L l = 0;
		for( int n = 0; n < tl; n++ ) {
			l += (L)(t[r+n]*k[n*kl+c]);
		}
		return l;
	}
	T & t;
	K & k;
	int tl;
	int kl;
};

class c_indexer {
public:
	c_indexer() {};
	virtual int operator[]( int i ) { return i; };
};

class c_fiboer {
public:
	c_fiboer( int f = 1, int n = 1 ) : first(f), next(n), current(0) {};
	virtual long long operator[]( int i ) {
		if( i == 0 ) {
			current = 0;
			first = 1;
			next = 1;
			return 1;
		} else if( i == 1 ) {
			current = 1;
			first = 1;
			next = 1;
			return 1;
		} else if( current < i ) {
			long long ret;
			while( current < i ) {
				ret = first+next; first = next; next = ret;
				current++;
			}
			return ret;
		} else if( current > i ) {
			long long ret;
			while( current > i ) {
				ret = next-first; next = first; first = ret;
				current--;
			}
			return ret;
		} else return first+next;

		/*else if( i == current ) {
			current++;
			int ret = first+next; first = next; next = ret; return ret;
		} else {
			current = 0;
			first = 1;
			next = 1;
			int ret =
			while( current <= i ) {

			}
			return ret;
		}*/
	};
	long long first;
	long long next;
	int current;
};

extern "C" JNIEXPORT int fiboer() {
	data.buffer = (int)new c_fiboer();
	data.type = -64;
	data.length = -1;

	return 0;
}

extern "C" JNIEXPORT int indexer() {
	data.buffer = (int)new c_indexer();
	data.type = -32;
	data.length = -1;

	return 0;
}

extern "C" JNIEXPORT int buffer( simlab bff ) {
	if( bff.type != 0 ) {
		if( bff.length == 0 ) {
			if( bff.type == 32 ) {
				int* ii = new int[1];
				*ii = bff.buffer;
				data.buffer = (int)new c_buffer<int>( ii, 1 );
			} else if( bff.type == 34 ) {
				float* ff = new float[1];
				*ff = *(float*)&bff.buffer;
				data.buffer = (int)new c_buffer<float>( ff, 1 );
			}
			data.length = -1;
			data.type *= -1;
		} else {
			if( bff.type == 32 ) data.buffer = (int)new c_buffer<int>( (int*)bff.buffer, bff.length );
			else if( bff.type == 34 ) data.buffer = (int)new c_buffer<float>( (float*)bff.buffer, bff.length );
			else if( bff.type == 66 ) data.buffer = (int)new c_buffer<double>( (double*)bff.buffer, bff.length );
		}
	} else {
		if( data.length > 0 ) {
			if( data.type == 32 ) data.buffer = (int)new c_buffer<int>( (int*)data.buffer, data.length );
			else if( data.type == 34 ) data.buffer = (int)new c_buffer<float>( (float*)data.buffer, data.length );
			else if( data.type == 66 ) data.buffer = (int)new c_buffer<double>( (double*)data.buffer, data.length );
			data.length = -1;
			data.type *= -1;
		} else {
			if( data.type == 32 ) {
				int* ii = new int[1];
				*ii = data.buffer;
				data.buffer = (int)new c_buffer<int>( ii, 1 );
			} else if( data.type == 34 ) {
				float* ff = new float[1];
				*ff = *(float*)&data.buffer;
				data.buffer = (int)new c_buffer<float>( ff, 1 );
			}
			data.type *= -1;
			data.length = -1;
		}
	}

	return 0;
}

extern "C" JNIEXPORT int cnst( simlab cnst ) {
	if( cnst.type == 32 ) data.buffer = (int)new c_const<int>( cnst.buffer );
	else if( cnst.type == 34 ) data.buffer = (int)new c_const<float>( *((float*)&cnst.buffer) );
	data.length = -1;
	data.type = -cnst.type;

	return 0;
}

template<typename T> void t_caster() {
	if( data.type < 0 ) {
		if( data.type == -32 ) data.buffer = (int)new c_caster<T,c_simlab<int> >( *(c_simlab<int>*)data.buffer );
		else if( data.type == -66 ) data.buffer = (int)new c_caster<T,c_simlab<double> >( *(c_simlab<double>*)data.buffer );
	} else {
		if( data.type == 32 ) {
			int** iii = new int*[1];
			*iii = (int*)data.buffer;
			data.buffer = (int)new c_caster<T,int*>( *iii );
		} else if( data.type == 66 ) {
			double** ddd = new double*[1];
			*ddd = (double*)data.buffer;
			data.buffer = (int)new c_caster<T,double*>( *ddd );
		}
	}
}

extern "C" JNIEXPORT int caster( simlab t ) {
	if( t.buffer == 32 ) t_caster<int>();
	else if( t.buffer == 66 ) t_caster<double>();

	return 0;
}

template<typename T> void t_adder( T t, int len ) {
	if( data.type > 0 ) {
		if( data.type == 32 ) data.buffer = (int)new c_adder<T,int*,int>( t, (int*)data.buffer );
		else if( data.type == 34 ) data.buffer = (int)new c_adder<T,float*,float>( t, (float*)data.buffer );
		else if( data.type == 66 ) data.buffer = (int)new c_adder<T,double*,double>( t, (double*)data.buffer );

		data.type *= -1;
		int min = len < data.length ? len : data.length;
		int max = len > data.length ? len : data.length;
		data.length = min == -1 ? max : min;
	} else {
		if( data.type == -32 ) data.buffer = (int)new c_adder<T,c_simlab<int> &,int>( t, *(c_simlab<int>*)data.buffer );
		else if( data.type == -34 ) data.buffer = (int)new c_adder<T,c_simlab<float> &,float>( t, *(c_simlab<float>*)data.buffer );
		else if( data.type == -66 ) data.buffer = (int)new c_adder<T,c_simlab<double> &,double>( t, *(c_simlab<double>*)data.buffer );
	}
}

JNIEXPORT int adder( simlab addr ) {
	if( addr.type > 0 ) {
		if( addr.length == 0 ) {
			if( addr.type == 32 ) t_adder<int*>( (int*)&addr.buffer, 1 );
			else if( addr.type == 34 ) t_adder<float*>( (float*)&addr.buffer, 1 );
		} else {
			if( addr.type == 32 ) t_adder<int*>( (int*)addr.buffer, addr.length );
			else if( addr.type == 66 ) t_adder<double*>( (double*)addr.buffer, addr.length );
		}
	} else {
		if( addr.type == -32 ) t_adder<c_simlab<int> &>( *(c_simlab<int>*)addr.buffer, addr.length );
		else if( addr.type == -34 ) t_adder<c_simlab<float> &>( *(c_simlab<float>*)addr.buffer, addr.length );
		else if( addr.type == -66 ) t_adder<c_simlab<double> &>( *(c_simlab<double>*)addr.buffer, addr.length );
	}

	return 1;
}

extern "C" JNIEXPORT int inverter() {
	if( data.type > 0 ) {
		void** ptr = new void*[1];
		*ptr = (void*)data.buffer;
		if( data.type == 32 ) data.buffer = (int)new c_inverter<int*,int>( *(int**)ptr );
		else if( data.type == 66 ) data.buffer = (int)new c_inverter<double*,double>( *(double**)ptr );
		data.type *= -1;
	} else {
		if( data.type == -32 ) data.buffer = (int)new c_inverter<c_simlab<int>,int>( *(c_simlab<int>*)data.buffer );
		else if( data.type == -66 ) data.buffer = (int)new c_inverter<c_simlab<double>,double>( *(c_simlab<double>*)data.buffer );
	}

	return 1;
}

extern "C" JNIEXPORT int flipper( simlab val ) {
	if( data.type > 0 ) {
		void** ptr = new void*[1];
		*ptr = (void*)data.buffer;
		if( data.type == 32 ) data.buffer = (int)new c_flipper<int*,int>( *(int**)ptr, val.buffer );
		else if( data.type == 66 ) data.buffer = (int)new c_flipper<double*,double>( *(double**)ptr, val.buffer );
		data.type *= -1;
	} else {
		if( data.type == -32 ) data.buffer = (int)new c_flipper<c_simlab<int>,int>( *(c_simlab<int>*)data.buffer, val.buffer );
		else if( data.type == -66 ) data.buffer = (int)new c_flipper<c_simlab<double>,double>( *(c_simlab<double>*)data.buffer, val.buffer );
	}

	return 1;
}

extern "C" JNIEXPORT int transposer( simlab cl, simlab rl ) {
	if( data.type > 0 ) {
		void** ptr = new void*[1];
		*ptr = (void*)data.buffer;
		if( data.type == 32 ) data.buffer = (int)new c_transposer<int*,int>( *(int**)ptr, cl.buffer, rl.buffer );
		else if( data.type == 64 ) data.buffer = (int)new c_transposer<long long*,long long>( *(long long**)ptr, cl.buffer, rl.buffer );
		else if( data.type == 66 ) data.buffer = (int)new c_transposer<double*,double>( *(double**)ptr, cl.buffer, rl.buffer );

		data.type *= -1;
	} else {
		if( data.type == -32 ) data.buffer = (int)new c_transposer<c_simlab<int>,int>( *(c_simlab<int>*)data.buffer, cl.buffer, rl.buffer );
		else if( data.type == -64 ) data.buffer = (int)new c_transposer<c_simlab<long long>,long long>( *(c_simlab<long long>*)data.buffer, cl.buffer, rl.buffer );
		else if( data.type == -66 ) data.buffer = (int)new c_transposer<c_simlab<double>,double>( *(c_simlab<double>*)data.buffer, cl.buffer, rl.buffer );
	}

	return 1;
}

template <typename T> void t_shifter( T & t, int siz) {
	if( data.type > 0 ) {
		void** ptr = new void*[1];
		*ptr = (void*)data.buffer;
		if( data.type == 32 ) data.buffer = (int)new c_shifter<int*,T,int>( *(int**)ptr, t, siz );
		else if( data.type == 64 ) data.buffer = (int)new c_shifter<long long*,T,long long>( *(long long**)ptr, t, siz );
		else if( data.type == 66 ) data.buffer = (int)new c_shifter<double*,T,double>( *(double**)ptr, t, siz );
		data.type *= -1;
	} else {
		if( data.type == -32 ) data.buffer = (int)new c_shifter<c_simlab<int>,T,int>( *(c_simlab<int>*)data.buffer, t, siz );
		else if( data.type == -64 ) data.buffer = (int)new c_shifter<c_simlab<long long>,T,long long>( *(c_simlab<long long>*)data.buffer, t, siz );
		else if( data.type == -66 ) data.buffer = (int)new c_shifter<c_simlab<double>,T,double>( *(c_simlab<double>*)data.buffer, t, siz );
	}
}

extern "C" JNIEXPORT int shifter( simlab val, simlab siz ) {
	if( val.type > 0 ) {
		void** ptr = new void*[1];
		*ptr = (void*)val.buffer;
		if( val.type == 32 ) t_shifter<int*>( *(int**)val.buffer, siz.buffer );
		else if( val.type == 64 )t_shifter<long long*>( *(long long**)val.buffer, siz.buffer );
		//val.type *= -1;
	} else {
		if( val.type == -32 ) t_shifter<c_simlab<int> >( *(c_simlab<int>*)val.buffer, siz.buffer );
		else if( val.type == -64 ) t_shifter<c_simlab<long long> >( *(c_simlab<long long>*)val.buffer, siz.buffer );
	}

	return 1;
}

template<typename T> void t_matmuler( T & t, int l, int v ) {
	if( data.type > 0 ) {
		void** ptr = new void*[1];
		*ptr = (void*)data.buffer;
		if( data.type == 32 ) data.buffer = (int)new c_matmuler<int*,T,int>( *(int**)ptr, t, l, v );
		else if( data.type == 64 ) data.buffer = (int)new c_matmuler<long long*,T,long long>( *(long long**)ptr, t, l, v );
		else if( data.type == 66 ) data.buffer = (int)new c_matmuler<double*,T,double>( *(double**)ptr, t, l, v );
		data.type *= -1;
	} else {
		if( data.type == -32 ) data.buffer = (int)new c_matmuler<c_simlab<int>,T,int>( *(c_simlab<int>*)data.buffer, t, l, v );
		else if( data.type == -64 ) data.buffer = (int)new c_matmuler<c_simlab<long long>,T,long long>( *(c_simlab<long long>*)data.buffer, t, l, v );
		else if( data.type == -66 ) data.buffer = (int)new c_matmuler<c_simlab<double>,T,double>( *(c_simlab<double>*)data.buffer, t, l, v );
	}
}

extern "C" JNIEXPORT int matmuler( simlab mat, simlab c, simlab r ) {
	if( mat.type > 0 ) {
		//void** ptr = new void*[1];
		//*ptr = (void*)val.buffer;
		if( mat.type == 32 ) t_matmuler<int*>( *(int**)mat.buffer, c.buffer, r.buffer );
		else if( mat.type == 64 )t_matmuler<long long*>( *(long long**)mat.buffer, c.buffer, r.buffer );
		else if( mat.type == 66 )t_matmuler<double*>( *(double**)mat.buffer, c.buffer, r.buffer );
		//val.type *= -1;
	} else {
		if( mat.type == -32 ) t_matmuler<c_simlab<int> >( *(c_simlab<int>*)mat.buffer, c.buffer, r.buffer );
		else if( mat.type == -64 ) t_matmuler<c_simlab<long long> >( *(c_simlab<long long>*)mat.buffer, c.buffer, r.buffer );
		else if( mat.type == -66 ) t_matmuler<c_simlab<double> >( *(c_simlab<double>*)mat.buffer, c.buffer, r.buffer );
	}

	return 1;
}

extern "C" JNIEXPORT int dfuncer( simlab dfunc ) {
	if( data.type > 0 ) {
		void** ptr = new void*[1];
		*ptr = (void*)data.buffer;
		if( data.type == 32 ) data.buffer = (int)new c_funcer<double,int*>( (double (*)(double))dfunc.buffer, *(int**)ptr );
		else if( data.type == 64 ) data.buffer = (int)new c_funcer<double,long long*>( (double (*)(double))dfunc.buffer, *(long long**)ptr );
		else if( data.type == 66 ) data.buffer = (int)new c_funcer<double,double*>( (double (*)(double))dfunc.buffer, *(double**)ptr );
	} else {
		if( data.type == -32 ) data.buffer = (int)new c_funcer<double,c_simlab<int> >( (double (*)(double))dfunc.buffer, *(c_simlab<int>*)data.buffer );
		else if( data.type == -64 ) data.buffer = (int)new c_funcer<double,c_simlab<long long> >( (double (*)(double))dfunc.buffer, *(c_simlab<long long>*)data.buffer );
		else if( data.type == -66 ) data.buffer = (int)new c_funcer<double,c_simlab<double> >( (double (*)(double))dfunc.buffer, *(c_simlab<double>*)data.buffer );
	}
	data.type = -66;
}
