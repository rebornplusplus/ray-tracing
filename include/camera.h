#ifndef ____CAMERA____
#define ____CAMERA____

#include "def.h"
#include "geo3d.h"

struct Camera {
	constexpr static Tf MOVEC = 5;
	constexpr static Tf ROTC = 5;	// (degrees)

	Pt3 pos;
	Vec3 look, up, right;
	Camera() : 
		pos(0, 0, 100),
		look(unit(Vec3(0, 0, -1))), 
		up(unit(Vec3(0, 1, 0))),
		right(unit(Vec3(1, 0, 0)))
	{ }

	void moveForward() {
		pos = pos + MOVEC * look;
	}
	void moveBackward() {
		pos = pos - MOVEC * look;
	}
	void moveUp() {
		pos = pos + MOVEC * up;
	}
	void moveDown() {
		pos = pos - MOVEC * up;
	}
	void moveRight() {
		pos = pos + MOVEC * right;
	}
	void moveLeft() {
		pos = pos - MOVEC * right;
	}

	void lookLeft() {
		look = rotateVecAroundVec(look, up, toRadians(ROTC));
		right = rotateVecAroundVec(right, up, toRadians(ROTC));
	}
	void lookRight() {
		look = rotateVecAroundVec(look, up, -toRadians(ROTC));
		right = rotateVecAroundVec(right, up, -toRadians(ROTC));
	}
	void lookUp() {
		look = rotateVecAroundVec(look, right, toRadians(ROTC));
		up = rotateVecAroundVec(up, right, toRadians(ROTC));
	}
	void lookDown() {
		look = rotateVecAroundVec(look, right, -toRadians(ROTC));
		up = rotateVecAroundVec(up, right, -toRadians(ROTC));
	}
	void tiltCCW() {
		up = rotateVecAroundVec(up, look, toRadians(ROTC));
		right = rotateVecAroundVec(right, look, toRadians(ROTC));
	}
	void tiltCW() {
		up = rotateVecAroundVec(up, look, -toRadians(ROTC));
		right = rotateVecAroundVec(right, look, -toRadians(ROTC));
	}

	void prnt() {
		cerr << fixed << setprecision(2) << "Camera at " << pos << " look = " << look << " up = " << up << " right = " << right << "\n";
	}
};

#endif
