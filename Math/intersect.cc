#include <cmath>
#include "intersect.h"
#include "constants.h"
#include "tools.h"

/* | algo           | difficulty | */
/* |----------------+------------| */
/* | BBoxBBox       |          1 | */
/* | BSPherePlane   |          2 | */
/* | BBoxPlane      |          4 | */

// @@ TODO: test if a BSpheres intersects a plane.
//! Returns :
//   +IREJECT outside
//   -IREJECT inside
//    IINTERSECT intersect

int BSpherePlaneIntersect(const BSphere *bs, Plane *pl) {
	//calculamos la distancia entre el centro de la esfera y el plano 
	float dist = pl->distance(bs->m_centre);
	// si la distancia es menor que el radio de la esfera, entonces intersecta
	if (dist<=bs->m_radius){
		return IINTERSECT;
	}else{
		// si no intersecta miraremos en que lado del plano esta la esfera
		int pos = pl->whichSide(bs->m_centre);
		if(pos==1){
			return +IREJECT;
		}else if(pos==-1){
			return -IREJECT;
		}
	}
}


// @@ TODO: test if two BBoxes intersect.
//! Returns :
//    IINTERSECT intersect
//    IREJECT don't intersect

int  BBoxBBoxIntersect(const BBox *bba, const BBox *bbb ) {
//para la proyeccion en x comprobar si intersectan o no
// si no intersectan, salimos.Si intersectan, comprobamos la proyeccion en y
//recomendacion hacer tres tres ifs diferentes y si no sucede ninguno sacamos al final que no sucede
// if(x)
	if(bba->m_min[0]>bbb->m_max[0] || bbb->m_min[0]> bba->m_max[0]){
		return IREJECT;
	}else{
		if(bba->m_min[1]>bbb->m_max[1] || bbb->m_min[1]> bba->m_max[1]){
			return IREJECT;
		}else{
			if (bba->m_min[2]>bbb->m_max[2] || bbb->m_min[2]> bba->m_max[2]){
				return IREJECT;
			}else{
				return IINTERSECT;
			}
			
		}
	}
}

// @@ TODO: test if a BBox and a plane intersect.
//! Returns :
//   +IREJECT outside
//   -IREJECT inside
//    IINTERSECT intersect
//    https://gdbooks.gitbooks.io/3dcollisions/content/Chapter2/static_aabb_plane.html
int  BBoxPlaneIntersect (const BBox *theBBox, Plane *thePlane) {
	// obtenemos el centro del BB
	Vector3 centro = (theBBox->m_max+theBBox->m_min)*0.5f;
	// calculamos las extensiones positivas
	Vector3 extendido = theBBox->m_max - centro;
	// ccalculamos la proyeccion del radio del BB 
	float proyeccion = extendido[0]*fabs(thePlane->m_n[0]) + extendido[1]*fabs(thePlane->m_n[1]) + extendido[2]*fabs(thePlane->m_n[2]);
	// calculamos la distancia entre el centro del BB y el plano
	float distancia = thePlane->m_n.dot(centro) - thePlane->m_d;
	// si la distancia es menor a la proyeccion, intersectan.
	// si eso no sucede, miramos a que lado del planose encuentra el BB
	if(fabs(distancia)<=proyeccion){
		return IINTERSECT;
	}else if(thePlane->whichSide(centro)==1){
		return +IREJECT;
	}else if(thePlane->whichSide(centro)==-1){
		return -IREJECT;
	}
}

// Test if two BSpheres intersect.
//! Returns :
//    IREJECT don't intersect
//    IINTERSECT intersect

int BSphereBSphereIntersect(const BSphere *bsa, const BSphere *bsb ) {

	Vector3 v;
	v = bsa->m_centre - bsb->m_centre;
	float ls = v.dot(v);
	float rs = bsa->m_radius + bsb->m_radius;
	if (ls > (rs * rs)) return IREJECT; // Disjoint
	return IINTERSECT; // Intersect
}


// Test if a BSpheres intersect a BBox.
//! Returns :
//    IREJECT don't intersect
//    IINTERSECT intersect

int BSphereBBoxIntersect(const BSphere *sphere, const BBox *box) {

	float d;
	float aux;
	float r;

	r = sphere->m_radius;
	d = 0;

	aux = sphere->m_centre[0] - box->m_min[0];
	if (aux < 0) {
		if (aux < -r) return IREJECT;
		d += aux*aux;
	} else {
		aux = sphere->m_centre[0] - box->m_max[0];
		if (aux > 0) {
			if (aux > r) return IREJECT;
			d += aux*aux;
		}
	}

	aux = (sphere->m_centre[1] - box->m_min[1]);
	if (aux < 0) {
		if (aux < -r) return IREJECT;
		d += aux*aux;
	} else {
		aux = sphere->m_centre[1] - box->m_max[1];
		if (aux > 0) {
			if (aux > r) return IREJECT;
			d += aux*aux;
		}
	}

	aux = sphere->m_centre[2] - box->m_min[2];
	if (aux < 0) {
		if (aux < -r) return IREJECT;
		d += aux*aux;
	} else {
		aux = sphere->m_centre[2] - box->m_max[2];
		if (aux > 0) {
			if (aux > r) return IREJECT;
			d += aux*aux;
		}
	}
	if (d > r * r) return IREJECT;
	return IINTERSECT;
}

// Test if a Triangle intersects a ray.
//! Returns :
//    IREJECT don't intersect
//    IINTERSECT intersect

int IntersectTriangleRay(const Vector3 & P0,
						 const Vector3 & P1,
						 const Vector3 & P2,
						 const Line *l,
						 Vector3 & uvw) {
	Vector3 e1(P1 - P0);
	Vector3 e2(P2 - P0);
	Vector3 p(crossVectors(l->m_d, e2));
	float a = e1.dot(p);
	if (fabs(a) < Constants::distance_epsilon) return IREJECT;
	float f = 1.0f / a;
	// s = l->o - P0
	Vector3 s(l->m_O - P0);
	float lu = f * s.dot(p);
	if (lu < 0.0 || lu > 1.0) return IREJECT;
	Vector3 q(crossVectors(s, e1));
	float lv = f * q.dot(l->m_d);
	if (lv < 0.0 || lv > 1.0) return IREJECT;
	uvw[0] = lu;
	uvw[1] = lv;
	uvw[2] = f * e2.dot(q);
	return IINTERSECT;
}

/* IREJECT 1 */
/* IINTERSECT 0 */

const char *intersect_string(int intersect) {

	static const char *iint = "IINTERSECT";
	static const char *prej = "IREJECT";
	static const char *mrej = "-IREJECT";
	static const char *error = "IERROR";

	const char *result = error;

	switch (intersect) {
	case IINTERSECT:
		result = iint;
		break;
	case +IREJECT:
		result = prej;
		break;
	case -IREJECT:
		result = mrej;
		break;
	}
	return result;
}
