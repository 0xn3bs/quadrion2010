#include "qgeom.h"




//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// RAY METHODS 
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

CRay::CRay()
{
	
}

CRay::CRay(vec3f vertex, vec3f direction)
{
	m_vertex = vertex;
	m_dir = direction;
	m_dir.normalize();
}

CRay::~CRay()
{
	
}

void CRay::SetVertex(vec3f v)
{
	m_vertex = v;
}

void CRay::SetDirection(vec3f d)
{
	m_dir = d;
	m_dir.normalize();
}

bool CRay::GetRayIntersection(CRay ray, point3f& intersection)
{
	vec3f U = ray.GetVertex() - m_vertex;
	vec3f rDir = ray.GetDirection();
	vec3f W = m_dir.crossProd(rDir);
	float lenW = W.getLength();
	float lenWSqr = lenW * lenW;
	if(lenWSqr <= 0.000001F)
		return false;
	
	mat4 dt0;
	dt0[0] = U.x;		dt0[4] = U.y;		dt0[8] = U.z;			dt0[12] = 0.0F;
	dt0[1] = rDir.x;	dt0[5] = rDir.y;	dt0[9] = rDir.z;		dt0[13] = 0.0F;
	dt0[2] = W.x;		dt0[6] = W.y;		dt0[10] = W.z;			dt0[14] = 0.0F;
	dt0[3] = 0.0F;		dt0[7] = 0.0F;		dt0[11] = 0.0F;			dt0[15] = 1.0F;
	
	
	mat4 dt1;		
	dt1[0] = U.x;		dt1[4] = U.y;		dt1[8] = U.z;			dt1[12] = 0.0F;
	dt1[1] = m_dir.x;	dt1[5] = m_dir.y;	dt1[9] = m_dir.z;		dt1[13] = 0.0F;
	dt1[2] = W.x;		dt1[6] = W.y;		dt1[10] = W.z;			dt1[14] = 0.0F;
	dt1[3] = 0.0F;		dt1[7] = 0.0F;		dt1[11] = 0.0F;			dt1[15] = 1.0F;

	float detDT0 = D3DXMatrixDeterminant(&D3DXMATRIX(dt0));
	float detDT1 = D3DXMatrixDeterminant(&D3DXMATRIX(dt1));
	
	float t0 = detDT0 / lenWSqr;
	float t1 = detDT1 / lenWSqr;
	
	point3f ta = m_vertex + (m_dir * t0);
	point3f tb = ray.GetVertex() + (rDir * t1);
	ta.x = -ta.x;
	tb.x = -tb.x;
		
	intersection.set((ta.x + tb.x) * 0.5F, (ta.y + tb.y) * 0.5F, (ta.z + tb.z) * 0.5F);
	
	return true;
}






//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// CONE METHODS
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=


CCone::CCone()
{
	// Initialize to 45 degs //
	m_theta = QMATH_PI * 0.25F;
	m_sinTheta = sinf(m_theta);
	m_cosTheta = 1.0F - m_sinTheta;
	m_recipSinTheta = 1.0F / m_sinTheta;
	m_sinSqr = m_sinTheta * m_sinTheta;
	m_cosSqr = m_cosTheta * m_cosTheta;
}

CCone::CCone(vec3f vertex, vec3f direction, float theta)
{
	m_vertex = vertex;
	m_dir = direction;
	m_dir.normalize();
	m_theta = theta;
	
	m_sinTheta = sinf(theta);
	m_cosTheta = 1.0F - m_sinTheta;
	m_recipSinTheta = 1.0F / m_sinTheta;
	m_sinSqr = m_sinTheta * m_sinTheta;
	m_cosSqr = m_cosTheta * m_cosTheta;
}


CCone::~CCone()
{
	
}

void CCone::SetTheta(float theta)
{
	m_theta = theta;
	
	m_sinTheta = sinf(theta);
	m_cosTheta = 1.0F - m_sinTheta;
	m_recipSinTheta = 1.0F / m_sinTheta;
	m_sinSqr = m_sinTheta * m_sinTheta;
	m_cosSqr = m_cosTheta * m_cosTheta;
}

void CCone::SetVertex(vec3f vertex)
{
	m_vertex = vertex;
}

void CCone::SetDirection(vec3f dir)
{
	m_dir = dir;
	m_dir.normalize();	
}

QMATH_INTERSECT_RESULT CCone::IsSphereInCone(vec3f sphereCtr, float sphereRad)
{
	vec3f U = m_vertex - m_dir * (sphereRad * m_recipSinTheta);	
	vec3f D = sphereCtr - U;
	float dsqr = D.dotProd(D);
	float e = m_dir.dotProd(D);
	
	// First check that sphere is within the outer cone //
	if((e > 0.0F) && ((e * e) >= dsqr * m_cosSqr))
	{
		D = sphereCtr - m_vertex;
		dsqr = D.dotProd(D);
		e = -m_dir.dotProd(D);
		
		// Then check that it is within the inner cone //
		// If so then it is fully inside //
		if((e > 0.0F) && ((e * e) >= dsqr * m_sinSqr))
			return QMATH_INSIDE;
		
		
		// Otherwise it intersects the inner cone //
		else
			return QMATH_INTERSECT;
	}
	
	// We fully outside //
	else
		return QMATH_OUTSIDE;
}





//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// CTRIANGLE METHODS 
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
CTriangle::CTriangle()
{

}

CTriangle::~CTriangle()
{
	
}

CTriangle::CTriangle(vec3f a, vec3f b, vec3f c)
{
	SetupTriangle(a, b, c);
}


void CTriangle::SetTriangle(vec3f a, vec3f b, vec3f c)
{
	SetupTriangle(a, b, c);
}


// TODO: Verify integrity of this function //
point3f CTriangle::GetCentroid()
{
	// Get two rays from 2 vertices to the midpoints of their respective sides //
	CRay A, B;
	vec3f vA = m_midSideA - m_a;
	vec3f vB = m_midSideB - m_b;
	
	A.SetDirection(vA);
	B.SetDirection(vB);
	A.SetVertex(m_a);
	B.SetVertex(m_b);
	
	point3f centroid;
	A.GetRayIntersection(B, centroid);
	
	return centroid;
}


void CTriangle::SetupTriangle(vec3f a, vec3f b, vec3f c)
{
	m_a = a;
	m_b = b;
	m_c = c;
	
	vec3f ab = b - a;
	vec3f ac = c - a;
	vec3f bc = c - b;
	
	m_AB.SetDirection(ab);
	m_AB.SetVertex(m_a);
	m_AC.SetDirection(ac);
	m_AC.SetVertex(m_a);
	m_BA.SetDirection(-m_AB.GetDirection());
	m_BA.SetVertex(m_b);
	m_BC.SetDirection(bc);
	m_BC.SetVertex(m_b);
	m_CA.SetDirection(-m_AC.GetDirection());
	m_CA.SetVertex(m_c);
	m_CB.SetDirection(-m_BC.GetDirection());
	m_CB.SetVertex(m_c);


	m_lenSideA = bc.getLength();
	m_lenSideB = ac.getLength();
	m_lenSideC = ab.getLength();
	
	m_midSideA = m_b + (m_BC.GetDirection() * (m_lenSideA * 0.5F));
	m_midSideB = m_a + (m_AC.GetDirection() * (m_lenSideB * 0.5F));
	m_midSideC = m_a + (m_AB.GetDirection() * (m_lenSideC * 0.5F));	
}










CBufferedPoly::CBufferedPoly()
{
	memset( m_vertexRange, -1, sizeof(int) * 2 );
	memset( m_indexRange, -1, sizeof(int) * 2 );
	
	m_vboHandle = QRENDER_INVALID_HANDLE;
	m_iboHandle = QRENDER_INVALID_HANDLE;
	
	m_nIndices = 0;
	m_nVertices = 0;
	m_minIndex = 0;
	m_primType = 0;
	m_indexOffset = 0;
	
	for( int i = 0; i < 4; ++i )
	{	
		m_texture[i].texHandle = QRENDER_INVALID_HANDLE;
		m_texture[i].texDesc = "";
	}
	
	memset( m_vertexStreams, 0, sizeof( unsigned int ) * 4 );
	m_nVertexStreams = 0;
	m_nRequiredDrawCalls = 1;
}

CBufferedPoly::~CBufferedPoly()
{
	
}

CBufferedPoly::CBufferedPoly( const CBufferedPoly& cpy )
{
	m_vertexRange[0] = cpy.m_vertexRange[0];
	m_vertexRange[1] = cpy.m_vertexRange[1];
	
	m_indexRange[0] = cpy.m_indexRange[0];
	m_indexRange[1] = cpy.m_indexRange[1];
	
	m_vboHandle = cpy.m_vboHandle;
	m_iboHandle = cpy.m_iboHandle;
	
	m_nIndices = cpy.m_nIndices;
	m_nVertices = cpy.m_nVertices;
	
	m_minVertex = cpy.m_minVertex;
	m_maxVertex = cpy.m_maxVertex;
	
	m_minIndex = cpy.m_minIndex;
	m_primType = cpy.m_primType;
	
	for( int i = 0; i < 4; ++i )
	{
		m_texture[i].texDesc = cpy.m_texture[i].texDesc;
		m_texture[i].texHandle = cpy.m_texture[i].texHandle;
	}
	
	memcpy( m_vertexStreams, cpy.m_vertexStreams, sizeof( unsigned int ) * 4 );
	m_nVertexStreams = cpy.m_nVertexStreams;
	m_indexOffset = cpy.m_indexOffset;
	m_nRequiredDrawCalls = cpy.m_nRequiredDrawCalls;
}


void CBufferedPoly::SetVertexBuffer( int handle )
{
	if( QRENDER_IS_VALID( handle ) )
		m_vboHandle = handle;
}

void CBufferedPoly::SetIndexBuffer( int handle )
{
	if( QRENDER_IS_VALID( handle ) )
		m_iboHandle = handle;
}


void CBufferedPoly::SetVertexRange( int minRange, int maxRange )
{
	if( minRange < 0 )
		return;	
		
	m_vertexRange[0] = minRange;
	m_vertexRange[1] = maxRange;
	
	m_nVertices = maxRange - minRange;
}


void CBufferedPoly::SetIndexRange( int minRange, int maxRange )
{
	if( minRange < 0 )
		return;
	
	m_indexRange[0] = minRange;
	m_indexRange[1] = maxRange;
	
	m_nIndices = maxRange - minRange;
}


void CBufferedPoly::SetPrimitiveType( unsigned int primType )
{
	m_primType = primType;
}


void CBufferedPoly::SetMinIndex( int minIndex )
{
	m_minIndex = minIndex;
}


void CBufferedPoly::SetBoundingBox( vec3f min, vec3f max )
{
	m_minVertex = min;
	m_maxVertex = max;
}


void CBufferedPoly::SetTexture( int handle, std::string desc )
{
	for( int i = 0; i < 4; ++i )
	{
		if( !QRENDER_IS_VALID(m_texture[i].texHandle) )
		{
			m_texture[i].texHandle = handle;
			m_texture[i].texDesc = desc;
		}
	}	
}


void CBufferedPoly::GetBoundingBox( vec3f& mins, vec3f& maxs )
{
	mins = m_minVertex;
	maxs = m_maxVertex;
}