#ifndef __PLAYPEN_H_
#define __PLAYPEN_H_


#include <iostream>
#include "qcamera.h"
#include "qrender.h"
#include "qmodelobject.h"
#include "qmem.h"
#include "app.h"
#include "qmath.h"
#include <math.h>

#include "qscriptengine.h"
#include "qscriptexec.h"
#include "qscriptmodule.h"
#include "angelscript.h"

qscriptengine *g_pScriptEngine;
qscriptexec *g_pScriptExec;
qscriptmodule *g_pScriptModule;

static CCamera* g_pCamera = NULL;
static CModelManager* g_pModelManager = NULL;

static int g_hModelHandle = QRENDER_INVALID_HANDLE;
static int g_hEffectHandle = QRENDER_INVALID_HANDLE;

static void kill()
{
	exit(-1);
}

static void PlayInit()
{

	g_pCamera = new CCamera;
	g_pModelManager = new CModelManager;

	// scripting init //
	g_pScriptEngine = new qscriptengine();
	g_pScriptEngine->registerScriptable<CCamera>();

	// TODO : write overloaded wrappers to AS
	int r = g_pScriptEngine->getEngine()->RegisterGlobalFunction("float cosf(float)",  asFUNCTIONPR(cos, (float), float), asCALL_CDECL); assert( r >= 0);
	r = g_pScriptEngine->getEngine()->RegisterGlobalFunction("float sinf(float)",  asFUNCTIONPR(sin, (float), float), asCALL_CDECL); assert( r >= 0);
	
	g_pScriptEngine->getEngine()->RegisterGlobalProperty("CCamera@ cam", &g_pCamera);

	char *script = 
		"float tx = 0;"
		"float tz = 0;"
		"float dt = 0;"
		"void main(float w, float h)				"
		"{							"
		"	dt += 0.001;"
		"	tx = 10*cosf(dt);"
		"	tz = 10*sinf(dt);"
		"	cam.SetCamera( tx, 0.0f, tz, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f);"
		"	cam.Apply();		"
		"}							";


	g_pScriptModule = g_pScriptEngine->pGetScriptModule("script");

	// TODO : fix the from file loader..
    //if(mod->addSectionFromFile("test.as") < 0) exit(-1);
    if(g_pScriptModule->addSection(script) < 0) exit(-1);
	if(g_pScriptModule->buildScript() < 0) exit(-1);

	g_pScriptExec = g_pScriptEngine->pGetScriptExec("script", "void main(float w, float h)");

	int w = g_pApp->GetWindowWidth();
	int h = g_pApp->GetWindowHeight();

	g_pScriptExec->ctx->SetArgFloat(0, w);
	g_pScriptExec->ctx->SetArgFloat(1, h);
	g_pScriptExec->exec();
	g_pScriptExec->reset();

	g_pCamera->SetCamera( 0.0f, 0.0f, -10.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f );
	g_pCamera->CreatePerspective( QMATH_DEG2RAD( 75.0f ), (float)w / (float)h, 0.8f, 50.0f );
	g_pCamera->Apply();

	g_hModelHandle = g_pModelManager->AddModel( "land_gear_rear.3ds", "Media/Models/" );
	g_hEffectHandle = g_pRender->AddEffect( "Phong.fx", "Media/Effects/" );
	
	mat4 id;
	QMATH_MATRIX_LOADIDENTITY( id );
	CModelObject* mdl = g_pModelManager->GetModel( "land_gear_rear.3ds", "Media/Models/", g_hModelHandle );
	mdl->SetModelPos( vec3f( 0.0f, 0.0f, 0.0f ) );
	mdl->SetModelOrientation( id );
	mdl->SetModelScale( vec3f( 0.01f, 0.01f, 0.01f ) );
	mdl->BindDiffuseTexture( -1 );
	mdl->BindNormalmapTexture( -1 );
}

void PlayUpdate();

static void PlayRender()
{
	PlayUpdate();

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

	g_pScriptExec->ctx->SetArgFloat(0, sx);
	g_pScriptExec->ctx->SetArgFloat(1, sy);
	g_pScriptExec->exec();
	g_pScriptExec->reset();

	//g_pApp->GetMousePosition( mx, my );

	//g_pCamera->RotateByMouse( mx, my, sx / 2, sy / 2 );
	//g_pCamera->Apply();
}


static void PlayDestroy()
{
	QMem_SafeDelete( g_pCamera );
	QMem_SafeDelete( g_pModelManager );
}

#endif