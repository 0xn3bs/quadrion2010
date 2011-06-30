#include "qRigidBodyPhysX.h"

#include "qrender.h"

#include <PxPhysicsAPI.h> 
#include <PxExtensionsAPI.h> 
#include <PxDefaultErrorCallback.h>
#include <PxDefaultAllocator.h> 
#include <PxDefaultSimulationFilterShader.h>
#include <PxDefaultCpuDispatcher.h>
#include <PxShapeExt.h>
#include <PxMat33Legacy.h> 
#include <PxSimpleFactory.h>

#include "qerrorlog.h"

qRigidBodyPhysX::qRigidBodyPhysX(PxRigidDynamic *_body)
{
	this->body = _body;
}

void getColumnMajor(PxMat33 m, PxVec3 t, float* mat) {
	mat[0] = m.column0[0];
   mat[1] = m.column0[1];
   mat[2] = m.column0[2];
   mat[3] = 0;

   mat[4] = m.column1[0];
   mat[5] = m.column1[1];
   mat[6] = m.column1[2];
   mat[7] = 0;

   mat[8] = m.column2[0];
   mat[9] = m.column2[1];
   mat[10] = m.column2[2];
   mat[11] = 0;

   mat[12] = t[0];
   mat[13] = t[1];
   mat[14] = t[2];
   mat[15] = 1;

}

void qRigidBodyPhysX::getPose(mat4& pose)
{

	PxU32 nShapes = this->body->getNbShapes(); 
    PxShape** shapes=new PxShape*[nShapes];
	
	this->body->getShapes(shapes, nShapes);    

	//mat4 mat;

	PxTransform pT = PxShapeExt::getGlobalPose(*shapes[0]);

	//qErrorLog::Instance()->WriteError("pos <%f,%f,%f>\n", pT.p.x, pT.p.y, pT.p.z);

	PxBoxGeometry bg;
	shapes[0]->getBoxGeometry(bg);

	PxMat33 m = PxMat33(pT.q );
	float mat[16];
	getColumnMajor(m,pT.p, mat);
	QMATH_MATRIX_TRANSPOSE(mat);
	/*QMATH_QUATERNION_MAKEMATRIX(mat, vec4f(pT.q.x, pT.q.y, pT.q.z, pT.q.w));
	
	mat4 T;
	QMATH_MATRIX_LOADTRANSLATION(T, vec3f(pT.p.x, pT.p.y, pT.p.z));
	
	QMATH_MATRIX_MULTIPLY(T, mat, mat);*/

	//pose = mat;
	QMATH_MATRIX_COPY(pose, mat);

	delete [] shapes;

	//vec3f he(1,1,1);
	//g_pRender->RenderBox(he*-bg.halfExtents.x, he*bg.halfExtents.x, QRENDER_MAKE_ARGB(0x80, 10,100,10));
	//return mat;

	//QMATH_MATRIX_LOADIDENTITY(pose);
}