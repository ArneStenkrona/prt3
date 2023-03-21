#include "geometry_util.h"

using namespace prt3;

/********************************************************/

/* AABB-triangle overlap test code                      */

/* by Tomas Akenine-Möller                              */

/* Function: int triBoxOverlap(float boxcenter[3],      */

/*          float box_halfsize[3],float triverts[3][3]); */

/* History:                                             */

/*   2001-03-05: released the code in its first version */

/*   2001-06-18: changed the order of the tests, faster */

/*                                                      */

/* Acknowledgement: Many thanks to Pierre Terdiman for  */

/* suggestions and discussions on how to optimize code. */

/* Thanks to David Hunt for finding a ">="-bug!         */

/********************************************************/

/*
Copyright 2020 Tomas Akenine-Möller

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files (the "Software"), to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and
to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial
portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <cmath>

#define X 0
#define Y 1
#define Z 2

#define FINDMINMAX(x0,x1,x2,min,max) \
  min = max = x0;   \
  if(x1<min) min=x1;\
  if(x1>max) max=x1;\
  if(x2<min) min=x2;\
  if(x2>max) max=x2;

bool plane_box_overlap(
    glm::vec3 normal,
    glm::vec3 vert,
    glm::vec3 box_halfsize
) {
    glm::vec3 vmin, vmax;
    float v;

    for (unsigned int q = 0; q < 3; ++q) {
        v = vert[q];
        if (normal[q] > 0.0f) {
            vmin[q] = -box_halfsize[q] - v;
            vmax[q] = box_halfsize[q] - v;
        } else {
            vmin[q] = box_halfsize[q] - v;
            vmax[q] = -box_halfsize[q] - v;
        }
    }
    if (glm::dot(normal, vmin) > 0.0f) return false;
    if (glm::dot(normal, vmax) >= 0.0f) return 1;
    return false;
}

/*======================== X-tests ========================*/
#define AXISTEST_X01(a, b, fa, fb)			   \
	p0 = a*v0[Y] - b*v0[Z];			       	   \
	p2 = a*v2[Y] - b*v2[Z];			       	   \
        if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
	rad = fa * box_halfsize[Y] + fb * box_halfsize[Z];   \
	if(min>rad || max<-rad) return false;

#define AXISTEST_X2(a, b, fa, fb)			   \
	p0 = a*v0[Y] - b*v0[Z];			           \
	p1 = a*v1[Y] - b*v1[Z];			       	   \
        if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
	rad = fa * box_halfsize[Y] + fb * box_halfsize[Z];   \
	if(min>rad || max<-rad) return false;

/*======================== Y-tests ========================*/
#define AXISTEST_Y02(a, b, fa, fb)			   \
	p0 = -a*v0[X] + b*v0[Z];		      	   \
	p2 = -a*v2[X] + b*v2[Z];	       	       	   \
        if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
	rad = fa * box_halfsize[X] + fb * box_halfsize[Z];   \
	if(min>rad || max<-rad) return false;

#define AXISTEST_Y1(a, b, fa, fb)			   \
	p0 = -a*v0[X] + b*v0[Z];		      	   \
	p1 = -a*v1[X] + b*v1[Z];	     	       	   \
        if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
	rad = fa * box_halfsize[X] + fb * box_halfsize[Z];   \
	if(min>rad || max<-rad) return false;

/*======================== Z-tests ========================*/
#define AXISTEST_Z12(a, b, fa, fb)			   \
	p1 = a*v1[X] - b*v1[Y];			           \
	p2 = a*v2[X] - b*v2[Y];			       	   \
        if(p2<p1) {min=p2; max=p1;} else {min=p1; max=p2;} \
	rad = fa * box_halfsize[X] + fb * box_halfsize[Y];   \
	if(min>rad || max<-rad) return false;

#define AXISTEST_Z0(a, b, fa, fb)			   \
	p0 = a*v0[X] - b*v0[Y];				   \
	p1 = a*v1[X] - b*v1[Y];			           \
        if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
	rad = fa * box_halfsize[X] + fb * box_halfsize[Y];   \
	if(min>rad || max<-rad) return false;

bool prt3::triangle_box_overlap(
    glm::vec3 const & box_center,
    glm::vec3 const & box_halfsize,
    glm::vec3 const & a,
    glm::vec3 const & b,
    glm::vec3 const & c
) {
  /*    use separating axis theorem to test overlap between triangle and box */
  /*    need to test for overlap in these directions: */
  /*    1) the {x,y,z}-directions (actually, since we use the AABB of the triangle */
  /*       we do not even need to test these) */
  /*    2) normal of the triangle */
  /*    3) crossproduct(edge from tri, {x,y,z}-directin) */
  /*       this gives 3x3=9 more tests */

   float min, max, p0, p1, p2, rad;

   glm::vec3 v0 = a - box_center;
   glm::vec3 v1 = b - box_center;
   glm::vec3 v2 = c - box_center;

   /* compute triangle edges */
   glm::vec3 e0 = v1 - v0;
   glm::vec3 e1 = v2 - v1;
   glm::vec3 e2 = v0 - v2;

   /* Bullet 3:  */

   /*  test the 9 tests first (this was faster) */
   float fex = fabsf(e0.x);
   float fey = fabsf(e0.y);
   float fez = fabsf(e0.z);

   AXISTEST_X01(e0[Z], e0[Y], fez, fey);
   AXISTEST_Y02(e0[Z], e0[X], fez, fex);
   AXISTEST_Z12(e0[Y], e0[X], fey, fex);

   fex = fabsf(e1[X]);
   fey = fabsf(e1[Y]);
   fez = fabsf(e1[Z]);

   AXISTEST_X01(e1[Z], e1[Y], fez, fey);
   AXISTEST_Y02(e1[Z], e1[X], fez, fex);
   AXISTEST_Z0(e1[Y], e1[X], fey, fex);

   fex = fabsf(e2[X]);
   fey = fabsf(e2[Y]);
   fez = fabsf(e2[Z]);

   AXISTEST_X2(e2[Z], e2[Y], fez, fey);
   AXISTEST_Y1(e2[Z], e2[X], fez, fex);
   AXISTEST_Z12(e2[Y], e2[X], fey, fex);

   /* Bullet 1: */

   /*  first test overlap in the {x,y,z}-directions */
   /*  find min, max of the triangle each direction, and test for overlap in */
   /*  that direction -- this is equivalent to testing a minimal AABB around */
   /*  the triangle against the AABB */

   /* test in X-direction */
   FINDMINMAX(v0[X],v1[X],v2[X],min,max);
   if(min>box_halfsize[X] || max<-box_halfsize[X]) return false;

   /* test in Y-direction */
   FINDMINMAX(v0[Y],v1[Y],v2[Y],min,max);
   if(min>box_halfsize[Y] || max<-box_halfsize[Y]) return false;

   /* test in Z-direction */
   FINDMINMAX(v0[Z],v1[Z],v2[Z],min,max);
   if(min>box_halfsize[Z] || max<-box_halfsize[Z]) return false;

   /* Bullet 2: */

   /*  test if the box intersects the plane of the triangle */
   /*  compute plane equation of triangle: normal*x+d=0 */
   glm::vec3 normal = glm::cross(e0, e1);

   return plane_box_overlap(normal, v0, box_halfsize);
}
