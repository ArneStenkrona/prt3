#include "geometry_util.h"

#include "src/util/log.h"

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

using namespace prt3;

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

// Implementation of Möller-Trumbore, courtesy of wikipedia
bool prt3::triangle_ray_intersect(
    glm::vec3 origin,
    glm::vec3 ray_vector,
    glm::vec3 tri_a,
    glm::vec3 tri_b,
    glm::vec3 tri_c,
    glm::vec3 & intersection
) {
    const float EPSILON = 0.0000001;
    glm::vec3 edge1, edge2, h, s, q;
    float a, f, u, v;
    edge1 = tri_b - tri_a;
    edge2 = tri_c - tri_a;
    h = glm::cross(ray_vector, edge2);
    a = glm::dot(edge1, h);

    if (a > -EPSILON && a < EPSILON)
        return false;    // This ray is parallel to this triangle.

    f = 1.0 / a;
    s = origin - tri_a;
    u = f * glm::dot(s, h);

    if (u < 0.0 || u > 1.0)
        return false;

    q = glm::cross(s, edge1);
    v = f * glm::dot(ray_vector, q);

    if (v < 0.0 || u + v > 1.0)
        return false;

    // At this stage we can compute t to find out where the intersection point is on the line.
    float t = f * glm::dot(edge2, q);

    if (t > EPSILON && t <= 1.0f) // ray intersection
    {
        intersection = origin + ray_vector * t;
        return true;
    }
    else // This means that there is a line intersection but not a ray intersection.
        return false;
}

inline glm::vec2 segment_intersect_2d(
    glm::vec2 p0,
    glm::vec2 p1,
    glm::vec2 q0,
    glm::vec2 q1
) {
    float x1x2 = p0.x - p1.x;
    float y1y2 = p0.y - p1.y;
    float x1x3 = p0.x - q0.x;
    float y1y3 = p0.y - q0.y;
    float x3x4 = q0.x - q1.x;
    float y3y4 = q0.y - q1.y;

    float d = x1x2 * y3y4 - y1y2 * x3x4;

    // parallel or coincident
    if (d == 0.0f) return glm::vec2{std::numeric_limits<float>::max()};

    float t = (x1x3 * y3y4 - y1y3 * x3x4) / d;
    float u = -(x1x2 * y1y3 - y1y2 * x1x3) / d;

    bool ist = t >= 0.0f && t <= 1.0f;
    bool isu = u >= 0.0f && u <= 1.0f;

    if (ist && isu) {
        return glm::vec2{p0.x + t * (p1.x - p0.x),
                         p0.y + t * (p1.y - p0.y)};
    }

    return glm::vec2{std::numeric_limits<float>::max()};
}

float sign(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3)
{
    return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

bool point_in_triangle(glm::vec2 p, glm::vec2 v1, glm::vec2 v2, glm::vec2 v3) {
    float d1, d2, d3;
    bool has_neg, has_pos;

    d1 = sign(p, v1, v2);
    d2 = sign(p, v2, v3);
    d3 = sign(p, v3, v1);

    has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    return !(has_neg && has_pos);
}

void prt3::triangle_segment_clip_2d(
    glm::vec2 p0,
    glm::vec2 p1,
    glm::vec2 a,
    glm::vec2 b,
    glm::vec2 c,
    glm::vec2 & t0,
    glm::vec2 & t1
) {
    constexpr float inf = std::numeric_limits<float>::infinity();

    t0 = glm::vec2{inf};
    t1 = glm::vec2{inf};

    if (point_in_triangle(p0, a, b, c)) {
        t0 = p0;
    }

    if (p0 == p1) {
        t1 = t0;
        return;
    }

    if (point_in_triangle(p1, a, b, c)) {
        t1 = p1;
    }

    if (t0.x != inf && t1.x != inf) {
        return;
    }

    glm::vec2 ab = segment_intersect_2d(p0, p1, a, b);
    glm::vec2 bc = segment_intersect_2d(p0, p1, b, c);
    glm::vec2 ca = segment_intersect_2d(p0, p1, c, a);

    if (t0.x == inf) {
        if (glm::distance(p0, ab) < glm::distance(p0, t0)) t0 = ab;
        if (glm::distance(p0, bc) < glm::distance(p0, t0)) t0 = bc;
        if (glm::distance(p0, ca) < glm::distance(p0, t0)) t0 = ca;
    }

    if (t1.x == inf) {
        if (glm::distance(p1, ab) < glm::distance(p1, t1)) t1 = ab;
        if (glm::distance(p1, bc) < glm::distance(p1, t1)) t1 = bc;
        if (glm::distance(p1, ca) < glm::distance(p1, t1)) t1 = ca;
    }
}

// thanks https://github.com/embree/embree/blob/master/tutorials/common/math/closest_point.h
glm::vec3 prt3::closest_point_on_triangle(
    glm::vec3 const & p,
    glm::vec3 const & a,
    glm::vec3 const & b,
    glm::vec3 const & c
) {
    glm::vec3 const ab = b - a;
    glm::vec3 const ac = c - a;
    glm::vec3 const ap = p - a;

    float const d1 = glm::dot(ab, ap);
    float const d2 = glm::dot(ac, ap);
    if (d1 <= 0.f && d2 <= 0.f) return a; //#1

    glm::vec3 const bp = p - b;
    float const d3 = dot(ab, bp);
    float const d4 = dot(ac, bp);
    if (d3 >= 0.f && d4 <= d3) return b; //#2

    glm::vec3 const cp = p - c;
    float const d5 = dot(ab, cp);
    float const d6 = dot(ac, cp);
    if (d6 >= 0.f && d5 <= d6) return c; //#3

    float const vc = d1 * d4 - d3 * d2;
    if (vc <= 0.f && d1 >= 0.f && d3 <= 0.f)
    {
        float const v = d1 / (d1 - d3);
        return a + v * ab; //#4
    }

    float const vb = d5 * d2 - d1 * d6;
    if (vb <= 0.f && d2 >= 0.f && d6 <= 0.f)
    {
        float const v = d2 / (d2 - d6);
        return a + v * ac; //#5
    }

    float const va = d3 * d6 - d5 * d4;
    if (va <= 0.f && (d4 - d3) >= 0.f && (d5 - d6) >= 0.f)
    {
        float const v = (d4 - d3) / ((d4 - d3) + (d5 - d6));
        return b + v * (c - b); //#6
    }

    float const denom = 1.f / (va + vb + vc);
    float const v = vb * denom;
    float const w = vc * denom;
    return a + v * ab + w * ac; //#0
}
