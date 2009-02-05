/*
 * view.cc
 *
 *  Created on: Jan 20, 2009
 *      Author: root
 */

#include "simlab.h"

#ifdef GTK
#include <gtk/gtk.h>
#ifdef GL
#include <gtk/gtkgl.h>
#endif
#endif

#ifdef GL
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <map>
#include <vector>
#include <string.h>

extern passa<4> 	passnext;
extern simlab		data;

JNIEXPORT int run( simlab runner );

typedef struct tfour {
	unsigned char one;
	unsigned char two;
	unsigned char tre;
} tfour;

typedef struct glstuff {
	int		drawtype;
	int		buffertype;
	int		stride;
	simlab	data;
} glstuff;
std::vector<glstuff>	glbuffer;

#ifdef GTK
JNIEXPORT int thread( simlab cmd ) {
#ifdef PTHREAD
	pthread_t	thread;
	pthread_create( &thread, NULL, threadrunner, (void*)cmd.buffer );
#else
	GError*	error;
	g_thread_create( (GThreadFunc)cmd.buffer, (gpointer)*((int*)(&passnext.big)), false, &error );
#endif
	return 0;
}

static gboolean image_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data ) {
  //GdkGLContext *glcontext = gtk_widget_get_gl_context(widget);
  //GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(widget);

	//int w = widget->allocation.width;
	//int h = widget->allocation.height;

	//gdk_draw_image( widget->window, widget->style->fg_gc[GTK_WIDGET_STATE (widget)], (GdkImage*)data, 0, 0, 0, 0, w, h );

  return TRUE;
}

static gboolean image_configure_event (GtkWidget *widget, GdkEventConfigure *event, gpointer dat ) {
	GdkImage* img = (GdkImage*)dat;
	int width = img->width;
	int height = img->height;
	//printf("erm %d\n", data.type );
	if( data.type > 0 ) {
		if( data.type == 66 ) {
	    	for( int i = 0; i < width*height; i++ ) {
	    		gdk_image_put_pixel( img, i%width, i/width, (unsigned int)((double*)data.buffer)[i] );
	    	}
	    } else if( data.type == 32 ) {
	    	for( int i = 0; i < width*height; i++ ) {
	    		gdk_image_put_pixel( img, i%width, i/width, (unsigned int)((int*)data.buffer)[i] );
	    	}
	    } else if( data.type == 24 ) {
	    	for( int i = 0; i < width*height; i++ ) {
	    		unsigned int cval = 0;
	    		memcpy( &cval, (void*)&((tfour*)data.buffer)[i], sizeof(tfour) );
	    	   	gdk_image_put_pixel( img, i%width, i/width, cval );
	    	}
	    }
	} else {
		if( data.type == -66 ) {
			c_simlab<double> & sl = *(c_simlab<double>*)data.buffer;
	    	for( int i = 0; i < width*height; i++ ) {
	    		gdk_image_put_pixel( img, i%width, i/width, (unsigned int)sl[i] );
	    	}
	    } if( data.type == -64 ) {
	    	c_simlab<long long> & sl = *(c_simlab<long long>*)data.buffer;
	    	for( int i = 0; i < width*height; i++ ) {
	    		gdk_image_put_pixel( img, i%width, i/width, (unsigned int)sl[i] );
	    	}
	    } else if( data.type == -32 ) {
	    	c_simlab<int> & sl = *(c_simlab<int>*)data.buffer;
	    	for( int i = 0; i < width*height; i++ ) {
	    		gdk_image_put_pixel( img, i%width, i/width, (unsigned int)sl[i] );
	    	}
	    }
	}

	return TRUE;
}

std::map<int,simlab>	timers;
gint timeout_callback(gpointer image) {
	GtkImage *imgWidget = (GtkImage*)image;
	GdkImage *img;
	gtk_image_get_image( imgWidget, &img, NULL );
	simlab & tmr = timers[(int)img];
	run(tmr);
	image_configure_event( NULL, NULL, img );
	gtk_widget_queue_draw( (GtkWidget*)imgWidget );

	return 1;
}

#ifdef GL
void drawGL() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColor3f( 1.0f, 1.0f, 1.0f );
	std::vector<glstuff>::iterator it = glbuffer.begin();
	while( it != glbuffer.end() ) {
		glInterleavedArrays( it->buffertype, 0, (GLvoid*)it->data.buffer );
		glDrawArrays( it->drawtype, 0, it->data.length/it->stride );
		it++;
	}
}

gint gl_timeout_callback(gpointer widget) {
	//GtkImage *imgWidget = (GtkImage*)image;
	//GdkImage *img;
	//gtk_image_get_image( imgWidget, &img, NULL );
	/*gdk_draw_rectangle (pixmap,
      widget->style->black_gc,
      TRUE,
      100, (i++)*5,8,8);*/

	simlab & tmr = timers[(int)widget];
	run(tmr);

	//run(timers[(int)img]);
	//printf("%d\n",(int)imgWidget);
	//image_configure_event( NULL, NULL, img );
	//gtk_image_expose();
	//gtk_signal_emit_by_name( (GtkObject*)imgWidget, "expose_event" );
	gtk_widget_queue_draw( (GtkWidget*)widget );

	return 1;
}

static gboolean expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data ) {
  GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
  GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);

  if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext))
    return FALSE;

  drawGL();

  /*glColor3f( 1.0f, 1.0f, 1.0f );
	if( data.length >= chunk.buffer ) {
		//glEnableClientState( GL_VERTEX_ARRAY );
		//glVertexPointer( chunk.buffer, GL_DOUBLE, 0, (GLvoid*)data.buffer );
		glInterleavedArrays( GL_V3F, 0, (GLvoid*)data.buffer );
		glDrawArrays( what.buffer, 0, data.length/chunk.buffer ); //data.length/3 ); //start.buffer, end.buffer );
	}*/

  if (gdk_gl_drawable_is_double_buffered (gldrawable))
    gdk_gl_drawable_swap_buffers (gldrawable);
  else
    glFlush ();

  gdk_gl_drawable_gl_end (gldrawable);

  return TRUE;
}

static gboolean configure_event (GtkWidget *widget, GdkEventConfigure *event, gpointer data ) {
  GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
  GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);

  int w = widget->allocation.width;
  int h = widget->allocation.height;

  if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext)) return FALSE;
  glViewport (0, 0, (GLsizei)w, (GLsizei)h );
  /*glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  glGetDoublev( GL_PROJECTION_MATRIX, n );
  glGetDoublev( GL_PROJECTION_MATRIX, o );
  if (w > h) {
      aspect = w / h;
      glFrustum (-aspect, aspect, -1.0, 1.0, 1.0, 500.0);
  } else {
      aspect = h / w;
      glFrustum (-1.0, 1.0, -aspect, aspect, 1.0, 500.0);
  }
  //glGetDoublev( GL_PROJECTION_MATRIX, o );
  glMatrixMode (GL_MODELVIEW);*/
  gdk_gl_drawable_gl_end (gldrawable);

  return TRUE;
}

bool fscreen = false;
static gboolean keypress_event(GtkWidget *widget, GdkEventKey *event, gpointer data ) {
    char ch = event->string[0];

    if( ch == 'f' ) {
    	fscreen = !fscreen;

    	if( fscreen ) gtk_window_fullscreen( GTK_WINDOW(widget) );
    	else gtk_window_unfullscreen( GTK_WINDOW(widget) );
    }

    return TRUE;
}

static gboolean buttonpress_event(GtkWidget *widget, GdkEventKey *event, gpointer data ) {
    char ch = event->string[0];

    //if( ch == 'f' ) {
    	fscreen = !fscreen;

    	if( fscreen ) gtk_window_fullscreen( GTK_WINDOW(widget) );
    	else gtk_window_unfullscreen( GTK_WINDOW(widget) );
    //}

    return TRUE;
}

JNIEXPORT int addtoglbuffer( simlab buffertype, simlab drawtype, simlab stride ) {
	glstuff	stuff;
	stuff.data = data;
	stuff.drawtype = drawtype.buffer;
	stuff.buffertype = buffertype.buffer;
	stuff.stride = stride.buffer;
	glbuffer.push_back( stuff );

	return 0;
}

JNIEXPORT int surface( simlab timer, simlab time ) {
	GtkWidget*    window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
    g_signal_connect( window, "destroy", gtk_main_quit, NULL );
    GtkWidget*    draw = gtk_drawing_area_new();
    gtk_container_add( GTK_CONTAINER(window), draw );

    GdkGLConfig* glconfig = gdk_gl_config_new_by_mode( (GdkGLConfigMode)(GDK_GL_MODE_RGB | GDK_GL_MODE_DEPTH | GDK_GL_MODE_DOUBLE) );
    gtk_widget_set_gl_capability( draw, glconfig, NULL, TRUE, GDK_GL_RGBA_TYPE );
    g_signal_connect( G_OBJECT(draw), "expose_event", G_CALLBACK(expose_event), NULL);
    g_signal_connect( G_OBJECT(draw), "configure_event", G_CALLBACK(configure_event), NULL);
    g_signal_connect( G_OBJECT(window), "key_press_event", G_CALLBACK(keypress_event), NULL );
    g_signal_connect( G_OBJECT(draw), "button_press_event", G_CALLBACK(buttonpress_event), NULL );
    int tag = 0;
	if( timer.type == 96 ) {
		timers[(int)draw] = timer;
		tag = g_timeout_add( time.buffer, gl_timeout_callback, draw );
	}

	gtk_widget_show( draw );
	gtk_widget_show( window );

	gtk_main();

	if( tag != 0 ) g_source_remove( tag );
	if( timer.type == 96 ) return 2;
	return 0;
}

JNIEXPORT int window( simlab timer, simlab time ) {
	GtkWidget*    window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
    g_signal_connect( window, "destroy", gtk_main_quit, NULL );
    //gtk_window_set_title( GTK_WINDOW(window), (char*)name.buffer );
    //gtk_widget_set_size_request( window, 800, 600 );

    GtkWidget*    draw = gtk_drawing_area_new();
    gtk_container_add( GTK_CONTAINER(window), draw );

    GdkGLConfig* glconfig = gdk_gl_config_new_by_mode( (GdkGLConfigMode)(GDK_GL_MODE_RGB | GDK_GL_MODE_DEPTH | GDK_GL_MODE_DOUBLE) );
    gtk_widget_set_gl_capability( draw, glconfig, NULL, TRUE, GDK_GL_RGBA_TYPE );
    g_signal_connect( G_OBJECT(draw), "expose_event", G_CALLBACK(expose_event), NULL);
    g_signal_connect( G_OBJECT(draw), "configure_event", G_CALLBACK(configure_event), NULL);
    g_signal_connect( G_OBJECT(window), "key_press_event", G_CALLBACK(keypress_event), NULL );
    //g_signal_connect( G_OBJECT(window), "key_release_event", G_CALLBACK(keyrelease_event), NULL );
    //int tag = g_timeout_add( 10, timeout_callback, window );

    int tag = 0;
	if( timer.type == 96 ) {
		printf("install timer %d\n", timer.buffer);
		timers[(int)draw] = timer;
		tag = g_timeout_add( time.buffer, gl_timeout_callback, draw );
	}

	gtk_widget_show( draw );
	gtk_widget_show( window );

	gtk_main();

	if( tag != 0 ) g_source_remove( tag );
	if( timer.type == 96 ) return 2;
	return 0;

    /*int tag = 0;
    if( timer.buffer != 0 ) {
    	timers[(int)draw] = timer;
    	tag = g_timeout_add( time.buffer, gl_timeout_callback, draw );
    } else if( time.buffer != 0 ) {
    	tag = g_timeout_add( time.buffer, gl_timeout_callback, draw );
    }

    gtk_widget_show( draw );
    gtk_widget_show( window );

    gtk_main();
    if( tag != 0 ) g_source_remove( tag );*/

    //return 0;
}
#endif

JNIEXPORT int image( simlab chunk, simlab chunk2, simlab timer, simlab time ) {
	GtkWidget*    window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
    g_signal_connect( window, "destroy", gtk_main_quit, NULL );
    gtk_window_set_title( GTK_WINDOW(window), (char*)"image" );//name.buffer );
    //gtk_widget_set_size_request( window, 800, 600 );

    //int val = chunk.buffer;

    int w = chunk.buffer;
    int h = chunk2.buffer;

    GtkWidget*    scrolled_window = gtk_scrolled_window_new( NULL, NULL );
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add( GTK_CONTAINER(window), scrolled_window );

    GdkImage*	img = gdk_image_new( GDK_IMAGE_NORMAL, gdk_visual_get_system(), w, h);

    image_configure_event( NULL, NULL, img );

    GtkWidget* 	imgWidget = gtk_image_new_from_image( img, NULL );
    gtk_scrolled_window_add_with_viewport( (GtkScrolledWindow*)scrolled_window, imgWidget );

    //g_signal_connect( G_OBJECT(scrolled_window), "expose_event", G_CALLBACK(image_expose_event), img);
    //g_signal_connect( G_OBJECT(scrolled_window), "configure_event", G_CALLBACK(image_configure_event), img);
    //g_signal_connect( G_OBJECT(window), "key_press_event", G_CALLBACK(keypress_event), NULL );
    //g_signal_connect( G_OBJECT(window), "key_release_event", G_CALLBACK(keyrelease_event), NULL );
    int tag = 0;
    if( timer.type == 96 ) {
    	printf("install timer %d\n", timer.buffer);
    	timers[(int)img] = timer;
    	tag = g_timeout_add( time.buffer, timeout_callback, imgWidget );
    }

    gtk_widget_show( imgWidget );
    gtk_widget_show( scrolled_window );
    gtk_widget_show( window );

    gtk_main();

    if( tag != 0 ) g_source_remove( tag );

    if( timer.type == 96 ) return 3;
    else return 1;
}
#endif
