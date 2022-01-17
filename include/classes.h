#ifndef ____CLASSES____
#define ____CLASSES____

#include <bits/stdc++.h>
#include <GL/glut.h>

#include "def.h"
#include "geo3d.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;

class Object;
struct Light;

extern vector<Object*> objects;
extern vector<Light> lights;

extern bool hasTextures;;
extern unsigned char* texDat[2];
extern int tex_width[2], tex_height[2], tex_channels[2];

struct Ray {
	Pt3 o;
	Vec3 v;
	Ray() { }
	Ray(Pt3 o, Vec3 v) : o(o), v(unit(v)) { }
};

struct Light {
	Pt3 pos;
	rgb col;

	Light() { }

	void draw() {
		glPushMatrix();

		glBegin(GL_POINTS);
		glColor3f(col.r, col.g, col.b);
		glPointSize(10);
		
		glVertex3f(pos.x, pos.y, pos.z);

		glEnd();

		glPopMatrix();
	}

	friend istream& operator >> (istream& is, Light& l) {
		return is >> l.pos >> l.col.r >> l.col.g >> l.col.b;
	}
};

class Object {
	protected:
		Pt3 ref_pt;
		rgb col;
		Tf C_AMB, C_DIFF, C_SPEC, C_REC_REFL;
		int shine;

	public:
		Object() : ref_pt(0, 0, 0) { }
		Object(Pt3 ref_pt) : ref_pt(ref_pt) { }

		virtual void draw() { }
		virtual Vec3 get_normal(Pt3 ins) { return Vec3(0, 0, 1); }
		virtual rgb get_color(Pt3 ins) { return col; }
		virtual Tf intersect(const Ray& r) { return -1; }

		Tf intersect(Ray& ray, rgb& color, int depth) {
			Tf t = intersect(ray);
			if(depth == 0 or t < 0) return t;

			Pt3 ins = ray.o + ray.v * t;
			Vec3 normal = get_normal(ins);
			if(dcmp(dot(normal, ray.v)) > 0) normal = normal * -1;
			rgb col = get_color(ins);
			color = col * C_AMB;

			for(Light l : lights) {
				Vec3 dir = l.pos - ins;
				Ray lray(ins, unit(dir));

				Tf tmin = INF;
				for(Object* pobj : objects) {
					rgb temp;
					Tf cur = pobj->intersect(lray, temp, 0);
					if(dcmp(cur) > 0 and dcmp(cur, tmin) < 0) tmin = cur;
				}

				if(dcmp(tmin, len(dir)) >= 0) {
					Tf lambert = dot(normal, lray.v);
					lambert = clamp(Tf(0), lambert, Tf(1));
					color = color + l.col * C_DIFF * lambert * col;

					Vec3 refl = reflection(lray.v, normal);
					Tf phong = dot(refl, ray.v);
					phong = clamp(Tf(0), phong, Tf(1));
					phong = pow(phong, shine);
					color = color + l.col * C_SPEC * phong * col;
				}
			}

			if(depth > 1) {
				Ray ref_ray(ins, reflection(ray.v, normal));
				ref_ray.o = ref_ray.o + ref_ray.v * EPS;

				rgb r_col;
				Tf t_min = INF;
				Object* o_min = nullptr;

				for(Object* obj : objects) {
					Tf t = obj->intersect(ref_ray, r_col, 0);
					if(t > 0 and dcmp(t, t_min) < 0) {
						o_min = obj;
						t_min = t;
					}
				}

				if(o_min) {
					r_col = rgb();
					t_min = o_min->intersect(ref_ray, r_col, depth - 1);
					color = color + r_col * C_REC_REFL;
				}
			}

			color.clamp();

			return t;
		}

		void setColor(rgb c) {
			col = c;
		}

		void setCoeffs(Tf amb, Tf diff, Tf spec, Tf rec_refl, int shn) {
			C_AMB = amb;
			C_DIFF = diff;
			C_SPEC = spec;
			C_REC_REFL = rec_refl;
			shine = shn;
		}

		friend istream& operator >> (istream& is, Object& obj) {
			is >> obj.col.r >> obj.col.g >> obj.col.b;
			is >> obj.C_AMB >> obj.C_DIFF >> obj.C_SPEC >> obj.C_REC_REFL;
			is >> obj.shine;
			return is;
		}
};

class Floor : public Object {
	private:
		Tf floor_width;
		Tf tile_width;
		Vec3 normal;
		constexpr static Tf C_BLACK = 0.1;

	public:
		Floor(Tf floor_width = 1000, Tf tile_width = 20) :
			Object(Pt3(0, 0, 0)),
			floor_width(floor_width),
			tile_width(tile_width),
			normal(Vec3(0, 0, 1))
		{ }

		void draw() {
			glPushMatrix();
			glTranslatef(ref_pt.x, ref_pt.y, ref_pt.z);

			int tiles = (int) ceil(floor_width / tile_width);
			for(int i=-tiles/2; i<=tiles/2; ++i) {
				for(int j=-tiles/2; j<=tiles/2; ++j) {
					glPushMatrix();
					glTranslatef(i * tile_width, j * tile_width, 0);

					if(abs(i + j) % 2) {
						drawRect(tile_width, tile_width, WHITE);
					}
					else {
						drawRect(tile_width, tile_width, BLACK);
					}

					glPopMatrix();
				}
			}

			glPopMatrix();
		}

		rgb get_color(Pt3 loc) {
			int tiles = (int) ceil(floor_width / tile_width);
			Pt3 topleft = ref_pt - Vec3(1, 0, 0) * (tiles / 2) * tile_width - Vec3(0, 1, 0) * (tiles / 2) * tile_width;

			int x = llround((loc.x - topleft.x) / tile_width);
			int y = llround((loc.y - topleft.y) / tile_width);

			if(hasTextures) {
				Pt3 tlc = topleft + Vec3(1, 0, 0) * x * tile_width + Vec3(0, 1, 0) * y * tile_width;
				tlc.x -= tile_width / 2;
				tlc.y -= tile_width / 2;
				int c = (x + y) % 2;

				int xpx = (int) round((loc.x - tlc.x) / tile_width * tex_width[c]);
				int ypx = (int) round((loc.y - tlc.y) / tile_width * tex_height[c]);
				xpx = max(0, min(xpx, tex_width[c] - 1));
				ypx = max(0, min(ypx, tex_height[c] - 1));
				int idx = (ypx * tex_width[c] + xpx) * tex_channels[c];
				rgb ret = rgb(texDat[c][idx], texDat[c][idx+1], texDat[c][idx+2]) / 255;
				return ret;
			}
			else {
				if(abs(x + y) % 2) return WHITE;
				else return BLACK;
			}
		}

		Vec3 get_normal(Pt3 ins) {
			return normal;
		}

		Tf intersect(const Ray& r) {
			Tf dn = dot(r.v, normal);
			if(dcmp(dn) == 0) return -1;

			Tf k = dot(ref_pt - r.o, normal) / dn;
			if(dcmp(k) < 0) return -1;

			Pt3 ins = r.o + r.v * k;
			if(
					dcmp(fabs(ins.x - ref_pt.x), floor_width / 2) > 0 or
					dcmp(fabs(ins.y - ref_pt.y), floor_width / 2) > 0) {
				return -1;
			}

			return k;
		}
};

class Sphere : public Object {
	private:
		Tf radius;

	public:
		Sphere() : radius(50) { }
		Sphere(Pt3 center, Tf radius) :
			Object(center),
			radius(radius)
		{ }

		void draw() {
			glPushMatrix();
			glTranslatef(ref_pt.x, ref_pt.y, ref_pt.z);

			int stacks = 100, slices = 200;
			if(stacks & 1) stacks++;
			vector< vector<Pt3> > pts(stacks + 1, vector<Pt3> (slices + 1));

			for(int j=0; j<=slices; ++j) {
				pts[0][j].x = radius * cos(PI * 2 * j / slices);
				pts[0][j].y = radius * sin(PI * 2 * j / slices);
				pts[0][j].z = 0;
			}
			for(int i=1; i<=stacks; ++i) {
				for(int j=0; j<=slices; ++j) {
					pts[i][j] = rotateVecAroundVec(pts[i-1][j], Vec3(1, 0, 0), PI / stacks);
				}
			}

			glColor3f(col.r, col.g, col.b);
			for(int i=0; i<stacks; ++i) {
				for(int j=0; j<slices; ++j) {
					glBegin(GL_QUADS);
					
					glVertex3f(pts[i][j].x, pts[i][j].y, pts[i][j].z);
					glVertex3f(pts[i][j+1].x, pts[i][j+1].y, pts[i][j+1].z);
					glVertex3f(pts[i+1][j+1].x, pts[i+1][j+1].y, pts[i+1][j+1].z);
					glVertex3f(pts[i+1][j].x, pts[i+1][j].y, pts[i+1][j].z);

					glEnd();
				}
			}

			glPopMatrix();
		}

		Vec3 get_normal(Pt3 ins) {
			return unit(ins - ref_pt);
		}

		Tf intersect(const Ray& r) {
			Vec3 oc = r.o - ref_pt;
			Tf a = dot(r.v, r.v);
			Tf b = 2 * dot(oc, r.v);
			Tf c = dot(oc, oc) - radius * radius;

			Tf discriminant = b * b - 4 * a * c;
			if(discriminant < 0) return -1;

			Tf t1 = (-b - sqrt(discriminant)) / (2 * a);
			if(t1 > 0) return t1;

			Tf t2 = (-b + sqrt(discriminant)) / (2 * a);
			if(t2 > 0) return t2;

			return -1;
		}

		friend istream& operator >> (istream& is, Sphere& sp) {
			is >> sp.ref_pt >> sp.radius;
			is >> static_cast<Object&>(sp);
			return is;
		}
};

class Triangle : public Object {
	private:
		Pt3 a, b, c;
	
	public:
		Triangle() { }
		Triangle(Pt3 a, Pt3 b, Pt3 c) : Object(), a(a), b(b), c(c) { }

		void draw() {
			glPushMatrix();
			glTranslatef(ref_pt.x, ref_pt.y, ref_pt.z);
			glColor3f(col.r, col.g, col.b);

			glBegin(GL_TRIANGLES);

			glVertex3f(a.x, a.y, a.z);
			glVertex3f(b.x, b.y, b.z);
			glVertex3f(c.x, c.y, c.z);

			glEnd();

			glPopMatrix();
		}

		Vec3 get_normal(Pt3 ins) {
			return unit(cross(b - a, c - a));
		}

		Tf intersect(const Ray& ray) {
			// from The Graphics Codex by Morgan McGuire
			const Vec3 ab = b - a, ac = c - a;
			const Vec3 n = unit(cross(ab, ac));
			if(dcmp(dot(n, ray.v)) >= 0) return -1;

			const Vec3 q = cross(ray.v, ac);
			const Tf aa = dot(ab, q);
			if(fabs(aa) <= EPS) return -1;

			const Vec3 s = (ray.o - a) / aa;
			const Vec3 r = cross(s, ab);

			Tf bu = dot(s, q);
			Tf bv = dot(r, ray.v);
			Tf bw = 1 - bu - bv;
			if(dcmp(bu) < 0 or dcmp(bv) < 0 or dcmp(bw) < 0) return -1;
			
			Tf t = dot(ac, r);
			if(dcmp(t) <= 0) return -1;

			return t;
		}

		friend istream& operator >> (istream& is, Triangle& tr) {
			is >> tr.a >> tr.b >> tr.c;
			is >> static_cast<Object&>(tr);
			return is;
		}
};

class Quadratic : public Object {
	protected:
		constexpr static int A = 0, B = 1, C = 2, D = 3, E = 4, F = 5, G = 6, H = 7, I = 8, J = 9;
		Tf coef[10];
		Tf length, width, height;
	
	public:
		Quadratic() { }

		Vec3 get_normal(Pt3 ins) {
			Tf x = 2 * coef[A] * ins.x + coef[D] * ins.y + coef[E] * ins.z + coef[G];
			Tf y = 2 * coef[B] * ins.x + coef[D] * ins.x + coef[F] * ins.z + coef[H];
			Tf z = 2 * coef[C] * ins.x + coef[E] * ins.y + coef[F] * ins.z + coef[I];
			return unit(Vec3(x, y, z));
		}

		Tf intersect(const Ray& ray) {
			// nasty geometry: http://skuld.bmsc.washington.edu/people/merritt/graphics/quadrics.html

			Tf a = coef[A] * ray.v.x * ray.v.x
				 + coef[B] * ray.v.y * ray.v.y
				 + coef[C] * ray.v.z * ray.v.z
				 + coef[D] * ray.v.x * ray.v.y
				 + coef[E] * ray.v.x * ray.v.z
				 + coef[F] * ray.v.y * ray.v.z;
			Tf b = 2 * coef[A] * ray.o.x * ray.v.x
				 + 2 * coef[B] * ray.o.y * ray.v.y 
				 + 2 * coef[C] * ray.o.z * ray.v.z 
				 + coef[D] * (ray.o.x * ray.v.y + ray.o.y * ray.v.x)
				 + coef[E] * (ray.o.x * ray.v.z + ray.o.z * ray.v.x)
				 + coef[F] * (ray.o.y * ray.v.z + ray.o.z * ray.v.y)
				 + coef[G] * ray.v.x
				 + coef[H] * ray.v.y
				 + coef[I] * ray.v.z;
			Tf c = coef[A] * ray.o.x * ray.o.x
				 + coef[B] * ray.o.y * ray.o.y
				 + coef[C] * ray.o.z * ray.o.z
				 + coef[D] * ray.o.x * ray.o.y
				 + coef[E] * ray.o.x * ray.o.z
				 + coef[F] * ray.o.y * ray.o.z
				 + coef[G] * ray.o.x
				 + coef[H] * ray.o.y
				 + coef[I] * ray.o.z
				 + coef[J];

			Tf det = b * b - 4 * a * c;
			if(det < 0) return -1;

			Tf t1 = (-b + sqrt(det)) / (a * 2);
			Tf t2 = (-b - sqrt(det)) / (a * 2);

			Pt3 ins1 = ray.o + ray.v * t1;
			Pt3 ins2 = ray.o + ray.v * t2;

			bool ok1 = dcmp(t1 > 0)
				and (dcmp(length) <= 0 or (dcmp(ins1.x, ref_pt.x) >= 0 and dcmp(ins1.x, ref_pt.x + length) <= 0))
				and (dcmp(width) <= 0 or (dcmp(ins1.y, ref_pt.y) >= 0 and dcmp(ins1.y, ref_pt.y + width) <= 0))
				and (dcmp(height) <= 0 or (dcmp(ins1.z, ref_pt.z) >= 0 and dcmp(ins1.z, ref_pt.z + height) <= 0));
			bool ok2 = dcmp(t2 > 0)
				and (dcmp(length) <= 0 or (dcmp(ins2.x, ref_pt.x) >= 0 and dcmp(ins2.x, ref_pt.x + length) <= 0))
				and (dcmp(width) <= 0 or (dcmp(ins2.y, ref_pt.y) >= 0 and dcmp(ins2.y, ref_pt.y + width) <= 0))
				and (dcmp(height) <= 0 or (dcmp(ins2.z, ref_pt.z) >= 0 and dcmp(ins2.z, ref_pt.z + height) <= 0));

			if(ok1 and ok2) return min(t1, t2);
			if(ok1) return t1;
			if(ok2) return t2;
			return -1;
		}

		friend istream& operator >> (istream& is, Quadratic& qd) {
			for(int i=0; i<10; ++i) is >> qd.coef[i];
			is >> qd.ref_pt >> qd.length >> qd.width >> qd.height;
			is >> static_cast<Object&>(qd);
			return is;
		}
};

#endif
