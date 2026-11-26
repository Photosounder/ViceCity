//+ rouz edit (Claude)
#include <math.h>
#include "CollisionProcess.h"
#include "VectorMath.h"

bool CollisionProcess_ProcessSphereSphere(const CColSphere *s1, const CColSphere *s2, CColPoint *point, float *mindistsq)
{
	VectorMath3 dist = {s1->bounds.center.x - s2->bounds.center.x, s1->bounds.center.y - s2->bounds.center.y, s1->bounds.center.z - s2->bounds.center.z};
	float d = sqrtf(dist.x*dist.x + dist.y*dist.y + dist.z*dist.z) - s2->bounds.radius;	// distance from s1's center to s2
	float depth = s1->bounds.radius - d;	// sphere overlap

	// Clamp to zero, i.e. if s1's center is inside s2
	if (d < 0.0f) d = 0.0f;

	// No collision if the sphere is not close enough
	if (d*d < *mindistsq && d < s1->bounds.radius) {
		VectorMath_Normalise(&dist);
		point->point.x = s1->bounds.center.x - dist.x*d;
		point->point.y = s1->bounds.center.y - dist.y*d;
		point->point.z = s1->bounds.center.z - dist.z*d;
		point->normal.x = dist.x;
		point->normal.y = dist.y;
		point->normal.z = dist.z;
		point->surfaceA = s1->surface;
		point->pieceA = s1->piece;
		point->surfaceB = s2->surface;
		point->pieceB = s2->piece;
		point->depth = depth;
		*mindistsq = d*d;	// collision radius
		return true;
	}
	return false;
}

bool CollisionProcess_ProcessSphereBox(const CColSphere *sph, const CColBox *box, CColPoint *point, float *mindistsq)
{
	float p[3];
	float dist[3];
	int xpos, ypos, zpos;
	float lensq;

	// First make sure we have a collision at all
	if (sph->bounds.center.x + sph->bounds.radius < box->bounds.min.x) return false;
	if (sph->bounds.center.x - sph->bounds.radius > box->bounds.max.x) return false;
	if (sph->bounds.center.y + sph->bounds.radius < box->bounds.min.y) return false;
	if (sph->bounds.center.y - sph->bounds.radius > box->bounds.max.y) return false;
	if (sph->bounds.center.z + sph->bounds.radius < box->bounds.min.z) return false;
	if (sph->bounds.center.z - sph->bounds.radius > box->bounds.max.z) return false;

	// Now find out where the sphere center lies in relation to all the sides
	xpos = sph->bounds.center.x < box->bounds.min.x ? 1 :
	       sph->bounds.center.x > box->bounds.max.x ? 2 :
	       0;
	ypos = sph->bounds.center.y < box->bounds.min.y ? 1 :
	       sph->bounds.center.y > box->bounds.max.y ? 2 :
	       0;
	zpos = sph->bounds.center.z < box->bounds.min.z ? 1 :
	       sph->bounds.center.z > box->bounds.max.z ? 2 :
	       0;

	if (xpos == 0 && ypos == 0 && zpos == 0) {
		// Sphere is inside the box, push out from the box center
		p[0] = (box->bounds.min.x + box->bounds.max.x)*0.5f;
		p[1] = (box->bounds.min.y + box->bounds.max.y)*0.5f;
		p[2] = (box->bounds.min.z + box->bounds.max.z)*0.5f;

		dist[0] = sph->bounds.center.x - p[0];
		dist[1] = sph->bounds.center.y - p[1];
		dist[2] = sph->bounds.center.z - p[2];
		lensq = dist[0]*dist[0] + dist[1]*dist[1] + dist[2]*dist[2];
		if (lensq < *mindistsq) {
			float inv = 1.0f/sqrtf(lensq);
			float dx, dy, dz;
			point->normal.x = dist[0] * inv;
			point->normal.y = dist[1] * inv;
			point->normal.z = dist[2] * inv;
			point->point.x = sph->bounds.center.x - point->normal.x;
			point->point.y = sph->bounds.center.y - point->normal.y;
			point->point.z = sph->bounds.center.z - point->normal.z;
			point->surfaceA = sph->surface;
			point->pieceA = sph->piece;
			point->surfaceB = box->surface;
			point->pieceB = box->piece;

			// Absolute distance to the closer side in each dimension
			dx = dist[0] > 0.0f ?
				box->bounds.max.x - sph->bounds.center.x :
				sph->bounds.center.x - box->bounds.min.x;
			dy = dist[1] > 0.0f ?
				box->bounds.max.y - sph->bounds.center.y :
				sph->bounds.center.y - box->bounds.min.y;
			dz = dist[2] > 0.0f ?
				box->bounds.max.z - sph->bounds.center.z :
				sph->bounds.center.z - box->bounds.min.z;

			// The collision depth is the maximum of those
			if (dx > dy && dx > dz)
				point->depth = dx;
			else if (dy > dz)
				point->depth = dy;
			else
				point->depth = dz;
			return true;
		}
	} else {
		// Sphere is outside, find the closest point on the box
		p[0] = xpos == 1 ? box->bounds.min.x :
		       xpos == 2 ? box->bounds.max.x :
		       sph->bounds.center.x;
		p[1] = ypos == 1 ? box->bounds.min.y :
		       ypos == 2 ? box->bounds.max.y :
		       sph->bounds.center.y;
		p[2] = zpos == 1 ? box->bounds.min.z :
		       zpos == 2 ? box->bounds.max.z :
		       sph->bounds.center.z;

		dist[0] = sph->bounds.center.x - p[0];
		dist[1] = sph->bounds.center.y - p[1];
		dist[2] = sph->bounds.center.z - p[2];
		lensq = dist[0]*dist[0] + dist[1]*dist[1] + dist[2]*dist[2];
		if (lensq < *mindistsq) {
			float len = sqrtf(lensq);
			float inv = 1.0f/len;
			point->point.x = p[0];
			point->point.y = p[1];
			point->point.z = p[2];
			point->normal.x = dist[0] * inv;
			point->normal.y = dist[1] * inv;
			point->normal.z = dist[2] * inv;
			point->surfaceA = sph->surface;
			point->pieceA = sph->piece;
			point->surfaceB = box->surface;
			point->pieceB = box->piece;
			point->depth = sph->bounds.radius - len;
			*mindistsq = lensq;
			return true;
		}
	}
	return false;
}

bool CollisionProcess_ProcessLineBox(const CColLine *line, const CColBox *box, CColPoint *point, float *mindist)
{
	float mint, t, x, y, z;
	CollisionVector3 normal = {0.0f, 0.0f, 0.0f};
	CollisionVector3 p = {0.0f, 0.0f, 0.0f};

	mint = 1.0f;
	// check if points are on opposite sides of min x plane
	if((box->bounds.min.x - line->p1.x) * (box->bounds.min.x - line->p0.x) < 0.0f){
		// parameter along line where we intersect
		t = (box->bounds.min.x - line->p0.x) / (line->p1.x - line->p0.x);
		// y of intersection
		y = line->p0.y + (line->p1.y - line->p0.y)*t;
		if(y > box->bounds.min.y && y < box->bounds.max.y){
			// z of intersection
			z = line->p0.z + (line->p1.z - line->p0.z)*t;
			if(z > box->bounds.min.z && z < box->bounds.max.z)
				if(t < mint){
					mint = t;
					p.x = box->bounds.min.x; p.y = y; p.z = z;
					normal.x = -1.0f; normal.y = 0.0f; normal.z = 0.0f;
				}
		}
	}

	// max x plane
	if((line->p1.x - box->bounds.max.x) * (line->p0.x - box->bounds.max.x) < 0.0f){
		t = (line->p0.x - box->bounds.max.x) / (line->p0.x - line->p1.x);
		y = line->p0.y + (line->p1.y - line->p0.y)*t;
		if(y > box->bounds.min.y && y < box->bounds.max.y){
			z = line->p0.z + (line->p1.z - line->p0.z)*t;
			if(z > box->bounds.min.z && z < box->bounds.max.z)
				if(t < mint){
					mint = t;
					p.x = box->bounds.max.x; p.y = y; p.z = z;
					normal.x = 1.0f; normal.y = 0.0f; normal.z = 0.0f;
				}
		}
	}

	// min y plne
	if((box->bounds.min.y - line->p0.y) * (box->bounds.min.y - line->p1.y) < 0.0f){
		t = (box->bounds.min.y - line->p0.y) / (line->p1.y - line->p0.y);
		x = line->p0.x + (line->p1.x - line->p0.x)*t;
		if(x > box->bounds.min.x && x < box->bounds.max.x){
			z = line->p0.z + (line->p1.z - line->p0.z)*t;
			if(z > box->bounds.min.z && z < box->bounds.max.z)
				if(t < mint){
					mint = t;
					p.x = x; p.y = box->bounds.min.y; p.z = z;
					normal.x = 0.0f; normal.y = -1.0f; normal.z = 0.0f;
				}
		}
	}

	// max y plane
	if((line->p0.y - box->bounds.max.y) * (line->p1.y - box->bounds.max.y) < 0.0f){
		t = (line->p0.y - box->bounds.max.y) / (line->p0.y - line->p1.y);
		x = line->p0.x + (line->p1.x - line->p0.x)*t;
		if(x > box->bounds.min.x && x < box->bounds.max.x){
			z = line->p0.z + (line->p1.z - line->p0.z)*t;
			if(z > box->bounds.min.z && z < box->bounds.max.z)
				if(t < mint){
					mint = t;
					p.x = x; p.y = box->bounds.max.y; p.z = z;
					normal.x = 0.0f; normal.y = 1.0f; normal.z = 0.0f;
				}
		}
	}

	// min z plne
	if((box->bounds.min.z - line->p0.z) * (box->bounds.min.z - line->p1.z) < 0.0f){
		t = (box->bounds.min.z - line->p0.z) / (line->p1.z - line->p0.z);
		x = line->p0.x + (line->p1.x - line->p0.x)*t;
		if(x > box->bounds.min.x && x < box->bounds.max.x){
			y = line->p0.y + (line->p1.y - line->p0.y)*t;
			if(y > box->bounds.min.y && y < box->bounds.max.y)
				if(t < mint){
					mint = t;
					p.x = x; p.y = y; p.z = box->bounds.min.z;
					normal.x = 0.0f; normal.y = 0.0f; normal.z = -1.0f;
				}
		}
	}

	// max z plane
	if((line->p0.z - box->bounds.max.z) * (line->p1.z - box->bounds.max.z) < 0.0f){
		t = (line->p0.z - box->bounds.max.z) / (line->p0.z - line->p1.z);
		x = line->p0.x + (line->p1.x - line->p0.x)*t;
		if(x > box->bounds.min.x && x < box->bounds.max.x){
			y = line->p0.y + (line->p1.y - line->p0.y)*t;
			if(y > box->bounds.min.y && y < box->bounds.max.y)
				if(t < mint){
					mint = t;
					p.x = x; p.y = y; p.z = box->bounds.max.z;
					normal.x = 0.0f; normal.y = 0.0f; normal.z = 1.0f;
				}
		}
	}

	if(mint >= *mindist)
		return false;

	point->point = p;
	point->normal = normal;
	point->surfaceA = 0;
	point->pieceA = 0;
	point->surfaceB = box->surface;
	point->pieceB = box->piece;
	*mindist = mint;

	return true;
}

bool CollisionProcess_ProcessLineSphere(const CColLine *line, const CColSphere *sphere, CColPoint *point, float *mindist)
{
	float v01[3] = {line->p1.x - line->p0.x, line->p1.y - line->p0.y, line->p1.z - line->p0.z};
	float v0c[3] = {sphere->bounds.center.x - line->p0.x, sphere->bounds.center.y - line->p0.y, sphere->bounds.center.z - line->p0.z};
	float linesq = v01[0]*v01[0] + v01[1]*v01[1] + v01[2]*v01[2];

	// Project v0c onto v01, scaled by |v01|, this is the midpoint of the two intersections
	float projline = v01[0]*v0c[0] + v01[1]*v0c[1] + v01[2]*v0c[2];

	// Tangent of p0 to the sphere, scaled by linesq just like projline squared
	float tansq = ((v0c[0]*v0c[0] + v0c[1]*v0c[1] + v0c[2]*v0c[2]) - sphere->bounds.radius*sphere->bounds.radius) * linesq;

	// This works out to be the square of the distance between the midpoint and the intersections
	float diffsq = projline*projline - tansq;
	float t;

	// No intersection
	if (diffsq < 0.0f)
		return false;

	// Point of the first intersection, in range [0,1] between p0 and p1
	t = (projline - sqrtf(diffsq)) / linesq;

	// If not on the line or beyond mindist, no intersection
	if (t < 0.0f || t > 1.0f || t >= *mindist)
		return false;
	point->point.x = line->p0.x + v01[0]*t;
	point->point.y = line->p0.y + v01[1]*t;
	point->point.z = line->p0.z + v01[2]*t;
	point->normal.x = point->point.x - sphere->bounds.center.x;
	point->normal.y = point->point.y - sphere->bounds.center.y;
	point->normal.z = point->point.z - sphere->bounds.center.z;
	ColPoint_NormaliseNormal(point);
	point->surfaceA = 0;
	point->pieceA = 0;
	point->surfaceB = sphere->surface;
	point->pieceB = sphere->piece;
	*mindist = t;
	return true;
}
//- rouz edit (Claude)
