/*
trans
sort
flip
shift

move
swap

add
sub
mul
div

sine
cosine
tan
atan
asin
acos

mod

idx
diff
integ

min
max
median
mean
sum

prim
fibo
rand

wlet
fft

sort -- with pseudobuffer (Dual)
sortidx
get
*/

#include "simlab.h"

#ifdef GL
#include <GL/gl.h>
#include <GL/glu.h>
//#include <SDL/SDL.h>
#endif
#include <string.h>
#include <cstdio>
#include <cstdarg>
#include <cmath>
#include <map>
#include <vector>
#include <set>
#include <string>
#include <algorithm>
#include <typeinfo>
#ifdef ALSA
#include <alsa/asoundlib.h>
#endif
//#include <occi.h>
//#include <dsound.h>
//#include <dxguid.h>
//#include <d3d9.h>
//#include <process.h>
//#include <d3dx9math.h>
#ifdef JAVA
#include <jni.h>
#endif

#ifdef JPG
#include <jpeglib.h>
#endif
//#include "fftw3.h"

#ifdef HILDON
#include <hildon-widgets/hildon-program.h>
#include <hildon-widgets/hildon-window.h>
#endif

#ifdef GTK
#include <gtk/gtk.h>
#ifdef GL
#include <gtk/gtkgl.h>
#endif

//#include <gtk/gtkwidget.h>
//#include <gtk/gtkmain.h>
//#include <gtk/gtkbutton.h>
#endif

//#ifndef max
#define  mx(a, b)  ((a) > (b) ? (a) :  (b))
//#endif
//#ifndef min
#define  mn(a, b)  ((a) < (b) ? (a) :  (b))
//#endif

#ifdef JAVA
//int java = 0;
jobject 	jobj;
jclass 		jcls;
jclass 		jsimlab;
JavaVM 		*jvm = NULL;
JNIEnv		*javaenv = NULL;
#endif

#ifdef WIN
HINSTANCE hinstance;
#endif

/*struct simlab {
	int		buffer;
	int		length;
	int		type;
	//int		dummy;
	//virtual ~simlab() {};
	//inline virtual int operator[]( int i ) { return length; };
};*/

template<class T> class t_simlab : public simlab {
public:
	//int		buffer;
	//int		type;
	virtual	~t_simlab() {};
	inline virtual int operator[]( int i ) { return (int)&((T*)buffer)[i]; };
};

/*typedef struct simlab {
	int buffer;
	int type;
	int length;

	inline virtual int operator[]( int i );
};*/

extern int												module;
FILE*													sout = stdout;
int														current;
int														previous;
simlab													adata;
simlab													data;
simlab													prev;
std::map<std::string,simlab>							retlib;

#define SWAPBUFFERS SwapBuffers(ghDC)
#define BLACK_INDEX     0
#define RED_INDEX       13
#define GREEN_INDEX     14
#define BLUE_INDEX      16
#define WIDTH           300
#define HEIGHT          200

#ifdef GL
GLfloat latitude, longitude, latinc, longinc;
GLdouble radius;
#endif

#define GLOBE    1
#define CYLINDER 2
#define CONE     3

/*struct args {
	int i0;
	int i1;
	int i2;
	int i3;
	int i4;
	int i5;
	int i6;
	int i7;
	int i8;
	int i9;
	int i10;
	int i11;
};

args passnext;
args passargs;*/

#ifdef JAVA
JavaVM 			*jvm = NULL;
JNIEnv			*javaenv = NULL;
#endif

extern "C" JNIEXPORT int fft();
extern "C" JNIEXPORT int vector( int type, int length );
extern "C" JNIEXPORT int garbage();
extern "C" JNIEXPORT int zero();
extern "C" JNIEXPORT int store( simlab name );
extern "C" JNIEXPORT int fetch( simlab name );
extern "C" JNIEXPORT int echo( simlab str, ... );
extern "C" JNIEXPORT int initjava();
extern "C" JNIEXPORT int cmd( simlab );
extern "C" JNIEXPORT int shift( simlab, simlab );
extern "C" JNIEXPORT int add( simlab );

//c_idx<int>	int_idx;

void* threadrunner(void*);

passa<4> 	passnext;
passa<4>	*ppassa4 = &passnext;
passb<4>	*ppassb4 = (passb<4>*)ppassa4;
int 		passcurr;
//passa<11>	ppass11[] = {&pass12.big;

void commandLoop( int file );

/*int jprintf( const char* format, ... ) {
	jclass clz = javaenv->FindClass( "Ljava/lang/System;" );
	jclass oclz = javaenv->FindClass( "Ljava/io/PrintStream;" );
	jfieldID fld = javaenv->GetStaticFieldID( clz, "out", "Ljava/io/PrintStream;" );
	jobject obj = javaenv->GetStaticObjectField( clz, fld );
	jmethodID mth = javaenv->GetMethodID( oclz, "printf", "(Ljava/lang/String;[Ljava/lang/Object;)Ljava/io/PrintStream;" );
	if( mth == NULL ) printf( "hello\n" );
	else {
		jstring jstr = javaenv->NewStringUTF(format);
		javaenv->CallVoidMethod( obj, mth, jstr );
		javaenv->ReleaseStringUTFChars( jstr, format );
	}

	return 0;
}*/

extern int (*prnt)( const char*, ... );
#ifdef JAVA
char	cc[100];
int jprintf( const char* str, ... ) {
	va_list l;
	va_start(l, str);
	passa<10> rest = va_arg(l, passa<10>);
	va_end(l);
	sprintf( cc, str, rest );
	//printf( cc );

	jclass clz = javaenv->FindClass( "Ljava/lang/System;" );
	jclass oclz = javaenv->FindClass( "Ljava/io/PrintStream;" );
	jfieldID fld = javaenv->GetStaticFieldID( clz, "out", "Ljava/io/PrintStream;" );
	jobject obj = javaenv->GetStaticObjectField( clz, fld );
	jmethodID mth = javaenv->GetMethodID( oclz, "print", "(Ljava/lang/String;)V" );
	jstring jstr = javaenv->NewStringUTF(cc);
	javaenv->CallVoidMethod( obj, mth, jstr );
	//javaenv->ReleaseStringUTFChars( jstr, cc );

	return 0;
}

int callJavaMethod( jclass cls, std::string sig, char* name ) {
	jclass clscls = javaenv->GetObjectClass( cls );

	jmethodID mid = javaenv->GetMethodID( clscls, "getMethods", "()[Ljava/lang/reflect/Method;" );
	jobject mts = javaenv->CallObjectMethod( cls, mid );

	jclass mtdcls = javaenv->FindClass("Ljava/lang/reflect/Method;");
	jclass tpcls = javaenv->FindClass("Ljava/lang/reflect/Type;");
	jmethodID mtdId = javaenv->GetMethodID( mtdcls, "getName", "()Ljava/lang/String;" );
	jmethodID tpId = javaenv->GetMethodID( mtdcls, "getGenericReturnType", "()Ljava/lang/reflect/Type;" );
	jmethodID strId = javaenv->GetMethodID( tpcls, "toString", "()Ljava/lang/String;" );

	int len = javaenv->GetArrayLength( (jarray)mts );
	for( int i = 0; i < len; i++ ) {
		jobject mtd = javaenv->GetObjectArrayElement( (jobjectArray)mts, i );
		jobject mtdName = javaenv->CallObjectMethod( mtd, mtdId );
		const char* c = javaenv->GetStringUTFChars( (jstring)mtdName, NULL );

		if( strcmp( c, name ) == 0 ) {
			jobject tp = javaenv->CallObjectMethod( mtd, tpId );
			jobject tpName = javaenv->CallObjectMethod( tp, strId );
			const char* cc = javaenv->GetStringUTFChars( (jstring)tpName, NULL );

			if( cc[0]=='c' && cc[1]=='l' && cc[2]=='a' ) {
				const char* c_c = cc+6;
				std::string tsig = sig + "L" + c_c + ";";
				for( unsigned int k = 0; k < tsig.length(); k++ ) {
					if( tsig[k] == '.' ) tsig[k] = '/';
				}
				mid = (jobj == NULL) ? javaenv->GetStaticMethodID( cls, name, tsig.c_str() ) : javaenv->GetMethodID( cls, name, tsig.c_str() );
				if( mid != NULL ) {
					jobject tretobj = ( (jobj == NULL) ? javaenv->CallStaticObjectMethod( cls, mid, passnext ) : javaenv->CallObjectMethod( (jobject)jobj, mid, passnext ) );
					jobject retobj = javaenv->NewGlobalRef( tretobj );
					javaenv->DeleteGlobalRef( tretobj );
					return (int)retobj;
				}
			} else if( cc[0]=='v' && cc[1]=='o' && cc[2]=='i' ) {
				std::string tsig = sig+"V";
				if( jobj == NULL ) {
					mid = javaenv->GetStaticMethodID( cls, name, tsig.c_str() );
					if( mid != NULL ) {
						javaenv->CallStaticVoidMethod( cls, mid, passnext );
						return (int)jobj;
					}
				} else {
					mid = javaenv->GetMethodID( cls, name, tsig.c_str() );
					if( mid != NULL ) {
						javaenv->CallVoidMethod( (jobject)jobj, mid, passnext );
						return (int)jobj;
					}
				}
			} else if( cc[0]=='i' && cc[1]=='n' && cc[2]=='t' ) {
				std::string tsig = sig+"I";
				mid = javaenv->GetMethodID( cls, name, tsig.c_str() );
				if( mid != NULL ) {
					return javaenv->CallIntMethod( (jobject)jobj, mid, passnext );
				}
			}
		}
	}
	if( cls != jsimlab ) {
		//jsimlab = javaenv->FindClass("Lsimple/Simlab;");
		jobj = jsimlab;
		return callJavaMethod( jsimlab, sig, name );
	}
	printf("nd\n");
	return (int)jobj;
}

int numpar = 0;
int parseJavaParameters( char* name, std::string &sig, int bytesize ) {
	char *result = strtok( NULL, " ,_)\n" );

	printf("erm %d %d\n", (int)javaenv, (int)jcls );
	/*jobject obj = jobj;
	printf("gaga %d\n", (int)jcls);
	jclass mycls = javaenv->FindClass("Lsimple/Simlab;");
	if( mycls != NULL ) {
		printf("erm\n");
		jmethodID mid = javaenv->GetStaticMethodID( mycls, "command", "(Ljava/lang/String;)V" );
		if( mid != NULL ) {
			printf("ermerm\n");
			jstring str = javaenv->NewStringUTF( "ho" );
			javaenv->CallStaticVoidMethod( mycls, mid, str );
		}
		jfieldID fld = javaenv->GetStaticFieldID( mycls, "cls", "Ljava/lang/Class;" );
		jcls = (jclass)javaenv->GetStaticObjectField( mycls, fld );
	}*/

	jclass clscls = javaenv->GetObjectClass( jcls );

	printf("erm\n");
	if( clscls == 0 ) return 0;
	printf("erm\n");
	if( result == NULL ) {
		char* here = (char*)&passnext;
		int value = 0;
		memcpy( here+bytesize, &value, sizeof(int) );
		if( jcls != NULL ) {
			sig += ")";

			if( strcmp( name, "<init>" ) == 0 ) {
				printf("heryhey\n");
				sig += "V";
				jmethodID mid = javaenv->GetMethodID( jcls, name, sig.c_str() );
				if( mid != NULL ) {
					javaenv->CallVoidMethod( jobj, mid, passnext );
					return (int)jobj;
				}
			} else {
				return callJavaMethod( jcls, sig, name );
			}
			return (int)jobj;

			/*jmethodID mid = javaenv->GetMethodID( cls, name, sig.c_str() );
			jobject mtd = javaenv->ToReflectedMethod( cls, mid, false );
			jclass  mtdcls = javaenv->GetObjectClass( mtd );
			jmethodID tmtd = javaenv->GetMethodID( mtdcls, "getReturnType", "()Ljava/lang/reflect/Type;" );

			jobject tp = javaenv->CallObjectMethod( mtd, tmtd );
			jclass tpcls = javaenv->GetObjectClass( tp );
			jmethodID tpmtd = javaenv->GetMethodID( tpcls, "toString", "()Ljava/lang/String;" );
			jobject retTpName = javaenv->CallObjectMethod( tp, tpmtd );

			const char* cc = javaenv->GetStringUTFChars( (jstring)retTpName, NULL );*/
		}
		return (int)jobj;
	} else {
		if( result[0] == '"' /*|| result[0] == '#'*/ ) {
			result[ strlen(result)-1 ] = 0;
			char* there = result+1;
			char* here = (char*)&passnext;
			jstring str = javaenv->NewStringUTF( there );
			int size = sizeof( jstring );
			memcpy( here+bytesize, &str, size );
			sig += "Ljava/lang/String;";
			return parseJavaParameters( name, sig, bytesize+size );
		} else if( result[0] == '-' ) {
			int value = result[1] - '0';
			int i = 2;
			while( result[i] != 0 && result[i] != '.' ) {
				value *= 10;
				value += result[i] - '0';
				i++;
			}
			if( result[i] == '.' ) {
				double dvalue = (double)value;
				double mnt = 1.0;
				int k = i+1;
				while( result[k] != 0 ) {
					mnt *= 10.0;
					dvalue += (result[k] - '0')/mnt;
					k++;
				}
				jdouble jdvalue = -dvalue;
				char* here = (char*)&passnext;
				memcpy( here+bytesize, &jdvalue, sizeof(jdouble) );
				sig += "D";
				return parseJavaParameters( name, sig, bytesize+sizeof(jdouble) );
			} else  {
				jint val = -value;
				char* here = (char*)&passnext;
				int size = sizeof( jint );
				memcpy( here+bytesize, &val, size );
				sig += "I";
				return parseJavaParameters( name, sig, bytesize+size );
			}
		} else if( result[0] >= '0' && result[0] <= '9' ) {
			int value = result[0] - '0';
			//printf( "uno %d\n", value );
			int i = 1;
			while( result[i] != 0 && result[i] != '.' ) {
				value *= 10;
				value += result[i] - '0';
				//printf( "neo %d\n", value );
				i++;
			}
			if( result[i] == '.' ) {
				double dvalue = (double)value;
				double mnt = 1.0;
				int k = i+1;
				while( result[k] != 0 ) {
					mnt *= 10.0;
					dvalue += (result[k] - '0')/mnt;
					k++;
				}
				jdouble jdvalue = dvalue;
				char* here = (char*)&passnext;
				memcpy( here+bytesize, &jdvalue, sizeof(jdouble) );
				sig += "D";
				return parseJavaParameters( name, sig, bytesize+sizeof(jdouble) );
			} else  {
				jint val = value;

				//printf( "%d\n", val );
				char* here = (char*)&passnext;
				int size = sizeof( jint );
				memcpy( here+bytesize, &val, size );
				sig += "I";
				return parseJavaParameters( name, sig, bytesize+size );
			}
		} else {
			simlab ftch;
			ftch.type = 8;
			ftch.length = strlen( result );
			ftch.buffer = (int)result;
			int cur = fetch( ftch );
			if( cur != 0 ) {
				/*jobject tobj = (jobject)cur;
				jclass pcls = javaenv->GetObjectClass( tobj );
				jclass pclscls = javaenv->GetObjectClass( cls );

				jmethodID mid = javaenv->GetMethodID( pclscls, "getName", "()Ljava/lang/String;" );
				if( mid == NULL ) {
					//jmethodID clsmid = javaenv->GetMethodID( cls, "getClass", "()Ljava/lang/Class;" );
					//jobject clsobj = javaenv->CallObjectMethod( tobj, mid );
					//jclass clscls = javaenv->GetObjectClass( clsobj );

					pclscls = javaenv->FindClass( "Ljava/lang/Class;" );
					if( pclscls != NULL ) {
						mid = javaenv->GetMethodID( pclscls, "getName", "()Ljava/lang/String;" );
					}
				}*/

				jmethodID mid = javaenv->GetMethodID( clscls, "getMethods", "()[Ljava/lang/reflect/Method;" );
				jobject mts = javaenv->CallObjectMethod( jcls, mid );

				jclass mtdcls = javaenv->FindClass("Ljava/lang/reflect/Method;");
				jclass tpcls = javaenv->FindClass("Ljava/lang/reflect/Type;");
				jmethodID mtdId = javaenv->GetMethodID( mtdcls, "getName", "()Ljava/lang/String;" );
				jmethodID tpId = javaenv->GetMethodID( mtdcls, "getGenericParameterTypes", "()[Ljava/lang/reflect/Type;" );
				jmethodID strId = javaenv->GetMethodID( tpcls, "toString", "()Ljava/lang/String;" );

				for( int i = 0; i < javaenv->GetArrayLength( (jarray)mts ); i++ ) {
					jobject mtd = javaenv->GetObjectArrayElement( (jobjectArray)mts, i );
					jobject mtdName = javaenv->CallObjectMethod( mtd, mtdId );
					const char* c = javaenv->GetStringUTFChars( (jstring)mtdName, NULL );

					if( strcmp( c, name ) == 0 ) {
						jobject tps = javaenv->CallObjectMethod( mtd, tpId );
						int tlen = javaenv->GetArrayLength( (jarray)tps );
						if( numpar < tlen ) {
						//for( int k = 0; k < javaenv->GetArrayLength( (jarray)tps ); k++ ) {
							jobject tp = javaenv->GetObjectArrayElement( (jobjectArray)tps, numpar );
							jobject tpName = javaenv->CallObjectMethod( tp, strId );
							const char* cc = javaenv->GetStringUTFChars( (jstring)tpName, NULL );
							if( cc[0] == 'c' && cc[1] == 'l' && cc[2] == 'a' ) {
								const char* nm = cc+6;
								sig += "L";
								sig += nm;
								sig += ";";
								for( unsigned int l = 0; l < sig.length(); l++ ) {
									if( sig[l] == '.' ) sig[l] = '/';
								}
								break;
							}
						}
					}
				}

				char* here = (char*)&passnext;
				memcpy( here+bytesize, &cur, sizeof(jobject) );
				return parseJavaParameters( name, sig, bytesize+sizeof(jobject) );
			} else {
				jboolean val = 0;
				if( result[0] == 't' && result[1] == 'r' && result[2] == 'u' ) {
					val = 1;
				}
				char* here = (char*)&passnext;
				int size = sizeof( jboolean );
				memcpy( here+bytesize, &val, size );
				sig += "Z";
				return parseJavaParameters( name, sig, bytesize+size );
			}
		}

		numpar++;

		/*byte* bp = (byte*)&passargs;

		int i = 0;
		while( signature[i] != ')' ) {
		if( signature[i] == 'I' ) {
			jint val = va_arg( args, int );
			int size = sizeof( jint );
			memcpy( bp, &val, size );
			bp += size;
		} else if( signature[i] == 'D' ) {
			jdouble val = va_arg( args, double );
			int size = sizeof( jdouble );
			memcpy( bp, &val, size );
			bp += size;
		} else if( signature[i] == 'Z' ) {
			jboolean bl = va_arg( args, unsigned char );
			int size = sizeof( jboolean );
			memcpy( bp, &bl, size );
			bp += size;
		} else if( signature[i] == 'S' ) {
			//int k = i;
			char* cp = va_arg( args, char* );
			jstring str = javaenv->NewStringUTF( cp );
			int size = sizeof( jstring );
			memcpy( bp, &str, size );
			bp += size;
		} else if( signature[i] == 'L' ) {
			//int k = i;
			while( signature[i] != ';' ) i++;
			jobject obj = (jobject)va_arg( args, int );
			//jstring str = javaenv->NewStringUTF( cp );
			int size = sizeof( jobject );
			memcpy( bp, &obj, size );
			bp += size;
		}
		i++;*/
	}
	return current;
}
#endif

void erm( int zero, ... ) {
	va_list args;
	va_start( args, zero );

	va_end( args );
}

/*GLvoid createObjects()
{
    GLUquadricObj *quadObj;

    glNewList(GLOBE, GL_COMPILE);
        quadObj = gluNewQuadric ();
        gluQuadricDrawStyle (quadObj, GLU_LINE);
        gluSphere (quadObj, 1.5, 16, 16);
    glEndList();

    glNewList(CONE, GL_COMPILE);
        quadObj = gluNewQuadric ();
        gluQuadricDrawStyle (quadObj, GLU_FILL);
        gluQuadricNormals (quadObj, GLU_SMOOTH);
        gluCylinder(quadObj, 0.3, 0.0, 0.6, 15, 10);
    glEndList();

    glNewList(CYLINDER, GL_COMPILE);
        glPushMatrix ();
        glRotatef ((GLfloat)90.0, (GLfloat)1.0, (GLfloat)0.0, (GLfloat)0.0);
        glTranslatef ((GLfloat)0.0, (GLfloat)0.0, (GLfloat)-1.0);
        quadObj = gluNewQuadric ();
        gluQuadricDrawStyle (quadObj, GLU_FILL);
        gluQuadricNormals (quadObj, GLU_SMOOTH);
        gluCylinder (quadObj, 0.3, 0.3, 0.6, 12, 2);
        glPopMatrix ();
    glEndList();
}

GLvoid initializeGL(GLsizei width, GLsizei height) {
    GLfloat     maxObjectSize, aspect;
    GLdouble    near_plane, far_plane;

    glClearIndex( (GLfloat)BLACK_INDEX);
    glClearDepth( 1.0 );

    glEnable(GL_DEPTH_TEST);

    glMatrixMode( GL_PROJECTION );
    aspect = (GLfloat) width / height;
    gluPerspective( 45.0, aspect, 3.0, 7.0 );
    glMatrixMode( GL_MODELVIEW );

    near_plane = 3.0;
    far_plane = 7.0;
    maxObjectSize = 3.0F;
    radius = near_plane + maxObjectSize/2.0;

    latitude = 0.0F;
    longitude = 0.0F;
    latinc = 6.0F;
    longinc = 2.5F;

    createObjects();
}

GLvoid resize( GLsizei width, GLsizei height ) {
    GLfloat aspect;

    glViewport( 0, 0, width, height );

    aspect = (GLfloat) width / height;

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective( 45.0, aspect, 3.0, 7.0 );
    glMatrixMode( GL_MODELVIEW );
}

void polarView(GLdouble radius, GLdouble twist, GLdouble latitude, GLdouble longitude) {
    glTranslated(0.0, 0.0, -radius);
    glRotated(-twist, 0.0, 0.0, 1.0);
    glRotated(-latitude, 1.0, 0.0, 0.0);
    glRotated(longitude, 0.0, 0.0, 1.0);
}

GLvoid drawScene(GLvoid) {
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glPushMatrix();

        latitude += latinc;
        longitude += longinc;

        polarView( radius, 0, latitude, longitude );

        glIndexi(RED_INDEX);
        glCallList(CONE);

        glIndexi(BLUE_INDEX);
        glCallList(GLOBE);

		glIndexi(GREEN_INDEX);
		glPushMatrix();
            glTranslatef(0.8F, -0.65F, 0.0F);
            glRotatef(30.0F, 1.0F, 0.5F, 1.0F);
            glCallList(CYLINDER);
        glPopMatrix();

    glPopMatrix();

    SWAPBUFFERS;
}

struct CUSTOMVERTEX {
    FLOAT x, y, z;
	DWORD diffuse;
};

LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) {
    switch( msg ) {
        case WM_DESTROY:
			if( m_d3ddevice ) m_d3ddevice->Release();
			if( m_d3d ) m_d3d->Release();
            ShowWindow( hWnd, SW_HIDE );
			//PostQuitMessage( 0 );
            return 0;

        case WM_PAINT:
            if( m_d3d ) Render();
            ValidateRect( hWnd, NULL );
            return 0;

		case WM_ERASEBKGND:
			return 0;
    }

    return DefWindowProc( hWnd, msg, wParam, lParam );
}

LRESULT CALLBACK MainWndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) {
	HGLRC			ghRC;
	RECT			rect;
	PAINTSTRUCT		ps;

	switch (uMsg) {
        case WM_CREATE:
			ghDC = GetDC(hwnd);
			if (!bSetupPixelFormat(ghDC))
				PostQuitMessage (0);

			ghRC = wglCreateContext(ghDC);
			wglMakeCurrent(ghDC, ghRC);
			GetClientRect(hwnd, &rect);
			initializeGL(rect.right, rect.bottom);

            return 0;

		case WM_PAINT:
			ghDC = GetDC(hwnd);
			BeginPaint(hwnd, &ps);
			EndPaint(hwnd, &ps);
			break;

		case WM_SIZE:
			GetClientRect(hwnd, &rect);
			resize(rect.right, rect.bottom);
			break;

		case WM_CLOSE:
			if (ghRC)
			    wglDeleteContext(ghRC);
			if (ghDC)
				ReleaseDC(hwnd, ghDC);
			ghRC = 0;
			ghDC = 0;

			DestroyWindow(hwnd);
			break;

        case WM_DESTROY:
			if (ghRC)
			    wglDeleteContext(ghRC);
			if (ghDC)
			    ReleaseDC(hwnd, ghDC);

			PostQuitMessage (0);
			break;

		case WM_KEYDOWN:
			switch (wParam) {
				case VK_LEFT:
					longinc += 0.5F;
					break;
				case VK_RIGHT:
					longinc -= 0.5F;
					break;
				case VK_UP:
					latinc += 0.5F;
					break;
				case VK_DOWN:
					latinc -= 0.5F;
					break;
			}


        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}*/

template <typename T> class PseudoBuffer {
public:
	virtual T inline operator[]( int ind ) { return (T)-1; };
	virtual ~PseudoBuffer() {};
	virtual int length() { return 0; };
};

template <typename T> int t_mod( simlab* data, int value ) {
	T*	buffer = (T*)data->buffer;
	for( int i = 0; i < data->length; i++ ) {
		buffer[i] = (int)buffer[i]%value;
	}
	return current;
}

template <typename T, typename K> void t_poly( T* buffer, int length, K* pl, int plen ) {
	for( int i = 0; i < length; i++ ) {
		T old = buffer[i];
		buffer[i] = 0;
		for( int k = 0; k < plen; k++ ) {
			buffer[i] += (T)(pl[k]*pow( (double)old,(double)k ));
		}
	}
}

template <typename T> void t_sqr( T* buffer, int length ) {
	for( int i = 0; i < length; i++ ) {
		buffer[i] *= buffer[i];
	}
}

template <typename T, typename K> void t_exp( T* buffer, int length, K* value, int vallen ) {
	if( vallen == 0 ) {
		T val = (T)*((K*)&value);
		for( int i = 0; i < length; i++ ) {
			buffer[i] = (T)pow( (double)val, (double)buffer[i] );
		}
	} else {
		for( int i = 0; i < length; i++ ) {
			buffer[i] = (T)pow( (double)value[i%vallen], (double)buffer[i] );
		}
	}
}

template <typename T, typename K> void t_pow( T* buffer, int length, K* value, int vallen ) {
	if( vallen == 0 ) {
		T val = (T)*((K*)&value);
		for( int i = 0; i < length; i++ ) {
			buffer[i] = (T)pow( (double)buffer[i], (double)val );
		}
	} else {
		for( int i = 0; i < length; i++ ) {
			buffer[i] = (T)pow( (double)buffer[i], (double)value[i%vallen] );
		}
	}
}

template <typename T, typename K> void t_ffunc( T* buffer, int length, float (*ffunc)( float ), K* ibuffer, int ilength ) {
	if( ibuffer == NULL ) {
		for( int i = 0; i < length; i++ ) {
			buffer[i] = (T)ffunc( (float)buffer[i] );
		}
	} else {
		for( int i = 0; i < ilength; i++ ) {
			if( ibuffer[i] >= 0 && ibuffer[i] < length ) buffer[ (unsigned int)ibuffer[i] ] = (T)ffunc( (float)buffer[ (unsigned int)ibuffer[i] ] );
		}
	}
}

template <typename T, typename K> void t_dfunc( T* buffer, int length, double (*dfunc)( double ), K* ibuffer, int ilength ) {
	if( ibuffer == NULL ) {
		for( int i = 0; i < length; i++ ) {
			buffer[i] = (T)dfunc( (double)buffer[i] );
		}
	} else {
		for( int i = 0; i < ilength; i++ ) {
			if( ibuffer[i] >= 0 && ibuffer[i] < length ) buffer[ (unsigned int)ibuffer[i] ] = (T)dfunc( (double)buffer[ (unsigned int)ibuffer[i] ] );
		}
	}
}

template <typename K> void t_dfunc( short* buffer, int length, double (*dfunc)(double), K* ibuffer, int ilength ) {
	if( ibuffer == NULL ) {
		for( int i = 0; i < length; i++ ) {
			buffer[i] = (short)(dfunc( (double)buffer[i] )*32767.0);
		}
	} else {
		for( int i = 0; i < ilength; i++ ) {
			if( ibuffer[i] >= 0 && ibuffer[i] < length ) buffer[ (unsigned int)ibuffer[i] ] = (short)(dfunc( (double)buffer[ (unsigned int)ibuffer[i] ] )*32767.0);
		}
	}
}

template <typename T> int t_bessel( simlab* data ) {
	T*	buffer = (T*)data->buffer;
	for( int i = 0; i < data->length; i++ ) {
		buffer[i] = y0( buffer[i] );
	}
	return current;
}

template <typename T, typename K, typename Q> void t_mul( T* buffer, int length, K* vbuffer, int vlength, Q* ibuffer, int ilength ) {
	if( ibuffer == NULL ) {
		//printf("%d %d\n",(int)*vbuffer,(int)*buffer);
		for( int i = 0; i < length; i++ ) {
			buffer[i] *= (T)vbuffer[i%vlength];
		}
	} else {
		for( int k = 0; k < ilength; k++ ) {
			if( ibuffer[k] >= 0 && ibuffer[k] < length ) buffer[ (int)ibuffer[k] ] *= (T)vbuffer[k%vlength];
		}
	}

	/*if( ibuffer == NULL ) {
			K val = *((K*)&value);
			for( int i = 0; i < length; i++ ) {
				buffer[i] *= (T)val;
			}
		} else {
			for( int i = 0; i < length; i++ ) {
				buffer[i] *= (T)value[i%vallen];
			}
		}
	} else {

	}*/
}

template <typename T> void t_invert( T* buffer, int length, int chunk ) {
	T* maxBuffer = NULL;//t_max( buffer, length, chunk );
	T* minBuffer = NULL;//t_min( buffer, length, chunk );
	for( int i = 0; i < length; i+=chunk ) {
		int u = i/chunk;
		T max = maxBuffer[u];
		T min = minBuffer[u];
		for( int k = i; k < i+chunk; k++ ) {
			buffer[k] = -buffer[k]+max+min;
		}
	}
}

template <typename T> void t_fibo( T* buffer, int length ) {
	if( length > 0 ) buffer[0] = 1;
	if( length > 1 ) buffer[1] = 1;
	for( int i = 2; i < length; i++ ) {
		buffer[i] = buffer[i-1]+buffer[i-2];
	}
}

int _gcd( int a, int b ) {
	if( b == 0 ) return a;
	else return _gcd( b, a%b );
}

template <typename T, typename K> void t_gcd( T* buffer, int length, K* buffer2, int length2 ) {
	for( int t = 0; t < length; t+=length2 ) {
		for( int k = t; k < mn(length,t+length2); k++ ) {
			buffer[k] = (T)_gcd( (int)buffer[k], (int)buffer2[k-t] );
		}
	}
}

template <typename T> void t_factor( T* buffer, int length ) {
	std::vector<int>	res;

	T val = buffer[0];

	//fprintf( stderr, "heyhey %d %d\n", (int)val, sizeof(T) );

	for( int i = 2; i < (int)(sqrt( (double)val )+1); i++ ) {
		while( (int)val%i == 0 && val > 1 ) {
			res.push_back( i );
			val /= i;
		}
		if( val == 1 ) break;
	}
	if( val != 1 ) res.push_back( val );

	//fprintf( stderr, "heyhey %d\n", res.size() );

	//create( 32, res.size(), 0 );
	int*	buff = (int*)data.buffer;
	for( int i = 0; i < res.size(); i++ ) {
		buff[i] = res[i];
	}
}

template <typename T> void t_prim( T* buffer, int length ) {
	if( length > 0 ) buffer[0] = 1;
	if( length > 1 ) buffer[1] = 2;
	if( length > 2 ) buffer[2] = 3;
	//if( length > 3 ) buffer[3] = 5;
	int p = 3;
	for( int i = 3; i < length; i++ ) {
		int k = 2;
		int l = (int)buffer[k];
		int sp = l;
		while( l <= sp ) {
			p += 2;
			sp = (int)sqrt( (float)p );
			while( l <= sp && p%l ) {
				l = (int)buffer[++k];
			}
		}
		buffer[i] = p;
		//p += 2;
	}
}

inline int tran( int i, int c, int r ) {
	int nr = i%c;
	int nc = i/c;
	return nr*r+nc;
}

template <typename T> void t_permute( T* buffer, int length, int c, int l ) {
	long m = length-1;
	T	to = buffer[l];
	long k = (l*c)%m;
	T	ti = buffer[k];

	while( k != l ) {
		buffer[k] = to;
		to = ti;
		k = (k*c)%m;
		ti = buffer[k];
	}
	buffer[k] = to;
}

template <typename T> void t_trans2( T* buffer, int length, int c, int r ) {
	T* buff2 = new T[ length ];
	double m = length - 1;

	//mod
	for( int i = 1; i < length-1; i++ ) {
		double l = i;
		l *= c;
		int h = (int)fmod( l, m );
		if( h < length-1 ) buff2[i] = buffer[ h ];
	}
	for( int i = 1; i < length-1; i++ ) {
		buffer[i] = buff2[i];
	}
	delete buff2;
}

template <typename T> void t_filetransbits( FILE* file, int length, int bits, int bytes, int c, int r ) {
	int dim = 0;
	int len = c*r;
	int mask = (1<<bits)-1;

	/*char cc;
	for( int i = 0 ; i < 6; i++ ) {
		fread( &cc, 1, 1, file );
		printf( "%d\n", (int)cc );
	}*/

	while( dim*len < length ) {
		double m = len-1;
		int i = 0;
		double l = 1;
		int 	totbits = bytes*8;

		while( i < m-2 && l < m ) {
			double k = fmod( (l*r), m );
			double t = fmod( (l*c), m );

			if( k > l && t > l ) {
				k = fmod( (k*r), m );
				while( k > l ) {
					k = fmod( (k*r), m );
				}
				if( k == l ) {
					double bk = bits*k;
					int kmod = (int)fmod( bk, totbits );
					int kind = (int)(bk/totbits);
					//T	val = *(T*)&buffer[kind*bytes];
					T val;
					fseek( file, kind*bytes, SEEK_SET );
					fread( &val, bytes, 1, file );
					val >>= kmod;
					unsigned char	to = val&mask;

					k = fmod( (l*r), m );
					bk = bits*k;
					kmod = (int)fmod( bk, totbits );
					kind = (int)(bk/totbits);
					//val = *(T*)&buffer[kind*bytes];
					fseek( file, kind*bytes, SEEK_SET );
					fread( &val, bytes, 1, file );
					T newval = val>>kmod;
					unsigned char	ti = newval&mask;

					while( k != l ) {
						//*(T*)&buffer[kind*bytes] &= ~(mask<<kmod);
						//*(T*)&buffer[kind*bytes] |= (to<<kmod);
						val &= ~(mask<<kmod);
						val |= (to<<kmod);
						fseek( file, kind*bytes, SEEK_SET );
						//val = 0;
						fwrite( &val, bytes, 1, file );

						to = ti;
						k = fmod( (k*r), m );
						bk = bits*k;
						kind = (int)(bk/totbits);
						kmod = (int)fmod( bk, totbits );
						//val = *(T*)&buffer[kind*bytes];
						fseek( file, kind*bytes, SEEK_SET );
						fread( &val, bytes, 1, file );
						newval = val>>kmod;
						ti = newval&mask;
						i++;
					}
					//*(T*)&buffer[kind*bytes] &= ~(mask<<kmod);
					//*(T*)&buffer[kind*bytes] |= (to<<kmod);
					val &= ~(mask<<kmod);
					val |= (to<<kmod);
					fseek( file, kind*bytes, SEEK_SET );
					//val = 0;
					fwrite( &val, bytes, 1, file );

					i++;
				}
			}
			l++;
		}

		//buffer += len;
		dim++;
	}
}

template <typename T> void t_transbits( unsigned char* buffer, int length, int bits, int bytes, int c, int r ) {
	int dim = 0;
	int len = c*r;
	int mask = (1<<bits)-1;
	//T	load;

	while( dim*len < length ) {
		double m = len-1;
		int i = 0;
		double l = 1;
		int 	totbits = bytes*8;
		//int		diff = (sizeof(T)-bytes)*8;

		while( i < m-2 && l < m ) {
			double k = fmod( (l*r), m );
			double t = fmod( (l*c), m );
			if( k > l && t > l ) {
				k = fmod( (k*r), m );
				while( k > l ) {
					k = fmod( (k*r), m );
				}
				if( k == l ) {
					double bk = bits*k;
					int kmod = (int)fmod( bk, totbits );
					int kind = (int)(bk/totbits);
					T	val = *(T*)&buffer[kind*bytes];
					//val >>= diff;
					val >>= kmod;
					unsigned char	to = val&mask;

					k = fmod( (l*r), m );
					bk = bits*k;
					kmod = (int)fmod( bk, totbits );
					kind = (int)(bk/totbits);
					val = *(T*)&buffer[kind*bytes];
					val >>= kmod;
					unsigned char	ti = val&mask;

					while( k != l ) {
						*(T*)&buffer[kind*bytes] &= ~(mask<<kmod);
						*(T*)&buffer[kind*bytes] |= (to<<kmod);
						to = ti;
						k = fmod( (k*r), m );
						bk = bits*k;
						kind = (int)(bk/totbits);
						kmod = (int)fmod( bk, totbits );
						val = *(T*)&buffer[kind*bytes];
						val >>= kmod;
						ti = val&mask;
						i++;
					}
					*(T*)&buffer[kind*bytes] &= ~(mask<<kmod);
					*(T*)&buffer[kind*bytes] |= (to<<kmod);

					i++;
				}
			}
			l++;
		}

		buffer += len;
		dim++;
	}
}

void t_transbit( unsigned char* buffer, int bits, int length, int c, int r ) {
	int dim = 0;
	int len = c*r;
	int mask = 1<<(bits-1);

	while( dim*len < length ) {
		double m = len-1;
		int i = 0;
		double l = 1;

		while( i < m-2 && l < m ) {
			double k = fmod( (l*r), m );
			double t = fmod( (l*c), m );
			if( k > l && t > l ) {
				k = fmod( (k*r), m );
				while( k > l ) {
					k = fmod( (k*r), m );
				}
				if( k == l ) {
					int kmod = (int)fmod( k, 8.0 );
					int kind = (int)(k/8.0);
					unsigned char	to = (buffer[kind]>>kmod)&mask;

					k = fmod( (l*r), m );
					kmod = (int)fmod( k, 8.0 );
					kind = (int)(k/8.0);
					unsigned char	ti = (buffer[kind]>>kmod)&mask;

					while( k != l ) {
						if( to ) buffer[kind] |= (1<<kmod);
						else buffer[kind] &= ~(1<<kmod);
						to = ti;
						k = fmod( (k*r), m );
						kind = (int)(k/8.0);
						kmod = (int)fmod( k, 8.0 );
						ti = (buffer[kind]>>kmod)&mask;
						i++;
					}
					if( to ) buffer[kind] |= (1<<kmod);
					else buffer[kind] &= ~(1<<kmod);

					i++;
				}
			}
			l++;
		}

		buffer += len;
		dim++;
	}
}

template <typename T> void t_sortidx( T* buffer, int length  ) {
	for( int i = 0; i < length; i++ ) {
		T min = buffer[i];
		int midx = i;

		int x;
		for( x = i; x < length; x++ ) {
			if( buffer[x] < min ) {
				min = buffer[x];
				midx = x;
			}
		}

		buffer[midx] = buffer[i];

		x = 0;
		while( 1 ) {
			for( x = 0; x < i; x++ ) {
				if( buffer[x] == (T)midx ) break;
			}

			if( x < i ) {
				midx = x;
			} else break;
		}

		buffer[i] = (T)midx;
	}
}

template <typename T> void t_invidx( T* buffer, int length  ) {
	for( int i = 0; i < length; i++ ) {
		int v = (int)buffer[i];
		while( v > i ) {
			v = (int)buffer[v];
		}
		if( v == i ) {
			int oi = i;
			v = (int)buffer[i];
			while( v > i ) {
				int tmp = (int)buffer[v];
				buffer[ v ] = (T)oi;
				oi = v;
				v = (int)tmp;
			}
			buffer[ v ] = (T)oi;
		}
	}
}

template <typename T> void t_transidx( T* buffer, int length, int c, int r ) {
	int dim = 1;
	int len = c*r;

	while( dim*len <= length ) {
		int m = len-1;

		int k = 0;
		for( int i = 0; i < m; i++ ) {
			buffer[i] = k;
			k = (k+r)%m;
		}
		buffer[m] = m;

		dim++;
	}
}

void t_transmem( char* buffer, int length, int c, int r, int numbytes ) {
	int dim = 1;
	int len = c*r;

	void*	ti = realloc( NULL, numbytes );

	while( dim*len <= length ) {
		double m = len-1;
		int i = 0;
		double l = 1;

		while( i < m-2 && l < m ) {
			double k = fmod( (l*r), m );
			double t = fmod( (l*c), m );
			if( k > l && t > l ) {
				k = fmod( (k*r), m );
				while( k > l ) {
					k = fmod( (k*r), m );
				}
				if( k == l ) {
					k = fmod( (l*r), m );
					memcpy( ti, buffer+(int)k*numbytes, numbytes );

					int lbytes = (int)(l*numbytes);
					int kbytes = (int)(k*numbytes);
					while( k != l ) {
						//printf("%d %d %d %d\n", kbytes, lbytes, numbytes, *(int*)ti );

						memcpy( buffer+kbytes, buffer+lbytes, numbytes );
						memcpy( buffer+lbytes, ti, numbytes );
						k = fmod( (k*r), m );
						kbytes = (int)(k*numbytes);
						memcpy( ti, buffer+kbytes, numbytes );
						i++;
					}
					//memcpy( buffer+(int)k*numbytes, buffer+(int)p*numbytes, numbytes );
					i++;
				}
			}
			l++;
		}

		buffer += dim*len;
		dim++;
	}

	free( ti );
}

template <typename T> void t_transold( T* buffer, int length, int c, int r ) {
	int len = c*r;

	double m = len-1;
	int i = 0;
	double l = 1;

	while( i < m-2 && l < m ) {
		double k = fmod((l*r) , m);
		double t = fmod((l*c) , m);
		if( k > l && t > l ) {
			/*k = fmod((k*r) , m);
			while( k > l ) {
				k = fmod((k*r) , m);
			}
			if( k == l ) {*/
				T	to = buffer[(int)l];
				k = fmod((l*r) , m);
				T	ti = buffer[(int)k];

				while( k != l ) {
					buffer[(int)k] = to;
					to = ti;
					k = fmod( (k*r) , m);
					ti = buffer[(int)k];
					i++;
				}
				buffer[(int)k] = to;
				i++;
			//}
		}
		l++;
	}
}

template <typename T, typename K> void t_trans( T buffer, int length, int c, int r ) {
	int dim = 0;
	int len = c*r;

	double m = len-1;
	int i = 0;
	double l = 1;

	if( len == length ) {
		while( i < m-2 && l < m ) {
			double k = fmod( (l*r), m );
			double t = fmod( (l*c), m );
			if( k > l && t > l ) {
				k = fmod( (k*r), m );
				while( k > l ) {
					k = fmod( (k*r), m );
				}
				if( k == l ) {
					K	to = buffer[(int)l];
					k = fmod( (l*r), m );
					K	ti = buffer[(int)k];

					while( k != l ) {
						buffer[(int)k] = to;
						to = ti;
						k = fmod( (k*r), m );
						ti = buffer[(int)k];
						i++;
					}
					buffer[(int)k] = to;
					i++;
				}
			}
			l++;
		}
	} else {
		while( i < m-2 && l < m ) {
			double k = fmod( (l*r), m );
			double t = fmod( (l*c), m );
			if( k > l && t > l ) {
				k = fmod( (k*r), m );
				while( k > l ) {
					k = fmod( (k*r), m );
				}
				if( k == l ) {
					T buf = buffer;
					while( dim*len < length ) {
						K	to = buf[(int)l];
						k = fmod( (l*r), m );
						K	ti = buf[(int)k];

						while( k != l ) {
							buf[(int)k] = to;
							to = ti;
							k = fmod( (k*r), m );
							ti = buf[(int)k];
							i++;
						}
						buf[(int)k] = to;
						//heyheyho
						//buf += len;
						dim++;
					}
					i++;
				}
			}
			l++;
		}
	}
}

template <typename T, typename K> void t_div( T* buffer, int length, K* value, int vallen ) {
	if( vallen == 0 ) {
		T val = (T)*((K*)&value);
		for( int i = 0; i < length; i++ ) {
			buffer[i] /= val;
		}
	} else {
		for( int i = 0; i < length; i++ ) {
			buffer[i] /= (T)value[i%vallen];
		}
	}
}

template <typename T> void t_idft( T* buffer, int length ) {
	T*  tmp = (T*)malloc( bytelength( data.type, data.length ) );
	double m_pi = acos( -1.0 );
	for( int k = 0; k < data.length; k+=2 ) {
		tmp[k] = 0;
		tmp[k+1] = 0;
		for( int n = 0; n < data.length; n+=2 ) {
			double c = cos( -(2.0*m_pi*k*n)/data.length );
			double s = sin( -(2.0*m_pi*k*n)/data.length );
			tmp[k] += buffer[n]*c-buffer[n+1]*s;
			tmp[k+1] += buffer[n]*s+buffer[n+1]*c;
		}
		tmp[k] /= data.length;
		tmp[k+1] /= data.length;
	}
	free( (void*)data.buffer );
	data.buffer = (int)tmp;
}

template <typename T> void t_dft( T* buffer, int length ) {
	T*  tmp = (T*)malloc( bytelength( data.type, data.length ) );
	double m_pi = acos( -1.0 );
	for( int k = 0; k < data.length; k+=2 ) {
		tmp[k] = 0;
		tmp[k+1] = 0;
		for( int n = 0; n < data.length; n+=2 ) {
			double c = std::cos( -(2.0*m_pi*k*n)/data.length );
			double s = std::sin( -(2.0*m_pi*k*n)/data.length );
			tmp[k] += buffer[n]*c-buffer[n+1]*s;
			tmp[k+1] += buffer[n]*s+buffer[n+1]*c;
		}
	}
	free( (void*)data.buffer );
	data.buffer = (long)tmp;
}

template <typename T> void t_dct( T* buffer, int length ) {
	T*  tmp = (T*)malloc( bytelength( data.type, data.length ) );
	double m_pi = acos( -1.0 );
	for( int k = 0; k < data.length; k++ ) {
		tmp[k] = 0;
		for( int n = 0; n < data.length; n++ ) {
			tmp[k] += buffer[n]*cos( (m_pi*k*(n+0.5))/data.length );
		}
	}
	free( (void*)data.buffer );
	data.buffer = (long)tmp;
}

void t_dct( float* buffer, int length ) {
	float*  tmp = (float*)malloc( bytelength( data.type, data.length ) );
	float m_pi = acosf( -1.0f );
	for( int k = 0; k < data.length; k++ ) {
		tmp[k] = 0;
		for( int n = 0; n < data.length; n++ ) {
			tmp[k] += buffer[n]*cosf( (m_pi*k*(n+0.5f))/data.length );
		}
	}
	free( (void*)data.buffer );
	data.buffer = (long)tmp;
}

template <typename T> void t_idct( T* buffer, int length ) {
	T*  tmp = (T*)malloc( bytelength( data.type, data.length ) );
	double m_pi = acos( -1.0 );
	for( int k = 0; k < data.length; k++ ) {
		tmp[k] = 0.5*buffer[0];
		for( int n = 1; n < data.length; n++ ) {
			tmp[k] += buffer[n]*cos( (m_pi*n*(k+0.5))/data.length );
		}
		tmp[k] *= 2;
		tmp[k] /= data.length;
	}
	free( (void*)data.buffer );
	data.buffer = (long)tmp;
}

void t_idct( float* buffer, int length ) {
	float*  tmp = (float*)malloc( bytelength( data.type, data.length ) );
	float m_pi = acosf( -1.0f );
	for( int k = 0; k < data.length; k++ ) {
		tmp[k] = 0.5f*buffer[0];
		for( int n = 1; n < data.length; n++ ) {
			tmp[k] += buffer[n]*cosf( (m_pi*n*(k+0.5f))/data.length );
		}
		tmp[k] *= 2;
		tmp[k] /= data.length;
	}
	free( (void*)data.buffer );
	data.buffer = (long)tmp;
}

template <typename T> int t_identity( T* buffer, int length ) {
	int d = (long)sqrt( (double)length )+1;
	for( int i = 0; i < length; i+=d ) {
		buffer[i] = 1;
	}
	return current;
}

template <typename T, typename K> int t_index( T buffer, int length ) {
	for( int i = 0; i < length; i++ ) {
		buffer[i] = (K)i;
	}
	return current;
}

//template <typename T> void (*t_val)( T* buffer );

/*template<typename T, typename K, typename Q> void c_set( cdata<T> & data, cdata<K> & value, cdata<Q> & where ) {
	for( int i = 0; i < where.len; i++ ) {
		data[ where[i] ] = value[i];
	}
}

template<typename T, typename K> void c_set( cdata<T> & data, cdata<K> & value, simlab & where ) {
	if( value.type == 66 ) c_set( (cdata<T>)data, (cdata<K>)value, (cdata<double>)where );
	if( value.type == 32 ) c_set( (cdata<T>)data, (cdata<K>)value, (cdata<int>)where );
}

template<typename T> void c_set( cdata<T> & data, simlab & value, simlab & where ) {
	if( value.type == 66 ) c_set( (cdata<T>)data, (cdata<double>)value, where );
	if( value.type == 32 ) c_set( (cdata<T>)data, (cdata<int>)value, where );
}*/

template <typename T, typename K, typename Q> void t_set( T* buffer, int length, K & wh, int wlength, Q & value, int vlength ) {
	if( wlength == 0 ) {
		for( int k = 0; k < length; k++ ) {
			buffer[k] = (T)value[k%vlength];
		}
	} else {
		for( int k = 0; k < wlength; k++ ) {
			//if( wh[k] < length ) {
			buffer[ (long)wh[k] ] = (T)value[k%vlength];
			//} else printf("wh%d %d\n",(int)wh[k], k);
		}
	}
}

template <typename T, typename K> void t_set( T* buffer, int length, K & wh, int wlength, simlab & value ) {
	if( value.length == -1 ) {
		//printf("len1 %d\n", int_ind.length());
		//printf("len2 %d\n", ((PseudoBuffer<int>*)value.buffer)->length());
		if( value.type == 66 ) t_set( buffer, length, wh, wlength, *(PseudoBuffer<double>*)value.buffer, ((PseudoBuffer<double>*)value.buffer)->length() );
		else if( value.type == 34 ) t_set( buffer, length, wh, wlength, *(PseudoBuffer<float>*)value.buffer, ((PseudoBuffer<float>*)value.buffer)->length() );
		else if( value.type == 32 ) t_set( buffer, length, wh, wlength, *(PseudoBuffer<int>*)value.buffer, ((PseudoBuffer<int>*)value.buffer)->length() );
	} else if( value.length == 0 ) {
		if( value.type == 32 ) {
			int*	val = (int*)&value.buffer;
			t_set( buffer, length, wh, wlength, val, 1 );
		} else if( value.type == 34 ) {
			float*	val = (float*)&value.buffer;
			t_set( buffer, length, wh, wlength, val, 1 );
		} else if( value.type == 66 ) {
			double*	val = (double*)&value.buffer;
			t_set( buffer, length, wh, wlength, val, 1 );
		}
	} else {
		if( value.type == 66 ) t_set( buffer, length, wh, wlength, *(double**)&value.buffer, value.length );
		else if( value.type == 34 ) t_set( buffer, length, wh, wlength, *(float**)&value.buffer, value.length );
		else if( value.type == 32 ) t_set( buffer, length, wh, wlength, *(int**)&value.buffer, value.length );
	}
}

template <typename T> void t_set( T* buffer, int length, simlab & where, simlab & value ) {
	//printf("lerm%d %d\n",where.length,where.type);
	if( where.length == -1 ) {
		if( where.type == 66 ) t_set( buffer, length, *(PseudoBuffer<double>*)where.buffer, ((PseudoBuffer<double>*)where.buffer)->length(), value );
		else if( where.type == 34 ) t_set( buffer, length, *(PseudoBuffer<float>*)where.buffer, ((PseudoBuffer<float>*)where.buffer)->length(), value );
		else if( where.type == 32 ) t_set( buffer, length, *(PseudoBuffer<int>*)where.buffer, ((PseudoBuffer<int>*)where.buffer)->length(), value );
	} else if( where.length == 0 ) {
		if( where.type == 32 ) {
			int*	val = (int*)&where.buffer;
			t_set( buffer, length, val, 1, value );
		} else if( where.type == 34 ) {
			float*	val = (float*)&where.buffer;
			t_set( buffer, length, val, 1, value );
		} else if( where.type == 66 ) {
			double*	val = (double*)&where.buffer;
			t_set( buffer, length, val, 1, value );
		} else if( where.type == 0 ) {
			int*	val = (int*)&where.buffer;
			t_set( buffer, length, val, 0, value );
		}
	} else {
		if( where.type == 66 ) t_set( buffer, length, *(double**)&where.buffer, where.length, value );
		else if( where.type == 34 ) t_set( buffer, length, *(float**)&where.buffer, where.length, value );
		else if( where.type == 32 ) t_set( buffer, length, *(int**)&where.buffer, where.length, value );
	}
}

template <typename T,typename K,typename Q> void t_draw( T* buffer, int length, K* ibuffer, int ilength, Q* dbuffer, int dlength ) {
	for( int i = 0; i < ilength; i++ ) {
		buffer[ (long)ibuffer[i] ] = (T)dbuffer[i];
	}
}

template <typename T, typename K> void t_get( T* buffer, int length, K indx, int ilength ) {
	T* ret = new T[ilength];

	for( int i = 0; i < ilength; i++ ) {
		if( indx[i] >= 0 && indx[i] < length ) ret[i] = buffer[(unsigned int)indx[i]];
	}

	data.buffer = (long)ret;
	data.length = ilength;
}

template <typename T> void t_min( T* buffer, int length, int chunk, int size ) {
	int retsize = (chunk-size+1);
	int retlen = (length*retsize)/chunk;
	T* ret = new T[ retlen ];

	int r = 0;
	for( int c = 0; c < length; c+=chunk ) {
		T val = buffer[c];
		int i;
		for( i = 1; i < size; i++ ) {
			if( buffer[c+i] < val ) val = buffer[c+i];
		}
		ret[r++] = val;
		while( i < chunk ) {
			T ld = buffer[c+i-size];
			T nw = buffer[c+i];

			if( nw < val ) val = nw;
			else if( ld == val ) {
				val = nw;
				for( int k = c+i-size+1; k < c+i-1; k++ ) {
					if( buffer[k] < val ) val = buffer[k];
				}
			}

			i++;
			ret[r++] = val;
		}
	}

	data.buffer = (long)ret;
	data.length = retlen;
}

template <typename T> void t_max( T* buffer, int length, int chunk, int size ) {
	int retsize = (chunk-size+1);
	int retlen = (length*retsize)/chunk;
	T* ret = new T[ retlen ];

	int r = 0;
	for( int c = 0; c < length; c+=chunk ) {
		T val = buffer[c];
		int i;
		for( i = 1; i < size; i++ ) {
			if( buffer[c+i] > val ) val = buffer[c+i];
		}
		ret[r++] = val;
		while( i < chunk ) {
			T ld = buffer[c+i-size];
			T nw = buffer[c+i];

			if( nw > val ) val = nw;
			else if( ld == val ) {
				val = nw;
				for( int k = c+i-size+1; k < c+i-1; k++ ) {
					if( buffer[k] > val ) val = buffer[k];
				}
			}

			i++;
			ret[r++] = val;
		}
	}

	data.buffer = (long)ret;
	data.length = retlen;
}

template <typename T> int t_minmax( simlab* data ) {
	T* buffer = (T*)data->buffer;
	T min = buffer[0],max = buffer[0];
	for( int i = 1; i < data->length; i++ ) {
		T val = buffer[i];
		if( val < min ) min = val;
		else if( val > max ) max = val;
	}
	current = vector( data->type, 2 );
	//set( 0, min );
	//set( 1, max );

	return current;
}

template <typename T> void t_norm( T* buffer, int length, int chunk ) {
	T* min = NULL;//t_min( buffer, length, chunk );
	T* max = NULL;//t_max( buffer, length, chunk );

	//int reslen = length/chunk;

	for( int i = 0; i < length; i+=chunk ) {
		int w = i/chunk;
		for( int k = i; k < i+chunk; k++ ) {
			buffer[k] = (buffer[k]-min[w])/(max[w]-min[w]);
		}
	}

	delete min;
	delete max;
}

template <typename T> void t_zero( T* buffer, int length ) {
	for( int i = 0; i < length; i++ ) {
		buffer[i] = 0;
	}
}

template <typename T> T* t_hist( T* buffer, int length, int chunk, int bin ) {
	int retlen = bin*(length/chunk);
	T* ret = new T[ retlen ];

	T* mn = NULL;//t_min( buffer, length, length );
	T* mx = NULL;//t_max( buffer, length, length );
	t_zero( ret, retlen );

	T	mmn = mn[0];
	T	mmx = mx[0];

	for( int i = 0; i < length; i+=chunk ) {
		int r = i/chunk;
		T* hst = &ret[r*bin];
		for( int k = i; k < i+chunk; k++ ) {
			if( buffer[i] == mmx ) hst[ bin-1 ]++;
			else hst[ bin*(int)((buffer[i]-mmn)/(mmx-mmn)) ]++;
		}
	}

	free( mn );
	free( mx );

	return ret;
}

template <typename T,typename K, typename V> void t_copy( const T & buffer, int length, const K & c_buffer ) {
	for( int i = 0; i < length; i++ ) {
		buffer[i] = (V)c_buffer[i];
	}
}

template <class T> void t_copy( const T & t, int l ) {
	if( data.type == 66 ) {
		t_copy<double*,T,double>( (double*)data.buffer, data.length, t );
	} else if( data.type == 34 ) {
		t_copy<float*,T,float>( (float*)data.buffer, data.length, t );
	} else if( data.type == 32 ) {
		t_copy<int*,T,int>( (int*)data.buffer, data.length, t );
	}
}

template <typename T,typename K> void t_memcopy( T* buffer, int length, K* c_buffer, int c_length ) {
	for( int i = 0; i < mn(length,c_length); i++ ) {
		memcpy( &buffer[i], &c_buffer[i], sizeof(K) );
	}
}

template <typename T, typename K> void t_fft( T* buffer, int length, K* cbuffer, int clength ) {
	for( int c = 0; c < clength; c++ ) {
		int chunk = (long)cbuffer[c];
		int val = (chunk+1)/2;
		int sec = chunk/2;
		for( int k = 0; k < length; k+=chunk ) {
			T* buf = &buffer[k];
			for( int i = 0; i < sec; i++ ) {
				T tmp = buf[i];
				buf[i] = (tmp+buf[i+val])/2;
				buf[i+val] = (tmp-buf[i+val])/2;
			}
		}
	}
}

template <typename T, typename K> void t_awlet( T* buffer, int length, K* cbuffer, int clength ) {
	for( int c = 0; c < clength; c++ ) {
		int chunk = (long)cbuffer[c];
		int val = (chunk+1)/2;
		int sec = chunk/2;
		for( int k = 0; k < length; k+=chunk ) {
			T* buf = &buffer[k];
			for( int i = 0; i < sec; i++ ) {
				T tmp = buf[i];
				buf[i] = tmp+buf[i+val];
				buf[i+val] = tmp-buf[i+val];
			}
		}
	}
}

template <typename T, typename K> void t_wlet( T* buffer, int length, K* cbuffer, int clength ) {
	for( int c = 0; c < clength; c++ ) {
		int chunk = (long)cbuffer[c];
		int val = (chunk+1)/2;
		int sec = chunk/2;
		for( int k = 0; k < length; k+=chunk ) {
			T* buf = &buffer[k];
			for( int i = 0; i < sec; i++ ) {
				T tmp = buf[i];
				buf[i] = (tmp+buf[i+val])/2;
				buf[i+val] = (tmp-buf[i+val])/2;
			}
		}
	}
}

template <typename T> void t_minlet( T* buffer, int length, int chunk ) {
	for( int k = 0; k < length; k+=chunk ) {
		T* buf = &buffer[k];
		for( int i = 0; i < chunk; i+=2 ) {
			T tmp = buf[i];
			buf[i] = mn(tmp,buf[i+1]);
			buf[i+1] = tmp-buf[i+1];
		}
	}
}

template <typename T> void t_maxlet( T* buffer, int length, int chunk ) {
	for( int k = 0; k < length; k+=chunk ) {
		T* buf = &buffer[k];
		for( int i = 0; i < chunk; i+=2 ) {
			T tmp = buf[i];
			buf[i] = mx(tmp,buf[i+1]);
			buf[i+1] = tmp-buf[i+1];
		}
	}
}

template <typename T> void t_amaxlet( T* buffer, int length, int chunk ) {
	for( int k = 0; k < length; k+=chunk ) {
		T* buf = &buffer[k];
		for( int i = 0; i < chunk; i+=2 ) {
			T tmp = buf[i+1];
			if( tmp < 0 ) {
				buf[i+1] = buf[i];
				buf[i] = tmp+buf[i];
			} else {
				buf[i+1] = buf[i]-tmp;
			}
		}
	}
}

template <typename T> void t_cmul( T* buffer, int length, int chunk ) {
	for( int k = 0; k < length; k+=chunk ) {
		T* buf = &buffer[k];
		for( int i = 0; i < chunk; i+=2 ) {
			T tmp = buf[i+1];
			if( tmp < 0 ) {
				buf[i+1] = buf[i];
				buf[i] = tmp+buf[i];
			} else {
				buf[i+1] = buf[i]-tmp;
			}
		}
	}
}

template <typename T, typename K> void t_concat( T* con, K* cat, int conlen, int catlen ) {

}

template <typename T, typename K> void t_search( simlab* buff, simlab* data ) {
	T*	buffer = (T*)buffer->buffer;
	K*	stuff = (K*)data->buffer;
	for( int i = 0; i < data->length; i++ ) {
		//printf( format, buffer[i] );
	}
}

template <typename T,typename K> void t_divd( T* buffer, int length, K* mulee, int m_length ) {
	//int rows = length/m_length;
	for( int i = 0; i < length; i+=m_length ) {
		for( int k = 0; k < m_length; k++ ) {
			buffer[i+k] /= (T)mulee[k];
		}
	}
}

template <typename T, typename K> T* t_find( T* buffer, int length, K* what, int f_length ) {
	return 0;
}

template <typename T> void t_flip( T* buffer, int length, int chunk ) {
	for( int i = 0; i < length; i+=chunk ) {
		T* nbuffer = &buffer[i];
		for( int k = 0; k < chunk/2; k++ ) {
			T tmp = nbuffer[k];
			nbuffer[k] = nbuffer[chunk-1-k];
			nbuffer[chunk-1-k] = tmp;
		}
	}
}

template <typename T> void t_shift( T* buffer, int length, int chunk, int shift ) {
	shift %= chunk;
	if( shift < 0 ) shift = chunk + shift;
	int ec = _gcd( shift, chunk );
	for( int r = 0; r < length; r+=chunk ) {
		T* rdata = &buffer[r];
		for( int i = 0; i < ec; i++ ) {
			int k = i;
			T	tmp1;
			T	tmp2 = rdata[k];
			do {
				tmp1 = tmp2;
				k = (k+shift)%chunk;
				tmp2 = rdata[k];
				rdata[k] = tmp1;
			} while( k != i );
		}
	}
}

template <typename T, typename K> int t_intersectcount( T* buffer1, K* buffer2, int len1, int len2 ) {

	return 0;
}

template <typename T, typename K> void t_intersect( T* buffer, int length, K* inter, int ilength ) {
	std::vector<T>	res;
	for( int i = 0; i < length; i++ ) {
		T	val = buffer[i];
		int k = 0;
		unsigned int l = 0;

		//while( l < ) {
		//= res.begin();
		while( l < res.size() || k == 0 ) {
			while( k < ilength ) {
				if( val == (T)inter[k] ) break;
				k++;
			}
			if( k < ilength ) {
				while( l < res.size() ) {
					if( res[l] == val ) break;
					l++;
				}

				if( l == res.size() ) res.push_back( val );
			} else break;

			k++;
			l++;
		}
	}

	T* retbuffer = new T[res.size()];
	memcpy( retbuffer, &res[0], sizeof(T)*res.size() );
	data.buffer = (long)retbuffer;
	data.length = res.size();
}

template <typename T> void t_median( T* buffer, int length, int chunk ) {
	for( int i = 0; i < length/*-chunk*/; i++ ) {
		for( int k = i+1; k < i+chunk; k++ ) {
			buffer[i] += buffer[k%length];
		}
		buffer[i] /= chunk;
	}
	t_shift( buffer, length, length, chunk/2 );
}

template <typename T> void t_mean( T* buffer, int length, int chunk, int size ) {
	int retsize = (chunk-size+1);
	int retlen = (length*retsize)/chunk;
	T* ret = new T[ retlen ];

	int r = 0;
	for( int c = 0; c < length; c+=chunk ) {
		double val = 0.0;
		int i;
		for( i = 0; i < size; i++ ) {
			val += buffer[c+i];
		}
		ret[r++] = (T)(val/size);
		while( i < chunk ) {
			val -= buffer[c+i-size];
			val += buffer[c+i];
			i++;
			ret[r++] = (T)(val/size);
		}
	}

	data.buffer = (long)ret;
	data.length = retlen;
}

template <typename T> void t_sum( T* buffer, int length, int chunk, int size ) {
	int retsize = (chunk-size+1);
	int retlen = (length*retsize)/chunk;
	T* ret = new T[ retlen ];

	int r = 0;
	for( int c = 0; c < length; c+=chunk ) {
		T val = 0;
		int i;
		for( i = 0; i < size; i++ ) {
			val += buffer[c+i];
		}
		ret[r++] = val;
		while( i < chunk ) {
			val -= buffer[c+i-size];
			val += buffer[c+i];
			i++;
			ret[r++] = val;
		}
	}

	data.buffer = (long)ret;
	data.length = retlen;
}

template <typename T> void t_diff( T* buffer, int length ) {
	for( int i = length-1; i > 0; i-- ) {
		buffer[i] -= buffer[i-1];
	}
}

template <typename T> void t_integ( T* buffer, int length ) {
	for( int i = 1; i < length; i++ ) {
		buffer[i] += buffer[i-1];
	}
}

template <typename T> void t_ceil( T* buffer, int length ) {
	for( int i = 0; i < length; i++ ) {
		buffer[i] = ceil( buffer[i] );
	}
}

template <typename T> void t_floor( T* buffer, int length ) {
	for( int i = 0; i < length; i++ ) {
		buffer[i] = floor( buffer[i] );
	}
}

template <typename T, typename K> void t_matmul( T* buffer, int length, K* mulbuffer, int mullength, int val ) {
	//int length = (data.length/val)*(mul.length/val);
	//printf( "%d\n", val );
	int retc = (mullength/val);
	int retr = (length/val);
	int retlen = retc*retr;
	T* ret = new T[retlen];
	memset( ret, 0, sizeof( T )*retlen );
	for( int r = 0; r < retr; r++ ) {
		int rretc = r*retc;
		for( int c = 0; c < retc; c++ ) {
			int reti = rretc+c;
			int rval = r*val;
			for( int i = 0; i < val; i++ ) {
				ret[reti] += buffer[rval+i]*(T)mulbuffer[i*retc+c];
			}
		}
	}
	data.buffer = (long)ret;
	data.length = retlen;
}

template <typename T> void t_idx( T* buffer, int len, void* cmp, int bytelen ) {
	std::vector<int>	idx;
	for( int i = 0; i < len; i++ ) {
		if( memcmp( (void*)&buffer[i], cmp, bytelen ) == 0 ) {
			idx.push_back( i );
		}
	}
	int* bff = new int[idx.size()];
	memcpy( bff, &idx[0], sizeof(int)*idx.size() );

	data.buffer = (long)bff;
	data.type = 32;
	data.length = idx.size();
}

template <typename T> int t_minidx( T* buffer, int length ) {
	T min = buffer[0];
	int minidx = 0;
	for( int i = 1; i < length; i++ ) {
		if( buffer[i] < min ) {
			min = buffer[i];
			minidx = i;
		}
	}
	return minidx;
}

//template<typename T>
class Dual {
public:
	int* b;
	int l;
	int i;

	Dual( int* buffer, int length ) {
		b = buffer;
		l = length;
		i = 0;
	}

	Dual& operator=( const Dual& val ) {
		b[i] = b[val.i];
		return *this;
	}

	Dual& operator[]( int ind ) {
		i = ind;
		return *this;
	}

	bool operator<(const Dual& f) {
		return b[i] < b[f.i];
	}
};

template <typename T> class c_add : public PseudoBuffer<T> {
	T	&	array;
};

template <typename K, typename T> class c_dfunc : public PseudoBuffer<T> {
	K	& 	array;
	double	(*dfunc)(double);
	int		len;

public:
	c_dfunc( K &  arr, int length, double (*fnc)(double) ) : array(arr), len(length), dfunc(fnc) {}
	//virtual ~c_dfunc();

	virtual T operator[]( int ind ) { return (T)dfunc((double)array[ind]); }
	virtual int length() { return len; }
};

template <typename K, typename T> class c_trans : public PseudoBuffer<T> {
	K	& 	array;
	int		col,row;
	int		len;

public:
	c_trans( K &  arr, int columns, int rows, int length ) : array(arr), col(columns), row(rows), len(length) {}
	//virtual ~c_dfunc();

	virtual T operator[]( int ind ) { return array[row*(ind%col)+ind/col]; }
	virtual int length() { return len; }
};

template <typename K, typename Q, typename T> class c_arr : public PseudoBuffer<T> {
	K	& 	array;
	Q	&	index;
	int		len;

public:
	c_arr( K & arr, Q & ind, int length ) : array(arr), index(ind), len(length) {}
	//virtual ~c_dfunc();

	virtual T operator[]( int ind ) { return (T)array[(int)index[ind]]; }
	virtual int length() { return len; }
};

/*template<typename T, typename K> class c_arr {
public:
	c_arr( T arr, K ind ) : array(arr), index(ind) {

	}

	T 	array;
	K	index;

	T operator[]( int ind ) {
		return array[ (long)index[ind] ];
	}
};*/

template<typename T> class c_ind : PseudoBuffer<T> {
public:
	inline T operator[]( int ind ) {
		return ind;
	}

	int length() {
		return 10;
	}
};

template<typename T> class c_rnd : PseudoBuffer<T> {
public:
	inline T operator[]( int ind ) {
		return ind;
	}

	int length() {
		return 1;
	}
};

extern c_ind<int>		int_ind;
extern c_rnd<double>	dbl_rnd;

void t_wrap( void* buffer, int length ) {
	garbage();
	data.buffer = (long)buffer;
	data.length = length;
}

void t_wrap( double* buffer, int length ) {
	t_wrap( (void*)buffer, length );
	data.type = 66;
}

void t_wrap( float* buffer, int length ) {
	t_wrap( (void*)buffer, length );
	data.type = 34;
}

void t_wrap( int* buffer, int length ) {
	t_wrap( (void*)buffer, length );
	data.type = 32;
}

extern "C" {
std::vector<void*>	vvec;
JNIEXPORT int pseudoarr( simlab arr ) {
	if( data.length == -1 ) {
		PseudoBuffer<double>*	pb = (PseudoBuffer<double>*)data.buffer;
		if( data.type == 66 ) {
			data.buffer = (long)new c_arr<PseudoBuffer<double>,PseudoBuffer<double>,double>( *(PseudoBuffer<double>*)data.buffer, *(PseudoBuffer<double>*)arr.buffer, pb->length() );
		}
	} else if( data.type == 66 ) {
		if( arr.type == 66 ) {
			vvec.push_back( (void*)arr.buffer );
			vvec.push_back( (void*)data.buffer );
			data.buffer = (long)new c_arr<double*,double*,double>( *(double**)&vvec[vvec.size()-1], *(double**)&vvec[vvec.size()-2], data.length );
		} else if( arr.type == 34 ) {
			vvec.push_back( (void*)arr.buffer );
			vvec.push_back( (void*)data.buffer );
			data.buffer = (long)new c_arr<double*,float*,double>( *(double**)&vvec[vvec.size()-1], *(float**)&vvec[vvec.size()-2], data.length );
		} else if( arr.type == 32 ) {
			vvec.push_back( (void*)arr.buffer );
			vvec.push_back( (void*)data.buffer );
			data.buffer = (long)new c_arr<double*,int*,double>( *(double**)&vvec[vvec.size()-1], *(int**)&vvec[vvec.size()-2], data.length );
		}
		//data.buffer = (long)new c_dfunc<double*,double>( *(double**)&data.buffer, data.length, (double (*)(double))func.buffer );
	}
	data.length = -1;

	return current;
}

JNIEXPORT int pseudotran( simlab c, simlab r ) {
	if( data.length == -1 ) {
		PseudoBuffer<double>*	pb = (PseudoBuffer<double>*)data.buffer;
		if( data.type == 66 ) data.buffer = (long)new c_trans<PseudoBuffer<double>,double>( *(PseudoBuffer<double>*)data.buffer, c.buffer, r.buffer, pb->length() );
	} 	else if( data.type == 66 ) {
		vvec.push_back( (void*)data.buffer );
		data.buffer = (long)new c_trans<double*,double>( *(double**)&vvec[vvec.size()-1], c.buffer, r.buffer, data.length );
		//data.buffer = (long)new c_dfunc<double*,double>( *(double**)&data.buffer, data.length, (double (*)(double))func.buffer );
	}
	data.length = -1;

	return current;
}

JNIEXPORT int pseudodfunc( simlab func ) {
	if( data.length == -1 ) {
		//PseudoBuffer<double>*	pb = (PseudoBuffer<double>*)data.buffer;
		if( data.type == 66 ) data.buffer = (long)new c_dfunc<PseudoBuffer<double>,double>( *(PseudoBuffer<double>*)data.buffer, ((PseudoBuffer<double>*)data.buffer)->length(), (double (*)(double))func.buffer );
		else if( data.type == 34 ) data.buffer = (long)new c_dfunc<PseudoBuffer<float>,float>( *(PseudoBuffer<float>*)data.buffer, ((PseudoBuffer<float>*)data.buffer)->length(), (double (*)(double))func.buffer );
	} 	else if( data.type == 66 ) {
		vvec.push_back( (void*)data.buffer );
		data.buffer = (long)new c_dfunc<double*,double>( *(double**)&vvec[vvec.size()-1], data.length, (double (*)(double))func.buffer );
		//data.buffer = (long)new c_dfunc<double*,double>( *(double**)&data.buffer, data.length, (double (*)(double))func.buffer );
	} else if( data.type == 34 ) {
		//float*	fb = (float*)data.buffer;
		vvec.push_back( (void*)data.buffer );
		data.buffer = (long)new c_dfunc<float*,float>( *(float**)&vvec[vvec.size()-1], data.length, (double (*)(double))func.buffer );
	} else if( data.type == 32 ) {
		//int*	ib = (int*)data.buffer;
		vvec.push_back( (void*)data.buffer );
		data.buffer = (long)new c_dfunc<int*,int>( *(int**)&vvec[vvec.size()-1], data.length, (double (*)(double))func.buffer );
	}
	data.length = -1;

	return current;
}

JNIEXPORT int createarrange( simlab index ) {
	if( data.type == 66 ) {
		if( index.type == 32 ) {
			//data.buffer = (long)new c_arr<double*,int*>( (double*)data.buffer, (int*)index.buffer );
		}
	}

	return current;
}

JNIEXPORT int tramat( int a ) {
	for( int r = 2; r < a; r++ ) {
		for( int c = 2; c < a; c++ ) {
			int m = r*c-1;
			int i = 0;
			int t = 1;
			int k = (t*r)%m;//tran( t, c, r ); //m%(t*c);
			//printf( "hoho%d %d %d\n", k, m, r );
			while( k != t ) {
				i++;
				k = (k*r)%m;//tran( k, c, r ); //m%(k*c);
			}
			printf( "%d\t", i );
		}
		printf( "\n" );
	}
	return current;
}

JNIEXPORT int ptra( int i, int a, int b ) {
	int k = tran( i,a,b );
	int l = 1;
	while( k != i ) {
		printf( "%d  ", k );
		k = tran( k,a,b );
		l++;
	}
	printf( "%d\n", k );
	printf( "%d\n", l );
	return 0;
}

JNIEXPORT int pgcd( int a, int b ) {
	printf( "%d\n", _gcd( a,b ) );
	return 0;
}

JNIEXPORT int fetch( simlab sl ) {
	if( sl.type == 8 && sl.length > 0 ) {
		char* name = (char*)sl.buffer;
		if( retlib.find(name) == retlib.end() ) return 1;
		data = retlib[name];
		/*if( strcmp( name, "image") == 0 ) {
			printf("%d %d\n", data.type, data.length);
		} else {
			printf("fetching: %s\n", name);
		}*/
	} else {
		data = sl;
	}

	return 1;
}

JNIEXPORT int store( simlab name ) {
	char* cc = (char*)(name.buffer);
	retlib[cc] = data;

	return current;
}

#ifdef JAVA
JNIEXPORT int Class( simlab name ) {
	if( javaenv == NULL ) initjava();

	jclass	cls = javaenv->FindClass( (char*)name.buffer );
	jobject	ret = javaenv->NewGlobalRef( cls );
	javaenv->DeleteLocalRef( cls );

	printf( "%d\n", ret );
	jcls = (jclass)ret;

	return current;
}

JNIEXPORT int New( ... ) {
	if( javaenv == NULL ) initjava();

	if( jcls != 0 ) {
		//java = 2;
		//jclass cls = (jclass)jobj;

		jobject tempobj = javaenv->AllocObject(jcls);
		jobject obj = javaenv->NewGlobalRef( tempobj );
		javaenv->DeleteLocalRef( tempobj );

		//current = (long)obj;
		jobj = obj;

		std::string sig = "(";
		numpar = 0;
		parseJavaParameters( "<init>", sig, 0 );

		return current;
	}
	return 0;
}

JNIEXPORT int Release() {
	return current;
}

JNIEXPORT int Data() {
	jobject obj = (jobject)current;

	simlab* data = new simlab;

	data->buffer = (long)javaenv->GetDirectBufferAddress( obj );
	data->length = javaenv->GetDirectBufferCapacity( obj );

	if( javaenv->IsInstanceOf( obj, javaenv->FindClass( "Ljava/nio/DoubleBuffer;" ) ) ) {
		data->type = 66;
	} else if( javaenv->IsInstanceOf( obj, javaenv->FindClass( "Ljava/nio/FloatBuffer;" ) ) ) {
		data->type = 34;
	} else if( javaenv->IsInstanceOf( obj, javaenv->FindClass( "Ljava/nio/IntBuffer;" ) ) ) {
		data->type = 32;
	}

	//java = 0;
	current = (long)data;

	javaenv->DeleteGlobalRef( obj );

	return current;
}

JNIEXPORT int Java() {
	simlab* data = (simlab*)current;
	if( data != NULL ) {
		jobject dbb = javaenv->NewDirectByteBuffer( (void*)data->buffer, bytelength( data->type, data->length ) );
		jclass  bbc = javaenv->GetObjectClass( dbb );
		if( dbb != NULL ) {
			jclass byteorderclass = javaenv->FindClass( "Ljava/nio/ByteOrder;" );
			if( byteorderclass != NULL ) {
				jmethodID mid = javaenv->GetStaticMethodID( byteorderclass, "nativeOrder", "()Ljava/nio/ByteOrder;" );
				if( mid != NULL ) {
					jobject byteorder = javaenv->CallStaticObjectMethod( byteorderclass, mid ); //env->CallObjectMethod( dbb, mid );
					if( byteorder != NULL ) {
						mid = javaenv->GetMethodID( bbc, "order", "(Ljava/nio/ByteOrder;)Ljava/nio/ByteBuffer;" );
						if( mid != NULL ) javaenv->CallObjectMethod( dbb, mid, byteorder );
					}
				}
			}

			jobject lref;
			if( data->type == 66 ) {
				jmethodID mid = javaenv->GetMethodID( bbc, "asDoubleBuffer", "()Ljava/nio/DoubleBuffer;" );
				if( mid != NULL ) lref = javaenv->CallObjectMethod( dbb, mid );
			} else if( data->type == 34 ) {
				jmethodID mid = javaenv->GetMethodID( bbc, "asFloatBuffer", "()Ljava/nio/FloatBuffer;" );
				if( mid != NULL ) lref = javaenv->CallObjectMethod( dbb, mid );
			} else if( data->type == 32 ) {
				jmethodID mid = javaenv->GetMethodID( bbc, "asIntBuffer", "()Ljava/nio/IntBuffer;" );
				if( mid != NULL ) lref = javaenv->CallObjectMethod( dbb, mid );
			}

			jobject gref = javaenv->NewGlobalRef( lref );
			javaenv->DeleteLocalRef( lref );

			return (long)gref;
		}
	}
	return current;
}
#endif

#ifdef GL
#ifdef SDL
SDL_Surface *screen;
JNIEXPORT int render( int w ) {
	int h = data.length/w;

	if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
	    fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
	    exit(1);
	}
	screen = SDL_SetVideoMode(w, h, 32, SDL_SWSURFACE);
	while (1) {
		if (SDL_MUSTLOCK(screen))
		    if (SDL_LockSurface(screen) < 0)
		      return 0;

		//int tick = SDL_GetTicks();
		//int i, j, yofs, ofs;
		//yofs = 0;
		/*for (i = 0; i < 480; i++) {
			for (j = 0, ofs = yofs; j < 640; j++, ofs++) {
				((unsigned int*)screen->pixels)[ofs] = get;
			}
			yofs += screen->pitch / 4;
		}*/
		memcpy( screen->pixels, (const void*)data.buffer, sizeof( unsigned int )*min( data.length, screen->w*screen->h ) );

		if (SDL_MUSTLOCK(screen))
			SDL_UnlockSurface(screen);

		SDL_UpdateRect(screen, 0, 0, w, h);

	    SDL_Event event;
	    while (SDL_PollEvent(&event)) {
	    	switch (event.type) {
	    		case SDL_KEYDOWN:
	    			SDL_Quit();
	    			break;
	    		case SDL_KEYUP:
	    			if (event.key.keysym.sym == SDLK_ESCAPE) {
	    				SDL_Quit();
	    				return 0;
	    			}
	    			break;
	    		case SDL_QUIT:
	    			SDL_Quit();
	    			return(0);
	    	}
		}
	}

	return current;
}
#endif
#endif

JNIEXPORT int setptr( int byteoffset ) {
	*((simlab*)(data.buffer+byteoffset)) = data;

	return current;
}

JNIEXPORT int getptr( int byteoffset ) {
	garbage();
	data = *((simlab*)(data.buffer+byteoffset));

	return current;
}

JNIEXPORT int welcome() {
	simlab str;
	str.buffer =  (long)"Welcome to SimLab 2.0";
	echo( str );
	return 0;
}

JNIEXPORT int back() {
	data = prev;
	return current;
}

JNIEXPORT int last() {
	return back();
}

JNIEXPORT int garbage() {
	/*if( prev.buffer != 0 ) {
		std::map<std::string,simlab>::iterator it = retlib.begin();
		while( it != retlib.end() ) {
			if( prev.buffer >= it->second.buffer && prev.buffer <= it->second.buffer+bytelength( it->second.type, it->second.length ) ) break;
			it++;
		}
		if( it == retlib.end() ) {
			free( (void*)prev.buffer );
			prev.buffer = 0;
		}
	}*/
	prev = data;

	return current;
}

JNIEXPORT int view( simlab lstart, simlab lsize, simlab ltype ) {
	int start = lstart.buffer;
	int typ = ltype.buffer;
	int size = lsize.buffer;
	if( typ == 0 ) {
		typ = data.type;
		if( size == 0 ) {
			size = data.length-start;
		}
	}

	data.type = typ;
	data.length = size;
	data.buffer = data.buffer+bytelength(typ,start);

	return current;
}

JNIEXPORT int create( int type, ... ) {
	va_list argptr;
	va_start(argptr, type);
	int total = 1;
	int val = 0;

	while( (val = va_arg( argptr, int )) != 0 ) {
		total *= val;
	}

	va_end( argptr );

	garbage();
	data.buffer = (long)malloc( bytelength( type, total ) );
	data.length = total;
	data.type = type;

	//java = 0;

	return current;
}

JNIEXPORT int ptr() {
	data.type = 32;
	data.length = 0;

	return current;
}

JNIEXPORT int wrt( char* str, char* fname, char* fo ) {
	FILE* f;
	if( fname == NULL ) f = stdout;
	else {
		if( fo == NULL ) fo = "w";
		f = fopen( fname, fo );
	}
	fprintf( f, "%s\n", str );
	if( f != stdout ) fclose(f);

	return current;
}

JNIEXPORT int change( int type, int size ) {


	return current;
}

JNIEXPORT int echo( simlab str, ... ) {
	char* buffer = (char*)str.buffer;
	if( buffer != NULL ) prnt( "%s\n", buffer );
	else prnt( "%s\n", "jo" );

	return 1;
}

		/*char* here = (char*)&passnext;
		here += bytesize;
		int value = 0;
		memcpy( here, &value, sizeof(int) );
		passcurr = (long)&passnext;
		int ret = func( passnext );
		return ret;*/

JNIEXPORT int call( int fnc, ... ) {
	passcurr += sizeof(int);
	int (*func)( ... );
	func = (int (*)(...))fnc;
	return func( *(passa<11>*)(passcurr) )+1;
}

JNIEXPORT int pair( int fnc1, int fnc2, ... ) {
	int passold = passcurr;
	call( fnc1, passcurr );
	return call( fnc2, passcurr )+(passcurr-passold);
}

JNIEXPORT int resize( simlab len ) {
	int bytelen = bytelength( data.type, len.buffer );
	data.length = len.buffer;
	data.buffer = (long)realloc( (void*)data.buffer, bytelen );

	return 1;
}

JNIEXPORT int printlen() {
	prnt( "%d\n", data.length );

	return 0;
}

JNIEXPORT int printtype() {
	prnt( "%d\n", data.type );

	return 0;
}

#ifdef JAVA
JNIEXPORT int printsl( simlab sl ) {
	char* format = "%f\t";
	char* end = NULL;
	if( format == NULL ) {
		char* c = (char*)sl.buffer;
		for( int i = 0; i < sl.length; i++ ) {
			putchar( c[i] );
		}
		putchar( '\n' );
	} else {
		if( end == NULL ) end = "\n";
		jprintf( "%d\n", sl.type );
		if( sl.type == 66 ) {
			if( format[1] == 'e' ) t_print<double,double>( format, end, (double*)sl.buffer, sl.length, 0,0,0 );
			else if( format[1] == 'd' ) t_print<double,int>( format, end, (double*)sl.buffer, sl.length, 0,0,0 );
			else if( format[1] == 'f' ) t_print<double,float>( format, end, (double*)sl.buffer, sl.length, 0,0,0 );
		} else if( sl.type == 32 ) {
			if( format[1] == 'e' ) t_print<int,double>( format, end, (int*)sl.buffer, sl.length, 0,0,0 );
			else if( format[1] == 'd' ) t_print<int,int>( format, end, (int*)sl.buffer, sl.length, 0,0,0 );
		} else if( sl.type == 8 ) {
			if( format[1] == 'e' ) t_print<char,double>( format, end, (char*)sl.buffer, sl.length, 0,0,0 );
			else if( format[1] == 'd' ) t_print<char,int>( format, end, (char*)sl.buffer, sl.length, 0,0,0 );
		}
	}

	return current;
}
#endif

JNIEXPORT int fetchlen() {
	data.buffer = data.length;
	data.length = 0;
	data.type = 32;
	return current;
}

JNIEXPORT int sqr() {
	if( data.length == 0 ) {
		if( data.type == 32 ) {
			t_sqr( (unsigned int*)&data.buffer, 1 );
		} else if( data.type == 34 ) {
			t_sqr( (float*)&data.buffer, 1 );
		}
	} else if( data.type == 66 ) t_sqr( (double*)data.buffer, data.length );
	else if( data.type == 34 ) t_sqr( (float*)data.buffer, data.length );
	else if( data.type == 32 ) t_sqr( (int*)data.buffer, data.length );
	else if( data.type == 8 ) t_sqr( (char*)data.buffer, data.length );

	return 0;
}

JNIEXPORT int expn( simlab value ) {
	if( data.length == 0 ) {
		if( data.type == 32 ) {
			if( value.length == 0 ) {
				if( value.type == 34 ) t_exp( (unsigned int*)&data.buffer, 1, (float*)&value.buffer, 1 );
				if( value.type == 32 || value.type == 0 ) t_exp( (unsigned int*)&data.buffer, 1, (int*)&value.buffer, 1 );
			} else {
				if( value.type == 66 ) t_exp( (unsigned int*)&data.buffer, data.length, (double*)value.buffer, value.length );
				if( value.type == 34 ) t_exp( (unsigned int*)&data.buffer, data.length, (float*)value.buffer, value.length );
				if( value.type == 32 ) t_exp( (unsigned int*)&data.buffer, data.length, (int*)value.buffer, value.length );
				if( value.type == 8 ) t_exp( (unsigned int*)&data.buffer, data.length, (char*)value.buffer, value.length );
			}
		} else if( data.type == 34 ) {
			if( value.length == 0 ) {
				if( value.type == 34 ) t_exp( (float*)&data.buffer, 1, (float*)&value.buffer, 1 );
				if( value.type == 32 || value.type == 0 ) t_exp( (float*)&data.buffer, 1, (int*)&value.buffer, 1 );
			} else {
				if( value.type == 66 ) t_exp( (float*)&data.buffer, data.length, (double*)value.buffer, value.length );
				if( value.type == 34 ) t_exp( (float*)&data.buffer, data.length, (float*)value.buffer, value.length );
				if( value.type == 32 ) t_exp( (float*)&data.buffer, data.length, (int*)value.buffer, value.length );
				if( value.type == 8 ) t_exp( (float*)&data.buffer, data.length, (char*)value.buffer, value.length );
			}
		}
	} else {
		if( data.type == 66 ) {
			if( value.length == 0 ) {
				if( value.type == 34 ) t_exp( (double*)data.buffer, data.length, (float*)&value.buffer, 1 );
				if( value.type == 32 || value.type == 0 ) t_exp( (double*)data.buffer, data.length, (int*)&value.buffer, 1 );
			} else {
				if( value.type == 66 ) t_exp( (double*)data.buffer, data.length, (double*)value.buffer, value.length );
				if( value.type == 34 ) t_exp( (double*)data.buffer, data.length, (float*)value.buffer, value.length );
				if( value.type == 32 ) t_exp( (double*)data.buffer, data.length, (int*)value.buffer, value.length );
				if( value.type == 8 ) t_exp( (double*)data.buffer, data.length, (char*)value.buffer, value.length );
			}
		} else if( data.type == 32 ) {
			if( value.length == 0 ) {
				if( value.type == 34 ) t_exp( (int*)data.buffer, data.length, (float*)&value.buffer, 1 );
				if( value.type == 32 || value.type == 0 ) t_exp( (int*)data.buffer, data.length, (int*)&value.buffer, 1 );
			} else {
				if( value.type == 66 ) t_exp( (int*)data.buffer, data.length, (double*)value.buffer, value.length );
				if( value.type == 34 ) t_exp( (int*)data.buffer, data.length, (float*)value.buffer, value.length );
				if( value.type == 32 ) t_exp( (int*)data.buffer, data.length, (int*)value.buffer, value.length );
				if( value.type == 8 ) t_exp( (int*)data.buffer, data.length, (char*)value.buffer, value.length );
			}
		} else if( data.type == 8 ) {
			if( value.length == 0 ) {
				if( value.type == 34 ) t_exp( (unsigned char*)data.buffer, data.length, (float*)&value.buffer, 1 );
				if( value.type == 32 || value.type == 0 ) t_exp( (unsigned char*)data.buffer, data.length, (int*)&value.buffer, 1 );
			} else {
				if( value.type == 66 ) t_exp( (unsigned char*)data.buffer, data.length, (double*)value.buffer, value.length );
				if( value.type == 34 ) t_exp( (unsigned char*)data.buffer, data.length, (float*)value.buffer, value.length );
				if( value.type == 32 ) t_exp( (unsigned char*)data.buffer, data.length, (int*)value.buffer, value.length );
				if( value.type == 8 ) t_exp( (unsigned char*)data.buffer, data.length, (char*)value.buffer, value.length );
			}
		}
	}
	return current;
}

JNIEXPORT int powr( simlab value ) {
	if( data.length == 0 ) {
		if( data.type == 32 ) {
			if( value.length == 0 ) {
				if( value.type == 34 ) t_pow( (unsigned int*)&data.buffer, 1, (float*)&value.buffer, 1 );
				if( value.type == 32 || value.type == 0 ) t_pow( (unsigned int*)&data.buffer, 1, (int*)&value.buffer, 1 );
			} else {
				if( value.type == 66 ) t_pow( (unsigned int*)&data.buffer, data.length, (double*)value.buffer, value.length );
				if( value.type == 34 ) t_pow( (unsigned int*)&data.buffer, data.length, (float*)value.buffer, value.length );
				if( value.type == 32 ) t_pow( (unsigned int*)&data.buffer, data.length, (int*)value.buffer, value.length );
				if( value.type == 8 ) t_pow( (unsigned int*)&data.buffer, data.length, (char*)value.buffer, value.length );
			}
		} else if( data.type == 34 ) {
			if( value.length == 0 ) {
				if( value.type == 34 ) t_pow( (float*)&data.buffer, 1, (float*)&value.buffer, 1 );
				if( value.type == 32 || value.type == 0 ) t_pow( (float*)&data.buffer, 1, (int*)&value.buffer, 1 );
			} else {
				if( value.type == 66 ) t_pow( (float*)&data.buffer, data.length, (double*)value.buffer, value.length );
				if( value.type == 34 ) t_pow( (float*)&data.buffer, data.length, (float*)value.buffer, value.length );
				if( value.type == 32 ) t_pow( (float*)&data.buffer, data.length, (int*)value.buffer, value.length );
				if( value.type == 8 ) t_pow( (float*)&data.buffer, data.length, (char*)value.buffer, value.length );
			}
		}
	} else {
		if( data.type == 66 ) {
			if( value.length == 0 ) {
				if( value.type == 34 ) t_pow( (double*)data.buffer, data.length, (float*)&value.buffer, 1 );
				if( value.type == 32 || value.type == 0 ) t_pow( (double*)data.buffer, data.length, (int*)&value.buffer, 1 );
			} else {
				if( value.type == 66 ) t_pow( (double*)data.buffer, data.length, (double*)value.buffer, value.length );
				if( value.type == 34 ) t_pow( (double*)data.buffer, data.length, (float*)value.buffer, value.length );
				if( value.type == 32 ) t_pow( (double*)data.buffer, data.length, (int*)value.buffer, value.length );
				if( value.type == 8 ) t_pow( (double*)data.buffer, data.length, (char*)value.buffer, value.length );
			}
		} else if( data.type == 32 ) {
			if( value.length == 0 ) {
				if( value.type == 34 ) t_pow( (int*)data.buffer, data.length, (float*)&value.buffer, 1 );
				if( value.type == 32 || value.type == 0 ) t_pow( (int*)data.buffer, data.length, (int*)&value.buffer, 1 );
			} else {
				if( value.type == 66 ) t_pow( (int*)data.buffer, data.length, (double*)value.buffer, value.length );
				if( value.type == 34 ) t_pow( (int*)data.buffer, data.length, (float*)value.buffer, value.length );
				if( value.type == 32 ) t_pow( (int*)data.buffer, data.length, (int*)value.buffer, value.length );
				if( value.type == 8 ) t_pow( (int*)data.buffer, data.length, (char*)value.buffer, value.length );
			}
		} else if( data.type == 8 ) {
			if( value.length == 0 ) {
				if( value.type == 34 ) t_pow( (unsigned char*)data.buffer, data.length, (float*)&value.buffer, 1 );
				if( value.type == 32 || value.type == 0 ) t_pow( (unsigned char*)data.buffer, data.length, (int*)&value.buffer, 1 );
			} else {
				if( value.type == 66 ) t_pow( (unsigned char*)data.buffer, data.length, (double*)value.buffer, value.length );
				if( value.type == 34 ) t_pow( (unsigned char*)data.buffer, data.length, (float*)value.buffer, value.length );
				if( value.type == 32 ) t_pow( (unsigned char*)data.buffer, data.length, (int*)value.buffer, value.length );
				if( value.type == 8 ) t_pow( (unsigned char*)data.buffer, data.length, (char*)value.buffer, value.length );
			}
		}
	}
	return current;
}

JNIEXPORT int null() {
	//if( prev.length > 0 && prev.buffer != 0 ) {
		//printf( "%d %d\n", prev.length, prev.buffer );
		//free( (void*)prev.buffer );
	//}
	//prev = data;
	data.buffer = 0;
	return 0;
}

JNIEXPORT int len( simlab len ) {
	data.length = len.buffer;
	return current;
}

JNIEXPORT int type( simlab type ) {
	//int oldbytelen = bytelength( data.type, data.length );
	int newtype;
	int oldtype;

	if( type.buffer < 8 ) newtype = type.buffer;
	else newtype = (type.buffer/8)*8;
	if( data.type < 8 ) oldtype = data.type;
	else oldtype = (data.type/8)*8;

	//int newbytelen = bytelength( data.type, data.length );
	data.length = (long)(((long)data.length*oldtype)/(long)newtype);

	data.type = type.buffer;

	return 1;
}

JNIEXPORT int gettype() {
	data.buffer = data.type;
	data.length = 0;
	data.type = 32;
	return current;
}

#ifdef HILDON
/* Backing pixmap for drawing area */
static GdkPixmap *pixmap = NULL;

/* Redraw the screen from the backing pixmap */
static gboolean expose_event( GtkWidget *widget, GdkEventExpose *event ) {
  gdk_draw_drawable(widget->window,
    widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
    pixmap,
    event->area.x, event->area.y,
    event->area.x, event->area.y,
    event->area.width, event->area.height);

  return FALSE;
}

/* Create a new backing pixmap of the appropriate size */
static gboolean configure_event( GtkWidget *widget, GdkEventConfigure *event ) {
  if (pixmap)
    g_object_unref(pixmap);

  pixmap = gdk_pixmap_new(widget->window,
  widget->allocation.width,
  widget->allocation.height,
  -1);
  gdk_draw_rectangle (pixmap,
      widget->style->white_gc,
      TRUE,
      0, 0,
      widget->allocation.width,
      widget->allocation.height);

  return TRUE;
}

int i = 0;
gint timeout_callback(gpointer data) {
	GtkWidget *widget = (GtkWidget*)data;
	gdk_draw_rectangle (pixmap,
      widget->style->black_gc,
      TRUE,
      100, (i++)*5,8,8);
}

JNIEXPORT int hildon() {
    HildonProgram *program;
    HildonWindow *window;

    /* Initialize the GTK. */
    /* Create the hildon program and setup the title */
    program = HILDON_PROGRAM(hildon_program_get_instance());
    g_set_application_name("Hello maemo!");

    /* Create HildonWindow and set it to HildonProgram */
    window = HILDON_WINDOW(hildon_window_new());
    hildon_program_add_window(program, window);

	GtkWidget* drawing_area = gtk_drawing_area_new();
    /* Create button and add it to main view */
    //button = gtk_button_new_with_label("Hello!");
    gtk_container_add(GTK_CONTAINER(window), drawing_area);

	gtk_signal_connect (GTK_OBJECT (drawing_area), "expose_event", (GtkSignalFunc) expose_event, NULL);
	gtk_signal_connect (GTK_OBJECT(drawing_area),"configure_event", (GtkSignalFunc)configure_event, NULL);

    /* Connect signal to X in the upper corner */
    g_signal_connect(G_OBJECT(window), "delete_event",
	G_CALLBACK(gtk_main_quit), NULL);

	g_timeout_add(1000,timeout_callback,drawing_area);

    /* Begin the main application */
    gtk_widget_show_all(GTK_WIDGET(window));
    gtk_main();

    /* Exit */
    return 0;
}
#endif

#ifdef GL
JNIEXPORT int setnormalbuffer( simlab chunk, simlab stride ) {
	glEnableClientState( GL_NORMAL_ARRAY );
	if( data.type == 66 ) glNormalPointer( GL_DOUBLE, stride.buffer, (GLvoid*)data.buffer );

	return current;
}

JNIEXPORT int setcolorbuffer( simlab chunk, simlab stride ) {
	glEnableClientState( GL_COLOR_ARRAY );
	if( data.type == 66 ) glColorPointer( chunk.buffer, GL_DOUBLE, stride.buffer, (GLvoid*)data.buffer );

	return current;
}

double d_data[] = {0.5,0.5,-1.0,-0.5,-0.5,-1.0,0.5,-0.5,1.0,-0.5,0.5,1.0};
JNIEXPORT int setvertexbuffer( simlab chunk, simlab stride ) {
	glEnableClientState( GL_VERTEX_ARRAY );
	if( data.type == 66 ) {
		printf("hoho\n");
		glVertexPointer( 3, GL_DOUBLE, 0, d_data );
		//glVertexPointer( chunk.buffer, GL_DOUBLE, stride.buffer, (GLvoid*)data.buffer );
	}

	return current;
}

JNIEXPORT int glclear() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	return current;
}

JNIEXPORT int gltest() {
	glColor3f( 1.0f, 1.0f, 1.0f );

	glBegin( GL_TRIANGLES );
	glVertex2d( 0.0, 1.0 );
	glVertex2d( 1.0, -1.0 );
	glVertex2d( -1.0, -1.0 );
	glVertex2d( 0.0, 1.0 );
	glVertex2d( -1.0, -1.0 );
	glVertex2d( 1.0, -1.0 );
	glEnd();

	return current;
}

GLuint texture;
JNIEXPORT int gldraw( simlab what, simlab chunk ) { //, simlab start, simlab end ) {
	glColor3f( 1.0f, 1.0f, 1.0f );
	if( data.length >= chunk.buffer ) {
		//glEnableClientState( GL_VERTEX_ARRAY );
		//glVertexPointer( chunk.buffer, GL_DOUBLE, 0, (GLvoid*)data.buffer );
		glBindTexture( GL_TEXTURE_2D, texture );
		glInterleavedArrays( GL_V3F, 0, (GLvoid*)data.buffer );
		glDrawArrays( what.buffer, 0, data.length/chunk.buffer ); //data.length/3 ); //start.buffer, end.buffer );
	}
	return current;
}

JNIEXPORT int integer( int i ) {
	return i;
}

JNIEXPORT int gllightpos( simlab pos, simlab number ) {
	glEnable( GL_LIGHTING );
	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	int lightNumber = GL_LIGHT0+number.buffer;
	glEnable( lightNumber );
	if( pos.type == 34 ) glLightfv(lightNumber, GL_POSITION, (GLfloat*)pos.buffer);

	return current;
}

JNIEXPORT int gldiffusecolor( simlab dff, simlab number ) {
	glEnable( GL_LIGHTING );
	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	int lightNumber = GL_LIGHT0+number.buffer;
	glEnable( lightNumber );
	if( dff.type == 34 ) glLightfv(lightNumber, GL_AMBIENT, (GLfloat*)dff.buffer);

	return current;
}

JNIEXPORT int glambientcolor( simlab amb, simlab number ) {
	glEnable( GL_LIGHTING );
	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	int lightNumber = GL_LIGHT0+number.buffer;
	glEnable( lightNumber );
	if( amb.type == 34 ) glLightfv(lightNumber, GL_AMBIENT, (GLfloat*)amb.buffer);

	return current;
}

JNIEXPORT int gllightcolor( simlab amb, simlab dff, simlab spc ) {
	glEnable( GL_LIGHTING );
	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	int lightNumber = GL_LIGHT0;//+current;
	glEnable( lightNumber );
	if( amb.type == 34 ) glLightfv(lightNumber, GL_AMBIENT, (GLfloat*)amb.buffer);
	if( dff.type == 34 ) glLightfv(lightNumber, GL_DIFFUSE, (GLfloat*)dff.buffer);
	if( spc.type == 34 ) glLightfv(lightNumber, GL_SPECULAR, (GLfloat*)spc.buffer);

	return current;
}

JNIEXPORT int gllightdirection( simlab direction, simlab number ) {
	glEnable( GL_LIGHTING );
	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	int lightNumber = GL_LIGHT0+number.buffer;
	glEnable( lightNumber );
	if( direction.type == 34 ) glLightfv(lightNumber, GL_SPOT_DIRECTION, (GLfloat*)direction.buffer);

	return current;
}

JNIEXPORT int gllightspot( simlab direction, simlab exponent, simlab cutoff ) {
	glEnable( GL_LIGHTING );
	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	int lightNumber = GL_LIGHT0+current;
	glEnable( lightNumber );
	if( direction.type == 34 ) glLightfv(lightNumber, GL_SPOT_DIRECTION, (GLfloat*)direction.buffer);
	if( exponent.type == 34 ) glLightfv(lightNumber, GL_SPOT_EXPONENT, (GLfloat*)exponent.buffer);
	if( cutoff.type == 34 ) glLightfv(lightNumber, GL_SPOT_CUTOFF, (GLfloat*)cutoff.buffer);

	return current;
}

JNIEXPORT int gllightattenuation( simlab* constant, simlab* linear, simlab* quadratic ) {
	glEnable( GL_LIGHTING );
	glShadeModel(GL_SMOOTH);
	glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );

	int lightNumber = GL_LIGHT0+current;
	glEnable( lightNumber );
	if( constant != NULL && constant->type == 34 ) glLightfv(lightNumber, GL_CONSTANT_ATTENUATION, (GLfloat*)constant->buffer);
	if( linear != NULL && linear->type == 34 ) glLightfv(lightNumber, GL_LINEAR_ATTENUATION, (GLfloat*)linear->buffer);
	if( quadratic != NULL && quadratic->type == 34 ) glLightfv(lightNumber, GL_QUADRATIC_ATTENUATION, (GLfloat*)quadratic->buffer);

	return current;
}

JNIEXPORT int glinit() {
	//glEnable( GL_LIGHTING );
	//glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	//glEnable(GL_LIGHT1);

	/*GLfloat ambientLight[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	GLfloat diffuseLight[] = { 0.8f, 0.8f, 0.8, 1.0f };
	GLfloat specularLight[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	GLfloat position[] = { -1.5f, 1.0f, -4.0f, 1.0f };

	glLightfv(GL_LIGHT1, GL_AMBIENT, ambientLight);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuseLight);
	glLightfv(GL_LIGHT1, GL_SPECULAR, specularLight);
	glLightfv(GL_LIGHT1, GL_POSITION, position);*/

	return current;
}

JNIEXPORT int gltranslate( simlab sx, simlab sy, simlab sz ) {
	double x = *(float*)&sx.buffer;
	double y = *(float*)&sy.buffer;
	double z = *(float*)&sz.buffer;
	glTranslated( x,y,z );

	return current;
}

JNIEXPORT int glrotate( double t, double x, double y, double z ) {
	glRotated( t, x, y, z );

	return current;
}

JNIEXPORT int glproject() {
	glMatrixMode( GL_PROJECTION );

	return current;
}

JNIEXPORT int glmodel() {
	glMatrixMode( GL_MODELVIEW );

	return current;
}

JNIEXPORT int gldumpmatrix( simlab matrix ) {
	//glGetDoublev( GL_PROJECTION_MATRIX, n );
	if( data.length == 16 ) {
		if( data.type == 66 ) glGetDoublev( matrix.buffer, (double*)data.buffer );
		else if( data.type == 34 ) glGetFloatv( matrix.buffer, (float*)data.buffer );
	}

	return current;
}

JNIEXPORT int glloadmatrix() {
	if( data.length == 16 ) {
		if( data.type == 66 ) {
			glLoadMatrixd( (double*)data.buffer );
		} else if( data.type == 34 ) {
			glLoadMatrixf( (float*)data.buffer );
		}
	}

	return current;
}

JNIEXPORT int glidentity() {
	glLoadIdentity();

	return current;
}

JNIEXPORT int glfrustum() {
	double aspect = 4.0/3.0;
	glFrustum (-aspect, aspect, -1.0, 1.0, 1.0, 5000.0);

	return current;
}

JNIEXPORT int glresize( simlab width, simlab height ) {
	float wid = width.type == 32 ? (float)width.buffer : *((float*)&width.buffer);
	float hgt = height.type == 32 ? (float)height.buffer : *((float*)&height.buffer);

	float fAspect = wid / hgt;
	glViewport(0, 0, (long)wid, (long)hgt);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, fAspect, 1.0f, 10000.0f);
	glTranslated( 0.0,0.0,-1.0 );
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
	//glClearDepth(1.0f);
	//glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	return current;
}

JNIEXPORT int glTexture( simlab chunk ) {
	int texWidth = chunk.buffer;
	int texHeight = data.length/(texWidth*4);
	if( texWidth < 0 ) {
		texHeight = -texWidth;
		texWidth = data.length/texHeight;
	}

	glEnable( GL_TEXTURE_2D );
	glGenTextures( 1, &texture );
	glBindTexture( GL_TEXTURE_2D, texture );

	int wrap = 0;
	//glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	//glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	//glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap ? GL_REPEAT : GL_CLAMP );
	//glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap ? GL_REPEAT : GL_CLAMP );

	if( data.type == 34 ) {
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA, GL_FLOAT, (void*)data.buffer );
	} else if( data.type == 32 ) {
		printf("imhere %d %d\n", texWidth, texHeight);
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA, GL_INT, (void*)data.buffer );
	} else if( data.type == 16 ) {
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA, GL_SHORT, (void*)data.buffer );
	}

	return current;
}
#endif

JNIEXPORT int var() {
	std::map<std::string,simlab>::iterator it = retlib.begin();
	while( it != retlib.end() ) {
		printf( "%s %d %d {%d}\n", it->first.c_str(), it->second.type, it->second.length, it->second.buffer );
		it++;
	}
	return 0;
}

/*JNIEXPORT int sphere() {
	SphereObject	*sphereObject = new SphereObject();
	//SphereObject	sphereObject();

	return (long)sphereObject;
}*/

JNIEXPORT int show( char* name ) {
	return current;
}

JNIEXPORT int zero() {
	int len = bytelength( data.type, data.length );
	memset( (void*)data.buffer, 0, len );

	return current;
}

JNIEXPORT int vector( int type, int length ) {
	data.type = type;
	data.length = length;
	data.buffer = (long)malloc( bytelength(type,length) );

	return current;
}

JNIEXPORT int copy( simlab cop ) {
	if( cop.type < 0 ) {
		if( cop.type == -66 ) t_copy<c_simlab<double> & >( (c_simlab<double> &)cop.buffer, cop.length );
	} else {
		if( cop.type == 66 ) {
			t_copy( (double*)cop.buffer, cop.length );
		} else if( cop.type == 32 ) {
			t_copy( (float*)cop.buffer, cop.length );
		} else if( cop.type == 16 ) {
			t_copy( (short*)cop.buffer, cop.length );
		} else if( cop.type == 8 ) {
			t_copy( (unsigned char*)cop.buffer, cop.length );
		}
	}
	return 1;
}

JNIEXPORT int cast( simlab type ) {
	simlab cop = data;
	data.buffer = (long)malloc( bytelength( type.buffer, data.length ) );
	data.type = type.buffer;
	copy( cop );
	//garbage();

	return current;
}

JNIEXPORT int clnn() {
	int size = bytelength( data.type, data.length );
	void* buffer = malloc( size );
	memcpy( buffer, (void*)data.buffer, size );
	garbage();
	data.buffer = (long)buffer;

	return current;
}

JNIEXPORT int get( simlab wh ) {
	if( data.type == 66 ) {
		if( wh.length == 0 ) {
			if( wh.type == 32 ) t_get( (double*)data.buffer, data.length, &wh.buffer, 1 );
			else if( wh.type == 34 ) t_get( (double*)data.buffer, data.length, (float*)&wh.buffer, 1 );
		} else if( wh.type == 66 ) t_get( (double*)data.buffer, data.length, (double*)wh.buffer, wh.length );
		else if( wh.type == 34 ) t_get( (double*)data.buffer, data.length, (float*)wh.buffer, wh.length );
		else if( wh.type == 32 ) t_get( (double*)data.buffer, data.length, (int*)wh.buffer, wh.length );
	}

	return current;
}

JNIEXPORT int newadd( simlab value, simlab where ) {
	//printf( "%s\n", typeid(value).name() );
	//if( data.type == 66 ) c_set( reinterpret_cast<cdata<double> > data, value, where );
	//if( data.type == 32 ) c_set( data, value, where );

	return 2;
}

JNIEXPORT int oldset( simlab value, simlab wh ) {
	if( data.length == 0 ) {
		if( data.type == 34 ) t_set( (float*)&data.buffer, 1, wh, value );
		else if( data.type == 32 ) t_set( (int*)&data.buffer, 1, wh, value );
	} else {
		if( data.type == 66 ) t_set( (double*)data.buffer, data.length, wh, value );
		else if( data.type == 34 ) t_set( (float*)data.buffer, data.length, wh, value );
		else if( data.type == 32 ) t_set( (int*)data.buffer, data.length, wh, value );
		else if( data.type == 16 ) t_set( (short*)data.buffer, data.length, wh, value );
		else if( data.type == 8 ) t_set( (char*)data.buffer, data.length, wh, value );
	}

	//if( wh.type == 0 ) return 1;
	return 2;
}

JNIEXPORT int draw( simlab value, simlab chunk, simlab x, simlab y ) {


	return current;
}

/*JNIEXPORT int set( simlab value, simlab wh ) {
	int* nil = NULL;
	if( data.type == 66 ) {
		if( wh.type == 0 ) {
			if( value.length == -1 ) {
				if( value.type == 66 ) {
					t_set( (double*)data.buffer, data.length, nil, 0, *(PseudoBuffer<double>*)value.buffer, ((PseudoBuffer<double>*)value.buffer)->length() );
				} else if( value.type == 34 ) {
					t_set( (double*)data.buffer, data.length, nil, 0, *(PseudoBuffer<float>*)value.buffer, ((PseudoBuffer<double>*)value.buffer)->length() );
				} else if( value.type == 32 ) {
					t_set( (double*)data.buffer, data.length, nil, 0, *(PseudoBuffer<int>*)value.buffer, ((PseudoBuffer<double>*)value.buffer)->length() );
				}
			} else if( value.length == 0 ) {
				if( value.type == 32 ) {
					int* val = (int*)&value.buffer;
					t_set( (double*)data.buffer, data.length, nil, 0, val, 1 );
				} else if( value.type == 34 ) {
					float* val = (float*)&value.buffer;
					t_set( (double*)data.buffer, data.length, nil, 0, val, 1 );
				}
			} else if( value.type == 32 ) {
				t_set( (double*)data.buffer, data.length, nil, 0, *(int**)&value.buffer, value.length );
			} else if( value.type == 34 ) {
				t_set( (double*)data.buffer, data.length, nil, 0, *(float**)&value.buffer, value.length );
			} else if( value.type == 66 ) {
				t_set( (double*)data.buffer, data.length, nil, 0, *(double**)&value.buffer, value.length );
			}
		} else if( wh.length == -1 ) {
			if( value.type == 66 ) {
				double* val = (double*)&value.buffer;
				if( wh.type == 32 ) t_set( (double*)data.buffer, data.length, *(PseudoBuffer<int>*)wh.buffer, ((PseudoBuffer<int>*)wh.buffer)->length(), val, 1 );
				else if( wh.type == 34 ) t_set( (double*)data.buffer, data.length, *(PseudoBuffer<float>*)wh.buffer, ((PseudoBuffer<float>*)wh.buffer)->length(), val, 1 );
				else if( wh.type == 66 ) t_set( (double*)data.buffer, data.length, *(PseudoBuffer<double>*)wh.buffer, ((PseudoBuffer<double>*)wh.buffer)->length(), val, 1 );
			} else if( value.type == 34 ) {
				float* val = (float*)&value.buffer;
				if( wh.type == 32 ) t_set( (double*)data.buffer, data.length, *(PseudoBuffer<int>*)wh.buffer, ((PseudoBuffer<int>*)wh.buffer)->length(), val, 1 );
				else if( wh.type == 34 ) t_set( (double*)data.buffer, data.length, *(PseudoBuffer<float>*)wh.buffer, ((PseudoBuffer<float>*)wh.buffer)->length(), val, 1 );
				else if( wh.type == 66 ) t_set( (double*)data.buffer, data.length, *(PseudoBuffer<double>*)wh.buffer, ((PseudoBuffer<double>*)wh.buffer)->length(), val, 1 );
			} else if( value.type == 32 ) {
				int* val = (int*)&value.buffer;
				if( wh.type == 32 ) t_set( (double*)data.buffer, data.length, *(PseudoBuffer<int>*)wh.buffer, ((PseudoBuffer<int>*)wh.buffer)->length(), val, 1 );
				else if( wh.type == 34 ) t_set( (double*)data.buffer, data.length, *(PseudoBuffer<float>*)wh.buffer, ((PseudoBuffer<float>*)wh.buffer)->length(), val, 1 );
				else if( wh.type == 66 ) t_set( (double*)data.buffer, data.length, *(PseudoBuffer<double>*)wh.buffer, ((PseudoBuffer<double>*)wh.buffer)->length(), val, 1 );
			}
		} else if( wh.length == 0 ) {
			int*	vival = (int*)&value.buffer;
			float*	vfval = (float*)&value.buffer;
			int*	wival = (int*)&wh.buffer;
			float*	wfval = (float*)&wh.buffer;
			if( value.type == 34 ) {
				if( wh.type == 32 ) t_set( (double*)data.buffer, data.length, wival, 1, vfval, 1 );
				else if( wh.type == 34 ) t_set( (double*)data.buffer, data.length, wfval, 1, vfval, 1 );
			} else if( value.type == 32 ) {
				if( wh.type == 32 ) t_set( (double*)data.buffer, data.length, wival, 1, vival, 1 );
				else if( wh.type == 34 ) t_set( (double*)data.buffer, data.length, wfval, 1, vival, 1 );
			}
		} else if( wh.type == 32 ) {
			if( value.length == 0 ) {
				int*	vival = (int*)&value.buffer;
				float*	vfval = (float*)&value.buffer;
				if( value.type == 32 ) t_set( (double*)data.buffer, data.length, *(int**)&wh.buffer, wh.length, vival, 1 );
				else if( value.type == 34 ) t_set( (double*)data.buffer, data.length, *(int**)&wh.buffer, wh.length, vfval, 1 );
			} else if( value.type == 32 ) {
				t_set( (double*)data.buffer, data.length, *(int**)&wh.buffer, wh.length, *(int**)&value.buffer, value.length );
			} else if( value.type == 34 ) {
				t_set( (double*)data.buffer, data.length, *(int**)&wh.buffer, wh.length, *(float**)&value.buffer, value.length );
			} else if( value.type == 66 ) {
				t_set( (double*)data.buffer, data.length, *(int**)&wh.buffer, wh.length, *(double**)&value.buffer, value.length );
			}
		} else if( wh.type == 34 ) {
			if( value.length == 0 ) {
				int*	vival = (int*)&value.buffer;
				float*	vfval = (float*)&value.buffer;
				if( value.type == 32 ) t_set( (double*)data.buffer, data.length, *(float**)&wh.buffer, wh.length, vival, 1 );
				else if( value.type == 34 ) t_set( (double*)data.buffer, data.length, *(float**)&wh.buffer, wh.length, vfval, 1 );
			} else if( value.type == 32 ) {
				t_set( (double*)data.buffer, data.length, *(float**)&wh.buffer, wh.length, *(int**)&value.buffer, value.length );
			} else if( value.type == 34 ) {
				t_set( (double*)data.buffer, data.length, *(float**)&wh.buffer, wh.length, *(float**)&value.buffer, value.length );
			} else if( value.type == 66 ) {
				t_set( (double*)data.buffer, data.length, *(float**)&wh.buffer, wh.length, *(double**)&value.buffer, value.length );
			}
		} else if( wh.type == 66 ) {
			if( value.length == 0 ) {
				int*	vival = (int*)&value.buffer;
				float*	vfval = (float*)&value.buffer;
				if( value.type == 32 ) t_set( (double*)data.buffer, data.length, *(double**)&wh.buffer, wh.length, vival, 1 );
				else if( value.type == 34 ) t_set( (double*)data.buffer, data.length, *(double**)&wh.buffer, wh.length, vfval, 1 );
			} else if( value.type == 32 ) {
				t_set( (double*)data.buffer, data.length, *(double**)&wh.buffer, wh.length, *(int**)&value.buffer, value.length );
			} else if( value.type == 34 ) {
				t_set( (double*)data.buffer, data.length, *(double**)&wh.buffer, wh.length, *(float**)&value.buffer, value.length );
			} else if( value.type == 66 ) {
				t_set( (double*)data.buffer, data.length, *(double**)&wh.buffer, wh.length, *(double**)&value.buffer, value.length );
			}
		}
	} else if( data.type == 34 ) {
		if( wh.type == 0 ) {
			if( value.length == 0 ) {
				int*	vival = (int*)&value.buffer;
				float*	vfval = (float*)&value.buffer;
				if( value.type == 32 ) t_set( (float*)data.buffer, data.length, nil, 0, vival, 1 );
				else if( value.type == 34 ) t_set( (float*)data.buffer, data.length, nil, 0, vfval, 1 );
			} else if( value.type == 32 ) {
				t_set( (float*)data.buffer, data.length, nil, 0, *(int**)&value.buffer, value.length );
			} else if( value.type == 34 ) {
				t_set( (float*)data.buffer, data.length, nil, 0, *(float**)&value.buffer, value.length );
			} else if( value.type == 66 ) {
				t_set( (float*)data.buffer, data.length, nil, 0, *(double**)&value.buffer, value.length );
			}
		} else if( wh.length == 0 ) {
			if( value.type == 66 ) {
				if( wh.type == 32 ) t_set( (float*)data.buffer, data.length, (int*)&wh.buffer, 1, (double*)&value.buffer, 1 );
				else if( wh.type == 34 ) t_set( (float*)data.buffer, data.length, (float*)&wh.buffer, 1, (double*)&value.buffer, 1 );
			} else if( value.type == 34 ) {
				if( wh.type == 32 ) t_set( (float*)data.buffer, data.length, (int*)&wh.buffer, 1, (float*)&value.buffer, 1 );
				else if( wh.type == 34 ) t_set( (float*)data.buffer, data.length, (float*)&wh.buffer, 1, (float*)&value.buffer, 1 );
			} else if( value.type == 32 ) {
				if( wh.type == 32 ) t_set( (float*)data.buffer, data.length, (int*)&wh.buffer, 1, (int*)&value.buffer, 1 );
				else if( wh.type == 34 ) t_set( (float*)data.buffer, data.length, (float*)&wh.buffer, 1, (int*)&value.buffer, 1 );
			}
		} else if( wh.type == 32 ) {
			if( value.length == 0 ) {
				int*	vival = (int*)&value.buffer;
				float*	vfval = (float*)&value.buffer;
				if( value.type == 32 ) t_set( (float*)data.buffer, data.length, *(int**)&wh.buffer, wh.length, vival, 1 );
				else if( value.type == 34 ) t_set( (float*)data.buffer, data.length, *(int**)&wh.buffer, wh.length, vfval, 1 );
			} else if( value.type == 32 ) {
				t_set( (float*)data.buffer, data.length, *(int**)&wh.buffer, wh.length, *(int**)&value.buffer, value.length );
			} else if( value.type == 34 ) {
				t_set( (float*)data.buffer, data.length, *(int**)&wh.buffer, wh.length, *(float**)&value.buffer, value.length );
			}
		} else if( wh.type == 34 ) {
			if( value.length == 0 ) {
				int*	vival = (int*)&value.buffer;
				float*	vfval = (float*)&value.buffer;
				if( value.type == 32 ) t_set( (float*)data.buffer, data.length, *(float**)&wh.buffer, wh.length, vival, 1 );
				else if( value.type == 34 ) t_set( (float*)data.buffer, data.length, *(float**)&wh.buffer, wh.length, vfval, 1 );
			} else if( value.type == 32 ) {
				t_set( (float*)data.buffer, data.length, *(float**)&wh.buffer, wh.length, *(int**)&value.buffer, value.length );
			} else if( value.type == 34 ) {
				t_set( (float*)data.buffer, data.length, *(float**)&wh.buffer, wh.length, *(float**)&value.buffer, value.length );
			}
		} else if( wh.type == 66 ) {
			if( value.length == 0 ) {
				int*	vival = (int*)&value.buffer;
				float*	vfval = (float*)&value.buffer;
				if( value.type == 32 ) t_set( (float*)data.buffer, data.length, *(double**)&wh.buffer, wh.length, vival, 1 );
				else if( value.type == 34 ) t_set( (float*)data.buffer, data.length, *(double**)&wh.buffer, wh.length, vfval, 1 );
			} else if( value.type == 32 ) {
				t_set( (float*)data.buffer, data.length, *(double**)&wh.buffer, wh.length, *(int**)&value.buffer, value.length );
			} else if( value.type == 34 ) {
				t_set( (float*)data.buffer, data.length, *(double**)&wh.buffer, wh.length, *(float**)&value.buffer, value.length );
			}
		}
	} else if( data.type == 32 ) {
		if( wh.type == 0 ) {
			if( value.length == 0 ) {
				int*	vival = (int*)&value.buffer;
				float*	vfval = (float*)&value.buffer;
				if( value.type == 32 ) t_set( (int*)data.buffer, data.length, nil, 0, vival, 1 );
				else if( value.type == 34 ) t_set( (int*)data.buffer, data.length, nil, 0, vfval, 1 );
			} else if( value.type == 32 ) {
				t_set( (int*)data.buffer, data.length, nil, 0, *(int**)&value.buffer, value.length );
			} else if( value.type == 34 ) {
				t_set( (int*)data.buffer, data.length, nil, 0, *(float**)&value.buffer, value.length );
			} else if( value.type == 66 ) {
				t_set( (int*)data.buffer, data.length, nil, 0, *(double**)&value.buffer, value.length );
			}
		} else if( wh.length == 0 ) {
			if( value.type == 66 ) {
				if( wh.type == 32 ) t_set( (int*)data.buffer, data.length, (int*)&wh.buffer, 1, (double*)&value.buffer, 1 );
				else if( wh.type == 34 ) t_set( (int*)data.buffer, data.length, (float*)&wh.buffer, 1, (double*)&value.buffer, 1 );
			} else if( value.type == 34 ) {
				if( wh.type == 32 ) t_set( (int*)data.buffer, data.length, (int*)&wh.buffer, 1, (float*)&value.buffer, 1 );
				else if( wh.type == 34 ) t_set( (int*)data.buffer, data.length, (float*)&wh.buffer, 1, (float*)&value.buffer, 1 );
			} else if( value.type == 32 ) {
				if( wh.type == 32 ) t_set( (int*)data.buffer, data.length, (int*)&wh.buffer, 1, (int*)&value.buffer, 1 );
				else if( wh.type == 34 ) t_set( (int*)data.buffer, data.length, (float*)&wh.buffer, 1, (int*)&value.buffer, 1 );
			}
		} else if( wh.type == 32 ) {
			if( value.length == 0 ) {
				int*	vival = (int*)&value.buffer;
				float*	vfval = (float*)&value.buffer;
				if( value.type == 32 ) t_set( (int*)data.buffer, data.length, *(int**)&wh.buffer, wh.length, vival, 1 );
				else if( value.type == 34 ) t_set( (int*)data.buffer, data.length, *(int**)&wh.buffer, wh.length, vfval, 1 );
			} else if( value.type == 32 ) {
				t_set( (int*)data.buffer, data.length, *(int**)&wh.buffer, wh.length, *(int**)&value.buffer, value.length );
			} else if( value.type == 34 ) {
				t_set( (int*)data.buffer, data.length, *(int**)&wh.buffer, wh.length, *(float**)&value.buffer, value.length );
			}
		} else if( wh.type == 34 ) {
			if( value.length == 0 ) {
				int*	vival = (int*)&value.buffer;
				float*	vfval = (float*)&value.buffer;
				if( value.type == 32 ) t_set( (int*)data.buffer, data.length, *(float**)&wh.buffer, wh.length, vival, 1 );
				else if( value.type == 34 ) t_set( (int*)data.buffer, data.length, *(float**)&wh.buffer, wh.length, vfval, 1 );
			} else if( value.type == 32 ) {
				t_set( (int*)data.buffer, data.length, *(float**)&wh.buffer, wh.length, *(int**)&value.buffer, value.length );
			} else if( value.type == 34 ) {
				t_set( (int*)data.buffer, data.length, *(float**)&wh.buffer, wh.length, *(float**)&value.buffer, value.length );
			}
		} else if( wh.type == 66 ) {
			if( value.length == 0 ) {
				int*	vival = (int*)&value.buffer;
				float*	vfval = (float*)&value.buffer;
				if( value.type == 32 ) t_set( (int*)data.buffer, data.length, *(double**)&wh.buffer, wh.length, vival, 1 );
				else if( value.type == 34 ) t_set( (int*)data.buffer, data.length, *(double**)&wh.buffer, wh.length, vfval, 1 );
			} else if( value.type == 32 ) {
				t_set( (int*)data.buffer, data.length, *(double**)&wh.buffer, wh.length, *(int**)&value.buffer, value.length );
			} else if( value.type == 34 ) {
				t_set( (int*)data.buffer, data.length, *(double**)&wh.buffer, wh.length, *(float**)&value.buffer, value.length );
			}
		}
	}
	return current;
}*/

JNIEXPORT int matrix( int type, int columns, int rows ) {
	return create( type, columns, rows );
}

JNIEXPORT int mod( int value ) {
	simlab* data = (simlab*)current;
	if( data->type == 66 ) return t_mod<double>( data, value );
	return current;
}

JNIEXPORT int intersect( simlab inter ) {
	if( data.type == 66 ) {
		if( inter.type == 66 ) {
			t_intersect( (double*)data.buffer, data.length, (double*)inter.buffer, inter.length );
		}
	}

	return current;
}

JNIEXPORT int intersecindex( simlab* intsct ) {
	simlab* data = (simlab*)current;
	if( data->type == 66 ) {
		if( intsct->type == 66 ) {
			//int count = t_intersectcount( (double*)intsct->buffer, (double*)data->buffer, intsct->length, data->length );
			//return t_wrap( t_intersectindex( (double*)intsct->buffer, (double*)data->buffer, intsct->length, data->length, count ), count );
		}
	}
	return current;
}

JNIEXPORT int identity() {
	double d = sqrt( (double)data.length );
	if( floor( d ) == d ) {
		zero();
		if( data.type == 66 ) t_identity( (double*)data.buffer, data.length );
		if( data.type == 34 ) t_identity( (float*)data.buffer, data.length );
		if( data.type == 33 ) t_identity( (unsigned int*)data.buffer, data.length );
		if( data.type == 32 ) t_identity( (int*)data.buffer, data.length );
		if( data.type == 8 ) t_identity( (int*)data.buffer, data.length );
	}
	return 0;
}

JNIEXPORT int idx() {
	if( data.length == 0 ) t_index<int*,int>( (int*)&data.buffer, 1 );
	else if( data.type < 0 ) {
		if( data.type == -66 ) t_index<c_simlab<double&>&,double>( *(c_simlab<double&>*)data.buffer, data.length );
		else if( data.type == -34 ) t_index<c_simlab<float&>&,float>( *(c_simlab<float&>*)data.buffer, data.length );
		else if( data.type == -32 ) t_index<c_simlab<int&>&,int>( *(c_simlab<int&>*)data.buffer, data.length );
	} else {
		if( data.type == 96 ) t_index<long double*,long double>( (long double*)data.buffer, data.length );
		else if( data.type == 66 ) t_index<double*,double>( (double*)data.buffer, data.length );
		else if( data.type == 34 ) t_index<float*,float>( (float*)data.buffer, data.length );
		//else if( data.type == 33 ) t_index( (unsigned int*)data.buffer, data.length );
		else if( data.type == 32 ) t_index<int*,int>( (int*)data.buffer, data.length );
		else if( data.type == 16 ) t_index<short*,short>( (short*)data.buffer, data.length );
		else if( data.type == 8 ) t_index<char*,char>( (char*)data.buffer, data.length );
	}

	return 0;
}

JNIEXPORT int cl() {
	if( data.type == 66 ) t_ceil( (double*)data.buffer, data.length );
	if( data.type == 34 ) t_ceil( (float*)data.buffer, data.length );
	return 0;
}

JNIEXPORT int flr() {
	if( data.type == 66 ) t_floor( (double*)data.buffer, data.length );
	if( data.type == 34 ) t_floor( (float*)data.buffer, data.length );
	return 0;
}

JNIEXPORT int diff() {
	if( data.type == 66 ) t_diff( (double*)data.buffer, data.length );
	else if( data.type == 34 ) t_diff( (float*)data.buffer, data.length );
	else if( data.type == 32 ) t_diff( (int*)data.buffer, data.length );
	else if( data.type == 16 ) t_diff( (short*)data.buffer, data.length );
	return 0;
}

JNIEXPORT int integ() {
	if( data.type == 66 ) t_integ( (double*)data.buffer, data.length );
	return 0;
}

JNIEXPORT int invert( int chunk ) {
	simlab* data = (simlab*)current;
	if( chunk == 0 ) chunk = data->length;
	if( data->type == 66 ) t_invert( (double*)data->buffer, data->length, chunk );
	else if( data->type == 32 ) t_invert( (int*)data->buffer, data->length, chunk );

	return current;
}

JNIEXPORT int printshape( simlab* shape ) {
	simlab* data = (simlab*)current;
	if( shape != NULL ) {
		if( shape->type == 32 ) {
			//t_printrecursive<double>( data, "%e\t", shape, shape->length-1, 0 );
		}
	} else if( data->type == 66 ) {
		//t_print<double>( data, "%e\t" );
	}

	return current;
}

/*JNIEXPORT int print( double value ) {
	return printf( "%e\n", value );
}*/


JNIEXPORT int html( char* syntax ) {
	if( syntax == NULL ) printf( "<html>" );
	else printf( "<%s>", syntax );
	return current;
}

JNIEXPORT int htmlc( char* syntax ) {
	if( syntax == NULL ) printf( "</html>" );
	else printf( "</%s>", syntax );
	return current;
}

JNIEXPORT int web() {
    printf( "Content-type:text/html\n\n" );
	return 0;
}

JNIEXPORT int commandLoop2( int file ) {
		char	command[256];
		fgets( command, sizeof(command), (FILE*)file );
		char quit[] = "quit";
		while( strncmp( command, quit, sizeof(quit)-1 ) ) {
			//cmd( command );
			fgets( command, sizeof(command), (FILE*)file );
		}
		return current;
}

JNIEXPORT int ret() {
	return printf( "%d\n", current );
}

/*JNIEXPORT int message( char* str ) {
	return printf( "%s\n", str );
}*/

JNIEXPORT int addold( simlab value ) {
	/*if( 1 ) { //value.length == -1 ) {
		c_simlab<float> & st = *((c_simlab<float>*)value.buffer);
		//int val = (long)&st;
		if( value.type == -66 ) t_add<c_simlab<double>& >( *(c_simlab<double>*)value.buffer, data.length );
		else if( value.type == -34 ) {
			printf( "trying float %f\n", st[1] );
			t_add<c_simlab<float>& >( *((c_simlab<float>*)value.buffer), data.length );
		}
	} else if( data.length == 0 ) {
		/*if( data.type == 32 ) {
			if( value.length == 0 ) {
				if( value.type == 34 ) t_add( (unsigned int*)&data.buffer, 1, (float*)&value.buffer, 1 );
				if( value.type == 32 || value.type == 0 ) t_add( (unsigned int*)&data.buffer, 1, (int*)&value.buffer, 1 );
			} else {
				if( value.type == 66 ) t_add( (unsigned int*)&data.buffer, data.length, (double*)value.buffer, value.length );
				if( value.type == 34 ) t_add( (unsigned int*)&data.buffer, data.length, (float*)value.buffer, value.length );
				if( value.type == 32 ) t_add( (unsigned int*)&data.buffer, data.length, (int*)value.buffer, value.length );
				if( value.type == 8 ) t_add( (unsigned int*)&data.buffer, data.length, (char*)value.buffer, value.length );
			}
		} else if( data.type == 34 ) {
			if( value.length == 0 ) {
				if( value.type == 34 ) t_add( (float*)&data.buffer, 1, (float*)&value.buffer, 1 );
				if( value.type == 32 || value.type == 0 ) t_add( (float*)&data.buffer, 1, (int*)&value.buffer, 1 );
			} else {
				if( value.type == 66 ) t_add( (float*)&data.buffer, data.length, (double*)value.buffer, value.length );
				if( value.type == 34 ) t_add( (float*)&data.buffer, data.length, (float*)value.buffer, value.length );
				if( value.type == 32 ) t_add( (float*)&data.buffer, data.length, (int*)value.buffer, value.length );
				if( value.type == 8 ) t_add( (float*)&data.buffer, data.length, (char*)value.buffer, value.length );
			}
		}*
	} else {
		if( data.type == 66 ) {
			if( value.length == 0 ) {
				//if( value.type == 34 ) t_add( (double*)data.buffer, data.length, (float*)&value.buffer, 1 );
				//if( value.type == 32 || value.type == 0 ) t_add( (double*)data.buffer, data.length, (int*)&value.buffer, 1 );
			} else {
				if( value.type == 66 ) t_add<double,double*,double*>( (double*)data.buffer, data.length, (double*)value.buffer, value.length );
				if( value.type == 34 ) t_add<double,double*,float*>( (double*)data.buffer, data.length, (float*)value.buffer, value.length );
				if( value.type == 32 ) t_add<double,double*,int*>( (double*)data.buffer, data.length, (int*)value.buffer, value.length );
				if( value.type == 8 ) t_add<double,double*,char*>( (double*)data.buffer, data.length, (char*)value.buffer, value.length );
			}
		} /*else if( data.type == 34 ) {
			if( value.length == 0 ) {
				if( value.type == 34 ) t_add( (float*)data.buffer, data.length, (float*)&value.buffer, 1 );
				if( value.type == 32 || value.type == 0 ) t_add( (float*)data.buffer, data.length, (int*)&value.buffer, 1 );
			} else {
				if( value.type == 66 ) t_add( (float*)data.buffer, data.length, (double*)value.buffer, value.length );
				if( value.type == 34 ) t_add( (float*)data.buffer, data.length, (float*)value.buffer, value.length );
				if( value.type == 32 ) t_add( (float*)data.buffer, data.length, (int*)value.buffer, value.length );
				if( value.type == 8 ) t_add( (float*)data.buffer, data.length, (char*)value.buffer, value.length );
			}
		} else if( data.type == 32 ) {
			if( value.length == 0 ) {
				if( value.type == 34 ) t_add( (int*)data.buffer, data.length, (float*)&value.buffer, 1 );
				if( value.type == 32 || value.type == 0 ) t_add( (int*)data.buffer, data.length, (int*)&value.buffer, 1 );
			} else {
				if( value.type == 66 ) t_add( (int*)data.buffer, data.length, (double*)value.buffer, value.length );
				if( value.type == 34 ) t_add( (int*)data.buffer, data.length, (float*)value.buffer, value.length );
				if( value.type == 32 ) t_add( (int*)data.buffer, data.length, (int*)value.buffer, value.length );
				if( value.type == 8 ) t_add( (int*)data.buffer, data.length, (char*)value.buffer, value.length );
			}
		} else if( data.type == 8 ) {
			if( value.length == 0 ) {
				if( value.type == 34 ) t_add( (unsigned char*)data.buffer, data.length, (float*)&value.buffer, 1 );
				if( value.type == 32 || value.type == 0 ) t_add( (unsigned char*)data.buffer, data.length, (int*)&value.buffer, 1 );
			} else {
				if( value.type == 66 ) t_add( (unsigned char*)data.buffer, data.length, (double*)value.buffer, value.length );
				if( value.type == 34 ) t_add( (unsigned char*)data.buffer, data.length, (float*)value.buffer, value.length );
				if( value.type == 32 ) t_add( (unsigned char*)data.buffer, data.length, (int*)value.buffer, value.length );
				if( value.type == 8 ) t_add( (unsigned char*)data.buffer, data.length, (char*)value.buffer, value.length );
			}
		}
	}*/
	return 1;
}

JNIEXPORT int mean( simlab l_chunk, simlab l_size ) {
	int chunk = l_chunk.buffer;
	int size = l_size.buffer;

	if( chunk == 0 ) chunk = data.length;
	if( size == 0 ) size = chunk;

	//int dlen = (data.length*(chunk-size+1))/chunk;
	if( data.type == 66 ) t_mean( (double*)data.buffer, data.length, chunk, size );
	else if( data.type == 32 ) t_mean( (int*)data.buffer, data.length, chunk, size );
	else if( data.type == 8 ) t_mean( (unsigned char*)data.buffer, data.length, chunk, size );

	return current;
}

JNIEXPORT int sum( simlab l_chunk, simlab l_size ) {
	int chunk = l_chunk.buffer;
	int size = l_size.buffer;

	if( chunk == 0 ) chunk = data.length;
	if( size == 0 ) size = chunk;

	//int dlen = (data.length*(chunk-size+1))/chunk;
	if( data.type == 66 ) t_sum( (double*)data.buffer, data.length, chunk, size );
	else if( data.type == 32 ) t_sum( (int*)data.buffer, data.length, chunk, size );
	else if( data.type == 8 ) t_sum( (unsigned char*)data.buffer, data.length, chunk, size );

	return current;
}

JNIEXPORT int poly( simlab* pl ) {
	simlab*	data = (simlab*)current;
	if( data->type == 66 ) {
		if( pl->type == 66 ) t_poly( (double*)data->buffer, data->length, (double*)pl->buffer, pl->length );
		else if( pl->type == 32 ) t_poly( (double*)data->buffer, data->length, (int*)pl->buffer, pl->length );
	} else if( data->type == 32 ) {
		if( pl->type == 66 ) t_poly( (int*)data->buffer, data->length, (double*)pl->buffer, pl->length );
		else if( pl->type == 32 ) t_poly( (int*)data->buffer, data->length, (int*)pl->buffer, pl->length );
	}

	return current;
}

JNIEXPORT int ffunc( simlab func, simlab wh ) {
	float (*ffunc)(float);
	ffunc = (float (*)(float))func.buffer;
	if( wh.type == 0 ) {
		if( data.type == 66 ) t_ffunc( (double*)data.buffer, data.length, ffunc, (int*)NULL, 0 );
		else if( data.type == 34 ) t_ffunc( (float*)data.buffer, data.length, ffunc, (int*)NULL, 0 );
		else if( data.type == 32 ) t_ffunc( (int*)data.buffer, data.length, ffunc, (int*)NULL, 0 );
		else if( data.type == 16 ) t_ffunc( (short*)data.buffer, data.length, ffunc, (int*)NULL, 0 );
	} else if( wh.length == 0 ) {
		if( wh.type == 32 ) {
			if( data.type == 66 ) t_ffunc( (double*)data.buffer, data.length, ffunc, &wh.buffer, 1 );
			else if( data.type == 34 ) t_ffunc( (float*)data.buffer, data.length, ffunc, &wh.buffer, 1 );
			else if( data.type == 32 ) t_ffunc( (int*)data.buffer, data.length, ffunc, &wh.buffer, 1 );
			else if( data.type == 16 ) t_ffunc( (short*)data.buffer, data.length, ffunc, &wh.buffer, 1 );
		} else if( wh.type == 34 ) {
			if( data.type == 66 ) t_ffunc( (double*)data.buffer, data.length, ffunc, (float*)&wh.buffer, 1 );
			else if( data.type == 34 ) t_ffunc( (float*)data.buffer, data.length, ffunc, (float*)&wh.buffer, 1 );
			else if( data.type == 32 ) t_ffunc( (int*)data.buffer, data.length, ffunc, (float*)&wh.buffer, 1 );
			else if( data.type == 16 ) t_ffunc( (short*)data.buffer, data.length, ffunc, (float*)&wh.buffer, 1 );
		}
	} else {
		if( wh.type == 32 ) {
			if( data.type == 66 ) t_ffunc( (double*)data.buffer, data.length, ffunc, (int*)wh.buffer, wh.length );
			else if( data.type == 34 ) t_ffunc( (float*)data.buffer, data.length, ffunc, (int*)wh.buffer, wh.length );
			else if( data.type == 32 ) t_ffunc( (int*)data.buffer, data.length, ffunc, (int*)wh.buffer, wh.length );
			else if( data.type == 16 ) t_ffunc( (short*)data.buffer, data.length, ffunc, (int*)wh.buffer, wh.length );
		} else if( wh.type == 34 ) {
			if( data.type == 66 ) t_ffunc( (double*)data.buffer, data.length, ffunc, (float*)wh.buffer, wh.length );
			else if( data.type == 34 ) t_ffunc( (float*)data.buffer, data.length, ffunc, (float*)wh.buffer, wh.length );
			else if( data.type == 32 ) t_ffunc( (int*)data.buffer, data.length, ffunc, (float*)wh.buffer, wh.length );
			else if( data.type == 16 ) t_ffunc( (short*)data.buffer, data.length, ffunc, (float*)wh.buffer, wh.length );
		} else if( wh.type == 66 ) {
			if( data.type == 66 ) t_ffunc( (double*)data.buffer, data.length, ffunc, (double*)wh.buffer, wh.length );
			else if( data.type == 34 ) t_ffunc( (float*)data.buffer, data.length, ffunc, (double*)wh.buffer, wh.length );
			else if( data.type == 32 ) t_ffunc( (int*)data.buffer, data.length, ffunc, (double*)wh.buffer, wh.length );
			else if( data.type == 16 ) t_ffunc( (short*)data.buffer, data.length, ffunc, (double*)wh.buffer, wh.length );
		}
	}

	return current;
}


JNIEXPORT int dfunc( simlab func, simlab wh ) {
	double (*dfunc)(double);
	dfunc = (double (*)(double))func.buffer;
	if( wh.type == 0 ) {
		if( data.type == 66 ) t_dfunc( (double*)data.buffer, data.length, dfunc, (int*)NULL, 0 );
		else if( data.type == 34 ) t_dfunc( (float*)data.buffer, data.length, dfunc, (int*)NULL, 0 );
		else if( data.type == 32 ) t_dfunc( (int*)data.buffer, data.length, dfunc, (int*)NULL, 0 );
		else if( data.type == 16 ) t_dfunc( (short*)data.buffer, data.length, dfunc, (int*)NULL, 0 );
	} else if( wh.length == 0 ) {
		if( wh.type == 32 ) {
			if( data.type == 66 ) t_dfunc( (double*)data.buffer, data.length, dfunc, &wh.buffer, 1 );
			else if( data.type == 34 ) t_dfunc( (float*)data.buffer, data.length, dfunc, &wh.buffer, 1 );
			else if( data.type == 32 ) t_dfunc( (int*)data.buffer, data.length, dfunc, &wh.buffer, 1 );
			else if( data.type == 16 ) t_dfunc( (short*)data.buffer, data.length, dfunc, &wh.buffer, 1 );
		} else if( wh.type == 34 ) {
			if( data.type == 66 ) t_dfunc( (double*)data.buffer, data.length, dfunc, (float*)&wh.buffer, 1 );
			else if( data.type == 34 ) t_dfunc( (float*)data.buffer, data.length, dfunc, (float*)&wh.buffer, 1 );
			else if( data.type == 32 ) t_dfunc( (int*)data.buffer, data.length, dfunc, (float*)&wh.buffer, 1 );
			else if( data.type == 16 ) t_dfunc( (short*)data.buffer, data.length, dfunc, (float*)&wh.buffer, 1 );
		}
	} else {
		if( wh.type == 32 ) {
			if( data.type == 66 ) t_dfunc( (double*)data.buffer, data.length, dfunc, (int*)wh.buffer, wh.length );
			else if( data.type == 34 ) t_dfunc( (float*)data.buffer, data.length, dfunc, (int*)wh.buffer, wh.length );
			else if( data.type == 32 ) t_dfunc( (int*)data.buffer, data.length, dfunc, (int*)wh.buffer, wh.length );
			else if( data.type == 16 ) t_dfunc( (short*)data.buffer, data.length, dfunc, (int*)wh.buffer, wh.length );
		} else if( wh.type == 34 ) {
			if( data.type == 66 ) t_dfunc( (double*)data.buffer, data.length, dfunc, (float*)wh.buffer, wh.length );
			else if( data.type == 34 ) t_dfunc( (float*)data.buffer, data.length, dfunc, (float*)wh.buffer, wh.length );
			else if( data.type == 32 ) t_dfunc( (int*)data.buffer, data.length, dfunc, (float*)wh.buffer, wh.length );
			else if( data.type == 16 ) t_dfunc( (short*)data.buffer, data.length, dfunc, (float*)wh.buffer, wh.length );
		} else if( wh.type == 66 ) {
			if( data.type == 66 ) t_dfunc( (double*)data.buffer, data.length, dfunc, (double*)wh.buffer, wh.length );
			else if( data.type == 34 ) t_dfunc( (float*)data.buffer, data.length, dfunc, (double*)wh.buffer, wh.length );
			else if( data.type == 32 ) t_dfunc( (int*)data.buffer, data.length, dfunc, (double*)wh.buffer, wh.length );
			else if( data.type == 16 ) t_dfunc( (short*)data.buffer, data.length, dfunc, (double*)wh.buffer, wh.length );
		}
	}

	return current;
}

JNIEXPORT int lg10( simlab wh ) {
	simlab func;
	func.type = 32;
	func.length = 0;

	func.buffer = (long)log10;
	dfunc( func, wh );

	return current;
}

JNIEXPORT int lg2( simlab wh ) {
	simlab func;
	func.type = 32;
	func.length = 0;

	func.buffer = (long)log2;
	dfunc( func, wh );

	return current;
}

JNIEXPORT int ln( simlab wh ) {
	simlab func;
	func.type = 32;
	func.length = 0;

	func.buffer = (long)log;
	dfunc( func, wh );

	return current;
}

JNIEXPORT int cosine( simlab wh ) {
	simlab func;
	func.type = 32;
	func.length = 0;

	if( data.type == 34 ) {
		func.buffer = (long)cos;
		dfunc( func, wh );
	} else {
		func.buffer = (long)cosf;
		ffunc( func, wh );
	}

	return current;
}

/*JNIEXPORT int arccos( simlab wh ) {
	simlab func;
	func.type = 32;
	func.length = 0;

	if( data.type == 34 ) {
		func.buffer = (long)acos;
		dfunc( func, wh );
	} else {
		func.buffer = (long)acosf;
		ffunc( func, wh );
	}
	return current;
}*/

JNIEXPORT int sine( simlab wh ) {
	simlab func;
	func.type = 32;
	func.length = 0;

	if( data.type == 34 ) {
		func.buffer = (long)sin;
		dfunc( func, wh );
	} else {
		func.buffer = (long)sinf;
		ffunc( func, wh );
	}
	return 1;
}

/*JNIEXPORT int arcsin( simlab wh ) {
	simlab func;
	func.type = 32;
	func.length = 0;

	if( data.type == 34 ) {
		func.buffer = (long)asin;
		dfunc( func, wh );
	} else {
		func.buffer = (long)acosf;
		ffunc( func, wh );
	}
	return current;
}*/

JNIEXPORT int bessel() {
	simlab* data = (simlab*)current;
	if( data->type == 66 ) t_bessel<double>( data );
	return current;
}

JNIEXPORT int extract( simlab val, simlab cnk ) {
	int chunk = cnk.buffer;
	int value = val.buffer;

	if( chunk == 0 ) chunk = data.length;

	int tlen = bytelength( data.type, data.length );
	int tchk = bytelength( data.type, value );
	int tcnk = bytelength( data.type, chunk );

	char*	src = (char*)data.buffer;
	char* 	dst = (char*)realloc( NULL, (tchk*tlen)/tcnk );
	data.buffer = (long)dst;
	data.length = (value*data.length)/chunk;

	for( int i = 0; i < tlen; i+=tcnk ) {
		memcpy( dst, src+i, tchk );
		dst += tchk;
	}

	return current;
}

JNIEXPORT int maxlet( simlab s_chunk ) {
	int chunk = data.length;
	if( s_chunk.buffer != 0 ) {
		chunk = s_chunk.buffer;
	}
	if( data.type == 128 ) t_maxlet( (long double*)data.buffer, data.length, chunk );
	else if( data.type == 66 ) t_maxlet( (double*)data.buffer, data.length, chunk );
	else if( data.type == 64 ) t_maxlet( (long long*)data.buffer, data.length, chunk );
	else if( data.type == 34 ) t_maxlet( (float*)data.buffer, data.length, chunk );
	else if( data.type == 32 ) t_maxlet( (int*)data.buffer, data.length, chunk );

	return current;
}

JNIEXPORT int amaxlet( simlab s_chunk ) {
	int chunk = data.length;
	if( s_chunk.buffer != 0 ) {
		chunk = s_chunk.buffer;
	}
	if( data.type == 128 ) t_amaxlet( (long double*)data.buffer, data.length, chunk );
	else if( data.type == 66 ) t_amaxlet( (double*)data.buffer, data.length, chunk );
	else if( data.type == 64 ) t_amaxlet( (long long*)data.buffer, data.length, chunk );
	else if( data.type == 34 ) t_amaxlet( (float*)data.buffer, data.length, chunk );
	else if( data.type == 32 ) t_amaxlet( (int*)data.buffer, data.length, chunk );

	return current;
}

JNIEXPORT int awlet( simlab s_chunk ) {
	if( s_chunk.length == 0 ) {
		if( data.type == 128 ) t_awlet( (long double*)data.buffer, data.length, &s_chunk.buffer, 1 );
		else if( data.type == 66 ) t_awlet( (double*)data.buffer, data.length, &s_chunk.buffer, 1 );
		else if( data.type == 64 ) t_awlet( (long long*)data.buffer, data.length, &s_chunk.buffer, 1 );
		else if( data.type == 34 ) t_awlet( (float*)data.buffer, data.length, &s_chunk.buffer, 1 );
		else if( data.type == 32 ) t_awlet( (int*)data.buffer, data.length, &s_chunk.buffer, 1 );
		else if( data.type == 8 ) t_awlet( (char*)data.buffer, data.length, &s_chunk.buffer, 1 );
	} else if( s_chunk.type == 66 ) {
		if( data.type == 128 ) t_awlet( (long double*)data.buffer, data.length, (double*)s_chunk.buffer, s_chunk.length );
		else if( data.type == 66 ) t_awlet( (double*)data.buffer, data.length, (double*)s_chunk.buffer, s_chunk.length );
		else if( data.type == 64 ) t_awlet( (long long*)data.buffer, data.length, (double*)s_chunk.buffer, s_chunk.length );
		else if( data.type == 34 ) t_awlet( (float*)data.buffer, data.length, (double*)s_chunk.buffer, s_chunk.length );
		else if( data.type == 32 ) t_awlet( (int*)data.buffer, data.length, (double*)s_chunk.buffer, s_chunk.length );
	} else if( s_chunk.type == 34 ) {
		if( data.type == 128 ) t_awlet( (long double*)data.buffer, data.length, (float*)s_chunk.buffer, s_chunk.length );
		else if( data.type == 66 ) t_awlet( (double*)data.buffer, data.length, (float*)s_chunk.buffer, s_chunk.length );
		else if( data.type == 64 ) t_awlet( (long long*)data.buffer, data.length, (float*)s_chunk.buffer, s_chunk.length );
		else if( data.type == 34 ) t_awlet( (float*)data.buffer, data.length, (float*)s_chunk.buffer, s_chunk.length );
		else if( data.type == 32 ) t_awlet( (int*)data.buffer, data.length, (float*)s_chunk.buffer, s_chunk.length );
	} else if( s_chunk.type == 32 ) {
		if( data.type == 128 ) t_awlet( (long double*)data.buffer, data.length, (int*)s_chunk.buffer, s_chunk.length );
		else if( data.type == 66 ) t_awlet( (double*)data.buffer, data.length, (int*)s_chunk.buffer, s_chunk.length );
		else if( data.type == 64 ) t_awlet( (long long*)data.buffer, data.length, (int*)s_chunk.buffer, s_chunk.length );
		else if( data.type == 34 ) t_awlet( (float*)data.buffer, data.length, (int*)s_chunk.buffer, s_chunk.length );
		else if( data.type == 32 ) t_awlet( (int*)data.buffer, data.length, (int*)s_chunk.buffer, s_chunk.length );
	}

	return current;
}

JNIEXPORT int wlet( simlab s_chunk ) {
	if( s_chunk.length == 0 ) {
		if( data.type == 128 ) t_wlet( (long double*)data.buffer, data.length, &s_chunk.buffer, 1 );
		else if( data.type == 66 ) t_wlet( (double*)data.buffer, data.length, &s_chunk.buffer, 1 );
		else if( data.type == 64 ) t_wlet( (long long*)data.buffer, data.length, &s_chunk.buffer, 1 );
		else if( data.type == 34 ) t_wlet( (float*)data.buffer, data.length, &s_chunk.buffer, 1 );
		else if( data.type == 32 ) t_wlet( (int*)data.buffer, data.length, &s_chunk.buffer, 1 );
		else if( data.type == 8 ) t_wlet( (char*)data.buffer, data.length, &s_chunk.buffer, 1 );
	} else if( s_chunk.type == 66 ) {
		if( data.type == 128 ) t_wlet( (long double*)data.buffer, data.length, (double*)s_chunk.buffer, s_chunk.length );
		else if( data.type == 66 ) t_wlet( (double*)data.buffer, data.length, (double*)s_chunk.buffer, s_chunk.length );
		else if( data.type == 64 ) t_wlet( (long long*)data.buffer, data.length, (double*)s_chunk.buffer, s_chunk.length );
		else if( data.type == 34 ) t_wlet( (float*)data.buffer, data.length, (double*)s_chunk.buffer, s_chunk.length );
		else if( data.type == 32 ) t_wlet( (int*)data.buffer, data.length, (double*)s_chunk.buffer, s_chunk.length );
		else if( data.type == 8 ) t_wlet( (char*)data.buffer, data.length, (double*)s_chunk.buffer, s_chunk.length );
	} else if( s_chunk.type == 34 ) {
		if( data.type == 128 ) t_wlet( (long double*)data.buffer, data.length, (float*)s_chunk.buffer, s_chunk.length );
		else if( data.type == 66 ) t_wlet( (double*)data.buffer, data.length, (float*)s_chunk.buffer, s_chunk.length );
		else if( data.type == 64 ) t_wlet( (long long*)data.buffer, data.length, (float*)s_chunk.buffer, s_chunk.length );
		else if( data.type == 34 ) t_wlet( (float*)data.buffer, data.length, (float*)s_chunk.buffer, s_chunk.length );
		else if( data.type == 32 ) t_wlet( (int*)data.buffer, data.length, (float*)s_chunk.buffer, s_chunk.length );
	} else if( s_chunk.type == 32 ) {
		if( data.type == 128 ) t_wlet( (long double*)data.buffer, data.length, (int*)s_chunk.buffer, s_chunk.length );
		else if( data.type == 66 ) t_wlet( (double*)data.buffer, data.length, (int*)s_chunk.buffer, s_chunk.length );
		else if( data.type == 64 ) t_wlet( (long long*)data.buffer, data.length, (int*)s_chunk.buffer, s_chunk.length );
		else if( data.type == 34 ) t_wlet( (float*)data.buffer, data.length, (int*)s_chunk.buffer, s_chunk.length );
		else if( data.type == 32 ) t_wlet( (int*)data.buffer, data.length, (int*)s_chunk.buffer, s_chunk.length );
	}

	return current;
}

JNIEXPORT int dft() {
	if( data.type == 96 ) t_dft( (long double*)data.buffer, data.length );
	else if( data.type == 66 ) t_dft( (double*)data.buffer, data.length );

	return current;
}

JNIEXPORT int fft() {
	if( data.type == 66 ) {
		/*double* out = (double*)fftw_malloc( bytelength( data->type, data->length ) );
		fftw_plan p;
		fftw_r2r_kind	kind = FFTW_DHT;
		p = fftw_plan_r2r_1d( data->length, (double*)data->buffer, out, kind, FFTW_ESTIMATE );
		fftw_execute( p );
		fftw_destroy_plan( p );
		free( (void*)data->buffer );
		data->buffer = (long)out;*/
	}
	return current;
}

JNIEXPORT int minmax() {
	simlab* data = (simlab*)current;
	if( data->type == 66 ) {
		return t_minmax<double>( data );
	}
	return current;
}

JNIEXPORT int min( simlab ck, simlab sz ) {
	int size = sz.buffer;
	int chunk = ck.buffer;
	if( chunk == 0 ) chunk = data.length;
	if( size == 0 ) size = chunk;
	if( data.type == 96 ) t_min( (long double*)data.buffer, data.length, chunk, size );
	else if( data.type == 66 ) t_min( (double*)data.buffer, data.length, chunk, size );
	else if( data.type == 64 ) t_min( (long long*)data.buffer, data.length, chunk, size );
	else if( data.type == 34 ) t_min( (float*)data.buffer, data.length, chunk, size );
	else if( data.type == 32 ) t_min( (int*)data.buffer, data.length, chunk, size );
	else if( data.type == 16 ) t_min( (short*)data.buffer, data.length, chunk, size );
	else if( data.type == 8 ) t_min( (char*)data.buffer, data.length, chunk, size );

	return current;
}

JNIEXPORT int max( simlab ck, simlab sz ) {
	int size = sz.buffer;
	int chunk = ck.buffer;
	if( chunk == 0 ) chunk = data.length;
	if( size == 0 ) size = chunk;
	if( data.type == 96 ) t_max( (long double*)data.buffer, data.length, chunk, size );
	else if( data.type == 66 ) t_max( (double*)data.buffer, data.length, chunk, size );
	else if( data.type == 64 ) t_max( (long long*)data.buffer, data.length, chunk, size );
	else if( data.type == 34 ) t_max( (float*)data.buffer, data.length, chunk, size );
	else if( data.type == 32 ) t_max( (int*)data.buffer, data.length, chunk, size );
	else if( data.type == 16 ) t_max( (short*)data.buffer, data.length, chunk, size );
	else if( data.type == 8 ) t_max( (char*)data.buffer, data.length, chunk, size );

	return current;
}

JNIEXPORT int norm( int chunk ) {
	simlab* data = (simlab*)current;
	if( chunk == 0 ) chunk = data->length;
	if( data->type == 66 ) {
		t_norm( (double*)data->buffer, data->length, chunk );
	}
	return current;
}

JNIEXPORT int ths( simlab tths ) {
	data = tths;

	return 1;
}

JNIEXPORT int cln() {
	simlab* data = (simlab*)current;
	simlab* sl = (simlab*)vector( data->type, data->length );
	memcpy( (void*)sl->buffer, (void*)data->buffer, bytelength(data->type,data->length) );

	return (long)sl;
}

JNIEXPORT int divd( simlab value ) {
	if( data.length == 0 ) {
		if( data.type == 32 ) {
			if( value.length == 0 ) {
				if( value.type == 34 ) t_div( (int*)&data.buffer, 1, (float*)&value.buffer, 1 );
				if( value.type == 32 || value.type == 0 ) t_div( (int*)&data.buffer, 1, (int*)&value.buffer, 1 );
			} else {
				if( value.type == 66 ) t_div( (unsigned int*)&data.buffer, data.length, (double*)value.buffer, value.length );
				if( value.type == 34 ) t_div( (unsigned int*)&data.buffer, data.length, (float*)value.buffer, value.length );
				if( value.type == 32 ) t_div( (unsigned int*)&data.buffer, data.length, (int*)value.buffer, value.length );
				if( value.type == 8 ) t_div( (unsigned int*)&data.buffer, data.length, (char*)value.buffer, value.length );
			}
		} else if( data.type == 34 ) {
			if( value.length == 0 ) {
				if( value.type == 34 ) t_div( (float*)&data.buffer, 1, (float*)&value.buffer, 1 );
				if( value.type == 32 || value.type == 0 ) t_div( (float*)&data.buffer, 1, (int*)&value.buffer, 1 );
			} else {
				if( value.type == 66 ) t_div( (float*)&data.buffer, data.length, (double*)value.buffer, value.length );
				if( value.type == 34 ) t_div( (float*)&data.buffer, data.length, (float*)value.buffer, value.length );
				if( value.type == 32 ) t_div( (float*)&data.buffer, data.length, (int*)value.buffer, value.length );
				if( value.type == 8 ) t_div( (float*)&data.buffer, data.length, (char*)value.buffer, value.length );
			}
		}
	} else {
		if( data.type == 66 ) {
			if( value.length == 0 ) {
				if( value.type == 34 ) t_div( (double*)data.buffer, data.length, (float*)&value.buffer, 1 );
				if( value.type == 32 || value.type == 0 ) t_div( (double*)data.buffer, data.length, (int*)&value.buffer, 1 );
			} else {
				if( value.type == 66 ) t_div( (double*)data.buffer, data.length, (double*)value.buffer, value.length );
				if( value.type == 34 ) t_div( (double*)data.buffer, data.length, (float*)value.buffer, value.length );
				if( value.type == 32 ) t_div( (double*)data.buffer, data.length, (int*)value.buffer, value.length );
				if( value.type == 8 ) t_div( (double*)data.buffer, data.length, (char*)value.buffer, value.length );
			}
		} else if( data.type == 34 ) {
			if( value.length == 0 ) {
				if( value.type == 34 ) t_div( (float*)data.buffer, data.length, (float*)&value.buffer, 1 );
				if( value.type == 32 || value.type == 0 ) t_div( (float*)data.buffer, data.length, (int*)&value.buffer, 1 );
			} else {
				if( value.type == 66 ) t_div( (float*)data.buffer, data.length, (double*)value.buffer, value.length );
				if( value.type == 34 ) t_div( (float*)data.buffer, data.length, (float*)value.buffer, value.length );
				if( value.type == 32 ) t_div( (float*)data.buffer, data.length, (int*)value.buffer, value.length );
				if( value.type == 8 ) t_div( (float*)data.buffer, data.length, (char*)value.buffer, value.length );
			}
		} else if( data.type == 32 ) {
			if( value.length == 0 ) {
				if( value.type == 34 ) t_div( (int*)data.buffer, data.length, (float*)&value.buffer, 1 );
				if( value.type == 32 || value.type == 0 ) t_div( (int*)data.buffer, data.length, (int*)&value.buffer, 1 );
			} else {
				if( value.type == 66 ) t_div( (int*)data.buffer, data.length, (double*)value.buffer, value.length );
				if( value.type == 34 ) t_div( (int*)data.buffer, data.length, (float*)value.buffer, value.length );
				if( value.type == 32 ) t_div( (int*)data.buffer, data.length, (int*)value.buffer, value.length );
				if( value.type == 8 ) t_div( (int*)data.buffer, data.length, (char*)value.buffer, value.length );
			}
		} else if( data.type == 8 ) {
			if( value.length == 0 ) {
				if( value.type == 34 ) t_div( (unsigned char*)data.buffer, data.length, (float*)&value.buffer, 1 );
				if( value.type == 32 || value.type == 0 ) t_div( (unsigned char*)data.buffer, data.length, (int*)&value.buffer, 1 );
			} else {
				if( value.type == 66 ) t_div( (unsigned char*)data.buffer, data.length, (double*)value.buffer, value.length );
				if( value.type == 34 ) t_div( (unsigned char*)data.buffer, data.length, (float*)value.buffer, value.length );
				if( value.type == 32 ) t_div( (unsigned char*)data.buffer, data.length, (int*)value.buffer, value.length );
				if( value.type == 8 ) t_div( (unsigned char*)data.buffer, data.length, (char*)value.buffer, value.length );
			}
		}
	}
	return 1;
}

JNIEXPORT int mula( simlab value, simlab wh ) {
	if( wh.type == 0 ) {
		if( data.length == 0 ) {
			if( data.type == 32 ) {
				if( value.length == 0 ) {
					if( value.type == 34 ) t_mul( (unsigned int*)&data.buffer, 1, (float*)&value.buffer, 1, (int*)NULL, 0 );
					else if( value.type == 32 || value.type == 0 ) {
						t_mul( (unsigned int*)&data.buffer, 1, (int*)&value.buffer, 1, (int*)NULL, 0 );
					}
				} else {
					if( value.type == 66 ) t_mul( (unsigned int*)&data.buffer, data.length, (double*)value.buffer, value.length, (int*)NULL, 0 );
					else if( value.type == 34 ) t_mul( (unsigned int*)&data.buffer, data.length, (float*)value.buffer, value.length, (int*)NULL, 0 );
					else if( value.type == 32 ) t_mul( (unsigned int*)&data.buffer, data.length, (int*)value.buffer, value.length, (int*)NULL, 0 );
					else if( value.type == 8 ) t_mul( (unsigned int*)&data.buffer, data.length, (char*)value.buffer, value.length, (int*)NULL, 0 );
				}
			} else if( data.type == 34 ) {
				if( value.length == 0 ) {
					if( value.type == 34 ) t_mul( (float*)&data.buffer, 1, (float*)&value.buffer, 1, (int*)NULL, 0 );
					else if( value.type == 32 || value.type == 0 ) t_mul( (float*)&data.buffer, 1, (int*)&value.buffer, 1, (int*)NULL, 0 );
				} else {
					if( value.type == 66 ) t_mul( (float*)&data.buffer, data.length, (double*)value.buffer, value.length, (int*)NULL, 0 );
					else if( value.type == 34 ) t_mul( (float*)&data.buffer, data.length, (float*)value.buffer, value.length, (int*)NULL, 0 );
					else if( value.type == 32 ) t_mul( (float*)&data.buffer, data.length, (int*)value.buffer, value.length, (int*)NULL, 0 );
					else if( value.type == 8 ) t_mul( (float*)&data.buffer, data.length, (char*)value.buffer, value.length, (int*)NULL, 0 );
				}
			}
		} else {
			if( data.type == 66 ) {
				if( value.length == 0 ) {
					if( value.type == 34 ) t_mul( (double*)data.buffer, data.length, (float*)&value.buffer, 1, (int*)NULL, 0 );
					else if( value.type == 32 || value.type == 0 ) t_mul( (double*)data.buffer, data.length, (int*)&value.buffer, 1, (int*)NULL, 0 );
				} else {
					if( value.type == 66 ) t_mul( (double*)data.buffer, data.length, (double*)value.buffer, value.length, (int*)NULL, 0 );
					else if( value.type == 34 ) t_mul( (double*)data.buffer, data.length, (float*)value.buffer, value.length, (int*)NULL, 0 );
					else if( value.type == 32 ) t_mul( (double*)data.buffer, data.length, (int*)value.buffer, value.length, (int*)NULL, 0 );
					else if( value.type == 8 ) t_mul( (double*)data.buffer, data.length, (char*)value.buffer, value.length, (int*)NULL, 0 );
				}
			} else if( data.type == 34 ) {
				if( value.length == 0 ) {
					if( value.type == 34 ) t_mul( (float*)data.buffer, data.length, (float*)&value.buffer, 1, (int*)NULL, 0 );
					else if( value.type == 32 || value.type == 0 ) t_mul( (float*)data.buffer, data.length, (int*)&value.buffer, 1, (int*)NULL, 0 );
				} else {
					if( value.type == 66 ) t_mul( (float*)data.buffer, data.length, (double*)value.buffer, value.length, (int*)NULL, 0 );
					else if( value.type == 34 ) t_mul( (float*)data.buffer, data.length, (float*)value.buffer, value.length, (int*)NULL, 0 );
					else if( value.type == 32 ) t_mul( (float*)data.buffer, data.length, (int*)value.buffer, value.length, (int*)NULL, 0 );
					else if( value.type == 8 ) t_mul( (float*)data.buffer, data.length, (char*)value.buffer, value.length, (int*)NULL, 0 );
				}
			} else if( data.type == 32 ) {
				if( value.length == 0 ) {
					if( value.type == 34 ) t_mul( (int*)data.buffer, data.length, (float*)&value.buffer, 1, (int*)NULL, 0 );
					else if( value.type == 32 || value.type == 0 ) t_mul( (int*)data.buffer, data.length, (int*)&value.buffer, 1, (int*)NULL, 0 );
				} else {
					if( value.type == 66 ) t_mul( (int*)data.buffer, data.length, (double*)value.buffer, value.length, (int*)NULL, 0 );
					else if( value.type == 34 ) t_mul( (int*)data.buffer, data.length, (float*)value.buffer, value.length, (int*)NULL, 0 );
					else if( value.type == 32 ) t_mul( (int*)data.buffer, data.length, (int*)value.buffer, value.length, (int*)NULL, 0 );
					else if( value.type == 8 ) t_mul( (int*)data.buffer, data.length, (char*)value.buffer, value.length, (int*)NULL, 0 );
				}
			} else if( data.type == 8 ) {
				if( value.length == 0 ) {
					if( value.type == 34 ) t_mul( (unsigned char*)data.buffer, data.length, (float*)&value.buffer, 1, (int*)NULL, 0 );
					else if( value.type == 32 || value.type == 0 ) t_mul( (unsigned char*)data.buffer, data.length, (int*)&value.buffer, 1, (int*)NULL, 0 );
				} else {
					if( value.type == 66 ) t_mul( (unsigned char*)data.buffer, data.length, (double*)value.buffer, value.length, (int*)NULL, 0 );
					else if( value.type == 34 ) t_mul( (unsigned char*)data.buffer, data.length, (float*)value.buffer, value.length, (int*)NULL, 0 );
					else if( value.type == 32 ) t_mul( (unsigned char*)data.buffer, data.length, (int*)value.buffer, value.length, (int*)NULL, 0 );
					else if( value.type == 8 ) t_mul( (unsigned char*)data.buffer, data.length, (char*)value.buffer, value.length, (int*)NULL, 0 );
				}
			}
		}
		//return 1;
	} else if( wh.length == 0 ) {

	} else {

	}

	return 2;
}

JNIEXPORT int matmul( simlab mul, simlab vl ) {
	int val = vl.buffer;
	if( val == 0 ) val = _gcd( data.length, mul.length );

	//int length = (data.length/val)*(mul.length/val);
	if( data.type == 66 ) {
		if( mul.type == 66 ) t_matmul( (double*)data.buffer, data.length, (double*)mul.buffer, mul.length, val );
		else if( mul.type == 34 ) t_matmul( (double*)data.buffer, data.length, (float*)mul.buffer, mul.length, val );
		else if( mul.type == 32 ) t_matmul( (double*)data.buffer, data.length, (int*)mul.buffer, mul.length, val );
	} else if( data.type == 34 ) {
		if( mul.type == 66 ) t_matmul( (float*)data.buffer, data.length, (double*)mul.buffer, mul.length, val );
		else if( mul.type == 34 ) t_matmul( (float*)data.buffer, data.length, (float*)mul.buffer, mul.length, val );
		else if( mul.type == 32 ) t_matmul( (float*)data.buffer, data.length, (int*)mul.buffer, mul.length, val );
	} else if( data.type == 32 ) {
		if( mul.type == 66 ) t_matmul( (int*)data.buffer, data.length, (double*)mul.buffer, mul.length, val );
		else if( mul.type == 34 ) t_matmul( (int*)data.buffer, data.length, (float*)mul.buffer, mul.length, val );
		else if( mul.type == 32 ) t_matmul( (int*)data.buffer, data.length, (int*)mul.buffer, mul.length, val );
	}

	return current;
}

JNIEXPORT int dct() {
	if( data.type == 34 ) t_dct( (float*)data.buffer, data.length );
	else if( data.type == 66 ) t_dct( (double*)data.buffer, data.length );
	else if( data.type == 96 ) t_dct( (long double*)data.buffer, data.length );
	return current;
}

JNIEXPORT int idct() {
	if( data.type == 34 ) t_idct( (float*)data.buffer, data.length );
	else if( data.type == 66 ) t_idct( (double*)data.buffer, data.length );
	else if( data.type == 96 ) t_idct( (long double*)data.buffer, data.length );
	return current;
}

JNIEXPORT int permute( int c, int start ) {
	simlab* data = (simlab*)current;

	if( data->type == 66 ) t_permute( (double*)data->buffer, data->length, c, start );
	else if( data->type == 34 ) t_permute( (float*)data->buffer, data->length, c, start );
	else if( data->type == 32 ) t_permute( (int*)data->buffer, data->length, c, start );

	return current;
}

JNIEXPORT int invidx() {
	if( data.type == 66 ) t_invidx( (double*)data.buffer, data.length );
	else if( data.type == 32 ) t_invidx( (int*)data.buffer, data.length );

	return 0;
}

JNIEXPORT int sortidx() {
	if( data.type == 66 ) t_sortidx( (double*)data.buffer, data.length );
	else if( data.type == 32 ) t_sortidx( (int*)data.buffer, data.length );

	return 0;
}

JNIEXPORT int transidx( simlab cl, simlab rl ) {
	int c = cl.buffer;
	if( c < 0 ) c = data.length/(-c);
	int r = rl.buffer;
	if( r == 0 ) r = data.length/c;

	if( data.type == 66 ) t_transidx( (double*)data.buffer, data.length, c, r );
	else if( data.type == 32 ) t_transidx( (int*)data.buffer, data.length, c, r );

	return 0;
}

JNIEXPORT int filetrans( simlab fl, simlab tp, simlab cl, simlab rl ) {
	int type = tp.buffer;
	FILE* file = fopen( (char*)fl.buffer, "r+" );
	long start = ftell(file);
	fseek( file, 0, SEEK_END );
	long stop = ftell(file);
	long length = stop-start;
	length = (length*8)/type;
	printf("%d\n",(int)length);

	int c = cl.buffer;
	if( c < 0 ) c = length/(-c);
	int r = rl.buffer;
	if( r == 0 ) r = length/c;

	if( type == 1 ) t_filetransbits<unsigned char>( file, length, type, 1, c, r );
	else if( type == 2 ) t_filetransbits<unsigned char>( file, length, type, 1, c, r );
	else if( type == 3 ) t_filetransbits<unsigned int>( file, length, type, 3, c, r );
	else if( type == 4 ) t_filetransbits<unsigned char>( file, length, type, 1, c, r );
	else if( type == 5 ) t_filetransbits<unsigned long long>( file, length, type, 5, c, r );
	else if( type == 6 ) t_filetransbits<unsigned int>( file, length, type, 3, c, r );
	else if( type == 7 ) t_filetransbits<unsigned long long>( file, length, type, 7, c, r );
	fclose( file );

	return 3;
}

JNIEXPORT int transold( simlab cl, simlab rl ) {
	int c = cl.buffer;
	if( c < 0 ) c = data.length/(-c);
	int r = rl.buffer;
	if( r == 0 ) r = data.length/c;

	//printf("%d\n", sizeof(long double) );
	if( data.type == sizeof(long double)*8 ) t_transold( (long double*)data.buffer, data.length, c, r );
	else if( data.type == 66 ) t_transold( (double*)data.buffer, data.length, c, r );
	else if( data.type == 64 ) t_transold( (long long*)data.buffer, data.length, c, r );
	else if( data.type == 34 ) t_transold( (float*)data.buffer, data.length, c, r );
	else if( data.type == 32 ) t_transold( (int*)data.buffer, data.length, c, r );
	else if( data.type == 24 ) t_transmem( (char*)data.buffer, data.length, c, r, 3 );
	else if( data.type == 16 ) t_transold( (short*)data.buffer, data.length, c, r );
	else if( data.type == 8 ) t_transold( (char*)data.buffer, data.length, c, r );
	else if( data.type > 96 ) t_transmem( (char*)data.buffer, data.length, c, r, data.type/8 );
	else if( data.type == 1 ) t_transbit( (unsigned char*)data.buffer, data.type, data.length, c, r );
	else if( data.type == 2 ) t_transbits<unsigned char>( *(unsigned char**)&data.buffer, data.length, data.type, 1, c, r );
	else if( data.type == 3 ) t_transbits<unsigned int>( *(unsigned char**)&data.buffer, data.length, data.type, 3, c, r );
	else if( data.type == 4 ) t_transbits<unsigned char>( *(unsigned char**)&data.buffer, data.length, data.type, 1, c, r );
	else if( data.type == 5 ) t_transbits<unsigned long long>( *(unsigned char**)&data.buffer, data.length, data.type, 5, c, r );
	else if( data.type == 6 ) t_transbits<unsigned int>( *(unsigned char**)&data.buffer, data.length, data.type, 3, c, r );
	else if( data.type == 7 ) t_transbits<unsigned long long>( *(unsigned char**)&data.buffer, data.length, data.type, 7, c, r );

	return current;
}

JNIEXPORT int trans( simlab cl, simlab rl ) {
	int c = cl.buffer;
	if( c < 0 ) c = data.length/(-c);
	int r = rl.buffer;
	if( r == 0 ) r = data.length/c;

	//printf("%d\n", sizeof(long double) );
	if( data.type < 0 ) {
		if( data.type == -66 ) t_trans<c_simlab<double&>&,double>( *(c_simlab<double&>*)data.buffer, data.length, c, r );
	} else {
		if( data.type == sizeof(long double)*8 ) t_trans<long double*,long double>( (long double*)data.buffer, data.length, c, r );
		else if( data.type == 66 ) t_trans<double*,double>( (double*)data.buffer, data.length, c, r );
		else if( data.type == 64 ) t_trans<long long*,long long>( (long long*)data.buffer, data.length, c, r );
		else if( data.type == 34 ) t_trans<float*,float>( (float*)data.buffer, data.length, c, r );
		else if( data.type == 32 ) t_trans<int*,int>( (int*)data.buffer, data.length, c, r );
		else if( data.type == 24 ) t_transmem( (char*)data.buffer, data.length, c, r, 3 );
		else if( data.type == 16 ) t_trans<short*,short>( (short*)data.buffer, data.length, c, r );
		else if( data.type == 8 ) t_trans<char*,char>( (char*)data.buffer, data.length, c, r );
		else if( data.type > 96 ) t_transmem( (char*)data.buffer, data.length, c, r, data.type/8 );
		else if( data.type == 1 ) t_transbit( (unsigned char*)data.buffer, data.type, data.length, c, r );
		else if( data.type == 2 ) t_transbits<unsigned char>( *(unsigned char**)&data.buffer, data.length, data.type, 1, c, r );
		else if( data.type == 3 ) t_transbits<unsigned int>( *(unsigned char**)&data.buffer, data.length, data.type, 3, c, r );
		else if( data.type == 4 ) t_transbits<unsigned char>( *(unsigned char**)&data.buffer, data.length, data.type, 1, c, r );
		else if( data.type == 5 ) t_transbits<unsigned long long>( *(unsigned char**)&data.buffer, data.length, data.type, 5, c, r );
		else if( data.type == 6 ) t_transbits<unsigned int>( *(unsigned char**)&data.buffer, data.length, data.type, 3, c, r );
		else if( data.type == 7 ) t_transbits<unsigned long long>( *(unsigned char**)&data.buffer, data.length, data.type, 7, c, r );
	}

	return current;
}

JNIEXPORT int oracleconnect( char* user, char* passwd, char* db ) {
	//oracle::occi::Environment	*env;
	//oracle::occi::Connection	*conn;
	//oracle::occi::Statement		*stmt;

	//env = oracle::occi::Environment::createEnvironment();
    //conn = env->createConnection (user, passwd, db);

	/*std::string sqlStmt = "SELECT author_id, author_name FROM autor_tab order by author_id";
    stmt = conn->createStatement (sqlStmt);
    oracle::occi::ResultSet *rset = stmt->executeQuery();
    try{
//		while( rset->next() ) {
//			std::cout << "author_id: " << rset->getInt(1) << "  author_name: " << rset->getString(2) << endl;
//		}
    } catch( oracle::occi::SQLException ex ) {
//		std::cout<<"Exception thrown for displayAllRows"<<endl;
//		std::cout<<"Error number: "<<  ex.getErrorCode() << endl;
//		std::cout<<ex.getMessage() << endl;
    }
    stmt->closeResultSet(rset);
    conn->terminateStatement(stmt);*/

    //env->terminateConnection (conn);
    //oracle::occi::Environment::terminateEnvironment(env);

	return 0;
}

JNIEXPORT int space() {
/*	DXUTSetCallbackDeviceCreated( OnCreateDevice );
    DXUTSetCallbackDeviceReset( OnResetDevice );
    DXUTSetCallbackDeviceLost( OnLostDevice );
    DXUTSetCallbackDeviceDestroyed( OnDestroyDevice );

    DXUTSetCallbackFrameRender( OnFrameRender );
    DXUTSetCallbackFrameMove( OnFrameMove );

    DXUTInit( TRUE, TRUE, TRUE );
    DXUTCreateWindow( L"BasicHLSL" );
    DXUTCreateDevice( D3DADAPTER_DEFAULT, TRUE, 640, 480 );

    DXUTMainLoop();*/

    return 0;//DXUTGetExitCode();
}

JNIEXPORT int draw3d() {
	/*simlab* data = (simlab*)current;

	m_d3d = Direct3DCreate9( D3D_SDK_VERSION );

	D3DDISPLAYMODE d3ddm;
	m_d3d->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3ddm );

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory( &d3dpp, sizeof(d3dpp) );
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN; //d3ddm.Format;
	m_d3d->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, currentWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &m_d3ddevice );
	m_d3ddevice->CreateVertexBuffer( 512*512*sizeof(CUSTOMVERTEX), 0 *Usage*, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &m_d3dvb, NULL );

	D3DXMATRIX matWorld;
	D3DXMatrixRotationY( &matWorld, 0.0f/150.0f );
	m_d3ddevice->SetTransform( D3DTS_WORLD, &matWorld );

	D3DXVECTOR3 vEyePt   ( 0.0f, 0.0f,-1.0f );
	D3DXVECTOR3 vLookatPt( 0.0f, 0.0f, 0.0f );
	D3DXVECTOR3 vUpVec   ( 0.0f, 1.0f, 0.0f );
	D3DXMATRIXA16 matView;
	D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &vUpVec );
	m_d3ddevice->SetTransform( D3DTS_VIEW, &matView );

	D3DXMATRIX matProj;
	D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/4, 1.0f, 1.0f, 100.0f );
	m_d3ddevice->SetTransform( D3DTS_PROJECTION, &matProj );*/

	//m_d3ddevice->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,255), 1.0f, 0 );

	return current;
}

#ifdef ALSA
JNIEXPORT int play( simlab rate ) {
	int err;
	unsigned int exact_rate = 44100;
	if( rate.buffer != 0 ) exact_rate = abs(rate.buffer);
	short*	buf = (short*)data.buffer;
	snd_pcm_t *playback_handle;
	snd_pcm_hw_params_t *hw_params;

	if ((err = snd_pcm_open (&playback_handle, "plughw:0,0", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
		fprintf (stderr, "cannot open audio device %s (%s)\n", "NULL", snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
		fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_any (playback_handle, hw_params)) < 0) {
		fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_access (playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		fprintf (stderr, "cannot set access type (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_format (playback_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
		fprintf (stderr, "cannot set sample format (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_rate_near (playback_handle, hw_params, &exact_rate, 0)) < 0) {
		fprintf (stderr, "cannot set sample rate (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_channels (playback_handle, hw_params, 2)) < 0) {
		fprintf (stderr, "cannot set channel count (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params (playback_handle, hw_params)) < 0) {
		fprintf (stderr, "cannot set parameters (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	snd_pcm_hw_params_free (hw_params);

	if ((err = snd_pcm_prepare (playback_handle)) < 0) {
		fprintf (stderr, "cannot prepare audio interface for use (%s)\n", snd_strerror (err));
		exit (1);
	}

	snd_pcm_writei(playback_handle, buf, data.length);
	/*for (i = 0; i < 10; ++i) {
		if ((err = snd_pcm_writei(playback_handle, buf, data.length)) != data.length) {
			fprintf (stderr, "write to audio interface failed (%s)\n", snd_strerror (err));
			exit (1);
		}
	}*/

	snd_pcm_close (playback_handle);

	return current;
}

JNIEXPORT int capture() {
	int err;
	short* buf = (short*)data.buffer;
	unsigned int exact_rate = 44100;
	snd_pcm_t *capture_handle;
	snd_pcm_hw_params_t *hw_params;

	if ((err = snd_pcm_open (&capture_handle, "plughw:0,0", SND_PCM_STREAM_CAPTURE, 0)) < 0) {
		fprintf (stderr, "cannot open audio device %s (%s)\n", "NULL", snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
		fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n", snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_any (capture_handle, hw_params)) < 0) {
		fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_access (capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		fprintf (stderr, "cannot set access type (%s)\n", snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_format (capture_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
		fprintf (stderr, "cannot set sample format (%s)\n", snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_rate_near (capture_handle, hw_params, &exact_rate, 0)) < 0) {
		fprintf (stderr, "cannot set sample rate (%s)\n", snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_channels (capture_handle, hw_params, 2)) < 0) {
		fprintf (stderr, "cannot set channel count (%s)\n", snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params (capture_handle, hw_params)) < 0) {
		fprintf (stderr, "cannot set parameters (%s)\n", snd_strerror (err));
		exit (1);
	}

	snd_pcm_hw_params_free (hw_params);

	if ((err = snd_pcm_prepare (capture_handle)) < 0) {
		fprintf (stderr, "cannot prepare audio interface for use (%s)\n", snd_strerror (err));
		exit (1);
	}

	snd_pcm_readi (capture_handle, buf, data.length);

	snd_pcm_close (capture_handle);

	return current;
}
#endif

JNIEXPORT int dual() {
	int a[] = {1,2,3};
	Dual d( a, 3 );
	d[2] = d[1];
	printf( "%d  %d  %d\n", a[0], a[1], a[2] );
	return current;
}


#ifdef WIN
#ifdef GL
int bSetupPixelFormat(HDC hdc) {
    PIXELFORMATDESCRIPTOR pfd, *ppfd;
    int pixelformat;

    ppfd = &pfd;

    ppfd->nSize = sizeof(PIXELFORMATDESCRIPTOR);
    ppfd->nVersion = 1;
    ppfd->dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    ppfd->dwLayerMask = PFD_MAIN_PLANE;
    ppfd->iPixelType = PFD_TYPE_COLORINDEX;
    ppfd->cColorBits = 24;
    ppfd->cDepthBits = 16;
    ppfd->cAccumBits = 0;
    ppfd->cStencilBits = 0;

    pixelformat = ChoosePixelFormat(hdc, ppfd);

    if ( (pixelformat = ChoosePixelFormat(hdc, ppfd)) == 0 )
    {
        MessageBox(NULL, "ChoosePixelFormat failed", "Error", MB_OK);
        return FALSE;
    }

    if (SetPixelFormat(hdc, pixelformat, ppfd) == FALSE)
    {
        MessageBox(NULL, "SetPixelFormat failed", "Error", MB_OK);
        return FALSE;
    }

    return TRUE;
}
#endif
RECT	r;
LRESULT CALLBACK MainWndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) {
	HDC				ghDC;
	HGLRC			ghRC;
	RECT			rect;
	PAINTSTRUCT		ps;

	switch (uMsg) {
        case WM_CREATE:
			ghDC = GetDC(hwnd);
			if (!bSetupPixelFormat(ghDC))
				PostQuitMessage (0);

			ghRC = wglCreateContext(ghDC);
			wglMakeCurrent(ghDC, ghRC);
			GetClientRect(hwnd, &r);
			//initializeGL(rect.right, rect.bottom);

            break;

		case WM_PAINT:
			ghDC = GetDC(hwnd);
			BeginPaint(hwnd, &ps);
			drawGL();
			EndPaint(hwnd, &ps);
			break;

		case WM_SIZE:
			GetClientRect(hwnd, &rect);
			//resize(rect.right, rect.bottom);
			break;

		case WM_CLOSE:
			/*if (ghRC)
			    wglDeleteContext(ghRC);
			if (ghDC)
				ReleaseDC(hwnd, ghDC);
			ghRC = 0;
			ghDC = 0;*/
			printf("close\n");
			DestroyWindow(hwnd);
			PostQuitMessage(0);
			break;

        case WM_DESTROY:
			/*if (ghRC)
			    wglDeleteContext(ghRC);
			if (ghDC)
			    ReleaseDC(hwnd, ghDC);*/
        	printf("destroy\n");
        	//DestroyWindow(hwnd);
			PostQuitMessage(0);
			break;

		case WM_KEYDOWN:
			switch (wParam) {
				case 'F':
					//HMONITOR hmon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
					//MONITORINFO mi = { sizeof(mi) };
					//if (!GetMonitorInfo(hmon, &mi)) break;
					int width = 1400;//mi.rcMonitor.right - mi.rcMonitor.left;
					int height = 1050;//mi.rcMonitor.bottom - mi.rcMonitor.top;
					if( width == r.right-r.left ) MoveWindow( hwnd, r.left, r.top, r.right-r.left, r.bottom-r.top, TRUE );
					else MoveWindow( hwnd, 0, 0, width, height, TRUE );
					break;
				/*case VK_LEFT:
					longinc += 0.5F;
					break;
				case VK_RIGHT:
					longinc -= 0.5F;
					break;
				case VK_UP:
					latinc += 0.5F;
					break;
				case VK_DOWN:
					latinc -= 0.5F;
					break;*/
			}
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);;
}

JNIEXPORT int thread( simlab cmd ) {
	//LPTHREAD_START_ROUTINE threadRoutine = (LPTHREAD_START_ROUTINE)GetProcAddress( hmodule, cmd );
	DWORD id;
	CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)cmd.buffer, *(void**)(&passnext.big.big.big), 0, &id );

	return 0;
}

JNIEXPORT int image( simlab chunk, simlab name ) {
	int val = chunk.buffer;

    int w = 0;
    int h = 0;

    if( val > 0 ) {
    	w = chunk.buffer;
    	h = data.length/w;
    } else if( val < 0 ) {
    	h = -chunk.buffer;
    	w = data.length/h;
    }

    WNDCLASSEX wcx;
    wcx.cbSize = sizeof(WNDCLASSEX);
    wcx.style = CS_HREDRAW | CS_VREDRAW;
	wcx.lpfnWndProc = MainWndProc;
    wcx.cbClsExtra = 0;
    wcx.cbWndExtra = 0;
    wcx.hInstance = hinstance;
    wcx.hIcon = NULL; //LoadIcon(NULL, IDI_APPLICATION);
    wcx.hCursor = NULL; //LoadCursor(NULL, IDC_ARROW);
    wcx.hbrBackground = (HBRUSH)(COLOR_WINDOW+1); //GetStockObject( WHITE_BRUSH );
    wcx.lpszMenuName = NULL;
    wcx.lpszClassName = "slab";
    wcx.hIconSm = NULL;

    RegisterClassEx(&wcx);

    DWORD style;
    //if( TRUE )
    //	style = WS_EX_TOPMOST | WS_VISIBLE | WS_POPUP;
    //else
    style = WS_OVERLAPPEDWINDOW;

    HWND hwnd = CreateWindow(
        "slab",        		// name of window class
        "image",			//(char*)name.buffer,
        style, 				// top-level window
        CW_USEDEFAULT,       // default horizontal position
        CW_USEDEFAULT,       // default vertical position
        CW_USEDEFAULT,       // default width
        CW_USEDEFAULT,       // default height
        (HWND) NULL,         // no owner window
        (HMENU) NULL,        // use class menu
        (HINSTANCE)module,   // handle to application instance
        (LPVOID) NULL);      // no window-creation data

    ShowWindow( hwnd, SW_SHOW );
    UpdateWindow( hwnd );

    MSG msg;
    int done = 0;
    while( !done ) {
    	if( PeekMessage(&msg, hwnd,  0, 0, PM_REMOVE ) ) {
    		TranslateMessage(&msg);
    		DispatchMessage(&msg);
    		if( !IsWindow(hwnd) ) done = 1;
   		}
   		//drawScene();
   	}
}

JNIEXPORT int window( simlab name ) {
    WNDCLASSEX wcx;

	//while( hinstance == NULL );
	//memset( &wcx, 0, sizeof(WNDCLASSEX) );
    wcx.cbSize = sizeof(WNDCLASSEX);
    wcx.style = CS_HREDRAW | CS_VREDRAW;
	wcx.lpfnWndProc = MainWndProc;
    wcx.cbClsExtra = 0;
    wcx.cbWndExtra = 0;
    wcx.hInstance = hinstance;
    wcx.hIcon = NULL; //LoadIcon(NULL, IDI_APPLICATION);
    wcx.hCursor = NULL; //LoadCursor(NULL, IDC_ARROW);
    wcx.hbrBackground = (HBRUSH)(COLOR_WINDOW+1); //GetStockObject( WHITE_BRUSH );
    wcx.lpszMenuName = NULL;
    wcx.lpszClassName = "slab";
    wcx.hIconSm = NULL;
		/*(HICON)LoadImage(hinstance,
        MAKEINTRESOURCE(5),
        IMAGE_ICON,
        GetSystemMetrics(SM_CXSMICON),
        GetSystemMetrics(SM_CYSMICON),
        LR_DEFAULTCOLOR);*/

    RegisterClassEx(&wcx);

    DWORD style;
    //if( TRUE )
    //	style = WS_EX_TOPMOST | WS_VISIBLE | WS_POPUP;
    //else
    style = WS_OVERLAPPEDWINDOW;

    HWND hwnd = CreateWindow(
        "slab",        // name of window class
        (char*)name.buffer,
        style, // top-level window
        CW_USEDEFAULT,       // default horizontal position
        CW_USEDEFAULT,       // default vertical position
        CW_USEDEFAULT,       // default width
        CW_USEDEFAULT,       // default height
        (HWND) NULL,         // no owner window
        (HMENU) NULL,        // use class menu
        (HINSTANCE)module,           // handle to application instance
        (LPVOID) NULL);      // no window-creation data

    ShowWindow( hwnd, SW_SHOW );
    UpdateWindow( hwnd );

    MSG msg;
    int done = 0;
    while( !done ) {
    	if( PeekMessage(&msg, hwnd,  0, 0, PM_REMOVE ) ) {
    		TranslateMessage(&msg);
    		DispatchMessage(&msg);
    		if( !IsWindow(hwnd) ) done = 1;
   		}
   		//drawScene();
   	}

	return current;
}
#endif

JNIEXPORT int showwindow( int showhide ) {
	//HWND hwnd = c

	//ShowWindow(currentWnd, showhide ? SW_SHOW : SW_HIDE );
	//ShowWindow(currentWnd, SW_SHOW );
    //UpdateWindow(currentWnd);

	return current;
}

JNIEXPORT int indexof( simlab val ) {
	if( data.type == 66 ) {
		t_idx<double>( (double*)data.buffer, data.length, (void*)val.buffer, bytelength(val.type,val.length) );
	}

	return 0;
}

JNIEXPORT int dummy() {
	return 0;
}

/*JNIEXPORT int crnt( simlab d ) {
	data = d;

	return current;
}*/

JNIEXPORT int str( simlab str ) {
	data = str;
	data.buffer = (long)new char[str.length];
	strcpy( (char*)data.buffer, (char*)str.buffer );

	return current;
}

#ifdef JPG
JNIEXPORT int jpegread( simlab filename ) {
	FILE*	fp = fopen( (char*)filename.buffer, "rb" );

	struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

	jpeg_stdio_src(&cinfo, fp);
    printf("erm\n");
	jpeg_read_header(&cinfo, true);
	printf("erm2\n");

    int w = cinfo.image_width;
    int h = cinfo.image_height;
    int d = cinfo.jpeg_color_space;

    int retlen = w * h * d;
    printf( "%d %d %d\n", w, h, d );
    unsigned char *dt = new unsigned char[retlen];

    while (cinfo.output_scanline < cinfo.output_height) {
        jpeg_read_scanlines(&cinfo, &dt, 1);
        dt += d * cinfo.output_width;
    }
    printf("erm3\n");

    printf("erm3.5\n");
    jpeg_finish_decompress(&cinfo);
    printf("erm4\n");
    jpeg_destroy_decompress(&cinfo);
    printf("erm5\n");

    fclose(fp);

    data.buffer = (long)dt;
    data.type = 8;
    data.length = retlen;

    return current;
}
#endif

JNIEXPORT int fileread( simlab filename ) {
	if( filename.type == 8 ) {
		char* fname = (char*)filename.buffer;

		FILE* f = fopen( fname, "r" );
		fseek( f, 0, SEEK_END );
		long size = ftell( f );
		fseek( f, 0, SEEK_SET );
		data.buffer = (long)new char[size];
		data.type = 8;
		data.length = size;
		fread( (void*)data.buffer, 1, size, f );
		fclose(f);
	} else if( filename.type == 96 ) {
		 //for( int i = 0;)
	}

	return current;
}

JNIEXPORT int filewrite( simlab filename ) {
	if( filename.type == 8 ) {
		char* fname = (char*)filename.buffer;

		//int u = bytelength(data.type, data.length);
		//printf( "about to write %d %d\n", data.length, u );
		FILE* f = fopen( fname, "w" );
		fwrite( (void*)data.buffer, data.type/8, data.length, f );
		fclose(f);
	} else if( filename.type == 96 ) {
		 //for( int i = 0;)
	}

	return current;
}

#ifdef THREAD
JNIEXPORT int thread( simlab cmd ) {
	pthread_t	thread;
	pthread_create( &thread, NULL, threadrunner, (void*)cmd.buffer );

	return current;
}
#endif

JNIEXPORT int flip( simlab chunk ) {
	if( chunk.buffer == 0 ) chunk.buffer = data.length;
	if( data.type == 66 ) t_flip( (double*)data.buffer, data.length, chunk.buffer );
	else if( data.type == 34 ) t_flip( (float*)data.buffer, data.length, chunk.buffer );
	else if( data.type == 32 ) t_flip( (int*)data.buffer, data.length, chunk.buffer );
	else if( data.type == 8 ) t_flip( (char*)data.buffer, data.length, chunk.buffer );
	return 0;
}

JNIEXPORT int hist( int bin, int chunk ) {
	simlab*	data = (simlab*)current;
	if( chunk == 0 ) chunk = data->length;
	if( data->type == 66 ) {
		if( data->type == 66 ) t_wrap( t_hist( (double*)data->buffer, data->length, chunk, bin ), bin*data->length/chunk );
	}
	return current;
}

JNIEXPORT int histeq() {
	return current;
}

JNIEXPORT int median( int chunk ) {
	simlab* data = (simlab*)current;
	if( data->type == 66 ) t_median( (double*)data->buffer, data->length, chunk );
	if( data->type == 32 ) t_median( (int*)data->buffer, data->length, chunk );
	return current;
}

JNIEXPORT int shift( simlab val, simlab chunk ) {
	//printf( "n %d %d\n", val.buffer, chunk.buffer );
	if( chunk.buffer == 0 ) chunk.buffer = data.length;
	if( val.length == 0 ) {
		if( data.type == 66 ) t_shift( (double*)data.buffer, data.length, chunk.buffer, val.buffer );
		else if( data.type == 34 ) t_shift( (float*)data.buffer, data.length, chunk.buffer, val.buffer );
		else if( data.type == 33 ) t_shift( (unsigned int*)data.buffer, data.length, chunk.buffer, val.buffer );
		else if( data.type == 32 ) t_shift( (int*)data.buffer, data.length, chunk.buffer, val.buffer );
		else if( data.type == 8 ) t_shift( (char*)data.buffer, data.length, chunk.buffer, val.buffer );
	} else {
		if( data.type == 66 ) t_shift( (double*)data.buffer, data.length, chunk.buffer, *(int*)val.buffer );
		else if( data.type == 34 ) t_shift( (float*)data.buffer, data.length, chunk.buffer, *(int*)val.buffer );
		else if( data.type == 33 ) t_shift( (unsigned int*)data.buffer, data.length, chunk.buffer, *(int*)val.buffer );
		else if( data.type == 32 ) t_shift( (int*)data.buffer, data.length, chunk.buffer, *(int*)val.buffer );
		else if( data.type == 8 ) t_shift( (char*)data.buffer, data.length, chunk.buffer, *(int*)val.buffer );
	}
	return 2;
}

JNIEXPORT int gcd( simlab* check ) {
	simlab* data = (simlab*)current;
	if( data->type == 66 ) {
		if( check->type == 66 ) t_gcd( (double*)data->buffer, data->length, (double*)check->buffer, check->length );
		else if( check->type == 32 ) t_gcd( (double*)data->buffer, data->length, (int*)check->buffer, check->length );
	} else if( data->type == 32 ) {
		if( check->type == 66 ) t_gcd( (int*)data->buffer, data->length, (double*)check->buffer, check->length );
		else if( check->type == 32 ) t_gcd( (int*)data->buffer, data->length, (int*)check->buffer, check->length );
	}
	return current;
}

JNIEXPORT int factor() {
	//simlab* data = (simlab*)current;
	/*if( data->type == 66 ) return (long)t_factor( (double*)data->buffer, data->length );
	else if( data->type == 34 ) return (long)t_factor( (float*)data->buffer, data->length );
	else if( data->type == 33 ) return (long)t_factor( (unsigned int*)data->buffer, data->length );
	else if( data->type == 32 ) return (long)t_factor( (int*)data->buffer, data->length );*/
	return current;
}

JNIEXPORT int prim() {
	simlab* data  = (simlab*)current;
	if( data->type == 66 ) t_prim( (double*)data->buffer, data->length );
	else if( data->type == 34 ) t_prim( (float*)data->buffer, data->length );
	else if( data->type == 33 ) t_prim( (unsigned int*)data->buffer, data->length );
	else if( data->type == 32 ) t_prim( (int*)data->buffer, data->length );
	return current;
}

JNIEXPORT int fibo() {
	simlab* data = (simlab*)current;
	if( data->type == 66 ) t_fibo( (double*)data->buffer, data->length );
	if( data->type == 34 ) t_fibo( (float*)data->buffer, data->length );
	if( data->type == 33 ) t_fibo( (unsigned int*)data->buffer, data->length );
	if( data->type == 32 ) t_fibo( (int*)data->buffer, data->length );
	return current;
}

/*int check( char* buffer, int k, int len ) {
	if( *buffer = name && k < len ) check( buffer+1, k+1, len );

	return k;
}*/

JNIEXPORT int find( simlab* what ) {
	simlab* data = (simlab*)current;
	if( data->type == 66 ) {
//		if( what->type == 66 ) t_wrap( t_find(
	}

	return current;
}

JNIEXPORT int concat( simlab* data ) {
	simlab* con = (simlab*)current;
	simlab* cat = data; //(simlab*)retlib[data];
	if( con->type == 66 ) {
		if( cat->type == 66 ) t_concat( (double*)con->buffer, (double*)cat->buffer, con->length, cat->length );
	}

	return current;
}

JNIEXPORT int sort( simlab cnk, simlab with ) {
	int chunk = data.length;
	if( cnk.type != 0 ) chunk = cnk.buffer;
	if( with.buffer == 0 ) {
		if( data.type == 96 ) {
			for( int i = 0; i < data.length; i+=chunk ) {
				std::sort( ((long double*)data.buffer)+i, ((long double*)data.buffer)+i+chunk );
			}
		} else if( data.type == 66 ) {
			for( int i = 0; i < data.length; i+=chunk ) {
				std::sort( ((double*)data.buffer)+i, ((double*)data.buffer)+i+chunk );
			}
		} if( data.type == 64 ) {
			for( int i = 0; i < data.length; i+=chunk ) {
				std::sort( ((unsigned long long*)data.buffer)+i, ((unsigned long long*)data.buffer)+i+chunk );
			}
		} else if( data.type == 34 ) {
			for( int i = 0; i < data.length; i+=chunk ) {
				std::sort( ((float*)data.buffer)+i, ((float*)data.buffer)+i+chunk );
			}
		} else if( data.type == 32 ) {
			for( int i = 0; i < data.length; i+=chunk ) {
				std::sort( ((int*)data.buffer)+i, ((int*)data.buffer)+i+chunk );
			}
		} else if( data.type == 16 ) {
			for( int i = 0; i < data.length; i+=chunk ) {
				std::sort( ((short*)data.buffer)+i, ((short*)data.buffer)+i+chunk );
			}
		}
	} else {
		int len = mn( data.length, with.length );
		for( int i = 0; i < len; i+=chunk ) {
			if( data.type == 66 ) {
				if( with.type == 66 ) {
					//concat
					//Dual( data->buffer, with->buffer, start, stop );
					//std::sort( ((std::pair*)data->buffer)+i, ((std::pair*)data->buffer)+i+chunk );
				}
			}
		}
	}

	//qsort();
	return current;
}

JNIEXPORT int search( simlab* buffer, simlab* data ) {
	return 0;
}

JNIEXPORT int sys( char* cmd ) {
	system( cmd );
	//_execl( cmd, "" );
	return current;
}

JNIEXPORT int listfunctions() {
	/*char	name[] = "simlab.exe";
	char	buffer[1024];
	long length = GetModuleFileName( hinstance, buffer, sizeof(buffer) );
	if( length != 0 ) {
		FILE* f = fopen( buffer, "rb" );
		if( f != NULL ) {
			length = fread( buffer, 1, sizeof(buffer), f );
			int nl = sizeof(name);
			while( length > 0 ) {
				char* fi;
				char* se = name;
				int i;
				if( nl < sizeof(name) ) {
					for( i = 0; i < length; i+=se-name ) {
						fi = buffer+i;
						se = name+(sizeof(name)-nl);
						nl--;
						while( *(fi++) == *(se++) && (se-name) < sizeof(name) );

						if( (se-name) == sizeof(name) ) {
							printf( "success1 %s\n", fi+1 );
						}
					}
				}
				nl = sizeof(name);
				for( i = 0; i < length-nl; i+=se-name ) {
					fi = buffer+i;
					se = name;
					while( (se-name) < nl && *(fi++) == *(se++) );

					if( (se-name) == sizeof(name) ) {
						buffer[sizeof(buffer)-1] = 0;
						while( fi-buffer < sizeof(buffer) && *fi != 0 ) {
							printf( "%s\n", fi );
							fi += strlen( fi )+1;
						}
						break;
					}
				}
				for( ; i < length; i+=se-name ) {
					fi = buffer+i;
					se = name;
					nl--;
					while( *(fi++) == *(se++) && (se-name) < nl );

					if( (se-name) == nl ) {
						//partialfound = nl;
					}
				}
				length = fread( buffer, 1, sizeof(buffer), f );
			}

			fclose( f );
			free( buffer );
		}
	}*/

	return 0;
}

JNIEXPORT int del() {
	if( data.buffer != 0 ) {
		free( (void*)data.buffer );
		data.buffer = 0;
	}

	return current;
}

//text/plain multipart/form-data
JNIEXPORT int submit() {
	printf( "<form action=simlab.cgi?parse_stdin method=post enctype=multipart/form-data>" );
	printf( "<input type=text name=text>" );
	printf( "<input type=file name=file>" );
	printf( "<input type=submit name=update>" );
	printf( "</form>" );

	return 0;
}

JNIEXPORT int tst( passa<4> str ) {
	printf( "%s\n", (char*)&str );
	return 0;
}

JNIEXPORT int jo() {
	passa<4> p4;
	char* he = "he";
	strcpy( (char*)&p4, he );
	int (*ff)(...);
	ff = (int (*)(...))tst;
	ff( p4 );
	return 0;
}

JNIEXPORT int crnt( simlab newdata ) {
	data = newdata;

	return current;
}

#ifdef JAVA
JNIEXPORT int initjava() {
	JavaVM 			*vm;
    JNIEnv 			*env;
    JavaVMInitArgs 	vm_args;
    JavaVMOption 	options[4];

    options[0].optionString = "-Djava.class.path=/home/sigmar/simlab/simple/bin/:/opt/eclipse/plugins/org.eclipse.equinox.launcher_1.0.0.v20070502.jar";
    options[1].optionString = "-Djava.library.path=/home/sigmar/workspace/simlab/Debug/";
    //options[2].optionString = "-Dosgi.parentClassLoader=app";
    options[2].optionString = "-verbose:class,gc,jni";
    vm_args.version = JNI_VERSION_1_2;
    vm_args.options = options;
    vm_args.nOptions = 3;
    vm_args.ignoreUnrecognized = true;
    //JNI_GetDefaultJavaVMInitArgs(&vm_args);

    int res = JNI_CreateJavaVM(&vm, (void**)&env, (void**)&vm_args);
    javaenv = env;
    jvm = vm;

    //jvm->AttachCurrentThread( (void**)&javaenv, NULL);
    jclass slb = javaenv->FindClass( "Lsimple/Simlab;" );
    //jclass cls = javaenv->FindClass("Lorg/eclipse/equinox/launcher/Main;");

    JNINativeMethod	nm;
	nm.name = "crt";
	nm.signature = "(Ljava/lang/Class;Ljava/lang/Object;)V";
	nm.fnPtr = (void*)Java_simple_Simlab_crt;
	javaenv->RegisterNatives( slb, &nm, 1 );

	nm.name = "get";
	nm.signature = "(I)I";
	nm.fnPtr = (void*)Java_simple_Simlab_get;
	javaenv->RegisterNatives( slb, &nm, 1 );

	nm.name = "cmd";
	nm.signature = "(Ljava/lang/String;)V";
	nm.fnPtr = (void*)Java_simple_Simlab_cmd;
	javaenv->RegisterNatives( slb, &nm, 1 );

	/*jclass mycls = javaenv->FindClass("Lsimple/Simlab;");
	if( mycls != NULL ) {
		jmethodID mid = javaenv->GetStaticMethodID( mycls, "command", "()V" );
		javaenv->ExceptionDescribe();
		printf( "ok %d\n", (long)mid );
		if( mid != NULL ) {
			javaenv->CallStaticVoidMethod( mycls, mid );
		}
	}*/
}

JNIEXPORT int reg() {
	jclass cls = javaenv->FindClass("Lsimple/Simlab;");
	javaenv->ExceptionDescribe();
	printf( "%d\n", (long)cls );

	return 0;
}

JNIEXPORT int gui() {
    if( javaenv == NULL ) initjava();

    jvm->AttachCurrentThread( (void**)&javaenv, NULL);
    //jclass slb = javaenv->FindClass( "Lsimple/Simlab;" );
    jclass cls = javaenv->FindClass("Lorg/eclipse/equinox/launcher/Main;");

    jclass slb = javaenv->FindClass("Lsimple/Simlab;");
    JNINativeMethod	nm;
	nm.name = "obj";
	nm.signature = "(Ljava/lang/Class;Ljava/lang/Object;)V";
	nm.fnPtr = (void*)Java_simple_Simlab_crt;
	javaenv->RegisterNatives( slb, &nm, 1 );

	nm.name = "get";
	nm.signature = "(I)I";
	nm.fnPtr = (void*)Java_simple_Simlab_get;
	javaenv->RegisterNatives( slb, &nm, 1 );

	nm.name = "cmd";
	nm.signature = "(Ljava/lang/String;)V";
	nm.fnPtr = (void*)Java_simple_Simlab_cmd;
	javaenv->RegisterNatives( slb, &nm, 1 );

	if( cls != NULL ) {
		jmethodID mainmid = javaenv->GetStaticMethodID(cls, "main", "([Ljava/lang/String;)V");

		if( mainmid != NULL ) {
			char* cp =  "-launcher /opt/eclipse/eclipse -name Eclipse -showsplash 600 -product simple.product -data /home/sigmar/workspace/../runtime-simple.product -configuration file:/home/sigmar/workspace/.metadata/.plugins/org.eclipse.pde.core/simple.product/ -dev file:/home/sigmar/workspace/.metadata/.plugins/org.eclipse.pde.core/simple.product/dev.properties -os linux -ws gtk -arch x86";
			jstring str = javaenv->NewStringUTF( cp );
			cp = " ";
			jstring spltstr = javaenv->NewStringUTF( cp );

			printf("%s\n", cp);

			jclass strcls = javaenv->GetObjectClass( str );
			jmethodID splitmid = javaenv->GetMethodID( strcls, "split", "(Ljava/lang/String;)[Ljava/lang/String;" );
			jobject splt = javaenv->CallObjectMethod( str, splitmid, spltstr );
			//jstring splt = javaenv->CallObjectMethod( strcls, splitmid, str );*/
			javaenv->CallStaticVoidMethod(cls, mainmid, splt);
			javaenv->ExceptionDescribe();
			//jobject obb = javaenv->FindClass( "Lsimple/Simlab;" );
			//printf( "jojo %d\n", (long)obb );
			//jvm->DestroyJavaVM();
		}
	}

	return current;
}
#endif
}

void* threadrunner( void* func ) {
	((int (*)(...))func)( passnext.big.big.big );

	return NULL;
}

/*double recursive_dfunc( double val, std::queu<double (*)(double)> stck ) {

}*/

/*BOOL APIENTRY DllMain( HANDLE hmodule, DWORD  ul_reason_for_call, LPVOID lpReserved ) {
	hinstance = (HINSTANCE)hmodule;
	//help( NULL );
    return TRUE;
}*/

void commandLoop( int file ) {
		char	command[256];
		fgets( command, sizeof(command), (FILE*)file );
		char quit[] = "quit";
		while( strncmp( command, quit, sizeof(quit)-1 ) ) {
			//cmd( command );
			fgets( command, sizeof(command), (FILE*)file );
		}
}

	/*current = create( 66, 1, 0 );
	set( 0, exp( 1.0 ) );
	store( "e" );

	current = create( 66, 1, 0 );
	set( 0, 1.0 );
	store( "one" );

	current = create( 66, 1, 0 );
	set( 0, 0.0 );
	store( "nil" );*/

/*#ifdef WIN
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow ) {
	hinstance = hInstance;
	start();
}
#else*/
//#endif
