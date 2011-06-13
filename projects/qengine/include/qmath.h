////////////////////////////////////////////////////////////////////////////////////////////////
//
// QMATH.H
//
// Written by Shawn Simonson 08/2008 for Quadrion Engine
//
// This file contains essential classes for mathematical, geometric, and other miscellaneous
// routines such as texture offset generation, gaussian blur weight distribution, bloom
// distribution etc...
//
// Standard vector math classes for Quadrion Engine are also contained herein. (vec2f, vec3f, vec4f)
// Standard matrix math classes for Quadrion Engine are contained herein (mat4)
// Standard quaternion classes for Quadrion Engine are contained herein (quat)
//
//////////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef __QMATH_H_
#define __QMATH_H_



#ifdef QRENDER_EXPORTS
#define QMATHEXPORT_API __declspec(dllexport)
#else
#define QMATHEXPORT_API __declspec(dllimport)
#endif



#include <windows.h>
#include <math.h>
#include <xmmintrin.h>
#include <d3dx9.h>



extern QMATHEXPORT_API float QMATH_PI;

// Quadrion standard Degrees to radians //
QMATHEXPORT_API float QMATH_DEG2RAD(const float& a);

// Quadrion standard radians to degrees //
QMATHEXPORT_API float QMATH_RAD2DEG(const float& a);

// Fast isqrt implementation //
QMATHEXPORT_API float QMATH_FAST_ISQRT( const float& a );

// Min/Max //
QMATHEXPORT_API float QMATH_MAX(const float& l, const float& r);
QMATHEXPORT_API float QMATH_MIN(const float& l, const float& r);

// Matrix definition //
typedef float mat;
typedef mat mat2[4];
typedef mat mat3[9];
typedef mat mat4[16];



enum QMATH_INTERSECT_RESULT
{
	QMATH_OUTSIDE = 0,
	QMATH_INTERSECT = 1,
	QMATH_INSIDE = 2,	
};


class point2f;
class point3f;


///////////////////////////////////////////////////////////////////////////////////
//
// vec2f
//
// Standard 2 dimensional floating point vector class for Quadrion Engine
//
////////////////////////////////////////////////////////////////////////////////////
class QMATHEXPORT_API vec2f
{
	public:
	
		float x, y;
		
		vec2f();
		~vec2f(){}
		
		vec2f(const float* v);
		vec2f(const vec2f& v);
		vec2f(const point2f& p);
		vec2f(const float& nx, const float& ny);
		
		const inline void set(const float* v) { x = v[0]; y = v[1]; }
		const inline void set(const vec2f& v) { x = v.x; y = v.y; }
		const inline void set(const float& nx, const float& ny) { x = nx; y = ny; }
		
		const inline void operator*= (const float& s) { x *= s; y *= s; }
		const inline void operator/= (const float& s) { x /= s; y /= s; }
		const inline void operator+= (const vec2f& v) { x += v.x; y += v.y; }
		const inline void operator+= (const float* v) { x += v[0]; y += v[1]; }
		const inline void operator-= (const vec2f& v) { x -= v.x; y -= v.y; }
		const inline void operator-= (const float* v) { x -= v[0]; y -= v[1]; }
		const inline void operator=  (const vec2f& v) { x = v.x; y = v.y; }
		const inline void operator=  (const float* v) { x = v[0]; y = v[1]; }
		const inline bool operator== (const vec2f& v) { return ((x == v.x) && (y == v.y)); }
		const inline bool operator== (const float* v) { return ((x == v[0]) && (y == v[1])); }
		const inline bool operator!= (const vec2f& v) { return !(*this == v); }
		const inline bool operator!= (const float* v) { return !(*this == v); }
		
		
		const inline float getLengthF()
		{
			return sqrtf((x * x) + (y * y));
		}
		
		const inline double getLengthD()
		{
			return sqrt((x * x) + (y * y));
		}
		
		float getLength();
		
		const inline float dotProd(const vec2f& v)
		{
			return (x * v.x) + (y * v.y);
		}
		
		const inline vec2f getPerpVector()
		{
			return vec2f(-y, x);
		}
		
		const inline void normalize()
		{
			float iLen = 1.0F / getLength();
			x *= iLen;
			y *= iLen;
		}
	
	private:
};


class QMATHEXPORT_API vec3f
{
	public:
	
		float x, y, z;
	
		vec3f();
		~vec3f(){}
		
		vec3f(const float* v);
		vec3f(const vec3f& v);
		vec3f(const point3f& p);
		vec3f(const float& nx, const float& ny, const float& nz);
		
		const inline void set(const float* v) { x = v[0]; y = v[1]; z = v[2]; }
		const inline void set(const vec3f& v) { x = v.x; y = v.y; z = v.z; }
		const inline void set(const float& nx, const float& ny, const float& nz) { x = nx; y = ny; z = nz; }
		
		const inline vec3f operator- () { return vec3f(-x, -y, -z); }
		const inline void operator*= (const float& s) { x *= s; y *= s; z *= s; }
		const inline void operator/= (const float& s) { float inv = 1.0F / s; x *= inv; y *= inv; z *= inv; }
		const inline void operator+= (const vec3f& v) { x += v.x; y += v.y; z += v.z; }
		const inline void operator+= (const float* v) { x += v[0]; y += v[1]; z += v[2]; }
		const inline void operator-= (const vec3f& v) { x -= v.x; y -= v.y; z -= v.z; }
		const inline void operator-= (const float* v) { x -= v[0]; y -= v[1]; z -= v[2]; }
		const inline void operator=  (const vec3f& v) { x = v.x; y = v.y; z = v.z; }
		const inline void operator=  (const float* v) { x = v[0]; y = v[1]; z = v[2]; }
		const inline bool operator== (const vec3f& v) { return ((x == v.x) && (y == v.y) && (z == v.z)); }
		const inline bool operator== (const float* v) { return ((x == v[0]) && (y == v[1]) && (z == v[2])); }
		const inline bool operator!= (const vec3f& v) { return !(*this == v); }
		const inline bool operator!= (const float* v) { return !(*this == v); }
		
		const inline vec3f lerp(const vec3f& op, const float& dv)
		{
			return vec3f(x + dv * (op.x - x), y + dv * (op.y - y), z + dv * (op.z - z));
		}
		
		const inline double getLengthD()
		{
			return (sqrt((x * x) + (y * y) + (z * z)));
		}
		
		const inline float getLengthF()
		{
			return (sqrtf((x * x) + (y * y) + (z * z)));
		}
		
		float getLength();

		const inline float dotProd(const vec3f& v)
		{
			return ((x * v.x) + (y * v.y) + (z * v.z));
		}
		
		const inline void normalize()
		{
			float mag = 1.0F / getLength(); x *= mag; y *= mag; z *= mag;
		}
		
		const inline vec3f crossProd(const vec3f& v)
		{
			return vec3f((y * v.z - z * v.y), (z * v.x - x * v.z), (x * v.y - y * v.x));
		}
		
		const inline vec3f crossProd(const float* v)
		{
			return vec3f((y * v[2] - z * v[1]), (z * v[0] - x * v[2]), (x * v[1] - y * v[0]));
		}
		
		const inline float tripleScalar(const vec3f& v, const vec3f& w)
		{
			return ((y * v.z - z * v.y) * w.x + (z * v.x - x * v.z) * w.y + (x * v.y - y * v.x) * w.z);
		}
		
		const inline void	assignIfLess( const vec3f& v )
		{
			x = ( v.x < x ) ? v.x : x;
			y = ( v.y < y ) ? v.y : y;
			z = ( v.z < z ) ? v.z : z;
		}
		
		const inline void	assignIfGreater( const vec3f& v )
		{
			x = ( v.x > x ) ? v.x : x;
			y = ( v.y > y ) ? v.y : y;
			z = ( v.z > z ) ? v.z : z;
		}
	
	private:
};

class QMATHEXPORT_API vec4f
{
	public:
		
		float x, y, z, w;
		
		vec4f(){}
		~vec4f(){}
		
		vec4f(const float* v);
		vec4f(const vec4f& v);
		vec4f(const float& nx, const float& ny, const float& nz, const float& nw);
		
		const inline void set(const float* v) { x = v[0]; y = v[1]; z = v[2]; w = v[3]; }
		const inline void set(const vec4f& v) { x = v.x; y = v.y; z = v.z; w = v.w; }
		const inline void set(const float& nx, const float& ny, const float& nz, const float& nw) { x = nx; y = ny; z = nz; w = nw; }
	
		const inline void operator=  (const vec4f& v) { x = v.x; y = v.y; z = v.z; w = v.w; }
		const inline void operator=  (const float* v) { x = v[0]; y = v[1]; z = v[2]; w = v[3]; }
		
		const inline void operator*= (const float& v) { x *= v; y *= v; z *= v; w *= w; }
	
	private:
};



class QMATHEXPORT_API point2f
{
	public:
		
		float x, y;
	
	
		point2f(){}
		~point2f(){}
		
		point2f(const float* p);
		point2f(const point2f& p);
		point2f(const vec2f& v);
		point2f(const float& nx, const float& ny);
		
		
		const inline void set(const float* p) { x = p[0]; y = p[1]; }
		const inline void set(const point2f& p) { x = p.x; y = p.y; }
		const inline void set(const float& nx, const float& ny) { x = nx; y = ny; }
		
		const inline void operator= (const point2f& p) { x = p.x; y = p.y; }
		const inline void operator= (const float* p) { x = p[0]; y = p[1]; }
		
		const inline bool operator== (const point2f& p) { return (x == p.x && y == p.y); }
		const inline bool operator== (const float* p) { return (x == p[0] && y == p[1]); }
		const inline bool operator!= (const point2f& p) { return !(*this == p); }
		const inline bool operator!= (const float* p) { return !(*this == p); }
	
	private:
};


class QMATHEXPORT_API point3f
{
	public:
	
		float x, y, z;
		
		point3f(){}
		~point3f(){}
		
		point3f(const float* p);
		point3f(const point3f& p);
		point3f(const vec3f& v);
		point3f(const float& nx, const float& ny, const float& nz);
		
		const inline void set(float nx, float ny, float nz) { x = nx; y = ny; z = nz; }
	
		const inline void operator= (const point3f& p) { x = p.x; y = p.y; z = p.z; }
		const inline void operator= (const float* p) { x = p[0]; y = p[1]; z = p[2]; }
		
		const inline bool operator== (const point3f& p) { return (x == p.x && y == p.y && z == p.z); }
		const inline bool operator== (const float* p) { return (x == p[0] && y == p[1] && z == p[2]); }
		const inline bool operator!= (const point3f& p) { return !(*this == p); }
		const inline bool operator!= (const float* p) { return !(*this == p); }
		
	
	private:
};









typedef vec4f quat;


// Point and vector operators //
QMATHEXPORT_API bool operator== (const vec2f& l, const vec2f& r);
QMATHEXPORT_API bool operator== (const vec3f& l, const vec3f& r);
QMATHEXPORT_API bool operator== (const vec4f& l, const vec4f& r);
QMATHEXPORT_API bool operator== (const point2f& l, const point2f& r);
QMATHEXPORT_API bool operator== (const point3f& l, const point3f& r);

QMATHEXPORT_API vec2f operator+ (const vec2f& l, const vec2f& r);
QMATHEXPORT_API vec3f operator+ (const vec3f& l, const vec3f& r);
QMATHEXPORT_API vec3f operator+ (const vec3f& l, const float& r);
QMATHEXPORT_API point2f operator+ (const point2f& l, const vec2f& r);
QMATHEXPORT_API point3f operator+ (const point3f& l, const vec3f& r);
QMATHEXPORT_API vec2f operator+ (const point2f& l, const point2f& r);
QMATHEXPORT_API vec3f operator+ (const point3f& l, const point3f& r);

QMATHEXPORT_API vec2f operator- (const vec2f& l, const vec2f& r);
QMATHEXPORT_API vec3f operator- (const vec3f& l, const vec3f& r);
QMATHEXPORT_API point2f operator- (const point2f& l, const vec2f& r);
QMATHEXPORT_API point3f operator- (const point3f& l, const vec3f& r);
QMATHEXPORT_API vec2f operator- (const point2f& l, const point2f& r);
QMATHEXPORT_API vec3f operator- (const point3f& l, const point3f& r);

QMATHEXPORT_API vec2f operator* (const vec2f& l, const float& r);
QMATHEXPORT_API vec3f operator* (const vec3f& l, const float& r);
QMATHEXPORT_API vec2f operator/ (const vec2f& l, const float& r);
QMATHEXPORT_API vec3f operator/ (const vec3f& l, const float& r);

QMATHEXPORT_API vec3f QMATH_VEC3F_CROSSPROD(const vec3f& l, const vec3f& r);
QMATHEXPORT_API float QMATH_VEC3F_DOTPROD(const vec3f& l, const vec3f& r);
QMATHEXPORT_API void  QMATH_VEC3F_NORMALIZE(vec3f& l);
QMATHEXPORT_API void  QMATH_VEC3F_COPY(vec3f& l, const vec3f& r);
QMATHEXPORT_API void  QMATH_VEC2F_COPY(vec2f& l, const vec2f& r);
QMATHEXPORT_API void  QMATH_VEC3F_CROSSPROD(const float* l, const float* r, float* out);
QMATHEXPORT_API void  QMATH_VEC3F_NORMALIZE(float* l);
QMATHEXPORT_API float QMATH_VEC3F_DOTPROD(const float* l, const float* r);
QMATHEXPORT_API void  QMATH_VEC3F_ADDCOPY(float* l, const float* r);
QMATHEXPORT_API void  QMATH_VEC3F_SUBCOPY(float* l, const float* r);
QMATHEXPORT_API void  QMATH_VEC3F_ADD(const float* l, const float* r, float* out);
QMATHEXPORT_API void  QMATH_VEC3F_SUB(const float* l, const float* r, float* out);
QMATHEXPORT_API void  QMATH_VEC3F_SCALE(float* l, const float& scale);


// Matrix operations //
QMATHEXPORT_API void QMATH_MATRIX_VECTORTOVECTOR(mat3& m, vec3f& from, vec3f& to);
QMATHEXPORT_API void QMATH_MATRIX_LOADIDENTITY(mat4& m);
QMATHEXPORT_API void QMATH_MATRIX_LOADIDENTITY(mat3& m);
QMATHEXPORT_API void QMATH_MATRIX_LOADXROLL(mat4& m, const float& rad);
QMATHEXPORT_API void QMATH_MATRIX_LOADYROLL(mat4& m, const float& rad);
QMATHEXPORT_API void QMATH_MATRIX_LOADZROLL(mat4& m, const float& rad);
QMATHEXPORT_API void QMATH_MATRIX_LOADSCALE(mat4& m, const vec3f& s);
QMATHEXPORT_API void QMATH_MATRIX_LOADXROLL(mat3& m, const float& rad);
QMATHEXPORT_API void QMATH_MATRIX_LOADYROLL(mat3& m, const float& rad);
QMATHEXPORT_API void QMATH_MATRIX_LOADZROLL(mat3& m, const float& rad);

QMATHEXPORT_API void QMATH_MATRIX_LOADTRANSLATION(mat4& m, const vec3f& t);
QMATHEXPORT_API void QMATH_MATRIX_LOADVIEW_DX(mat4& m, const vec3f& pos, const vec3f& lookAt, const vec3f& up);
QMATHEXPORT_API void QMATH_MATRIX_LOADORTHO_DX(mat4& m, const float& l, const float& r, const float& b, const float& t, const float& n, const float& f);
QMATHEXPORT_API void QMATH_MATRIX_LOADPERSPECTIVE_DX(mat4& m, const float& fov, const float& aspect, const float& n, const float& f);
QMATHEXPORT_API void QMATH_MATRIX_TRANSPOSE(mat4& m);
QMATHEXPORT_API void QMATH_MATRIX_TRANSPOSE(mat3& m);
QMATHEXPORT_API void QMATH_MATRIX_INVERT(mat4& m);
QMATHEXPORT_API void QMATH_MATRIX_COPY(mat4& to, const mat4& from);
QMATHEXPORT_API void QMATH_MATRIX_COPY3TO4(mat4& l, const mat3& r);
QMATHEXPORT_API void QMATH_MATRIX_ADD(const mat4& l, const mat4& r, mat4& out);
QMATHEXPORT_API void QMATH_MATRIX_SUBTRACT(const mat4& l, const mat4& r, mat4& out);
QMATHEXPORT_API void QMATH_MATRIX_MULTIPLY(const mat4& l, const mat4& r, mat4& out);
QMATHEXPORT_API void QMATH_MATRIX_MULVEC(const mat4& l, const vec4f& r, vec4f& out);
QMATHEXPORT_API void QMATH_MATRIX_MULVEC(const mat3& l, const vec3f& r, vec3f& out);

QMATHEXPORT_API float QMATH_MATRIX_DETERMINANT( const mat3& m );
QMATHEXPORT_API void  QMATH_MATRIX_DETERMINANT( const mat3& m, float& res );


// Quaternion operations //
QMATHEXPORT_API void QMATH_QUATERNION_ROTATE(quat& q, const float& rad, vec3f& axis);
QMATHEXPORT_API void QMATH_QUATERNION_MAKEMATRIX(mat4& m, const quat& q);
QMATHEXPORT_API void QMATH_QUATERNION_MULQUAT(const quat& l, const quat& r, quat& out);
QMATHEXPORT_API void QMATH_QUATERNION_CONJUGATE(quat& q);
QMATHEXPORT_API float QMATH_QUATERNION_GETANGLE(const quat& q);


// Geometric functions //
QMATHEXPORT_API void QMATH_CREATE_VERTEX_NORMALS(const vec3f* verts, const unsigned int& nVerts, const unsigned int* polys, const unsigned int& nPolys, float* norms);
QMATHEXPORT_API void QMATH_CREATE_TANGENT_SPACE(const vec3f* verts, const unsigned int& nVerts, const unsigned int* polys, const unsigned int& nPolys, const vec2f* texcoords, const vec3f* norms, vec3f* tangent);
QMATHEXPORT_API float QMATH_POINT_ROTATEZ(const vec3f& p, const mat4& m);

// Frustum shape functions //
QMATHEXPORT_API vec2f QMATH_GET_FRUSTUM_DIMENSIONS(const float& fov, const float& dist, const float& aspect);
QMATHEXPORT_API void  QMATH_GET_FRUSTUM_PLANES(const mat4& MVP, vec4f* planes);


// Texture space geometry functions //
class QMATHEXPORT_API cTextureRect
{
	public:
	
		float leftU, rightU;
		float topV, bottomV;
	
	private:
};

class QMATHEXPORT_API cTextureDims
{
	public:
	
		float width, height;
	
	private:
};


QMATHEXPORT_API void QMATH_GET_TEXTURE_COORDINATES(const cTextureDims& srcDims, const RECT* src, const cTextureDims& destDims, const RECT* dest, cTextureRect* pCoords);
QMATHEXPORT_API void QMATH_GET_TEXTURE_RECT(const cTextureDims& dims, RECT* rect);
QMATHEXPORT_API float QMATH_GET_GAUSSIAN_DISTRIBUTION(const float& x, const float& y, const float& rho);
QMATHEXPORT_API void QMATH_GET_GAUSSIAN5X5_OFFSETS(const unsigned int& width, const unsigned int& height, vec2f* offsets, vec4f* weights, const float& mul);
QMATHEXPORT_API void QMATH_GET_GAUSSIAN10X10_OFFSETS(const unsigned int& width, const unsigned int& height, vec2f* offsets, vec4f* weights, const float& mul);
QMATHEXPORT_API void QMATH_GET_BLOOM_OFFSETS(const int& texSize, float texCoordOffset[15], vec4f* colorWeight, const float& dev, const float& mul);
QMATHEXPORT_API void QMATH_GET_SAMPLE4X4_OFFSETS(const int& bbWidth, const int& bbHeight, vec2f* avSampleOffsets);
QMATHEXPORT_API void QMATH_GET_SAMPLE2X2_OFFSETS(const int& bbWidth, const int& bbHeight, vec2f* avSampleOffsets);
QMATHEXPORT_API void QMATH_GET_SAMPLE3X3_OFFSETS(const int& bbWidth, const int& bbHeight, vec2f* avSampleOffsets);
QMATHEXPORT_API void QMATH_GET_BILATERAL_OFFSETS(const int& bbWidth, const int& bbHeight, vec2f* avSampleOffsets);


// Interpolative Functions //
QMATHEXPORT_API void QMATH_LERP_FLOAT3(const float* x, const float* y, const float& dv, float* out);
QMATHEXPORT_API void QMATH_LERP_FLOAT(const float x, const float y, const float& dv, float& out);
QMATHEXPORT_API void QMATH_SMOOTHSTEP_FLOAT(const float min, const float max, const float& dv, float& out);


// Visibility functions //
QMATHEXPORT_API bool QMATH_POINT_IN_SPHERE(const vec3f& test, const vec3f& sphereOrigin, const float& sphereRad);
QMATHEXPORT_API bool QMATH_POINT_IN_FRUSTUM(const vec4f* planes, vec3f& point);


#endif