/*
 * java.cc
 *
 *  Created on: Jan 13, 2009
 *      Author: root
 */

#ifdef JAVA
JNIEXPORT void JNICALL Java_simple_Simlab_crt(JNIEnv *env, jclass cls, jclass thecls, jobject theobj ) {
	jcls = thecls;
	jobj = theobj;
}

/*
 * Class:     org_simmi_Simlab
 * Method:    cmd
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_simple_Simlab_cmd(JNIEnv *env, jclass cls, jstring str ) {
	//prnt = jprintf;
	jboolean iscopy;
	const char* command = env->GetStringUTFChars( str, &iscopy );
	javaenv = env;
	cmd( (char*)command );
	env->ReleaseStringUTFChars( str, command );

	env->ExceptionDescribe();

	//fclose( sout );
	//FILE* fsout = fopen( "OUT", "rb" );
	//fseek( sout, 0, SEEK_SET );
	/*int rd = fread( cc, 1, 1000, fsout );
	cc[rd] = 0;
	gothere( cc );
	char* ccc = &cc[900];
	sprintf( ccc, "%d", rd );
	gothere( ccc );
	fclose( fsout );
	sout = stdout;*/
}

JNIEXPORT jboolean JNICALL Java_simple_Simlab_isSelected(JNIEnv *env, jclass cls, jstring str ) {
	jboolean ret = 0;

	jboolean iscopy;
	const char* name = env->GetStringUTFChars( str, &iscopy );

	simlab ndata = retlib[name];
	//int val = memcmp( &data, &ndata, sizeof(data) );
	ret = (ndata.buffer == data.buffer);
	//if( val == 0 ) ret = 1;

	env->ReleaseStringUTFChars( str, name );

	return ret;
}

JNIEXPORT void JNICALL Java_simple_Simlab_setobject(JNIEnv *env, jclass, jobject obj) {
	//printf( "heyrdu %d %d %d\n", (int)env->GetDirectBufferCapacity( obj ), (int)obj, (int)env );
	jobject gref = env->NewGlobalRef( obj );
	env->DeleteLocalRef( obj );

	current = (int)gref;
}

JNIEXPORT jobject JNICALL Java_simple_Simlab_getobject(JNIEnv *env, jclass) {
	return (jobject)current;
}

JNIEXPORT void JNICALL Java_simple_Simlab_setdata(JNIEnv *env, jclass, jobject buffer) {
	data.buffer = (int)env->GetDirectBufferAddress( buffer );
	data.length = env->GetDirectBufferCapacity( buffer );

	if( env->IsInstanceOf( buffer, env->FindClass( "Ljava/nio/DoubleBuffer;" ) ) ) {
		data.type = 66;
	} else if( env->IsInstanceOf( buffer, env->FindClass( "Ljava/nio/FloatBuffer;" ) ) ) {
		data.type = 34;
	} else if( env->IsInstanceOf( buffer, env->FindClass( "Ljava/nio/IntBuffer;" ) ) ) {
		data.type = 32;
	} else if( env->IsInstanceOf( buffer, env->FindClass( "Ljava/nio/ByteBuffer;" ) ) ) {
		data.type = 8;
	} else {
		data.type = 8;
	}

	//return (jobject)current;
}

JNIEXPORT jint JNICALL Java_simple_Simlab_get(JNIEnv *env, jclass cls, jint i) {
	int* buffer = (int*)data.buffer;
	return buffer[i];
}

JNIEXPORT jobject JNICALL Java_simple_Simlab_getdata(JNIEnv *env, jclass) {
	jobject dbb = env->NewDirectByteBuffer( (void*)data.buffer, bytelength( data.type, data.length ) );
	jclass  bbc = env->GetObjectClass( dbb );
	if( dbb != NULL ) {
		jclass byteorderclass = env->FindClass( "Ljava/nio/ByteOrder;" );
		if( byteorderclass != NULL ) {
			jmethodID mid = env->GetStaticMethodID( byteorderclass, "nativeOrder", "()Ljava/nio/ByteOrder;" );
			if( mid != NULL ) {
				jobject byteorder = env->CallStaticObjectMethod( byteorderclass, mid ); //env->CallObjectMethod( dbb, mid );
				if( byteorder != NULL ) {
					mid = env->GetMethodID( bbc, "order", "(Ljava/nio/ByteOrder;)Ljava/nio/ByteBuffer;" );
					if( mid != NULL ) env->CallObjectMethod( dbb, mid, byteorder );
					else printf( "order not found\n" );
				} else printf( "byteorder obj null\n" );
			} else printf( "nativeorder method not found\n" );
		} else printf( "byteorderclass null\n" );
		if( data.type == 66 ) {
			jmethodID mid = env->GetMethodID( bbc, "asDoubleBuffer", "()Ljava/nio/DoubleBuffer;" );
			if( mid != NULL ) return env->CallObjectMethod( dbb, mid );
		} else if( data.type == 34 ) {
			jmethodID mid = env->GetMethodID( bbc, "asFloatBuffer", "()Ljava/nio/FloatBuffer;" );
			if( mid != NULL ) return env->CallObjectMethod( dbb, mid );
		} else if( data.type == 32 ) {
			jmethodID mid = env->GetMethodID( bbc, "asIntBuffer", "()Ljava/nio/IntBuffer;" );
			if( mid != NULL ) {
				jobject lref = env->CallObjectMethod( dbb, mid );
				return lref;
			}
		} else {
			jobject retobj = javaenv->NewGlobalRef( dbb );
			javaenv->DeleteLocalRef( dbb );

			return retobj;
		}
	}
	return NULL;
}
#endif

#ifdef JAVA
JNIEXPORT int method( char* name, char* signature, ... ) {
	if( javaenv != NULL && current != 0 ) {
		jobject obj = (jobject)current;
		jclass cls = javaenv->GetObjectClass( obj );
		jmethodID mid = javaenv->GetMethodID( cls, name, signature );

		if( mid != NULL ) {
			/*va_list args;
			va_start(args, signature);

			char* bp = (char*)&passargs;

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
					jboolean bl = va_arg( args, int );
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
				i++;
			}
			va_end( args );*/

			//printf( "about to call %s   %s\n", name, signature );
			//javaenv->CallVoidMethod( obj, mid, passargs );
		} else printf( "%s   %s\n", name, signature );
	}

	return current;
}

JNIEXPORT int callO( char* name, ... ) {
	if( javaenv != NULL && current != 0 ) {
		//jobject obj = (jobject)current;
		//jclass cls = javaenv->GetObjectClass( obj );

		//sig = "(Ljava/awt/Component;)Ljava/lang/Object;";
		//sig += ")Ljava/awt/Component;";
		//sig = "("+sig.substr( 2, sig.length() );
		/*jmethodID mid = javaenv->GetMethodID( cls, name, sig.c_str() );
		while( mid == NULL ) {
			cls = javaenv->GetSuperclass( cls );
			//javaenv->from
			mid = javaenv->GetMethodID( cls, name, sig.c_str() );

			//javaenv->is
		}

		va_list args;
		va_start(args, name);

		byte* bp = (byte*)&passargs;

		printf( sig.c_str() );

		int i = 0;
		while( sig[i] != ')' ) {
			if( sig[i] == 'I' ) {
				jint val = va_arg( args, int );
				int size = sizeof( jint );
				memcpy( bp, &val, size );
				bp += size;
			} else if( sig[i] == 'D' ) {
				jdouble val = va_arg( args, double );
				int size = sizeof( jdouble );
				memcpy( bp, &val, size );
				bp += size;
			} else if( sig[i] == 'Z' ) {
				jboolean bl = va_arg( args, unsigned char );
				int size = sizeof( jboolean );
				memcpy( bp, &bl, size );
				bp += size;
			} else if( sig[i] == 'S' ) {
				//int k = i;
				char* cp = va_arg( args, char* );
				jstring str = javaenv->NewStringUTF( cp );
				int size = sizeof( jstring );
				memcpy( bp, &str, size );
				bp += size;
			} else if( sig[i] == 'L' ) {
				//int k = i;
				while( sig[i] != ';' ) i++;
				jobject obj = (jobject)va_arg( args, int );
				//jstring str = javaenv->NewStringUTF( cp );
				int size = sizeof( jobject );
				memcpy( bp, &obj, size );
				bp += size;
			}
			i++;
		}
		va_end( args );
		current = (int)javaenv->CallObjectMethod( obj, mid, passargs );*/
	}

	return current;
}

JNIEXPORT int erm() {
	printf( "%d\n", sizeof( jobject ) );
	return current;
}
#endif
