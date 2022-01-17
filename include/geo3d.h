#ifndef ____GEO_3D_____
#define ____GEO_3D_____ 

#include <bits/stdc++.h>
#include "def.h"

using namespace std;

const Tf PI = acos(-1);
const Tf EPS = 1e-7;

inline int dcmp(Tf x, Tf y = 0) { return fabs(x - y) < EPS ? 0 : (x < y ? -1 : 1); }

inline Tf toRadians(Tf deg) { return deg * PI / 180; }
inline Tf toDegrees(Tf rad) { return rad * 180 / PI; }

typedef struct Pt3 {
	Tf x, y, z;
	Pt3() { }
	Pt3(Tf x, Tf y, Tf z) : x(x), y(y), z(z) { }

	Pt3 operator + (const Pt3& p) const { return Pt3(x + p.x, y + p.y, z + p.z); }
	Pt3 operator - (const Pt3& p) const { return Pt3(x - p.x, y - p.y, z - p.z); }
	Pt3 operator * (const Tf& v) const { return Pt3(x * v, y * v, z * v) ; }
	Pt3 operator / (const Tf& v) const { return Pt3(x / v, y / v, z / v) ; }
	friend Pt3 operator * (const Tf& v, const Pt3& p) { return Pt3(p.x * v, p.y * v, p.z * v); }

	friend ostream& operator << (ostream& os, const Pt3& p) {
		return os << "(" << p.x << ", " << p.y << ", " << p.z << ")";
	}
	friend istream& operator >> (istream& is, Pt3& p) {
		return is >> p.x >> p.y >> p.z;
	}
} Vec3;

Tf dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

Vec3 cross(Vec3 a, Vec3 b) {
	return Vec3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

Tf sqlen(Vec3 a) { return dot(a, a); }
Tf len(Vec3 a) { return sqrtl(sqlen(a)); }

Vec3 unit(Vec3 a) { return a / len(a); }

Tf angleBetween(Vec3 a, Vec3 b) {
	Tf costh = dot(a, b) / len(a) / len(b);
	return acos(max(-1.0, min(1.0, costh)));
}

Vec3 rotateVecAroundVec(Vec3 v, Vec3 axis, Tf ang) {
	/** https://en.wikipedia.org/wiki/Rodrigues%27_rotation_formula#Statement **/
	Vec3 k = unit(axis);
	Vec3 vrot = v * cos(ang) + cross(k, v) * sin(ang) + k * dot(k, v) * (1 - cos(ang));
	return vrot;
}

// intersection of a ray and a plane, TRUE if exists, output in ins
// ray from r0 with vec rv, plane point p0, normal pn
bool rayPlaneIntersection(Pt3 r0, Vec3 rv, Pt3 p0, Vec3 pn, Pt3& ins) {
	Tf dn = dot(rv, pn);
	if(dcmp(dn) == 0) return false;
	Tf k = dot(p0 - r0, pn) / dn;
	if(dcmp(k) < 0) return false;
	ins = r0 + k * rv;
	return true;
}

// reflection of ray v towards intersection point 
// where n is the normal
// https://math.stackexchange.com/a/13263
Vec3 reflection(Vec3 v, Vec3 n) {
	n = unit(n);
	Vec3 r = v - n * 2 * dot(v, n);
	if(dcmp(dot(v, n)) >= 0) r = r * (-1);
	return r;
}

#endif
