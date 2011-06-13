#include "stdafx.h"
#include "qmath.h"



static inline void swap(float& a, float& b)
{
	float tmp = a;
	a = b;
	b = tmp;
}



QMATHEXPORT_API float QMATH_PI = 3.14159265F;


QMATHEXPORT_API float QMATH_DEG2RAD(const float& a)
{
	return QMATH_PI * a / 180.0F;
}

QMATHEXPORT_API float QMATH_RAD2DEG(const float& a)
{
	return (180.0F / QMATH_PI) * a;
}

QMATHEXPORT_API float QMATH_FAST_ISQRT( const float& a )
{
	float ha = 0.5F * a;
	float ret;
	int i = *(int*)&a;
	i = 0x5F3759D5 - ( i >> 1 );
	ret = *(float*)&i;
	ret = ret * ( 1.5F - ha * ret * ret );
	return ret;	
}


QMATHEXPORT_API float QMATH_MAX(const float& l , const float& r)
{
	return (l > r) ? l : r;
}

QMATHEXPORT_API float QMATH_MIN(const float& l, const float& r)
{
	return (l < r) ? l : r;
}




vec3f::vec3f() : x(0), y(0), z(0) {}
vec2f::vec2f() : x(0), y(0) {}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// VEC2F METHODS
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=


vec2f::vec2f(const float* v)
{
	x = v[0];
	y = v[1];
}

vec2f::vec2f(const vec2f& v)
{
	x = v.x;
	y = v.y;
}

vec2f::vec2f(const float& nx, const float& ny)
{
	x = nx;
	y = ny;
}

vec2f::vec2f(const point2f& p)
{
	x = p.x;
	y = p.y;
}

float vec2f::getLength()
{
	return 1.0F / ( QMATH_FAST_ISQRT( x * x + y * y ) );	
}



//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// VEC3F METHODS
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=



float vec3f::getLength() 
{
	return 1.0F / ( QMATH_FAST_ISQRT( x * x + y * y + z * z ) );
}

vec3f::vec3f(const float* v)
{
	x = v[0];
	y = v[1];
	z = v[2];
}

vec3f::vec3f(const vec3f& v)
{
	x = v.x;
	y = v.y;
	z = v.z;
}

vec3f::vec3f(const point3f& p)
{
	x = p.x;
	y = p.y;
	z = p.z;
}

vec3f::vec3f(const float& nx, const float& ny, const float& nz)
{
	x = nx;
	y = ny;
	z = nz;
}



//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// VEC4F METHODS 
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=


vec4f::vec4f(const float* v)
{
	x = v[0];
	y = v[1];
	z = v[2];
	w = v[3];
}

vec4f::vec4f(const vec4f& v)
{
	x = v.x;
	y = v.y;
	z = v.z;
	w = v.w;
}

vec4f::vec4f(const float& nx, const float& ny, const float& nz, const float& nw)
{
	x = nx;
	y = ny;
	z = nz;
	w = nw;
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// POINT2F METHODS 
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=


point2f::point2f(const float* p)
{
	x = p[0];
	y = p[1];
}

point2f::point2f(const point2f& p)
{
	x = p.x;
	y = p.y;
}

point2f::point2f(const float& nx, const float& ny)
{
	x = nx;
	y = ny;
}

point2f::point2f(const vec2f& v)
{
	x = v.x;
	y = v.y;
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// POINT3F METHODS
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=



point3f::point3f(const float* p)
{
	x = p[0];
	y = p[1];
	z = p[2];
}

point3f::point3f(const point3f& p)
{
	x = p.x;
	y = p.y;
	z = p.z;
}

point3f::point3f(const float& nx, const float& ny, const float& nz)
{
	x = nx;
	y = ny;
	z = nz;
}

point3f::point3f(const vec3f& v)
{
	x = v.x;
	y = v.y;
	z = v.z;
}










QMATHEXPORT_API bool operator== (const vec2f& l, const vec2f& r)
{
	return( (l.x == r.x) && (l.y == r.y) );
}

QMATHEXPORT_API bool operator== (const vec3f& l, const vec3f& r)
{
	return( (l.x == r.x) && (l.y == r.y) && (l.z == r.z) );
}

QMATHEXPORT_API bool operator== (const vec4f& l, const vec4f& r)
{
	return( (l.x == r.x) && (l.y == r.y) && (l.z == r.z) && (l.w == r.w) );
}

QMATHEXPORT_API bool operator== (const point2f& l, const point2f& r)
{
	return( (l.x == r.x) && (l.y == r.y) );
}

QMATHEXPORT_API bool operator== (const point3f& l, const point3f& r)
{
	return( (l.x == r.x) && (l.y == r.y) && (l.z == r.z) );
}

QMATHEXPORT_API vec2f operator+ (const vec2f& l, const vec2f& r)
{
	return vec2f(l.x + r.x, l.y + r.y);
}

QMATHEXPORT_API vec3f operator+ (const vec3f& l, const vec3f& r)
{
	return vec3f(l.x + r.x, l.y + r.y, l.z + r.z);
}

QMATHEXPORT_API point2f operator+ (const point2f& l, const vec2f& r)
{
	return point2f(l.x + r.x, l.y + r.y);
}

QMATHEXPORT_API point3f operator+ (const point3f& l, const vec3f& r)
{
	return point3f(l.x + r.y, l.x + r.y, l.z + r.z);
}

QMATHEXPORT_API vec2f operator+ (const point2f& l, const point2f& r)
{
	return vec2f(l.x + r.x, l.y + r.y);
}

QMATHEXPORT_API vec3f operator+ (const point3f& l, const point3f& r)
{
	return vec3f(l.x + r.x, l.y + r.y, l.z + r.z);
}

QMATHEXPORT_API vec2f operator- (const vec2f& l, const vec2f& r)
{
	return vec2f(l.x - r.x, l.y - r.y);
}

QMATHEXPORT_API vec3f operator- (const vec3f& l, const vec3f& r)
{
	return vec3f(l.x - r.x, l.y - r.y, l.z - r.z);
}

QMATHEXPORT_API point2f operator- (const point2f& l, const vec2f& r)
{
	return point2f(l.x - r.x, l.y - r.y);
}

QMATHEXPORT_API point3f operator- (const point3f& l, const vec3f& r)
{
	return point3f(l.x - r.x, l.y - r.y, l.z - r.z);
}

QMATHEXPORT_API vec2f operator- (const point2f& l, const point2f& r)
{
	return vec2f(l.x - r.x, l.y - r.y);
}

QMATHEXPORT_API vec3f operator- (const point3f& l, const point3f& r)
{
	return vec3f(l.x - r.x, l.y - r.y, l.z - r.z);
}

QMATHEXPORT_API vec2f operator* (const vec2f& l, const float& r)
{
	return vec2f(l.x * r, l.y * r);
}

QMATHEXPORT_API vec3f operator* (const vec3f& l, const float& r)
{
	return vec3f(l.x * r, l.y * r, l.z * r);
}

QMATHEXPORT_API vec2f operator/ (const vec2f& l, const float& r)
{	
	float inv = 1.0 / r;
	return vec2f(l.x * inv, l.y * inv);
}

QMATHEXPORT_API vec3f operator/ (const vec3f& l, const float& r)
{
	float inv = 1.0 / r;
	return vec3f(l.x * inv, l.y * inv, l.z * inv);
}


QMATHEXPORT_API vec3f operator+ (const vec3f& l, const float& r)
{
	return vec3f(l.x + r, l.y + r, l.z + r);
}




QMATHEXPORT_API vec3f QMATH_VEC3F_CROSSPROD(const vec3f& l, const vec3f& r)
{
	return vec3f((l.y * r.z - l.z * r.y), (l.z * r.x - l.x * r.z), (l.x * r.y - l.y * r.x));
}

QMATHEXPORT_API void  QMATH_VEC3F_CROSSPROD(const float* l, const float* r, float* out)
{
	out[0] = l[1] * r[2] - l[2] * r[1];
	out[1] = l[2] * r[0] - l[0] * r[2];
	out[2] = l[0] * r[1] - l[1] * r[0];
}

QMATHEXPORT_API float QMATH_VEC3F_DOTPROD(const vec3f& l, const vec3f& r)
{
	return ((l.x * r.x) + (l.y * r.y) + (l.z * r.z));
}

QMATHEXPORT_API float QMATH_VEC3F_DOTPROD(const float* l, const float* r)
{
	return ((l[0] * r[0]) + (l[1] * r[1]) + (l[2] * r[2]));
}

QMATHEXPORT_API void  QMATH_VEC3F_NORMALIZE(vec3f& l)
{
	float iLen = 1.0F / sqrtf((l.x * l.x) + (l.y * l.y) + (l.z * l.z));
	l.x *= iLen;
	l.y *= iLen;
	l.z *= iLen;	
}

QMATHEXPORT_API void  QMATH_VEC3F_NORMALIZE(float* l)
{
	float iLen = 1.0F / sqrtf((l[0] * l[0]) + (l[1] * l[1]) + (l[2] * l[2]));
	l[0] *= iLen;
	l[1] *= iLen;
	l[2] *= iLen;
}

QMATHEXPORT_API void  QMATH_VEC3F_COPY(vec3f& l, const vec3f& r)
{
	l.x = r.x;
	l.y = r.y;
	l.z = r.z;
}

QMATHEXPORT_API void  QMATH_VEC2F_COPY(vec2f& l, const vec2f& r)
{
	l.x = r.x;
	l.y = r.y;
}

QMATHEXPORT_API void  QMATH_VEC3F_ADDCOPY(float* l, const float* r)
{
	l[0] += r[0];
	l[1] += r[1];
	l[2] += r[2];
}

QMATHEXPORT_API void  QMATH_VEC3F_SUBCOPY(float* l, const float* r)
{
	l[0] -= r[0];
	l[1] -= r[1];
	l[2] -= r[2];	
}

QMATHEXPORT_API void  QMATH_VEC3F_SCALE(float* l, const float& scale)
{
	l[0] *= scale;
	l[1] *= scale;
	l[2] *= scale;
}

QMATHEXPORT_API void  QMATH_VEC3F_ADD(const float* l, const float* r, float* out)
{
	out[0] = l[0] + r[0];
	out[1] = l[1] + r[1];
	out[2] = l[2] + r[2];
}

QMATHEXPORT_API void  QMATH_VEC3F_SUB(const float* l, const float* r, float* out)
{
	out[0] = l[0] - r[0];
	out[1] = l[1] - r[1];
	out[2] = l[2] - r[2];
}

//----------------------------------------------------------------------------

QMATHEXPORT_API void QMATH_MATRIX_VECTORTOVECTOR(mat3& m, vec3f& from, vec3f& to)
{
	vec3f v = from.crossProd(to);
	float e = from.dotProd(to);
	float h;
	
	if(e > 1.0F - 0.01F)
	{
		QMATH_MATRIX_LOADIDENTITY(m);
		return;
	}
	
	else if(e < -1.0F + 0.01F)
	{
		vec3f up, left;
		float fxx, fyy, fzz, fxy, fxz, fyz;
		float uxx, uyy, uzz, uxy, uxz, uyz;
		float lxx, lyy, lzz, lxy, lxz, lyz;
		float iLen;
		
		left.set(0.0F, from.z, -from.y);
		if(left.dotProd(left) < 0.01F)
			left.set(-from.z, 0.0F, from.x);
		
		left.normalize();
		up = left.crossProd(from);
		
		fxx = from.x * from.x;   fyy = from.y * from.y;  fzz = from.z * from.z;
		fxy = from.x * from.y;   fxz = from.x * from.z;  fyz = from.y * from.z;
		
		uxx = up.x * up.x;   uyy = up.y * up.y;    uzz = up.z * up.z;
		uxy = up.x * up.y;   uxz = up.x * up.z;    uyz = up.y * up.z;
		
		lxx = -left.x * left.x;    lyy = -left.y * left.y;    lzz = -left.z * left.z;
		lxy = -left.x * left.y;    lxz = -left.x * left.z;    lyz = -left.y * left.z;
		
		m[0] = fxx + uxx + lxx;		m[3] = fxy + uxy + lxy;		m[6] = fxz + uxz + lxz;
		m[1] = m[3];				m[4] = fyy + uyy + lyy;		m[7] = fyz + uyz + lyz;
		m[2] = m[6];				m[5] = m[7];				m[8] = fzz + uzz + lzz;
	}
	
	else
	{
		h = (1.0F - e) / v.dotProd(v);
		m[0] = e + h * (v.x * v.x);    m[3] = h * v.x * v.y - v.z;    m[6] = h * v.x * v.z + v.y;
		m[1] = h * v.x * v.y + v.z;    m[4] = e + h * v.y * v.y;      m[7] = h * v.y * v.z - v.x;
		m[2] = h * v.x * v.z - v.y;    m[5] = h * v.y * v.z + v.x;    m[8] = e + h * v.z * v.z;	
	}
}

QMATHEXPORT_API void QMATH_MATRIX_LOADIDENTITY(mat3& m)
{
	m[0] = 1.0f;	m[3] = 0.0f;	m[6] = 0.0f;	
	m[1] = 0.0f;	m[4] = 1.0f;	m[7] = 0.0f;	
	m[2] = 0.0f;	m[5] = 0.0f;	m[8] = 1.0f;	
}

QMATHEXPORT_API void QMATH_MATRIX_LOADIDENTITY(mat4& m)
{
	m[0] = 1.0f;	m[4] = 0.0f;	m[8] = 0.0f;	m[12] = 0.0f;
	m[1] = 0.0f;	m[5] = 1.0f;	m[9] = 0.0f;	m[13] = 0.0f;
	m[2] = 0.0f;	m[6] = 0.0f;	m[10] = 1.0f;	m[14] = 0.0f;
	m[3] = 0.0f;	m[7] = 0.0f;	m[11] = 0.0f;	m[15] = 1.0f;	
}

QMATHEXPORT_API void QMATH_MATRIX_LOADXROLL(mat4& m, const float& rad)
{
	float cosRad = cos(rad);
	float sinRad = sin(rad);

	m[0] = 1.0f;	m[4] = 0.0f;	m[8] = 0.0f;		m[12] = 0.0f;
	m[1] = 0.0f;	m[5] = cosRad;	m[9] = -sinRad;		m[13] = 0.0f;
	m[2] = 0.0f;	m[6] = sinRad;	m[10] = cosRad;		m[14] = 0.0f;
	m[3] = 0.0f;	m[7] = 0.0f;	m[11] = 0.0f;		m[15] = 1.0f;	
}


QMATHEXPORT_API void QMATH_MATRIX_LOADYROLL(mat4& m, const float& rad)
{
	float cosRad = cos(rad);
	float sinRad = sin(rad);
	
	m[0] = cosRad;	m[4] = 0.0f;	m[8] = sinRad;		m[12] = 0.0f;
	m[1] = 0.0f;	m[5] = 1.0f;	m[9] = 0.0f;		m[13] = 0.0f;
	m[2] = -sinRad; m[6] = 0.0f;	m[10] = cosRad;		m[14] = 0.0f;
	m[3] = 0.0f;	m[7] = 0.0f;	m[11] = 0.0f;		m[15] = 1.0f;	
}


QMATHEXPORT_API void QMATH_MATRIX_LOADZROLL(mat4& m, const float& rad)
{
	float cosRad = cos(rad);
	float sinRad = sin(rad);
	
	m[0] = cosRad;	m[4] = -sinRad;		m[8] = 0.0f;	m[12] = 0.0f;
	m[1] = sinRad;	m[5] = cosRad;		m[9] = 0.0f;	m[13] = 0.0f;
	m[2] = 0.0f;	m[6] = 0.0f;		m[10] = 1.0f;	m[14] = 0.0f;
	m[3] = 0.0f;	m[7] = 0.0f;		m[11] = 0.0f;	m[15] = 1.0f;	
}

QMATHEXPORT_API void QMATH_MATRIX_LOADXROLL(mat3& m, const float& rad)
{
	float cosRad = cos(rad);
	float sinRad = sin(rad);
	
	m[0] = 1.0F;		m[3] = 0.0F;		m[6] = 0.0F;
	m[1] = 0.0F;		m[4] = cosRad;		m[7] = -sinRad;
	m[2] = 0.0F;		m[5] = sinRad;		m[8] = cosRad; 
}

QMATHEXPORT_API void QMATH_MATRIX_LOADYROLL(mat3& m, const float& rad)
{
	float cosRad = cos(rad);
	float sinRad = sin(rad);
	
	m[0] = cosRad;			m[3] = 0.0F;		m[6] = sinRad;
	m[1] = 0.0F;			m[4] = 1.0F;		m[7] = 0.0F;
	m[2] = -sinRad;			m[5] = 0.0F;		m[8] = cosRad;
}
QMATHEXPORT_API void QMATH_MATRIX_LOADZROLL(mat3& m, const float& rad)
{
	float cosRad = cos(rad);
	float sinRad = sin(rad);
	
	m[0] = cosRad;		m[3] = -sinRad;		m[6] = 0.0F;
	m[1] = sinRad;		m[4] = cosRad;		m[7] = 0.0F;
	m[2] = 0.0F;		m[5] = 0.0F;		m[8] = 1.0F;
}


QMATHEXPORT_API void QMATH_MATRIX_LOADSCALE(mat4& m, const vec3f& s)
{
	m[0] = s.x;		m[4] = 0.0f;	m[8] = 0.0f;	m[12] = 0.0f;
	m[1] = 0.0f;	m[5] = s.y;		m[9] = 0.0f;	m[13] = 0.0f;
	m[2] = 0.0f;	m[6] = 0.0f;	m[10] = s.z;	m[14] = 0.0f;
	m[3] = 0.0f;	m[7] = 0.0f;	m[11] = 0.0f;	m[15] = 1.0f;	
}


QMATHEXPORT_API void QMATH_MATRIX_LOADTRANSLATION(mat4& m, const vec3f& t)
{
	m[0] = 1.0f;	m[4] = 0.0f;	m[8] = 0.0f;	m[12] = t.x;
	m[1] = 0.0f;	m[5] = 1.0f;	m[9] = 0.0f;	m[13] = t.y;
	m[2] = 0.0f;	m[6] = 0.0f;	m[10] = 1.0f;	m[14] = t.z;
	m[3] = 0.0f;	m[7] = 0.0f;	m[11] = 0.0f;	m[15] = 1.0f;	
}

QMATHEXPORT_API void QMATH_MATRIX_LOADVIEW_DX(mat4& m, const vec3f& pos, const vec3f& lookAt, const vec3f& up)
{
	vec3f zAxis = pos - lookAt;
	QMATH_VEC3F_NORMALIZE(zAxis);
	vec3f xAxis = QMATH_VEC3F_CROSSPROD(up, zAxis);
	QMATH_VEC3F_NORMALIZE(xAxis);
	vec3f yAxis = QMATH_VEC3F_CROSSPROD(zAxis, xAxis);

	
	// first col
	m[0] = xAxis.x;
	m[4] = xAxis.y;
	m[8] = xAxis.z;
	m[12] = -QMATH_VEC3F_DOTPROD(xAxis, pos);
	
	// second col
	m[1] = yAxis.x;
	m[5] = yAxis.y;
	m[9] = yAxis.z;
	m[13] = -QMATH_VEC3F_DOTPROD(yAxis, pos);
	
	// third col
	m[2] = zAxis.x;
	m[6] = zAxis.y;
	m[10] = zAxis.z;
	m[14] = -QMATH_VEC3F_DOTPROD(zAxis, pos);	
	
	// fourth col
	m[3] = 0.0f;
	m[7] = 0.0f;
	m[11] = 0.0f;
	m[15] = 1.0f;	

}

QMATHEXPORT_API void QMATH_MATRIX_LOADORTHO_DX(mat4& m, const float& l, const float& r, const float& b, const float& t, const float& n, const float& f)
{
	m[0] = 2.0f / (r - l);		m[1] = 0.0f;
	m[4] = 0.0f;				m[5] = 2.0f / (t - b);
	m[8] = 0.0f;				m[9] = 0.0f;
	m[12] = 0.0f;				m[13] = 0.0f;
	
	m[2] = 0.0f;				m[3] = 0.0f;
	m[6] = 0.0f;				m[7] = 0.0f;
	m[10] = 1.0f / (f - n);		m[11] = 0.0f;
	m[14] = (-n) / (f - n);		m[15] = 1.0f;	
}

QMATHEXPORT_API void QMATH_MATRIX_LOADPERSPECTIVE_DX(mat4& m, const float& fov, const float& aspect, const float& n, const float& f)
{
	float itan = 1.0f / tan(fov / 2.0f);
	
	m[0] = itan / aspect;	m[1] = 0.0f;	m[2] = 0.0f;
	m[4] = 0.0f;			m[5] = itan;	m[6] = 0.0f;
	m[8] = 0.0f;			m[9] = 0.0f;	m[10] = (f) / (n - f);
	m[12] = 0.0f;			m[13] = 0.0f;	m[14] = (f * n) / (n - f);
	
	m[3] = 0.0f;
	m[7] = 0.0f;
	m[11] = -1.0f;
	m[15] = 0.0f;
}

QMATHEXPORT_API void QMATH_MATRIX_TRANSPOSE(mat4& m)
{
	swap(m[1], m[4]);
	swap(m[2], m[8]);
	swap(m[3], m[12]);
	swap(m[6], m[9]);
	swap(m[7], m[13]);
	swap(m[11], m[14]);	
}

QMATHEXPORT_API void QMATH_MATRIX_TRANSPOSE(mat3& m)
{
	swap(m[1], m[3]);
	swap(m[2], m[6]);
	swap(m[5], m[7]);
}

QMATHEXPORT_API void QMATH_MATRIX_INVERT(mat4& m)
{
	m[0] = -m[0];
	m[1] = -m[1];
	m[2] = -m[2];
	
	m[4] = -m[4];
	m[5] = -m[5];
	m[6] = -m[6];
	
	m[8] = -m[8];
	m[9] = -m[9];
	m[10] = -m[10];	
}

QMATHEXPORT_API void QMATH_MATRIX_COPY(mat4& to, const mat4& from)
{
	to[0] = from[0];	to[4] = from[4];	to[8] = from[8];		to[12] = from[12];
	to[1] = from[1];	to[5] = from[5];	to[9] = from[9];		to[13] = from[13];
	to[2] = from[2];	to[6] = from[6];	to[10] = from[10];		to[14] = from[14];
	to[3] = from[3];	to[7] = from[7];	to[11] = from[11];		to[15] = from[15];	
}

QMATHEXPORT_API void QMATH_MATRIX_COPY3TO4(mat4& l, const mat3& r)
{
	l[0] = r[0]; l[4] = r[3]; l[8] = r[6]; l[12] = 0.0F;
	l[1] = r[1]; l[5] = r[4]; l[9] = r[7]; l[13] = 0.0F;
	l[2] = r[2]; l[6] = r[5]; l[10] = r[8]; l[14] = 0.0F;
	l[3] = 0.0F; l[7] = 0.0F; l[11] = 0.0F; l[15] = 1.0F;	
}

QMATHEXPORT_API void QMATH_MATRIX_ADD(const mat4& l, const mat4& r, mat4& out)
{
	out[0] = l[0] + r[0];	out[4] = l[4] + r[4];	out[8] = l[8] + r[8];
	out[1] = l[1] + r[1];	out[5] = l[5] + r[5];	out[9] = l[9] + r[9];
	out[2] = l[2] + r[2];	out[6] = l[6] + r[6];	out[10] = l[10] + r[10];
	out[3] = l[3] + r[3];	out[7] = l[7] + r[7];	out[11] = l[11] + r[11];
	
	out[12] = l[12] + r[12];
	out[13] = l[13] + r[13];
	out[14] = l[14] + r[14];
	out[15] = l[15] + r[15];	
}

QMATHEXPORT_API void QMATH_MATRIX_SUBTRACT(const mat4& l, const mat4& r, mat4& out)
{
	out[0] = l[0] - r[0];	out[4] = l[4] - r[4];	out[8] = l[8] - r[8];
	out[1] = l[1] - r[1];	out[5] = l[5] - r[5];	out[9] = l[9] - r[9];
	out[2] = l[2] - r[2];	out[6] = l[6] - r[6];	out[10] = l[10] - r[10];
	out[3] = l[3] - r[3];	out[7] = l[7] - r[7];	out[11] = l[11] - r[11];
	
	out[12] = l[12] - r[12];
	out[13] = l[13] - r[13];
	out[14] = l[14] - r[14];
	out[15] = l[15] - r[15];	
}

QMATHEXPORT_API void QMATH_MATRIX_MULTIPLY(const mat4& l, const mat4& r, mat4& out)
{
	out[0] = (l[0] * r[0]) + (l[4] * r[1]) + (l[8] * r[2]) + (l[12] * r[3]);
	out[4] = (l[0] * r[4]) + (l[4] * r[5]) + (l[8] * r[6]) + (l[12] * r[7]);
	out[8] = (l[0] * r[8]) + (l[4] * r[9]) + (l[8] * r[10]) + (l[12] * r[11]);
	out[12] = (l[0] * r[12]) + (l[4] * r[13]) + (l[8] * r[14]) + (l[12] * r[15]);

	out[1] = (l[1] * r[0]) + (l[5] * r[1]) + (l[9] * r[2]) + (l[13] * r[3]);
	out[5] = (l[1] * r[4]) + (l[5] * r[5]) + (l[9] * r[6]) + (l[13] * r[7]);
	out[9] = (l[1] * r[8]) + (l[5] * r[9]) + (l[9] * r[10]) + (l[13] * r[11]);
	out[13] = (l[1] * r[12]) + (l[5] * r[13]) + (l[9] * r[14]) + (l[13] * r[15]);

	out[2] = (l[2] * r[0]) + (l[6] * r[1]) + (l[10] * r[2]) + (l[14] * r[3]);
	out[6] = (l[2] * r[4]) + (l[6] * r[5]) + (l[10] * r[6]) + (l[14] * r[7]);
	out[10] = (l[2] * r[8]) + (l[6] * r[9]) + (l[10] * r[10]) + (l[14] * r[11]);
	out[14] = (l[2] * r[12]) + (l[6] * r[13]) + (l[10] * r[14]) + (l[14] * r[15]);

	out[3] = (l[3] * r[0]) + (l[7] * r[1]) + (l[11] * r[2]) + (l[15] * r[3]);
	out[7] = (l[3] * r[4]) + (l[7] * r[5]) + (l[11] * r[6]) + (l[15] * r[7]);
	out[11] = (l[3] * r[8]) + (l[7] * r[9]) + (l[11] * r[10]) + (l[15] * r[11]);
	out[15] = (l[3] * r[12]) + (l[7] * r[13]) + (l[11] * r[14]) + (l[15] * r[15]);	
}

QMATHEXPORT_API void QMATH_MATRIX_MULVEC(const mat4& l, const vec4f& r, vec4f& out)
{
	out.x = l[0] * r.x + l[4] * r.y + l[8] * r.z + l[12] * r.w;
	out.y = l[1] * r.x + l[5] * r.y + l[9] * r.z + l[13] * r.w;
	out.z = l[2] * r.x + l[6] * r.y + l[10] * r.z + l[14] * r.w;
	out.w = l[3] * r.x + l[7] * r.y + l[11] * r.z + l[15] * r.w; 	
}


QMATHEXPORT_API void QMATH_MATRIX_MULVEC(const mat3& l, const vec3f& r, vec3f& out)
{
	out.x = l[0] * r.x + l[3] * r.y + l[6] * r.z;
	out.y = l[1] * r.x + l[4] + r.y + l[7] * r.z;
	out.z = l[2] * r.x + l[5] * r.y + l[8] * r.z;
}

QMATHEXPORT_API float QMATH_MATRIX_DETERMINANT( const mat3& m )
{
	float aei, afh, bdi, bfg, cdh, ceg;
	aei = m[0] * m[4] * m[8];
	afh = m[0] * m[7] * m[5];
	bdi = m[3] * m[1] * m[8];
	bfg = m[3] * m[7] * m[2];
	cdh = m[6] * m[2] * m[5];
	ceg = m[6] * m[5] * m[2];
	
	return aei - afh - bdi + bfg + cdh - ceg; 			
}

QMATHEXPORT_API void  QMATH_MATRIX_DETERMINANT( const mat3& m, float& res )
{
	float aei, afh, bdi, bfg, cdh, ceg;
	aei = m[0] * m[4] * m[8];
	afh = m[0] * m[7] * m[5];
	bdi = m[3] * m[1] * m[8];
	bfg = m[3] * m[7] * m[2];
	cdh = m[6] * m[2] * m[5];
	ceg = m[6] * m[5] * m[2];
	
	res = aei - afh - bdi + bfg + cdh - ceg; 		
}


QMATHEXPORT_API void QMATH_QUATERNION_ROTATE(quat& q, const float& rad, vec3f& axis)
{
	if((axis.x != 0 && axis.x != 1) || (axis.y != 0 && axis.y != 1) || (axis.z != 0 && axis.z != 1))
		axis.normalize();
	
	float s = (float)sin(rad / 2.0f);
	
	vec3f tm(axis.x * s, axis.y * s, axis.z * s);
	float w = (float)cos(rad / 2.0F);
	float len = 1.0F / (float)sqrt(tm.x * tm.x + tm.y * tm.y + tm.z * tm.z + w * w);
	tm.x *= len;
	tm.y *= len;
	tm.z *= len;
	q.set(tm.x, tm.y, tm.z, w);
		
}

QMATHEXPORT_API float QMATH_QUATERNION_GETANGLE(const quat& q)
{
	return 2.0 * acosf(q.w);
}

QMATHEXPORT_API void QMATH_QUATERNION_MAKEMATRIX(mat4& m, const quat& q)
{
	m[0] = 1.0f - 2.0f * (q.y * q.y + q.z * q.z);
	m[1] = 2.0f * (q.x * q.y + q.z * q.w);
	m[2] = 2.0f * (q.x * q.z - q.y * q.w);
	m[3] = 0.0f;
	
	m[4] = 2.0f * (q.x * q.y - q.z * q.w);
	m[5] = 1.0f - 2.0f * (q.x * q.x + q.z * q.z);
	m[6] = 2.0f * (q.z * q.y + q.x * q.w);
	m[7] = 0.0f;
	
	m[8] = 2.0f * (q.x * q.z + q.y * q.w);
	m[9] = 2.0f * (q.y * q.z - q.x * q.w);
	m[10] = 1.0f - 2.0f * (q.x * q.x + q.y * q.y);
	m[11] = 0.0f;
	
	m[12] = 0.0f;
	m[13] = 0.0f;
	m[14] = 0.0f;
	m[15] = 1.0f;	
}


QMATHEXPORT_API void QMATH_QUATERNION_MULQUAT(const quat& l, const quat& r, quat& out)
{
	out.x = l.w * r.x + l.x * r.w + l.y * r.z - l.z * r.y;
	out.y = l.w * r.y - l.x * r.z + l.y * r.w + l.z * r.x;
	out.z = l.w * r.z + l.x * r.y - l.y * r.x + l.z * r.w;
	out.w = l.w * r.w - l.x * r.x - l.y * r.y - l.z * r.z;	
}

QMATHEXPORT_API void QMATH_QUATERNION_CONJUGATE(quat& q)
{
	q.x = -q.x; q.y = -q.y; q.z = -q.z; q.w = q.w;	
}



QMATHEXPORT_API void QMATH_CREATE_VERTEX_NORMALS(const vec3f* verts, const unsigned int& nVerts, const unsigned int* polys, const unsigned int& nPolys, float* norms)
{
	unsigned int* tmp;
	vec3f a, b, c, tmpNorm;
	vec3f tmpList[64];
	int counter;
	
	// iterate through every vertex //
	for(unsigned int j = 0; j < nVerts; ++j)
	{
		counter = 0;
		
		// check every poly for this vert //
		for(unsigned int v = 0; v < nPolys; ++v)
		{
			tmp = (unsigned int*)&polys[v * 3];
			if(*tmp != j)
			{
				++tmp;
				if(*tmp != j)
				{
					++tmp;
					if(*tmp != j)
						continue;
				}
			}
			
			// if we're here, this vert is in the tri, so fetch verts //
			unsigned int i1, i2, i3;
			i1 = polys[v * 3];
			i2 = polys[v * 3 + 1];
			i3 = polys[v * 3 + 2];
			
			a.x = verts[i1].x; 
			a.y = verts[i1].y;
			a.z = verts[i1].z;
			b.x = verts[i2].x;
			b.y = verts[i2].y;
			b.z = verts[i2].z;
			c.x = verts[i3].x;
			c.y = verts[i3].y;
			c.z = verts[i3].z;
			
			vec3f ab, ac;
			ac = c - a;
			ab = b - a;
			tmpNorm = QMATH_VEC3F_CROSSPROD(ab, ac);
//			tmpNorm = QMATH_VEC3F_CROSSPROD(ac, ab);
			QMATH_VEC3F_NORMALIZE(tmpNorm);

			// check through the buffer list for this vert and look for dupes //
			for(int z = 0; z < counter; ++z)
			{
				if(tmpNorm == tmpList[z])
					goto end;
			}
			
			// add to buffer list //
			tmpList[counter].x = tmpNorm.x;
			tmpList[counter].y = tmpNorm.y;
			tmpList[counter].z = tmpNorm.z;
			
			++counter;
			end:;
		}
		
		tmpNorm.x = 0.0F;
		tmpNorm.y = 0.0F;
		tmpNorm.z = 0.0F;
		for(int q = 0; q < counter; ++q)
		{
			tmpNorm.x += tmpList[q].x;
			tmpNorm.y += tmpList[q].y;
			tmpNorm.z += tmpList[q].z;
		}
		
		// copy to norm list //
		QMATH_VEC3F_NORMALIZE(tmpNorm);
		norms[j * 3] = tmpNorm.x;
		norms[j * 3 + 1] = tmpNorm.y;
		norms[j * 3 + 2] = tmpNorm.z;
	}
}


struct tanIndex
{
	vec3f tan;
	int vertRef;
};


QMATHEXPORT_API void QMATH_CREATE_TANGENT_SPACE(const vec3f* verts, const unsigned int& nVerts, const unsigned int* polys, const unsigned int& nPolys, const vec2f* texcoords, const vec3f* norms, vec3f* tangent)
{
	tanIndex* tan1 = static_cast<tanIndex*>(calloc(nPolys * 3, sizeof(tanIndex)));
	
	if(!tan1)
		return;
	
	for(unsigned int i = 0; i < nPolys; ++i)
	{
		unsigned int i1 = polys[i * 3];
		unsigned int i2 = polys[i * 3 + 1];
		unsigned int i3 = polys[i * 3 + 2];
		
		tan1[i * 3].vertRef = i1;
		tan1[i * 3 + 1].vertRef = i2;
		tan1[i * 3 + 2].vertRef = i3;
	
		vec3f a, b, c;
		QMATH_VEC3F_COPY(a, verts[i1]);
		QMATH_VEC3F_COPY(b, verts[i2]);
		QMATH_VEC3F_COPY(c, verts[i3]);

		vec2f ta, tb, tc;
		QMATH_VEC2F_COPY(ta, texcoords[i1]);
		QMATH_VEC2F_COPY(tb, texcoords[i2]);
		QMATH_VEC2F_COPY(tc, texcoords[i3]);

		float x1 = b.x - a.x;
		float x2 = c.x - a.x;
		float y1 = b.y - a.y;
		float y2 = c.y - a.y;
		float z1 = b.z - a.z;
		float z2 = c.z - a.z;
		
		float s1 = tb.x - ta.x;
		float s2 = tc.x - ta.x;
		float t1 = tb.y - ta.y;
		float t2 = tc.y - ta.y;
		
		float r = 1.0F / (s1 * t2 - s2 * t1);
		vec3f sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);
		vec3f tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);

		tan1[i * 3].tan = tan1[i * 3].tan + sdir;
		tan1[i * 3 + 1].tan = tan1[i * 3 + 1].tan + sdir;
		tan1[i * 3 + 2].tan = tan1[i * 3 + 2].tan + sdir;
		
		vec3f n, t, tan;
		
		for(unsigned int j = 0; j < 3; ++j )
		{
			QMATH_VEC3F_COPY(n, norms[tan1[i * 3 + j].vertRef]);
			QMATH_VEC3F_COPY(t, tan1[i * 3 + j].tan);
			
			float dp3 = QMATH_VEC3F_DOTPROD(n, t);
			n = n * dp3;
			tan = t - n;
			QMATH_VEC3F_NORMALIZE(tan);
			QMATH_VEC3F_COPY(tangent[tan1[i * 3 + j].vertRef], tan);
		}
	}
	
	if(tan1)
	{
		free(tan1);
		tan1 = NULL;
	}	
}


QMATHEXPORT_API void QMATH_GET_TEXTURE_COORDINATES(const cTextureDims& srcDims, const RECT* src, const cTextureDims& destDims, const RECT* dest, cTextureRect* pCoords)
{
   float tU, tV;
	
	// Setup default values //
   pCoords->leftU = 0.0F;
   pCoords->topV = 0.0F;
   pCoords->rightU = 1.0F; 
   pCoords->bottomV = 1.0F;

	// If source will be rendered to then we need to adjust sample coordinates //
   if(src)
   {	
	  // texel sample locations in pixel space //
      tU = 1.0F / srcDims.width;
      tV = 1.0F / srcDims.height;
  
		// adjust address space accordingly in U and V //
      pCoords->leftU += src->left * tU;
      pCoords->topV += src->top * tV;
      pCoords->rightU -= (srcDims.width - src->right) * tU;
      pCoords->bottomV -= (srcDims.height - src->bottom) * tV;
   }
   

	// If destination will be rendered to then we need to adjust the sample coordinates further //
   if(dest)
   {
	  // texel sample locations in pixel space //
      tU = 1.0F / destDims.width;
      tV = 1.0F / destDims.height;
  
	  // adjust address space in U and V //
      pCoords->leftU -= dest->left * tU;
      pCoords->topV -= dest->top * tV;
      pCoords->rightU += (destDims.width - dest->right ) * tU;
      pCoords->bottomV += (destDims.height - dest->bottom) * tV;
   }	
}

QMATHEXPORT_API void QMATH_GET_TEXTURE_RECT(const cTextureDims& dims, RECT* rect)
{
	if(!rect)
		return;
		
	int width = (int)dims.width;
	int height = (int)dims.height;

	rect->left = 0.0F;
	rect->top = 0.0F;
	rect->right = width;
	rect->bottom = height;	
}


QMATHEXPORT_API float QMATH_GET_GAUSSIAN_DISTRIBUTION(const float& x, const float& y, const float& rho)
{
	float g = 1.0F / sqrtf(2.0F * QMATH_PI * rho * rho);
	g *= expf(-(x * x + y * y) / (2 * rho * rho));
	return g;	
}

QMATHEXPORT_API void QMATH_GET_GAUSSIAN5X5_OFFSETS(const unsigned int& width, const unsigned int& height, vec2f* offsets, vec4f* weights, const float& mul)
{
	float tu = 1.0F / (float)width;
	float tv = 1.0F / (float)height;
	
	vec4f white(1.0F, 1.0F, 1.0F, 1.0F);
	float totalWeight = 0.0F;
	int index = 0;
	
	for(int x = -2; x <= 2; ++x)
	{
		for(int y = -2; y <= 2; ++y)
		{
			if(abs(x) + abs(y) > 2)
				continue;
				
			offsets[index].x = x * tu;
			offsets[index].y = y * tv;
			weights[index].x = white.x * QMATH_GET_GAUSSIAN_DISTRIBUTION((float)x, (float)y, 1.0F);
			weights[index].y = white.y * QMATH_GET_GAUSSIAN_DISTRIBUTION((float)x, (float)y, 1.0F);
			weights[index].z = white.z * QMATH_GET_GAUSSIAN_DISTRIBUTION((float)x, (float)y, 1.0F);
			weights[index].w = white.w * QMATH_GET_GAUSSIAN_DISTRIBUTION((float)x, (float)y, 1.0F);
			totalWeight += weights[index].x;
			++index;
		}
	}
	
	for(int i = 0; i < index; ++i)
	{
		weights[i].x /= totalWeight; weights[i].x *= mul;
		weights[i].y /= totalWeight; weights[i].y *= mul;
		weights[i].z /= totalWeight; weights[i].z *= mul;
		weights[i].w /= totalWeight; weights[i].w *= mul;
	}	
}

QMATHEXPORT_API void QMATH_GET_BILATERAL_OFFSETS(const int& bbWidth, const int& bbHeight, vec2f* avSampleOffsets)
{
	if(!avSampleOffsets)
		return;
	
	float tu = 1.0F / (float)bbWidth;
	float tv = 1.0F / (float)bbHeight;
	
	int index = 0;
	for(int x = -2; x <= 2; ++x)
	{
		for(int y = -2; y <= 2; ++y)
		{
			if(abs(x) + abs(y) > 2)
				continue;
			
			avSampleOffsets[index].x = x * tu;
			avSampleOffsets[index].y = y * tv;
			++index;
		}
	}
}

QMATHEXPORT_API void QMATH_GET_GAUSSIAN10X10_OFFSETS(const unsigned int& width, const unsigned int& height, vec2f* offsets, vec4f* weights, const float& mul)
{
	float tu = 1.0F / (float)width;
	float tv = 1.0F / (float)height;
	
	vec4f white(1.0, 1.0, 1.0, 1.0);
	float totalWeight = 0.0F;
	int index = 0;
	
	for(int x = -5; x <= 5; ++x)
	{
		for(int y = -5; y <= 5; ++y)
		{
			if(abs(x) + abs(y) > 5)
				continue;
			
			offsets[index].x = x * tu;
			offsets[index].y = y * tv;
			weights[index].x = white.x * QMATH_GET_GAUSSIAN_DISTRIBUTION((float)x, (float)y, 1.0F);
			weights[index].y = white.y * QMATH_GET_GAUSSIAN_DISTRIBUTION((float)x, (float)y, 1.0F);
			weights[index].z = white.z * QMATH_GET_GAUSSIAN_DISTRIBUTION((float)x, (float)y, 1.0F);
			weights[index].w = white.w * QMATH_GET_GAUSSIAN_DISTRIBUTION((float)x, (float)y, 1.0F);
			totalWeight += weights[index].x;
			++index;
		}
	}
	
	for(int i = 0; i < index; ++i)
	{
		weights[i].x /= totalWeight; weights[i].x *= mul;
		weights[i].y /= totalWeight; weights[i].y *= mul;
		weights[i].z /= totalWeight; weights[i].z *= mul;
		weights[i].w /= totalWeight; weights[i].w *= mul;
	}
}


QMATHEXPORT_API void QMATH_GET_BLOOM_OFFSETS(const int& texSize, float texCoordOffset[15], vec4f* colorWeight, const float& dev, const float& mul)
{
	int i = 0;
	
	// obtain horizontal sampling interval //
	float tu = 1.0F / (float)texSize;		
	
	// get a weight for sampling center //
	float weight = mul * QMATH_GET_GAUSSIAN_DISTRIBUTION(0.0F, 0.0F, dev);
	colorWeight[0].x = weight;
	colorWeight[0].y = weight;
	colorWeight[0].z = weight;
	colorWeight[0].w = 1.0F;

	texCoordOffset[0] = 0.0F;

	// for the 8 closest sampling neighbors obtain an average color weight //
	for(i = 1; i < 8; ++i)
	{
		weight = mul * QMATH_GET_GAUSSIAN_DISTRIBUTION((float)i, 0.0F, dev);
		texCoordOffset[i] = i * tu;
		colorWeight[i].x = weight;
		colorWeight[i].y = weight;
		colorWeight[i].z = weight;
		colorWeight[i].w = 1.0F;
	}

	// Interpolate the other weights and texture offsets //
	for(i = 8; i < 15; ++i)
	{
		colorWeight[i].set( colorWeight[i - 7] );
		texCoordOffset[i] = -(texCoordOffset[i - 7]);
	}	
}

QMATHEXPORT_API void QMATH_GET_SAMPLE4X4_OFFSETS(const int& bbWidth, const int& bbHeight, vec2f* avSampleOffsets)
{
	if(!avSampleOffsets)
		return;
	
	float tu = 1.0F / bbWidth;
	float tv = 1.0F / bbHeight;
	
	int index = 0;
	for(int y = 0; y < 4; ++y)
	{
		for(int x = 0; x < 4; ++x)
		{
			avSampleOffsets[index].x = (x - 1.5F) * tu;
			avSampleOffsets[index].y = (y - 1.5F) * tv;
			++index;
		}
	}	
}

QMATHEXPORT_API void QMATH_GET_SAMPLE2X2_OFFSETS(const int& bbWidth, const int& bbHeight, vec2f* avSampleOffsets)
{
	if(!avSampleOffsets)
		return;

	// Obtain step size in u and v
	float tu = 1.0F / bbWidth;
	float tv = 1.0F / bbHeight;

	int index = 0;
	
	// Subdivide by 2 in U and V 
	for(int y = 0; y < 2; ++y)
	{
		for(int x = 0; x < 2; ++x)
		{
			// adjust new sample offsets //
			avSampleOffsets[index].x = (x - 0.5F) * tu;
			avSampleOffsets[index].y = (y - 0.5F) * tu;
			++index;
		}
	}
}


QMATHEXPORT_API void QMATH_GET_SAMPLE3X3_OFFSETS(const int& bbWidth, const int& bbHeight, vec2f* avSampleOffsets)
{
	if(!avSampleOffsets)
		return;
	
	float tu = 1.0F / bbWidth;
	float tv = 1.0F / bbHeight;
	
	int index = 0;
	for(int y = 0; y < 3; ++y)
	{
		for(int x = 0; x < 3; ++x)
		{
			avSampleOffsets[index].x = (x - 1.0F) * tu;
			avSampleOffsets[index].y = (y - 1.0F) * tv;
			++index;	
		}
	}	
}


QMATHEXPORT_API float QMATH_POINT_ROTATEZ(const vec3f& p, const mat4& m)
{
	return (p.x * m[2]) + (p.y * m[6]) + (p.z * m[10]);
}

QMATHEXPORT_API void QMATH_LERP_FLOAT(const float* x, const float* y, const float& dv, float* out)
{
	if(!out)
		return;
	
	out[0] = x[0] + dv * (y[0] - x[0]);
	out[1] = x[1] + dv * (y[1] - x[1]);
	out[2] = x[2] + dv * (y[2] - x[2]);
}

QMATHEXPORT_API void QMATH_LERP_FLOAT(const float x, const float y, const float& dv, float& out)
{
	out = x + dv * (y - x);
}

QMATHEXPORT_API void QMATH_SMOOTHSTEP_FLOAT(const float min, const float max, const float& dv, float& out)
{
	if(dv < min)
	{
		out = 0.0F;
		return;
	}
	if(dv >= max)
	{
		out = 1.0F;
		return;
	}
	
	out = -2.0F * pow((dv - min) / (max - min), 3.0F) + 3.0F  * pow((dv - min) / (max - min), 2.0F);
}


QMATHEXPORT_API bool QMATH_POINT_IN_SPHERE(const vec3f& test, const vec3f& sphereOrigin, const float& sphereRad)
{
	vec3f sphereToPoint = test - sphereOrigin;
	return (sphereToPoint.getLength() <= sphereRad);
}

QMATHEXPORT_API vec2f QMATH_GET_FRUSTUM_DIMENSIONS(const float& fov, const float& dist, const float& aspect)
{
	vec2f r;
	r.y = 2.0F * tanf(fov * 0.5F) * dist;
	r.x = r.y * aspect;
	return r;
}


QMATHEXPORT_API void  QMATH_GET_FRUSTUM_PLANES(const mat4& MVP, vec4f* planes)
{
	planes[0].x = MVP[3] + MVP[0];
	planes[0].y = MVP[7] + MVP[4];
	planes[0].z = MVP[11] + MVP[8];
	planes[0].w = MVP[15] + MVP[12];

	// figure right clip plane //
	planes[1].x = MVP[3] - MVP[0];
	planes[1].y = MVP[7] - MVP[4];
	planes[1].z = MVP[11] - MVP[8];
	planes[1].w = MVP[15] - MVP[12];

	// figure top clip plane //
	planes[2].x = MVP[3] - MVP[1];
	planes[2].y = MVP[7] - MVP[5];
	planes[2].z = MVP[11] - MVP[9];
	planes[2].w = MVP[15] - MVP[13];

	// figure bottom clip plane //
	planes[3].x = MVP[3] + MVP[1];
	planes[3].y = MVP[7] + MVP[5];
	planes[3].z = MVP[11] + MVP[9];
	planes[3].w = MVP[15] + MVP[13];

	// figure near clip plane //
	planes[4].x = MVP[2];  
	planes[4].y = MVP[6];
	planes[4].z = MVP[10];
	planes[4].w = MVP[14];

	// figure far clip plane //
	planes[5].x = MVP[3] - MVP[2];
	planes[5].y = MVP[7] - MVP[6];
	planes[5].z = MVP[11] - MVP[10];
	planes[5].w = MVP[15] - MVP[14];	
}

QMATHEXPORT_API bool QMATH_POINT_IN_FRUSTUM(const vec4f* planes, vec3f& point)
{
	for(int i = 0; i < 6; ++i)
	{
		vec3f pn(planes[i].x, planes[i].y, planes[i].z);
		if(point.dotProd(pn) + planes[i].w < 0.0F)
			return false;
	}	
	
	return true;
}