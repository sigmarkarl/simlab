/*
 * physicalobject.cc
 *
 *  Created on: Jan 13, 2009
 *      Author: root
 */

#include "simlab.h"

#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>

#include <string.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <math.h>

#define  mn(a, b)  ((a) < (b) ? (a) :  (b))

extern simlab data;

#ifdef GL
class PhysicalObject {
public:
	//static bool PhysicalObject::gravity;

	PhysicalObject() :
		m(1.0),
		x(0.0),
		y(0.0),
		z(0.0),
		dx(20.0),
		dy(20.0),
		dz(20.0),
		parentObject(NULL) {
		initialize();
	}

	PhysicalObject( std::string m )  :
		m(1.0),
		x(0.0),
		y(0.0),
		z(0.0),
		dx(20.0),
		dy(20.0),
		dz(20.0),
		parentObject(NULL) {

		initialize();
		//current = (int)this;
		//store( m.c_str() );

		name = m;
	}

	virtual ~PhysicalObject() {

	}

	void initialize() {
		//PhysicalObject::gravity = true;

		vx = 0.0;
		vy = 0.0;
		vz = 0.0;
		wx = 0.0;
		wy = 0.0;
		wz = 0.0;

		I[0] = 1.0;
		I[1] = 0.0;
		I[2] = 0.0;
		I[3] = 0.0;
		I[4] = 1.0;
		I[5] = 0.0;
		I[6] = 0.0;
		I[7] = 0.0;
		I[8] = 1.0;

		memcpy( Ir, I, sizeof(I) );

		parentObject = NULL;
		//printf( "heyhey%s\n", name.c_str() );
		//store( name.c_str() );
	}

	std::string name;
	double m;
	double I[9];
	double Ir[9];
	double x;
	double y;
	double z;
	double dx;
	double dy;
	double dz;
	double vx;
	double vy;
	double vz;
	double wx;
	double wy;
	double wz;
	double t;
	double temperature;
	std::vector<PhysicalObject*>	childObjects;
	PhysicalObject*					parentObject;

	void add( PhysicalObject* po ) {
		po->parentObject = this;
		childObjects.push_back( po );
	}

	void applyGravity( PhysicalObject* po ) {
		if( po != NULL && po != this ) {
			double ddx = po->x-x;
			double ddy = po->y-y;
			double ddz = po->z-z;
			double r2 = ddx*ddx+ddy*ddy+ddz*ddz;
			double m2 = m*po->m;

			po->vx -= m2*ddx/r2;
			po->vy -= m2*ddy/r2;
			po->vz -= m2*ddz/r2;
		}

		for( unsigned int i = 0; i < childObjects.size(); i++ ) {
			childObjects[i]->applyGravity( po );
		}
	}

	virtual void render() {
		double r = getOuterRadius();
		double rr = r*r;
		double rt = sqrt(rr/3.0);
		double rd = sqrt(2.0*rr/3.0);
		double n = sqrt(1.0/3.0);
		double n2 = sqrt(2.0/3.0);

		glTranslated( x, y, z );
		glBegin( GL_TRIANGLES );
			glVertex3d( x, y+r, z ); glNormal3d( 0.0, 1.0, 0.0 );
			glVertex3d( x, y-rt, z+rd ); glNormal3d( 0.0, -n, n2 );
			glVertex3d( x+rt, y-rt, z-rt ); glNormal3d( n, -n, -n );

			glVertex3d( x, y+r, z ); glNormal3d( 0.0, 1.0, 0.0 );
			glVertex3d( x-rt, y-rt, z-rt ); glNormal3d( -n, -n, -n );
			glVertex3d( x, y-rt, z+rd ); glNormal3d( 0.0, -n, n2 );

			glVertex3d( x, y+r, z ); glNormal3d( 0.0, 1.0, 0.0 );
			glVertex3d( x-rt, y-rt, z-rt ); glNormal3d( -n, -n, -n );
			glVertex3d( x+rt, y-rt, z-rt ); glNormal3d( n, -n, -n );

			glVertex3d( x, y-rt, z+rd ); glNormal3d( 0.0, -n, n2 );
			glVertex3d( x-rt, y-rt, z-rt ); glNormal3d( -n, -n, -n );
			glVertex3d( x+rt, y-rt, z-rt ); glNormal3d( n, -n, -n );
		glEnd();
		glTranslated( -x, -y, -z );

		//if( PhysicalObject::gravity ) {
			PhysicalObject* po = this;
			while( po->parentObject != NULL ) po = po->parentObject;
			po->applyGravity( this );
		//}

		x += vx;
		y += vy;
		z += vz;

		//for_each( childObjects.begin(), childObjects.end(), *PhysicalObject::render );
		for( unsigned int i = 0; i < childObjects.size(); i++ ) {
			childObjects[i]->render();
		}
	}

	double getOuterRadius() {
		return sqrt( dx*dx+dy*dy+dz*dz );
	}

	double getInnterRadius() {
		return mn( dx, mn( dy,dx ) );
	}

	double getTotalEnergy() {
		return getKineticEnergy() + getRotationalEnergy();
	}
	double getRotationalEnergy() {
		return 0;
	}
	double getKineticEnergy() {
		return m*(vx*vx+vy*vy+vz*vz)/2.0;
	}

	bool containsPoint( double x, double y, double z );
	bool containsLine( double a, double b, double c );
};

std::map<int,std::set<PhysicalObject*> >			visibleObjects;

class RectangleObject : public PhysicalObject {
public:
	RectangleObject( double tdx, double tdy, double tdz ) {
		dx = tdx;
		dy = tdy;
		dz = tdz;
	}

	virtual void render() {
		double dx2 = dx/2.0;
		double dy2 = dy/2.0;
		double dz2 = dz/2.0;
		glBegin( GL_QUADS );
			glVertex3d( x-dx2, y-dy2, z-dz2 );
			glVertex3d( x+dx2, y-dy2, z-dz2 );
			glVertex3d( x+dx2, y+dy2, z-dz2 );
			glVertex3d( x-dx2, y+dy2, z-dz2 );
		glEnd();
	}

	bool containsPoint( double px, double py, double pz ) {
		return true;
	}
};

class CylinderObject : public PhysicalObject {
public:
	GLUquadricObj *quadObj;

	CylinderObject() {
		glNewList(0, GL_COMPILE);
        glPushMatrix ();
        glRotatef ((GLfloat)90.0, (GLfloat)1.0, (GLfloat)0.0, (GLfloat)0.0);
        glTranslatef ((GLfloat)0.0, (GLfloat)0.0, (GLfloat)-1.0);
        quadObj = gluNewQuadric ();
        gluQuadricDrawStyle (quadObj, GLU_FILL);
        gluQuadricNormals (quadObj, GLU_SMOOTH);
        gluCylinder (quadObj, 0.3, 0.3, 0.6, 12, 2);
        glPopMatrix ();
		glEndList();

		vz = -1.0;
	}

	virtual void render() {
		glColor3f( 1.0f, 1.0f, 1.0f );
		quadObj = gluNewQuadric ();
        gluQuadricDrawStyle (quadObj, GLU_FILL);
        gluQuadricNormals (quadObj, GLU_SMOOTH);
		glTranslated( x, y, z );
		x += vx;
		y += vy;
		z += vz;
		//gluCylinder(quadObj, 100.0, 100.0, 100.0, 12, 2);
		gluSphere( quadObj, 100.0, 12, 2 );
		//glTranslatef(0.0f, 0.0f, 200.0f);
		//gluCylinder(quadObj, 100.0, 100.0, 100.0, 12, 2);
		glBegin( GL_LINES );
		for( double i = -100.0; i <= 100.0; i+=10.0 ) {
			glVertex3d( i, -100.0, 100.0 );
			glVertex3d( i, 100.0, 100.0 );

           glVertex3d( i, 100.0, -100.0 );
           glVertex3d( i, 100.0, 100.0 );

           glVertex3d( i, -100.0, -100.0 );
           glVertex3d( i, 100.0, -100.0 );

           glVertex3d( i, -100.0, -100.0 );
           glVertex3d( i, -100.0, 100.0 );

           glVertex3d( -100.0, i, 100.0 );
           glVertex3d( 100.0, i, 100.0 );

           glVertex3d( 100.0, i, -100.0 );
           glVertex3d( 100.0, i, 100.0 );

           glVertex3d( -100.0, i, -100.0 );
           glVertex3d( 100.0, i, -100.0 );

           glVertex3d( -100.0, i, -100.0 );
           glVertex3d( -100.0, i, 100.0 );

           glVertex3d( -100.0, 100.0, i );
           glVertex3d( 100.0, 100.0, i );

           glVertex3d( 100.0, -100.0, i );
           glVertex3d( 100.0, 100.0, i );

           glVertex3d( -100.0, -100.0, i );
           glVertex3d( 100.0, -100.0, i );

           glVertex3d( -100.0, -100.0, i );
           glVertex3d( -100.0, 100.0, i );
    }
    glEnd();

		/*glBegin( GL_LINES );
		glColor3f( 1.0f, 1.0f, 1.0f );
		glVertex3f( -5.0,5.0,-5.0 );
		glVertex3f( 5.0,5.0,-5.0 );
		glVertex3f( 5.0,-5.0,-5.0 );
		glVertex3f( -5.0,-5.0,-5.0 );
		glVertex3f( -5.0,5.0,5.0 );
		glVertex3f( 5.0,5.0,5.0 );
		glVertex3f( 5.0,-5.0,5.0 );
		glVertex3f( -5.0,-5.0,5.0 );
		glVertex3f( -1.0,5.0,-0.2 );
		glVertex3f( 1.0,5.0,-0.2 );
		glVertex3f( 1.0,-5.0,-0.2 );
		glVertex3f( -1.0,-5.0,-0.2 );
		glVertex3f( -1.0,.0,0.2 );
		glVertex3f( 1.0,.0,0.2 );
		glVertex3f( 1.0,-1.0,0.2 );
		glVertex3f( -1.0,-1.0,0.2 );
		glEnd();
		glFlush();*/
	}
};

class EllipsoidObject : public PhysicalObject {
public:
	bool containsPoint( double px, double py, double pz ) {
		double xdist = px-x;
		double ydist = py-y;
		double zdist = pz-z;
		return (xdist*xdist)/(dx*dx)+(ydist*ydist)/(dy*dy)+(zdist*zdist)/(dz*dz) < 1;
	}
};

class SphereObject : public EllipsoidObject {
public:
	bool containsPoint( double px, double py, double pz ) {
		double xdist = px-x;
		double ydist = py-y;
		double zdist = pz-z;
		return (xdist < dx) & (ydist < dy) & (zdist < dz);
	}
};

void prender( PhysicalObject* pobj ) {
	pobj->render();
}

JNIEXPORT int createObject( char* name ) {
	return (int)new PhysicalObject( name );
}


JNIEXPORT int addObject( PhysicalObject* newpo ) {
	PhysicalObject* po = (PhysicalObject*)data.buffer;
	po->add( newpo );

	return 0;
}

JNIEXPORT int parentObject() {
	PhysicalObject* po = (PhysicalObject*)data.buffer;
	return (int)po->parentObject;
}

JNIEXPORT int printChildObjects() {
	PhysicalObject* po = (PhysicalObject*)data.buffer;
	for( unsigned int i = 0; i < po->childObjects.size(); i++ ) {
		printf( "%s\n", po->childObjects[i]->name.c_str() );
	}

	return 0;
}

JNIEXPORT int addLocation( double x, double y, double z ) {
	PhysicalObject* po = (PhysicalObject*)data.buffer;
	po->x += x;
	po->y += y;
	po->z += z;

	return 3;
}

JNIEXPORT int setLocation( double x, double y, double z ) {
	PhysicalObject* po = (PhysicalObject*)data.buffer;
	po->x = x;
	po->y = y;
	po->z = z;

	return 3;
}

JNIEXPORT int addSpeed( double x, double y, double z ) {
	PhysicalObject* po = (PhysicalObject*)data.buffer;
	po->vx += x;
	po->vy += y;
	po->vz += z;

	return 3;
}

JNIEXPORT int setSpeed( double x, double y, double z ) {
	PhysicalObject* po = (PhysicalObject*)data.buffer;
	po->vx = x;
	po->vy = y;
	po->vz = z;

	return 3;
}

JNIEXPORT int glscene() {
	int w = 800;
	int h = 600;

	float aspect = (float)w/(float)h;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//gluPerspective(45.0f, fAspect, 1.0f, 500.0f);
	if (w > h) {
      aspect = w / h;
      glFrustum (-aspect, aspect, -1.0, 1.0, 1.0, 50.0);
	} else {
      aspect = h / w;
      glFrustum (-1.0, 1.0, -aspect, aspect, 1.0, 50.0);
	}
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	std::set<PhysicalObject*> st = visibleObjects[1];
	for_each( st.begin(), st.end(), prender );

	return 0;
}
#endif
