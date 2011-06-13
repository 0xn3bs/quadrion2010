#ifndef __QGEOM_H_
#define __QGEOM_H_


#include "qrender.h"


#ifdef QRENDER_EXPORTS
	#define QGEOMEXPORT_API __declspec(dllexport)
#else
	#define QGEOMEXPORT_API __declspec(dllimport)
#endif







/////////////////////////////////////////////////////////////////////////////////////
//
// CRay
//
// Ray object which makes easier some of the more mundane and
// common ray/object, ray/ray, and ray casted collision detection
//
/////////////////////////////////////////////////////////////////////////////////////
class QGEOMEXPORT_API CRay
{
	public:
	
		CRay();
		CRay(vec3f vertex, vec3f direction);
		~CRay();
		
		// Set/GetVertex
		// Sets and gets the vertex of the ray
		void			SetVertex(vec3f v);
		inline vec3f	GetVertex() const { return m_vertex; }
		
		// Set/GetDirection
		// Sets and gets the direction of the ray
		void			SetDirection(vec3f d);
		inline vec3f	GetDirection() const { return m_dir; }
		
		
		// GetRayIntersection
		// Obtains the nearest point of the intersection between two rays
		// return value
		//		bool: false if the rays do not intersect, true if they do
		// params
		//		ray: the ray to test against
		//		intersection (output): the point of intersection where the result will be stored
		bool	GetRayIntersection(CRay ray, point3f& intersection);
		
	
	private:
		
		vec3f		m_vertex;		// Vertex of the ray
		vec3f		m_dir;			// Normalized direction of the ray
};





/////////////////////////////////////////////////////////////////////////////////////////////
// 
// CTriangle
// 
// Encapsulates a triangle type and facilitates finding the centroid among other things
// such as collision with rays, cones, and the frustum
//
/////////////////////////////////////////////////////////////////////////////////////////////
class QGEOMEXPORT_API CTriangle
{
	public:
	
		CTriangle();
		CTriangle(vec3f a, vec3f b, vec3f c);
		~CTriangle();
		
		// Get/SetTriangle
		// Sets and Gets the triangle vertices
		void		SetTriangle(vec3f a, vec3f b, vec3f c);
		
		// GetCentroid
		// Obtains the centroid point of the triangle and returns it
		// return-
		//		vec3f- centroid of the triangle
		point3f		GetCentroid();
	
	private:
	
		void		SetupTriangle(vec3f a, vec3f b, vec3f c);
	
		vec3f		m_a, m_b, m_c;		// The three points that make up the triangle
		CRay		m_AB, m_AC;			// The sides AB and AC
		CRay		m_BA, m_BC;			// The sides BA and BC
		CRay		m_CA, m_CB;			// The sides CA and CB
		
		float		m_lenSideA;			// Length of side across from vertex A
		float		m_lenSideB;			// Length of side across from vertex B
		float		m_lenSideC;			// Length of side across from vertex C
		
		vec3f		m_midSideA;			// Midpoint of the side across from vertex A
		vec3f		m_midSideB;			// Midpoint of the side across from vertex B
		vec3f		m_midSideC;			// Midpoint of the side across from vertex C
};





/////////////////////////////////////////////////////////////////////
//
// CCone
//
// Cone structure for determining linear and spherical collision
// with cone shapes. Can be used by camera and lights to determine
// bounds and collision w/ entities and the map or camera 
//
////////////////////////////////////////////////////////////////////
class QMATHEXPORT_API CCone
{
	public:
	
		CCone();
		
		// Constructor
		// params:
		//		vertex-	Cone vertex position. 
		//		direction- Direction the cone looks
		//		theta- angle of the cone fan in radians
		CCone(vec3f vertex, vec3f direction, float theta);
		~CCone();
		
		// Set/GetTheta
		// Set the cone angle in radians
		// Get the cone angle in radians
		void			SetTheta(float theta);
		inline float	GetTheta() const { return m_theta; }
		
		// Set/GetVertex
		// Set the cone vertex position
		// Get the cone vertex position
		void			SetVertex(vec3f vertex);
		inline vec3f	GetVertex() const { return m_vertex; }
		
		// Set/GetDirection
		// Set the cone direction vector
		// Get the cone direction vector
		void			SetDirection(vec3f dir);
		inline vec3f	GetDirection() const { return m_dir; }
		
		// CollidesWithSphere
		// Returns the intersection result of the collision of this cone with a sphere
		// params:
		//		sphereCtr- Position of the center of the sphere
		//		sphereRad- Radius of the collision sphere
		QMATH_INTERSECT_RESULT	IsSphereInCone(vec3f sphereCtr, float sphereRad);
		
	
	private:
	
		vec3f	m_vertex;		// Vertex of the cone
		vec3f	m_dir;			// Normalized direction of the cone
		float	m_theta;		// Angle of the cone in radians
		
		float	m_sinTheta;			// Precomputed sine of theta
		float	m_recipSinTheta;	// Precomputed 1.0F / m_sinTheta
		float	m_cosTheta;			// Precomputed cosine of theta
		float	m_cosSqr;			// m_cosTheta squared
		float	m_sinSqr;			// m_sinTheta squared
		
};



struct CPolyTexture
{
	int				texHandle;      // Texture handle
	std::string		texDesc;		// Texture description
};



class QGEOMEXPORT_API CBufferedPoly
{
	public:
	
		CBufferedPoly();
		CBufferedPoly( const CBufferedPoly& cpy );
		~CBufferedPoly();
		
		
		void		SetVertexRange( int minRange, int maxRange );
		void		SetVertexBuffer( int handle );
		
		void		SetIndexRange( int minRange, int maxRange );
		void		SetIndexBuffer( int handle );
		
		void		SetBoundingBox( vec3f min, vec3f max );
		
		
		void		SetPrimitiveType( unsigned int primType );
		
		void		SetMinIndex( int minIndex );
		
		void		SetTexture( int handle, std::string desc );
		
		void		GetBoundingBox( vec3f& mins, vec3f& maxs );
		
		vec3f		GetCenter( )
		{
			return vec3f( m_minVertex + m_maxVertex ) * 0.5;
		}
		
		int			GetVertexBufferHandle() { return m_vboHandle; }
		int			GetIndexBufferHandle() { return m_iboHandle; }

		
		
		// Private //
		int			m_nRequiredDrawCalls;
		
		int			m_vertexRange[2];
		int			m_indexRange[2];
		
		int			m_vboHandle;
		int			m_iboHandle;
		
		int			m_nIndices;
		int			m_nVertices;
		int			m_minIndex;
		int			m_indexOffset;
		
		vec3f		m_minVertex;
		vec3f		m_maxVertex;
		
		unsigned int		m_primType;
		unsigned int		m_vertexStreams[4];
		unsigned int		m_nVertexStreams;
		
		
		CPolyTexture		m_texture[4];
};



#endif