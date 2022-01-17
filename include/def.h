#ifndef ____DEF____
#define ____DEF____

#include <bits/stdc++.h>

using namespace std;

const unsigned int WINDOW_WIDTH = 700;
const unsigned int WINDOW_HEIGHT = 700;
const unsigned int VIEW_ANGLE = 80;

typedef double Tf;

const Tf INF = 1e10;

struct rgb {
	Tf r, g, b;

	rgb() { }
	rgb(Tf r, Tf g, Tf b) : r(r), g(g), b(b) { }

	rgb operator + (const rgb& p) const { return rgb(r + p.r, g + p.g, b + p.b); }
	rgb operator - (const rgb& p) const { return rgb(r - p.r, g - p.g, b - p.b); }
	rgb operator * (const rgb& p) const { return rgb(r * p.r, g * p.g, b * p.b); }
	rgb operator * (const Tf& k) const { return rgb(r * k, g * k, b * k); }
	rgb operator / (const Tf& k) const { return rgb(r / k, g / k, b / k); }
	void clamp() {
		r = std::clamp(Tf(0), r, Tf(1));
		g = std::clamp(Tf(0), g, Tf(1));
		b = std::clamp(Tf(0), b, Tf(1));
	}
};

const rgb WHITE = rgb(1, 1, 1);
const rgb BLACK = rgb(0, 0, 0);

void drawRect(Tf, Tf, rgb = WHITE, Tf = 0);

#endif
