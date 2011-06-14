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

	// scripting ivnit //
	g_pScriptEngine = new qscriptengine();
	g_pScriptEngine->registerScriptable<CCamera>();

	//OutputDebugString("\n\nhelp\n\n");
//std::cin.ignore();s
	
	
	g_pScriptEngine->getEngine()->RegisterGlobalProperty("CCamera@ cam", &g_pCamera);
	
	int r = g_pScriptEngine->getEngine()->RegisterGlobalFunction("void kill()", asFUNCTIONPR(kill, (void), void), asCALL_CDECL); assert( r >= 0 );
	//REGISTER_GLOBAL_FUNCTION(g_pScriptEngine, "void DEBUG(string &str)", OutputDebugString);

	char *script = 
		//"CCamera@ cam;"
		" float tx = 0;"
		"void main(float w, float h)				"
		"{							"
		//"	cam.SetCamera( 0.0f, 0.0f-tx, 0.0f+tx, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f);	"
		//"	cam.CreatePerspective(75.0f*3.141692f/180.0f, w/h, 0.01f, 500.0f);"
		"	cam.Apply();		"
		//"	tx+=0.001;"
		"}							";


	g_pScriptModule = g_pScriptEngine->pGetScriptModule("script");
    //if(mod->addSectionFromFile("test.as") < 0) exit(-1);
    if(g_pScriptModule->addSection(script) < 0) exit(-1);
	if(g_pScriptModule->buildScript() < 0) exit(-1);

	g_pScriptExec = g_pScriptEngine->pGetScriptExec("script", "void main(float w, float h)");


	int w = g_pApp->GetWindowWidth();
	int h = g_pApp->GetWindowHeight();

	//g_pScriptExec->ctx->SetArgObject(0, &g_pCamera);
	g_pScriptExec->ctx->SetArgFloat(0, w);
	g_pScriptExec->ctx->SetArgFloat(1, h);
	//g_pScriptExec->exec();
	//g_pScriptExec->reset();

	//printf("pachow!\n");
	//std::cout << "fux";


	g_pCamera->SetCamera( 0.0f, 0.0f, -50.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f );
	g_pCamera->CreatePerspective( QMATH_DEG2RAD( 75.0f ), (float)w / (float)h, 0.01f, 500.0f );
	g_pCamera->Apply();

	//g_hModelHandle = g_pModelManager->AddModel( "land_gear_rear.3ds", "Media/Models/" );
	//g_hEffectHandle = g_pRender->AddEffect( "Phong.fx", "Media/Effects/" );

	g_hModelHandle = g_pModelManager->AddModel( "home.3ds", "Media/Models/" );
	g_hEffectHandle = g_pRender->AddEffect( "Phong.fx", "Media/Effects/" );

	mat4 id;
	//QMATH_MATRIX_LOADIDENTITY( id );
	QMATH_MATRIX_LOADXROLL(id, QMATH_DEG2RAD( 90.0f));
	//CModelObject* mdl = g_pModelManager->GetModel( "land_gear_rear.3ds", "Media/Models/", g_hModelHandle );
	CModelObject* mdl = g_pModelManager->GetModel( "home.3ds", "Media/Models/", g_hModelHandle );
	mdl->SetModelPos( vec3f( 0.0f, -10.0f, 10.0f ) );
	mdl->SetModelOrientation( id );
	mdl->SetModelScale( vec3f( 0.001f, 0.001f, 0.001f ) );
	mdl->BindDiffuseTexture( -1 );
	mdl->BindNormalmapTexture( -1 );
}

void PlayUpdate();

float droll = 0;
vec3f camPos(20,0,0);

static void PlayRender()
{
	droll += 0.01;
	//PlayUpdate();
	/*CCamera **handle = &g_pCamera;
	g_pScriptExec->ctx->SetArgAddress(0, handle);
	g_pScriptExec->exec();
	g_pScriptExec->reset();*/

//g_pRender->BeginRendering();
//g_pRender->ClearBuffers(QRENDER_CLEAR_BACKBUFFER,  QRENDER_MAKE_ARGB(0, 0, 0, 0), 0);

	//CModelObject* mdl = g_pModelManager->GetModel( "land_gear_rear.3ds", "Media/Models/", g_hModelHandle );
	CModelObject* mdl = g_pModelManager->GetModel( "home.3ds", "Media/Models/", g_hModelHandle );
	
	//mat4 id;
	//QMATH_MATRIX_LOADIDENTITY( id );
	///QMATH_MATRIX_LOADXROLL(id, droll);
	
	//mdl->SetModelPos( vec3f( 0.0f, 0.0f, 5.0f ) );
	//mdl->SetModelOrientation( id );
	//mdl->SetModelScale( vec3f( 0.01f, 0.01f, 0.01f ) );
	//mdl->BindDiffuseTexture( -1 );
	//mdl->BindNormalmapTexture( -1 );*/

	g_pRender->RenderLine(vec3f(0,0,0),camPos,  QRENDER_MAKE_ARGB(1, 1, 1, 1));

	CQuadrionEffect* fx = g_pRender->GetEffect( g_hEffectHandle );
	unsigned int mat = QRENDER_MATRIX_MODELVIEWPROJECTION;
	mat4 modelMat, prev;
	//vec3f camPos(0,10,-10); //= g_pCamera->GetPosition();
	camPos.x = 2000*cos(droll);
	camPos.y = 2000*sin(droll/2.0f);
	camPos.z = 2000*sin(droll);

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
//g_pRender->EndRendering();
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
	/*g_pScriptExec->ctx->SetArgObject(0, &g_pCamera);
	g_pScriptExec->ctx->SetArgWord(1, 1440);
	g_pScriptExec->ctx->SetArgWord(2, 900);
	g_pScriptExec->exec();
	g_pScriptExec->reset();*/
	
	//g_pCamera->Apply();
}


static void PlayDestroy()
{
	QMem_SafeDelete( g_pCamera );
	QMem_SafeDelete( g_pModelManager );
}

#endif