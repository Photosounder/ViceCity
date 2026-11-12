//+ rouz edit (Claude)
#include <math.h>
#include "CollisionTests.h"

bool Collision_TestSphereSphere(const CSphere *s1, const CSphere *s2)
{
	// Closer than the sum of the radii
	float d = s1->radius + s2->radius;
	float dx = s1->center.x - s2->center.x;
	float dy = s1->center.y - s2->center.y;
	float dz = s1->center.z - s2->center.z;
	return dx*dx + dy*dy + dz*dz < d*d;
}

bool Collision_TestSphereBox(const CSphere *sph, const CBox *box)
{
	if(sph->center.x + sph->radius < box->min.x) return false;
	if(sph->center.x - sph->radius > box->max.x) return false;
	if(sph->center.y + sph->radius < box->min.y) return false;
	if(sph->center.y - sph->radius > box->max.y) return false;
	if(sph->center.z + sph->radius < box->min.z) return false;
	if(sph->center.z - sph->radius > box->max.z) return false;
	return true;
}

bool Collision_TestLineBox(const CColLine *line, const CBox *box)
{
	float t, x, y, z;
	// If either line point is in the box, we have a collision
	if(line->p0.x > box->min.x && line->p0.x < box->max.x &&
	   line->p0.y > box->min.y && line->p0.y < box->max.y &&
	   line->p0.z > box->min.z && line->p0.z < box->max.z)
		return true;
	if(line->p1.x > box->min.x && line->p1.x < box->max.x &&
	   line->p1.y > box->min.y && line->p1.y < box->max.y &&
	   line->p1.z > box->min.z && line->p1.z < box->max.z)
		return true;

	// check if points are on opposite sides of min x plane
	if((box->min.x - line->p1.x) * (box->min.x - line->p0.x) < 0.0f){
		// parameter along line where we intersect
		t = (box->min.x - line->p0.x) / (line->p1.x - line->p0.x);
		// y of intersection
		y = line->p0.y + (line->p1.y - line->p0.y)*t;
		if(y > box->min.y && y < box->max.y){
			// z of intersection
			z = line->p0.z + (line->p1.z - line->p0.z)*t;
			if(z > box->min.z && z < box->max.z)
				return true;
		}
	}

	// same test with max x plane
	if((line->p1.x - box->max.x) * (line->p0.x - box->max.x) < 0.0f){
		t = (line->p0.x - box->max.x) / (line->p0.x - line->p1.x);
		y = line->p0.y + (line->p1.y - line->p0.y)*t;
		if(y > box->min.y && y < box->max.y){
			z = line->p0.z + (line->p1.z - line->p0.z)*t;
			if(z > box->min.z && z < box->max.z)
				return true;
		}
	}

	// min y plne
	if((box->min.y - line->p0.y) * (box->min.y - line->p1.y) < 0.0f){
		t = (box->min.y - line->p0.y) / (line->p1.y - line->p0.y);
		x = line->p0.x + (line->p1.x - line->p0.x)*t;
		if(x > box->min.x && x < box->max.x){
			z = line->p0.z + (line->p1.z - line->p0.z)*t;
			if(z > box->min.z && z < box->max.z)
				return true;
		}
	}

	// max y plane
	if((line->p0.y - box->max.y) * (line->p1.y - box->max.y) < 0.0f){
		t = (line->p0.y - box->max.y) / (line->p0.y - line->p1.y);
		x = line->p0.x + (line->p1.x - line->p0.x)*t;
		if(x > box->min.x && x < box->max.x){
			z = line->p0.z + (line->p1.z - line->p0.z)*t;
			if(z > box->min.z && z < box->max.z)
				return true;
		}
	}

	// min z plne
	if((box->min.z - line->p0.z) * (box->min.z - line->p1.z) < 0.0f){
		t = (box->min.z - line->p0.z) / (line->p1.z - line->p0.z);
		x = line->p0.x + (line->p1.x - line->p0.x)*t;
		if(x > box->min.x && x < box->max.x){
			y = line->p0.y + (line->p1.y - line->p0.y)*t;
			if(y > box->min.y && y < box->max.y)
				return true;
		}
	}

	// max z plane
	if((line->p0.z - box->max.z) * (line->p1.z - box->max.z) < 0.0f){
		t = (line->p0.z - box->max.z) / (line->p0.z - line->p1.z);
		x = line->p0.x + (line->p1.x - line->p0.x)*t;
		if(x > box->min.x && x < box->max.x){
			y = line->p0.y + (line->p1.y - line->p0.y)*t;
			if(y > box->min.y && y < box->max.y)
				return true;
		}
	}
	return false;
}

bool Collision_TestVerticalLineBox(const CColLine *line, const CBox *box)
{
	if(line->p0.x <= box->min.x) return false;
	if(line->p0.y <= box->min.y) return false;
	if(line->p0.x >= box->max.x) return false;
	if(line->p0.y >= box->max.y) return false;
	if(line->p0.z < line->p1.z){
		if(line->p0.z > box->max.z) return false;
		if(line->p1.z < box->min.z) return false;
	}else{
		if(line->p1.z > box->max.z) return false;
		if(line->p0.z < box->min.z) return false;
	}
	return true;
}

bool Collision_TestLineSphere(const CColLine *line, const CColSphere *sph)
{
	// Same as the original with the vectors written out
	float v01[3] = {line->p1.x - line->p0.x, line->p1.y - line->p0.y, line->p1.z - line->p0.z};
	float v0c[3] = {sph->bounds.center.x - line->p0.x, sph->bounds.center.y - line->p0.y, sph->bounds.center.z - line->p0.z};
	float linesq = v01[0]*v01[0] + v01[1]*v01[1] + v01[2]*v01[2];
	float projline = -2.0f * (v01[0]*v0c[0] + v01[1]*v0c[1] + v01[2]*v0c[2]);
	const CollisionVector3 *c = &sph->bounds.center;
	const CollisionVector3 *p = &line->p0;
	float csq = c->x*c->x + c->y*c->y + c->z*c->z;
	float cdotp = c->x*p->x + c->y*p->y + c->z*p->z;
	float psq = p->x*p->x + p->y*p->y + p->z*p->z;
	float tansq = 4.0f * linesq * (csq - 2.0f*cdotp + psq - sph->bounds.radius*sph->bounds.radius);
	float diffsq = projline*projline - tansq;
	float f;

	// No real solution means the line misses the sphere
	if (diffsq < 0.0f)
		return false;

	// Parameter of the first intersection along the line
	f = (-projline - sqrtf(diffsq)) / (2.0f*linesq);
	return f >= 0.0f && f <= 1.0f;
}

float Collision_DistToLine(const float *l0, const float *l1, const float *point)
{
	float d[3] = {l1[0] - l0[0], l1[1] - l0[1], l1[2] - l0[2]};
	float pl[3] = {point[0] - l0[0], point[1] - l0[1], point[2] - l0[2]};
	float lensq = d[0]*d[0] + d[1]*d[1] + d[2]*d[2];
	float dot = pl[0]*d[0] + pl[1]*d[1] + pl[2]*d[2];
	float distSqr;

	// Beyond either end the distance is to that end point
	if (dot <= 0.0f)
		return sqrtf(pl[0]*pl[0] + pl[1]*pl[1] + pl[2]*pl[2]);
	if (dot >= lensq) {
		float e[3] = {point[0] - l1[0], point[1] - l1[1], point[2] - l1[2]};
		return sqrtf(e[0]*e[0] + e[1]*e[1] + e[2]*e[2]);
	}

	// Distance to the line itself
	distSqr = (pl[0]*pl[0] + pl[1]*pl[1] + pl[2]*pl[2]) - dot * dot / lensq;
	if (distSqr <= 0.0f)
		return 0.0f;
	return sqrtf(distSqr);
}

float Collision_DistToLineClosest(const float *l0, const float *l1, const float *point, float *closest)
{
	float d[3] = {l1[0] - l0[0], l1[1] - l0[1], l1[2] - l0[2]};
	float pl[3] = {point[0] - l0[0], point[1] - l0[1], point[2] - l0[2]};
	float lensq = d[0]*d[0] + d[1]*d[1] + d[2]*d[2];
	float dot = pl[0]*d[0] + pl[1]*d[1] + pl[2]*d[2];
	float e[3];

	// Find which point of the segment is closest
	if (dot <= 0.0f) {
		closest[0] = l0[0]; closest[1] = l0[1]; closest[2] = l0[2];
	} else if (dot >= lensq) {
		closest[0] = l1[0]; closest[1] = l1[1]; closest[2] = l1[2];
	} else {
		float t = dot/lensq;
		closest[0] = l0[0] + d[0]*t;
		closest[1] = l0[1] + d[1]*t;
		closest[2] = l0[2] + d[2]*t;
	}

	// The distance to it
	e[0] = point[0] - closest[0];
	e[1] = point[1] - closest[1];
	e[2] = point[2] - closest[2];
	return sqrtf(e[0]*e[0] + e[1]*e[1] + e[2]*e[2]);
}
//- rouz edit (Claude)
