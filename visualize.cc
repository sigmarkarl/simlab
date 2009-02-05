/*
 * visualize.cc
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

#include <math.h>
#include <map>

JNIEXPORT int run( simlab v );
extern std::map<int,simlab>	timers;
gint old_timeout_callback(gpointer image) {
	//GtkImage *imgWidget = (GtkImage*)image;
	GdkImage *img = (GdkImage*)image;
	simlab & tmr = timers[(int)img];
	run(tmr);
	//image_configure_event( NULL, NULL, img );
	//gtk_widget_queue_draw( (GtkWidget*)imgWidget );

	return 1;
}

float t = 0.0f;
gboolean expose( GtkWidget* widget, GdkEventExpose* event, gpointer data ) {
	GdkRectangle &	rect = event->area;
	GdkGC* gc = widget->style->fg_gc[GTK_WIDGET_STATE (widget)];

	int width = rect.width;
	int height = rect.height;

	//GdkColor color;
	//color.red = 255;
	//color.blue = 255;
	//color.green = 255;
	//color.pixel = color.red*65536 + color.green*256 + color.blue;// + 64*65536*256;
	//gdk_gc_set_foreground(gc, &color);

	float ls[6];
	ls[0] = cosf(t)*100.0f;
	ls[1] = 0.0f;
	ls[2] = sinf(t)*100.0f;
	ls[3] = 0.5f;
	ls[4] = 100.0f;
	ls[5] = 100.0f;
	ls[6] = -100.0f;
	ls[7] = 0.1f;

	float	xs[14];
	xs[0] = 0.0f;
	xs[1] = 0.0f;
	xs[2] = 255.0f;
	xs[3] = 64.0f;
	xs[4] = 255.0f;
	xs[5] = 0.0f;
	xs[6] = 0.0f;

	xs[7] = 50.0f;
	xs[8] = 50.0f;
	xs[9] = 255.0f;
	xs[10] = 48.0f;
	xs[11] = 0.0f;
	xs[12] = 255.0f;
	xs[13] = 0.0f;

	float x1 = 0.0f;
	float y1 = 0.0f;
	float z1 = -255.0f;

	GdkImage* image = (GdkImage*)data;
	for( int x = 0; x < image->width; x++ ) {
		for( int y = 0; y < image->height; y++ ) {
			float* fp = NULL;
			float xv = 0.0f;
			float yv = 0.0f;
			float zv = 0.0f;
			float nl = 10000000.0f;
			for( unsigned int i = 0; i < sizeof(xs)/sizeof(float); i+=7 ) {
				float* ff = &xs[i];

				float x3 = ff[0];
				float y3 = ff[1];
				float z3 = ff[2];
				float r = ff[3];
				//float cb = ff[4];
				//float cg = ff[5];
				//float cr = ff[6];

				float xf = x1-x3;
				float yf = y1-y3;
				float zf = z1-z3;

				float c = x3*x3 + y3*y3 + z3*z3 + x1*x1 + y1*y1 + z1*z1 - 2*(x3*x1 + y3*y1 + z3*z1) - r*r;

				float x2 = x-128.0f;
				float y2 = y-128.0f;
				float z2 = 0.0f;

				float xd = x2-x1;
				float yd = y2-y1;
				float zd = z2-z1;
				float a = xd*xd + yd*yd + zd*zd;
				float b = 2*( xd*xf + yd*yf + zd*zf );

				if( b*b > 4*a*c ) {
					float s = sqrt( b*b - 4*a*c );
					float u = (-b - s)/(2*a);

					float nxv = x1 + u*(x2-x1);
					float nyv = y1 + u*(y2-y1);
					float nzv = z1 + u*(z2-z1);

					float ax = (nxv-x3);
					float ay = (nyv-y3);
					float az = (nzv-z3);

					float al = ax*ax + ay*ay + az*az;
					if( al < nl ) {
						xv = nxv;
						yv = nyv;
						zv = nzv;
						fp = ff;
						nl = al;
					}
				}
			}

			int color = 0;
			if( fp != NULL ) {
				float x3 = fp[0];
				float y3 = fp[1];
				float z3 = fp[2];
				float cb = fp[3];
				float cg = fp[4];
				float cr = fp[5];
				for( int v = 0; v < 8; v+=4 ) {
					float *vv = &ls[v];
					float xl = vv[0];
					float yl = vv[1];
					float zl = vv[2];
					float cl = vv[3];

					float cx = (xv-xl);
					float cy = (yv-yl);
					float cz = (zv-zl);

					float ax = (xv-x3);
					float ay = (yv-y3);
					float az = (zv-z3);

					float bx = (xl-x3);
					float by = (yl-y3);
					float bz = (zl-z3);

					float av = (ax*ax + ay*ay + az*az);
					float bv = (bx*bx + by*by + bz*bz);
					float cv = (cx*cx + cy*cy + cz*cz);
					float abs = ax*bx + ay*by + az*bz;
					float cosa = (abs > 0 ? abs : -abs) / sqrt( av*bv );

					int c = (int)( (cl*cb*cosa) + ((int)(cl*cg*cosa)<<8) + ((int)(cl*cr*cosa)<<16) );
					color += c;
				}
			}

			if( color > 0 /*&& cv < bv*/ ) {
				gdk_image_put_pixel( image, x, y, color );
			}
		}
	}
	t += 0.1f;
	gdk_draw_image( widget->window, gc, image, 0, 0, 0, 0, width, height );

	/*vector<p3d> & v = it->second;
	for( unsigned int i = 0; i < v.size(); i++ ) {
		p3d & pt = v[i];

		double xx = (pt.x - mx)*cx - (pt.z - mz)*sx;
		double yy = (pt.y - my);
		double zz = (pt.z - mz)*cx + (pt.x - mx)*sx;

		double xxx = xx + mx;
		double yyy = yy*cy - zz*sy + my;
		double zzz = zz*cy + yy*sy + mz;

		int x = 15+((xxx - minx) * width) / (maxx - minx);
		int y = 15+((yyy - miny) * height) / (maxy - miny);

		int s = (2.0)/(zzz+0.2);
		//gdk_draw_rectangle(widget->window, gc, true, x, y, 10, 10);
		gdk_draw_arc( widget->window, gc, true, x, y, s, s, 0, angle );
	}
	k++;
	it++;*/

	return 0;
}

JNIEXPORT int raytrace( simlab timer, simlab time ) {

	/*char	name[64];
	char	where[256];
	char	cont[64];
	float	d[6];
	p3d		p;
	FILE* f = fopen( "../info.WORLD", "r" );
	while( fscanf( f, "%s %s %s %e %e %e %e %e %e\n", name, where, cont, d, d+1, d+2, d+3, d+4, d+5 ) == 9 ) {
		p.x = d[0];
		p.y = d[1];
		p.z = d[2];

		if( p.x < minx ) minx = p.x;
		else if( p.x > maxx ) maxx = p.x;
		if( p.y < miny ) miny = p.y;
		else if( p.y > maxy ) maxy = p.y;
		if( p.z < minz ) minz = p.z;
		else if( p.z > maxz ) maxz = p.z;

		pts[cont].push_back( p );
	}
	fclose( f );*/

	GtkWidget* widget = gtk_window_new( GTK_WINDOW_TOPLEVEL );
	gtk_window_set_title( (GtkWindow*)widget, "simlab" );
	g_signal_connect( widget, "destroy", gtk_main_quit, NULL );

	GtkWidget* fixed = gtk_fixed_new();

	GdkVisual* visual = gdk_visual_get_system(); //((GdkDrawable)fixed)->get_drawable();
	GdkImage*	image = gdk_image_new( GDK_IMAGE_NORMAL, visual, 256, 256 );

	g_signal_connect( fixed, "expose-event", G_CALLBACK(expose), image );
	gtk_widget_add_events( widget, GDK_BUTTON_PRESS_MASK);
	gtk_widget_add_events( widget, GDK_BUTTON_RELEASE_MASK);
	gtk_widget_add_events( widget, GDK_POINTER_MOTION_MASK);

	int tag = 0;
	if( timer.type == 96 ) {
		printf("install timer %d\n", timer.buffer);
		timers[(int)image] = timer;
		tag = g_timeout_add( time.buffer, old_timeout_callback, image );
	}
	//g_signal_connect( widget, "button-press-event", G_CALLBACK(btnp), NULL );
	//g_signal_connect( widget, "button-release-event", G_CALLBACK(btnr), NULL );
	//g_signal_connect( widget, "motion-notify-event", G_CALLBACK(mnfe), NULL );
	gtk_container_add( (GtkContainer*)widget, fixed );
	gtk_widget_show_all( widget );
	gtk_main();

	if( tag != 0 ) g_source_remove( tag );

	if( timer.type == 96 ) return 3;
	else return 1;
}
