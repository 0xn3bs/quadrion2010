//////////////////////////////////////////////////////////////////////////////////
//
// CAMERA.H
//
// Written by Shawn Simonson for Quadrion Engine 11/2005
//
// Provides facility for basic free-roaming camera object. 
// Cameras support either Orthographic or Perspective projection. You will
// also find facility for frustum plane extraction herein.
//
//////////////////////////////////////////////////////////////////////////////////

class CCamera;

#include "qmath.h"
#include "qrender.h"
#include "qgeom.h"

#include "qscriptable.h"
class qscriptengine;

#ifdef QRENDER_EXPORTS
	#define QCAMERAEXPORT_API		__declspec(dllexport)
#else
	#define QCAMERAEXPORT_API		__declspec(dllimport)
#endif



#ifndef __QCAMERA_H_
#define __QCAMERA_H_


const unsigned char			QCAMERA_TYPE_PERSPECTIVE	= 0x01;
const unsigned char			QCAMERA_TYPE_ORTHOGRAPHIC	= 0x02;


const unsigned int			QCAMERA_CLIP_PLANE_LEFT		= 0x00000001;
const unsigned int			QCAMERA_CLIP_PLANE_RIGHT	= 0x00000002;
const unsigned int			QCAMERA_CLIP_PLANE_TOP		= 0x00000004;
const unsigned int			QCAMERA_CLIP_PLANE_BOTTOM   = 0x00000008;
const unsigned int			QCAMERA_CLIP_PLANE_NEAR		= 0x00000010;
const unsigned int			QCAMERA_CLIP_PLANE_FAR		= 0x00000020;
const unsigned int			QCAMERA_CLIP_PLANE_ALL		= (QCAMERA_CLIP_PLANE_LEFT | QCAMERA_CLIP_PLANE_RIGHT | QCAMERA_CLIP_PLANE_TOP | QCAMERA_CLIP_PLANE_BOTTOM |	\
														   QCAMERA_CLIP_PLANE_NEAR | QCAMERA_CLIP_PLANE_FAR);




////////////////////////////////////////////////////////////////////////////////////
//
// CCamera
// 
// Interactive camera object which can either be orthographic  or perspective.
// Within you will also find functionality for various frustum plane tests 
//
////////////////////////////////////////////////////////////////////////////////////
class QCAMERAEXPORT_API CCamera : public qscriptable<CCamera>
{
	public:
		
		CCamera();
		CCamera(CQuadrionRender* ptr);
		~CCamera() {}	

		CCamera & operator=(const CCamera &rhs)
		{
			return (CCamera&)rhs;
		}

		CCamera * operator=(const CCamera *rhs)
		{
			return (CCamera*)rhs;
		}

		
		// set view matrix //
		// all parameters assumed to be in world space //
		void SetCamera(float xPos, float yPos, float zPos,    \
					   float xLook, float yLook, float zLook, \
					   float xUp, float yUp, float zUp);
		
		// Set view matrix from vector position //
		void SetCamera(const vec3f& pos);
		
		// applies changes made before a frame render //
		void Apply();
		
		// Undo any changes from the last "SetCamera" or "CreatePerspective" or "setOrtho" //
		void Restore();
		
		// setup projection matrix as either perspective or orthographic projection //
		void CreatePerspective(float fov, float aspectRatio, float nearP, float farP);
		void CreateOrthographic(float l, float r, float b, float t, float n, float f);
		
		
		// cam movement //
		void RotateByMouse(int mouseX, int mouseY, int midScreenX, int midScreenY);     // rotate cam based on changes in mouse dir		
		
		// obtain camera type //
		const inline unsigned char		GetType() { return camType; }
				
		
		// return camera position as vec3f //			
		const inline vec3f			GetPosition() { return vec3f(camPos.x, camPos.y, camPos.z); }	
		
		// return last known camera position - usually from exactly one frame ago //
		const inline vec3f			GetLastPosition() { return lastCamPos; }	
		
		// return camera position in QBSP map coordinates //
		const inline vec3f			GetWorldPosition() { return vec3f(camPos.x, -camPos.z, camPos.y); }	
		
		
		// obtain view vector as vec3f //
		const inline vec3f			GetViewVector() { return vec3f(lookPos.x - camPos.x, lookPos.y - camPos.y, lookPos.z - camPos.z); }
		const inline vec3f			GetUpVector() { return vec3f(upVec); }


		// obtain the strafe vector
		const inline vec3f			GetStrafeVector() { return vec3f(strafeDir); }
		
		// obtain near and far clipping plane distances //
		const inline float			GetFarClip() { return farPlane; }
		const inline float			GetNearClip() { return nearPlane; }
		
		const inline float			GetFOV() { return m_fov; }
		
		// obtain clip planes //
		const inline void			GetPerspectiveClipPlanes( vec4f* outPlanes )	{ memcpy( outPlanes, frustumPlanes, sizeof( float ) * 24 ); }	
		const inline void			GetWorldClipPlanes( vec4f* outPlanes ) { memcpy( outPlanes, m_frustumPlanesWS, sizeof( float ) * 24 ); }
		
		// Frustum cull //
		QMATH_INTERSECT_RESULT IsSphereInFrustum(vec3f v, const float& radius, const float& bias = 0.0F);
		QMATH_INTERSECT_RESULT IsAABBInFrustum(vec3f mins, vec3f maxs, bool trivial = false);
		QMATH_INTERSECT_RESULT IsLineInFrustum( vec3f a, vec3f b );
	
		// SphereInCameraCone
		// Checks that a bounding sphere is within the camera's worldspace cone shape
		// This routine is significantly faster than the Frustum collision routines. 
		// You should preferably use this routine instead
		// params:
		//		sphereCtr- Position of the center of the sphere to be checked in worldspace
		//		sphereRad- Radius of the collision sphere
		QMATH_INTERSECT_RESULT	SphereInCameraCone(vec3f sphereCtr, float sphereRad);
	
		const inline float	GetRotationAngle() { return curRotAngle; }

		void CCamera::SetRenderDevice(CQuadrionRender* ptr);

		/// SCRIPTING
		virtual void REGISTER_SCRIPTABLES(qscriptengine *engine);
	
	private:
		void Rotate(float ang, vec3f axis);
		void GetFrustumPlanes();
		
		friend class			CQuadrionRender;
		CQuadrionRender*		m_pQuadrionRender;
	
		unsigned char   camType;			// Type of cam projection, orthographic or perspective
		
		// current values //
		vec3f   camPos;						// Position of cam
		vec3f   lookPos;					// Position the cam is viewing
		vec3f   upVec;						// Perp vector to the vector from camPos to lookPos
		
		// save values //
		vec3f lastCamPos;					// Stores last camera position
		vec3f lastLookPos;					// Stores last look position
		vec3f lastUpVec;					// Stores last up vector
		
		vec3f   strafeDir;				 	// Used in camera movement
		
		float				nearPlane;		// near plane distance (from camera pos)
		float				farPlane;		// far plane distance (from camera pos)
		float				curRotAngle;	// Current y-rotational angle
		float				m_fov;
		float				m_aspect;
			
		bool				camHasChanged;	// Stores camera state
			
		mat4 curProjMat;					// The current projection matrix
		
		CCone			m_cameraCone;
		
		vec4f			frustumPlanes[6];			// Current camera frustum planes in clip space
		vec4f			m_frustumPlanesWS[6];		// Current camera frustum planes in world space
};






#endif