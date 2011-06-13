#ifndef __PLAYPEN_H_
#define __PLAYPEN_H_


#include <iostream>
#include "qcamera.h"
#include "qrender.h"
#include "qmodelobject.h"
#include "qmem.h"
#include "app.h"
#include "qmath.h"


static CCamera* g_pCamera = NULL;
static CModelManager* g_pModelManager = NULL;

static int g_hModelHandle = QRENDER_INVALID_HANDLE;
static int g_hEffectHandle = QRENDER_INVALID_HANDLE;


static void PlayInit()
{

	g_pCamera = new CCamera;
	g_pModelManager = new CModelManager;

	int w = g_pApp->GetWindowWidth();
	int h = g_pApp->GetWindowHeight();

	g_pCamera->SetCamera( 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f );
	g_pCamera->CreatePerspective( QMATH_DEG2RAD( 75.0f ), (float)w / (float)h, 0.01f, 500.0f );
	g_pCamera->Apply();

	g_hModelHandle = g_pModelManager->AddModel( "land_gear_rear.3ds", "Media/Models/" );
	g_hEffectHandle = g_pRender->AddEffect( "Phong.fx", "Media/Effects/" );

	mat4 id;
	QMATH_MATRIX_LOADIDENTITY( id );
	CModelObject* mdl = g_pModelManager->GetModel( "land_gear_rear.3ds", "Media/Models/", g_hModelHandle );
	mdl->SetModelPos( vec3f( 0.0f, 0.0f, 5.0f ) );
	mdl->SetModelOrientation( id );
	mdl->SetModelScale( vec3f( 0.01f, 0.01f, 0.01f ) );
	mdl->BindDiffuseTexture( -1 );
	mdl->BindNormalmapTexture( -1 );
}

static void PlayRender()
{
	CModelObject* mdl = g_pModelManager->GetModel( "land_gear_rear.3ds", "Media/Models/", g_hModelHandle );
	CQuadrionEffect* fx = g_pRender->GetEffect( g_hEffectHandle );
	unsigned int mat = QRENDER_MATRIX_MODELVIEWPROJECTION;
	mat4 modelMat, prev;
	vec3f camPos = g_pCamera->GetPosition();

	mdl->CreateFinalTransform( modelMat );
	g_pRender->GetMatrix( QRENDER_MATRIX_MODEL, prev );
	g_pRender->MulMatrix( QRENDER_MATRIX_MODEL, modelMat );

	fx->BeginEffect( "Phong" );
	fx->UploadParameters( "g_mMVP", QEFFECT_VARIABLE_STATE_MATRIX, 1, &mat );
	fx->UploadParameters( "g_lightPos", QEFFECT_VARIABLE_FLOAT_ARRAY, 3, &camPos );
	fx->UploadParameters( "g_camPos", QEFFECT_VARIABLE_FLOAT_ARRAY, 3, &camPos );
	fx->RenderEffect( 0 );
	mdl->RenderModel();
	fx->EndRender( 0 );
	fx->EndEffect();

	g_pRender->SetMatrix( QRENDER_MATRIX_MODEL, prev );
}

static void PlayUpdate()
{
	int mx = 0;
	int my = 0;
	int sx = g_pApp->GetWindowWidth();
	int sy = g_pApp->GetWindowHeight();
	g_pApp->GetMousePosition( mx, my );

	g_pCamera->RotateByMouse( mx, my, sx / 2, sy / 2 );
	g_pCamera->Apply();
}


static void PlayDestroy()
{
	QMem_SafeDelete( g_pCamera );
	QMem_SafeDelete( g_pModelManager );
}

#endif