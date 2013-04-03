#include "qcamera.h"



CCamera::CCamera()
{
	camPos.set(0.0F, 0.0F, 0.0F);
	strafeDir.set(0.0F, 0.0F, 0.0F);
	lookPos.set(0.0F, 0.0F, 0.0F);
	upVec.set(0.0F, 0.0F, 0.0F);
	lastCamPos.set(0.0F, 0.0F, 0.0F);
	lastLookPos.set(0.0F, 0.0F, 0.0F);
	lastUpVec.set(0.0F, 0.0F, 0.0F);


	m_fov = 0;
	camType = 0;
	curRotAngle = 0.0F;
	camHasChanged = false;
}




///////////////////////////////////////////////////////////////////////////////////////////////////////////
// setCamera
// xPos, yPos, zPos: camera's world space position
// xLook, yLook, zLook: the point in world space the camera is aimed at
// xUp, yUp, zUp: defines the camera's orientation by providing which vector relative to the cam is "up"
// Sets the modelview matrix for GL and the View matrix for DX
void CCamera::SetCamera(float xPos, float yPos, float zPos, float xLook, float yLook, float zLook, float xUp, float yUp, float zUp)
{
	// backup //
	lastCamPos.set(camPos);
	lastLookPos.set(lookPos);
	lastUpVec.set(upVec);

	camPos.set(xPos, yPos, zPos);
	lookPos.set(xLook, yLook, zLook);
	upVec.set(xUp, yUp, zUp);
	camHasChanged = true;

	// get the strafe vector
	vec3f cDir, cCross;
	cDir = lookPos - camPos;
	if(cDir.x != 0 && cDir.y != 0 && cDir.z != 0)
		cDir.normalize();
	cCross = cDir.crossProd(upVec);
	strafeDir = cCross;
	camHasChanged = true;
	
	// Set camera cone //
	m_cameraCone.SetVertex(camPos);
	m_cameraCone.SetDirection(lookPos - camPos);
}

/////////////////////////////////////////////////////////////////////////////////////
// setCamera
// Same as above but sets it from a 3 component vector vec3f
void CCamera::SetCamera(const vec3f& pos)
{
	// backup //
	lastCamPos.set(camPos);
	lastLookPos.set(lookPos);
	lastUpVec.set(upVec);


	// get view vector and set the new look pos //
	vec3f lookVec = pos - lastCamPos;
	lookPos.set(lastLookPos + lookVec);
	camPos.set(pos);


	// get the strafe vector
	vec3f cDir, cCross;
	cDir = lookPos - camPos;
	if(cDir.x != 0 && cDir.y != 0 && cDir.z != 0)
		cDir.normalize();
	cCross = cDir.crossProd(upVec);
	strafeDir = cCross;
	camHasChanged = true;
	
	// Set camera cone //
	m_cameraCone.SetVertex(camPos);
}

////////////////////////////////////////////////////////////////////////////////////
// MoveCamerRelative
// Move the camera relative to its orientation
void CCamera::MoveCameraRelative(float tx, float ty, float tz)
{
	vec3f currPos = this->GetPosition();
	vec3f viewVec = this->GetViewVector();
	viewVec.normalize();
	vec3f upVec = this->GetUpVector();
	upVec.normalize();
	vec3f strafeVec = this->GetStrafeVector();
	strafeVec.normalize();

	currPos += viewVec*tz;
	currPos += upVec*ty;
	currPos += strafeVec*tx;

	vec3f curLookPos = lookPos;
	curLookPos += viewVec * tz;
	curLookPos += upVec * ty;
	curLookPos += strafeVec * tx;

	viewVec += this->lookPos;

	this->SetCamera(currPos.x, currPos.y, currPos.z,
					curLookPos.x, curLookPos.y, curLookPos.z, 
					upVec.x, upVec.y, upVec.z);
}


////////////////////////////////////////////////////////////////////////////////////
// setPerspective
// fov: field of view in radians, aspectRatio: the aspect ratio of viewport
// nearP, farP: near and far plane distances
// Sets up a perspective transform and applies it for "this" camera object 
void CCamera::CreatePerspective(float fov, float aspectRatio, float nearP, float farP)
{
	// create a perspective matrix
	mat4 P;
	QMATH_MATRIX_LOADPERSPECTIVE_DX(P, fov, aspectRatio, nearP, farP);

	QMATH_MATRIX_COPY(curProjMat, P);

	// set perspective matrix
	g_pRender->SetMatrix(QRENDER_MATRIX_PROJECTION, P);


	nearPlane = nearP;
	farPlane = farP;

	camType = QCAMERA_TYPE_PERSPECTIVE;
	camHasChanged = true;
	m_fov = fov;
	m_aspect = aspectRatio;
	
	// Set camera cone angle //
	m_cameraCone.SetTheta(fov);
}


///////////////////////////////////////////////////////////////////////////////////////
// setOrtho
// l, r: left and right bounnds,     b, t:  bottom and top bounds
// n, f: near and far bounds
// Sets up an orthographic projection and applies it for "this" camera object
void CCamera::CreateOrthographic(float l, float r, float b, float t, float n, float f)
{
	// load in an orthographic matrix
	mat4 O;
	QMATH_MATRIX_LOADORTHO_DX(O, l, r, b, t, n, f);
	g_pRender->SetMatrix(QRENDER_MATRIX_PROJECTION, O);

	// re-calc the camera view planes

	nearPlane = n;
	farPlane = f;

	camType = QCAMERA_TYPE_ORTHOGRAPHIC;
	camHasChanged = true;
	
	// Set Camera cone angle //
	m_cameraCone.SetTheta(QMATH_PI);
}



/////////////////////////////////////////////////////////////////////////////////////
// rotateCam- used internally by rotateMouse
// Uses quaternions to construct a rotation about an arbitrary axis 
// Resets new camera look at position
void CCamera::Rotate(float ang, vec3f axis)
{

	// backup //
//	lastCamPos.set(camPos);
	lastLookPos.set(lookPos);
//	lastUpVec.set(upVec);

	quat qRot, qView, qNewView;
	QMATH_QUATERNION_ROTATE(qRot, QMATH_DEG2RAD(ang), axis);
	//	quat_rotate(qRot, DEG2RAD(ang), axis);       // DEG2RAD(ang)

	qView.x = lookPos.x - camPos.x;
	qView.y = lookPos.y - camPos.y;
	qView.z = lookPos.z - camPos.z;
	qView.w = 0.0f;

	QMATH_QUATERNION_MULQUAT(qRot, qView, qNewView);
	QMATH_QUATERNION_CONJUGATE(qRot);
	QMATH_QUATERNION_MULQUAT(qNewView, qRot, qNewView);


	lookPos.x = camPos.x + qNewView.x;
	lookPos.y = camPos.y + qNewView.y;
	lookPos.z = camPos.z + qNewView.z;
	camHasChanged = true;
}



///////////////////////////////////////////////////////////////////////////////////////
// rotateMouse
// mouseX, mouseY: mouse position (x, y) in screen space
// midScreenX, midScreenY: DISP_WIDTH / 2 and DISP_HEIGHT / 2
// Rotates the camera relative to the mouse's movements in screen space
void CCamera::RotateByMouse( int mouseX, int mouseY, int midScreenX, int midScreenY )
{
//	lastCamPos.set(camPos);
	lastLookPos.set(lookPos);
//	lastUpVec.set(upVec);

	float xDir, yRot;
	xDir = yRot = 0.0f;

	if( ( mouseX == midScreenX ) && ( mouseY == midScreenY ) )
		return;

	// Get direction vectors in screen space //
	xDir = float( (midScreenX - mouseX) ) / 10.0f * 1.0f;         // 1.0 == sensitivity
	yRot = float( (midScreenY - mouseY) ) / 10.0f * 1.0f;		  // 1.0 == sensitivity

	curRotAngle += yRot;



	// Obtain rotation vector for vertical rot //
	vec3f axis(0.0F, 0.0F, 0.0F);
	vec3f dir(0.0F, 0.0F, 0.0F);
	dir = lookPos - camPos;
	axis = dir.crossProd(upVec);
	axis.normalize();


	// Rotate cam about up vector (horizontal) //
	vec3f up(0.0F, 1.0F, 0.0F);
	Rotate(xDir, up);

	// Rotate cam bout strafe axis (vertical) //
	Rotate( yRot, axis );



	// Check that we dont pivot too far by obtaining the new view vector and 
	// Checking the angle between it and the view vector on the x/z plane
	vec3f viewVec = GetViewVector();
	vec3f flatVec(viewVec.x, 0.0F, viewVec.z);
	viewVec.normalize();
	flatVec.normalize();
	float ang = QMATH_RAD2DEG(acos(viewVec.dotProd(flatVec)));

	camHasChanged = true;

	// If we're overextended, reset the rotation angle and the look position //
	if(ang > 85.0F)
	{
		lookPos.set(lastLookPos);
		if(curRotAngle < 0)
			curRotAngle = -85.0F;
		else
			curRotAngle = 85.0F;
	}
	
	// Set camera cone //
	m_cameraCone.SetDirection(lookPos - camPos);
}





void CCamera::GetFrustumPlanes()
{
	mat4 viewProj;
	g_pRender->GetMatrix( QRENDER_MATRIX_MODELVIEWPROJECTION, viewProj );

	frustumPlanes[0].x = viewProj[3] + viewProj[0];
	frustumPlanes[0].y = viewProj[7] + viewProj[4];
	frustumPlanes[0].z = viewProj[11] + viewProj[8];
	frustumPlanes[0].w = viewProj[15] + viewProj[12];
	
	frustumPlanes[1].x = viewProj[3] - viewProj[0];
	frustumPlanes[1].y = viewProj[7] - viewProj[4];
	frustumPlanes[1].z = viewProj[11] - viewProj[8];
	frustumPlanes[1].w = viewProj[15] - viewProj[12];
	
	frustumPlanes[2].x = viewProj[3] - viewProj[1];
	frustumPlanes[2].y = viewProj[7] - viewProj[5];
	frustumPlanes[2].z = viewProj[11] - viewProj[9];
	frustumPlanes[2].w = viewProj[15] - viewProj[13];
	
	frustumPlanes[3].x = viewProj[3] + viewProj[1];
	frustumPlanes[3].y = viewProj[7] + viewProj[5];
	frustumPlanes[3].z = viewProj[11] + viewProj[9];
	frustumPlanes[3].w = viewProj[15] + viewProj[13];
	
	frustumPlanes[4].x = viewProj[3] + viewProj[2];
	frustumPlanes[4].y = viewProj[7] + viewProj[6];
	frustumPlanes[4].z = viewProj[11] + viewProj[10];
	frustumPlanes[4].w = viewProj[15] + viewProj[14];
	
	frustumPlanes[5].x = viewProj[3] - viewProj[2];
	frustumPlanes[5].y = viewProj[7] - viewProj[6];
	frustumPlanes[5].z = viewProj[11] - viewProj[10];
	frustumPlanes[5].w = viewProj[15] - viewProj[14];
	
	
	float mag = 0.0f;
	float iMag = 0.0f;
	for(int i = 0; i < 6; ++i)
	{
		mag = sqrt( frustumPlanes[i].x * frustumPlanes[i].x + frustumPlanes[i].y * frustumPlanes[i].y +    
					frustumPlanes[i].z * frustumPlanes[i].z );
		iMag = 1.0F / mag;
		
		frustumPlanes[i].x *= iMag;
		frustumPlanes[i].y *= iMag;
		frustumPlanes[i].z *= iMag;
		frustumPlanes[i].w *= iMag;
	}
}

QMATH_INTERSECT_RESULT CCamera::IsSphereInFrustum(vec3f v, const float& radius, const float& bias)
{ 
	float dist;
	vec3f pn;
	for(int i = 0; i < 6; ++i)
	{
		pn.set( frustumPlanes[i].x, frustumPlanes[i].y, frustumPlanes[i].z );
		dist = ( pn.dotProd( v ) + frustumPlanes[i].w );
		if( dist < -( radius + bias ) )
			return QMATH_OUTSIDE;

		if( fabs( dist ) < radius + bias )
			return QMATH_INTERSECT;
	}

	return QMATH_INSIDE;
}


QMATH_INTERSECT_RESULT CCamera::IsAABBInFrustum( vec3f mins, vec3f maxs,  bool trivial )
{
	vec3f bbox[8] = 
	{
		vec3f( mins.x, mins.y, mins.z ),
		vec3f( mins.x, mins.y, maxs.z ),
		vec3f( mins.x, maxs.y, mins.z ),
		vec3f( mins.x, maxs.y, maxs.z ),
		
		vec3f( maxs.x, mins.y, mins.z ),
		vec3f( maxs.x, mins.y, maxs.z ),
		vec3f( maxs.x, maxs.y, mins.z ),
		vec3f( maxs.x, maxs.y, maxs.z )
	};

	int totalIn = 0;

	for( int i = 0; i < 6; ++i )
	{
		int in = 8;
		int ptin = 1;

		for( int j = 0; j < 8; ++j )
		{
			vec3f pn( frustumPlanes[i].x, frustumPlanes[i].y, frustumPlanes[i].z);
			vec3f tmp = bbox[j];
			
			if( tmp.dotProd( pn ) + frustumPlanes[i].w < 0 )
			{
				ptin = 0;
				--in;
			}
			
			else
				if( trivial )
					return QMATH_INTERSECT;
		}

		if( in == 0 )
			return QMATH_OUTSIDE;

		totalIn += ptin;
	}

	if(totalIn == 6)
		return QMATH_INSIDE;

	return QMATH_INTERSECT;
}

QMATH_INTERSECT_RESULT CCamera::IsLineInFrustum( vec3f a, vec3f b )
{
	bool aIn, bIn;
	aIn = bIn = true;
	
	for( int i = 0; i < 6; ++i )
	{
		vec3f pn( frustumPlanes[i].x, frustumPlanes[i].y, frustumPlanes[i].z );
		if( a.dotProd( pn ) + frustumPlanes[i].w < 0 )
			aIn = false;
			
		if( b.dotProd( pn ) + frustumPlanes[i].w < 0 )
			bIn = false;
		
		if( !aIn && !bIn ) return QMATH_OUTSIDE;
	}
	
	if( aIn && bIn ) return QMATH_INSIDE;
	
	return QMATH_INTERSECT;
}


QMATH_INTERSECT_RESULT CCamera::SphereInCameraCone(vec3f sphereCtr, float sphereRad)
{
	return m_cameraCone.IsSphereInCone(sphereCtr, sphereRad);
}


/////////////////////////////////////////////////////////////////////////////////////
// applyCam
// Applies the camera's movement changes and orientation changes before a frame
// render. This should be called if the camera has changed prior to a frame render
void CCamera::Apply()
{
	if(camHasChanged)
	{
		mat4 V;
		QMATH_MATRIX_LOADVIEW_DX(V, vec3f(camPos), vec3f(lookPos), vec3f(upVec));
		g_pRender->SetMatrix(QRENDER_MATRIX_MODELVIEW, V );


		mat4 MVP;
		g_pRender->GetMatrix(QRENDER_MATRIX_VIEWPROJECTION, MVP);
		QMATH_MATRIX_TRANSPOSE(MVP);

		GetFrustumPlanes();

//		float planes[24];
//		memcpy(planes, frustumPlanes, sizeof(float) * 24);
//		g_pRender->SetFrustumClipPlanes( planes, 6 );

		camHasChanged = false;
	}
}


