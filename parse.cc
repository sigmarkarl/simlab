/*
 * parse.cc
 *
 *  Created on: Jan 3, 2009
 *      Author: root
 */

#include "simlab.h"

#include <stdio.h>
#include <string.h>
#include <vector>
#include <string>

JNIEXPORT int fetch( simlab what );
JNIEXPORT int echo( simlab what );
JNIEXPORT int resize( simlab what );

extern int	  passcurr;
extern simlab prev;
extern simlab data;
extern passa<4>	passnext;
extern int module;

int			bsize;

int parseParameters( int bytesize ) {
	char *result = strtok( NULL, " ,)\n" );
	if( result != NULL ) {
		if( result[0] == '"' || result[0] == '.' ) {
			std::string str = result;
			if( str[ str.length()-1 ] != '"' ) {
				char *rs = strtok( NULL, "\"" );
				str += " ";
				str += rs;
				str += "\"";
			}

			char*	c_str = new char[ str.length()-1 ];
			str.copy( c_str, str.length()-2, 1 );
			c_str[ str.length() - 2 ] = 0;

			simlab there;
			there.buffer = (long)c_str;
			there.type = 8;
			there.length = str.size() - 2;
			char* here = (char*)&passnext;
			int size = sizeof(data); // len-2
			memcpy( here+bytesize, &there, size );

			return parseParameters( bytesize+size );
		} else if( result[0] == '[' ) {
			int len = strlen(result);
			std::vector<double>		d_vec;
			float					fval;
			sscanf( result+1, "%e", &fval );
			d_vec.push_back( (double)fval );

			result = strtok( NULL, " ,)\n" );
			len = strlen(result);
			while( result[ len-1 ] != ']' ) {
				sscanf( result, "%e", &fval );
				d_vec.push_back( (double)fval );
				result = strtok( NULL, " ,)\n" );
				len = strlen(result);
			}
			sscanf( result, "%e]", &fval );
			d_vec.push_back( (double)fval );

			//t_simlab<double>		data;
			simlab adata;
			adata.length = d_vec.size();
			adata.type = 66;
			adata.buffer = (long)new double[adata.length];
			memcpy( (void*)(adata.buffer), &d_vec[0], adata.length*sizeof(double) );

			char* here = (char*)&passnext;
			memcpy( here+bytesize, &adata, sizeof(simlab) );
			//sig += "S";
			return parseParameters( bytesize+sizeof(simlab) );
		} else if( result[0] == '-' ) {
			int value = result[1] - '0';
			int i = 2;
			int mul = 1;
			int add = 0;
herem:
			while( result[i] != 0 && result[i] != '.' && result[i] != '*' ) {
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
				float fvalue = -(float)dvalue;
				//printf( "%f %d\n", fvalue, sizeof(fvalue) );
				char* here = (char*)&passnext;
				here += bytesize;
				simlab fval;
				fval.buffer = *((int*)&fvalue);
				fval.type = 34;
				fval.length = 0;
				memcpy( here, &fval, sizeof(simlab) );
				return parseParameters( bytesize+sizeof(simlab) );
			} else if( result[i] == '*' ) {
				mul *= value;
				i++;
				value = result[i] - '0';
				i++;
				goto herem;
			} else  {
				char* here = (char*)&passnext;
				here += bytesize;
				simlab val;
				val.buffer = -(long)(value*mul+add);
				val.type = 32;
				val.length = 0;

				//char cc[100];
				//sprintf( cc, "%d", val.buffer );
				//prnt(cc);

				memcpy( here, &val, sizeof(simlab) );
				return parseParameters( bytesize+sizeof(simlab) );
			}
			/*int value = result[1] - '0';
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
				dvalue = -dvalue;
				char* here = (char*)&passnext;
				here += bytesize;
				//sig += "D";
				memcpy( here, &dvalue, sizeof(double) );
				return parseParameters( bytesize+sizeof(double) );
			} else  {
				value = -value;
				char* here = (char*)&passnext;
				here += bytesize;
				//sig += "I";
				memcpy( here, &value, sizeof(long) );
				return parseParameters( bytesize+sizeof(long) );
			}*/
		} else if( result[0] >= '0' && result[0] <= '9' ) {
			int value = result[0] - '0';
			int i = 1;
			int mul = 1;
			int add = 0;
here:
			while( result[i] != 0 && result[i] != '.' && result[i] != '*' ) {
				value *= 10;
				value += result[i] - '0';
				i++;
			}
			if( result[i] == '.' ) {
				float fvalue = (float)value;
				double mnt = 1.0;
				int k = i+1;
				while( result[k] != 0 ) {
					mnt *= 10.0;
					fvalue += (result[k] - '0')/mnt;
					k++;
				}
				char* here = (char*)&passnext;
				here += bytesize;
				simlab fval;
				fval.buffer = *((int*)&fvalue);
				fval.type = 34;
				fval.length = 0;
				memcpy( here, &fval, sizeof(simlab) );
				return parseParameters( bytesize+sizeof(simlab) );
			} else if( result[i] == '*' ) {
				mul *= value;
				i++;
				value = result[i] - '0';
				i++;
				goto here;
			} else  {
				char* here = (char*)&passnext;
				here += bytesize;
				simlab val;
				val.buffer = (long)(value*mul+add);
				val.type = 32;
				val.length = 0;

				//char cc[100];
				//sprintf( cc, "%d", val.buffer );
				//prnt(cc);

				memcpy( here, &val, sizeof(simlab) );
				return parseParameters( bytesize+sizeof(simlab) );
			}
		} else {
			int fnc = dsym( module, result );
			if( strcmp( result, "prev" ) == 0 ) {
				char* here = (char*)&passnext;
				here += bytesize;
				memcpy( here, &prev, sizeof(simlab) );
				return parseParameters( bytesize+sizeof(simlab) );
			} else if( strcmp( result, "len" ) == 0 ) {
				char* here = (char*)&passnext;
				here += bytesize;
				simlab len;
				len.buffer = data.length;
				len.type = 32;
				len.length = 0;
				memcpy( here, &len, sizeof(simlab) );
				return parseParameters( bytesize+sizeof(simlab) );
			} else if( fnc != 0 ) {
				char* here = (char*)&passnext;
				here += bytesize;
				simlab fdata;
				fdata.buffer = fnc;
				fdata.length = 0;
				fdata.type = 32;
				memcpy( here, &fdata, sizeof(simlab) );
				return parseParameters( bytesize+sizeof(simlab) );
			} else {
				simlab ftch;
				ftch.buffer = (long)result;
				ftch.type = 8;
				ftch.length = strlen( result );
				simlab tmp = data;
				fetch( ftch );

				char* here = (char*)&passnext;
				here += bytesize;
				memcpy( here, &data, sizeof(simlab) );

				data = tmp;
				return parseParameters( bytesize+sizeof(simlab) );
			}

			/*} else {
				int val = 0;
				if( result[0] == 't' && result[1] == 'r' && result[2] == 'u' ) val = 1;
				char* here = (char*)&passnext;
				memcpy( here+bytesize, &val, sizeof(long) );
				return parseParameters( func, bytesize+sizeof(long) );
			}*/
		}
	} else {
		//printf( "null res parse\n" );

		char* here = (char*)&passnext;
		here += bytesize;
		int value = 0;
		memcpy( here, &value, sizeof(long) );
	}

	bsize = bytesize;
	return bytesize;
}

JNIEXPORT int run( simlab runner ) {
	simlab olddata = data;

	if( runner.buffer == 0 ) runner = data;
	simlab* databuffer = (simlab*)runner.buffer;
	int i = 0;
	//printf("%d\n", runner.length);
	while( i < runner.length ) {
		simlab & val = databuffer[i];
		int cmd = val.buffer;
		int ret = ((int (*)( ... ))cmd)( *((passa<12>*)(&databuffer[i+1])) );
		i += ret;
		i++;
	}

	data = olddata;

	return 1;
}

JNIEXPORT int compile( simlab fnc, ... ) {
	//passcurr += sizeof(long);
	//int (*func)( ... );
	//func = (int (*)(...))fnc.buffer;
	//return func( *(passa<11>*)(passcurr) )+1;

	//printf("%d\n",bsize);
	//bsize -= 12;
	int datasize = (bsize+11)/12;
	if( data.buffer == 0 ) {
		data.length = datasize+1;
		data.type = 96;
		data.buffer = (long)new simlab[ data.length ];
	} else {
		int nz = data.length+datasize+1;
		if( nz > data.length ) {
			simlab newsize;
			newsize.buffer = nz;
			newsize.type = 32;
			newsize.length = 0;
			resize( newsize );
		}
	}
	simlab*	databuffer = (simlab*)data.buffer;
	int ind = data.length-datasize-1;
	simlab & subdata = databuffer[ind];
	subdata = fnc;
	if( bsize > 0 ) memcpy( &databuffer[data.length-datasize], &passnext, bsize );
}

JNIEXPORT int interprete( simlab cmd ) {
	char*	command = (char*)cmd.buffer;
	if( *command == '"' ) {
		command[ strlen(command)-1 ] = 0;
		simlab str;
		str.buffer = (long)(command+1);
		echo( str );
	} else {
		char*	result = strtok( command, " (_\n" );
		int func = dsym( module, result );
		if( func != 0 /*&& (jobj == 0 || jcls == 0 || func == (long)store || func == (long)fetch || func == (long)Class || func == (long)Data || func == (long)create)*/ ) {
			simlab fnc;
			fnc.buffer = func;
			fnc.type = 32;
			fnc.length = 0;
			parseParameters( 0 );
			compile( fnc, passnext );

			//passcurr = (long)&passnext;
			//((int (*)(...))func)( passnext );
			//if( memcmp( &old, &data, sizeof(data) ) == 0 ) prev = old;
		}
	}
	return 1;
}

JNIEXPORT int cmd( simlab cmnd ) {
	char* command = (char*)cmnd.buffer;

	if( *command == '"' ) {
		command[ strlen(command)-1 ] = 0;
		simlab str;
		str.buffer = (long)(command+1);
		echo( str );
	} else { //if( *command != '\n' ) {
		char*	result = strtok( command, " (_\n" );
		int func = dsym( module, result );

		if( func != 0 ) {//&& (java == 0 || func == (long)store || func == (long)fetch || func == (long)Class || func == (long)New || func == (long)Data || func == (long)create) ) {
			simlab old = data;
			memset( &passnext, 0, sizeof(passnext) );
			parseParameters( 0 );
			passcurr = (long)&passnext;
			//if( *(int*)&passnext != 0 ) printf( "%s\n", (char*)*(int*)&passnext );
			((int (*)(...))func)( passnext );
			if( old.buffer != 0 && (data.buffer < old.buffer || data.buffer > old.buffer+bytelength(old.type,old.length)) ) {
				prev = old;
			}
		}
#ifdef JAVA
		else if( strlen( result ) > 0 && javaenv != NULL ) {
			std::string sig = "(";
			numpar = 0;
			jvm->AttachCurrentThread( (void**)&javaenv, NULL );

			/*jclass cls = javaenv->FindClass( "Lsimple/Simlab;" );
			JNINativeMethod	nm;
			nm.name = "crt";
			nm.signature = "(Ljava/lang/Class;Ljava/lang/Object;)V";
			nm.fnPtr = (void*)Java_simple_Simlab_crt;
			javaenv->RegisterNatives( cls, &nm, 1 );

			nm.name = "get";
			nm.signature = "(I)I";
			nm.fnPtr = (void*)Java_simple_Simlab_get;
			javaenv->RegisterNatives( cls, &nm, 1 );

			nm.name = "cmd";
			nm.signature = "(Ljava/lang/String;)V";
			nm.fnPtr = (void*)Java_simple_Simlab_cmd;
			javaenv->RegisterNatives( cls, &nm, 1 );

			//jclass cls = javaenv->FindClass("Lsimple/Simlab;");
			jfieldID fld = javaenv->GetStaticFieldID( cls, "cls", "Ljava/lang/Class;" );
			jcls = (jclass)javaenv->GetStaticObjectField( cls, fld );
			fld = javaenv->GetStaticFieldID( cls, "obj", "Ljava/lang/Object;" );
			jobj = javaenv->GetStaticObjectField( cls, fld );*/

			parseJavaParameters( result, sig, 0 );

			javaenv->ExceptionDescribe();
			//jobj = (jobject)current;
		}
#endif
	}

	return 1;
}

FILE*	currf;
JNIEXPORT int parse( simlab fname, simlab func ) {
	data.buffer = 0;
	data.length = 0;
	data.type = 32;

	FILE*	f = stdin;
	char	line[256];
	if( fname.type == 8 ) {
		char* 	filename = (char*)(fname.buffer);
		if( filename != 0 ) {
			if( strcmp( filename, "this" ) == 0 ) {
				f = currf;
			} else if( strcmp( filename, "stdin" ) != 0 ) {
				f = fopen( filename, "r" );
			}
		}
	} else if( fname.buffer != 0 ) {
		f = (FILE*)fname.buffer;
	}

	currf = f;

	int (*fnc)( simlab );
	if( func.buffer == 0 ) fnc = cmd;
	else {
		fnc = (int (*)(simlab))func.buffer;
	}

	char quit[] = "quit";
	char* res = fgets( line, sizeof(line), f );
	simlab s_line;
	s_line.type = 8;

	while( res != NULL && strncmp( line, quit, sizeof(quit)-1 ) ) {
		s_line.buffer = (long)line;
		s_line.length = strlen(line);
		if( *line != '\n' ) fnc( s_line );
		res = fgets( line, sizeof(line), (FILE*)f );
	}
	//if( f != stdin ) fclose( f );

	return 1;
}
