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

char keys[256];

void processKeys()
{
	if(keys[VkKeyScan('a')])//keys[0x41])
	{
		g_pScriptExec->ctx->SetArgFloat(0, -0.1f);
		g_pScriptExec->ctx->SetArgFloat(1, 0.0f);
		g_pScriptExec->ctx->SetArgFloat(2, 0.0f);
		g_pScriptExec->exec();
		g_pScriptExec->reset();
	}

	if(keys[VkKeyScan('d')])//keys[0x41])
	{
		g_pScriptExec->ctx->SetArgFloat(0, 0.1f);
		g_pScriptExec->ctx->SetArgFloat(1, 0.0f);
		g_pScriptExec->ctx->SetArgFloat(2, 0.0f);
		g_pScriptExec->exec();
		g_pScriptExec->reset();
	}

	if(keys[VkKeyScan('w')])//keys[0x41])
	{
		g_pScriptExec->ctx->SetArgFloat(0, 0.0f);
		g_pScriptExec->ctx->SetArgFloat(1, 0.0f);
		g_pScriptExec->ctx->SetArgFloat(2, 0.2f);
		g_pScriptExec->exec();
		g_pScriptExec->reset();
	}

	if(keys[VkKeyScan('s')])//keys[0x41])
	{
		g_pScriptExec->ctx->SetArgFloat(0, 0.0f);
		g_pScriptExec->ctx->SetArgFloat(1, 0.0f);
		g_pScriptExec->ctx->SetArgFloat(2, -0.2f);
		g_pScriptExec->exec();
		g_pScriptExec->reset();
	}
}

static LRESULT CALLBACK PlaypenEventCallback(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
		switch(uMsg)
		{
			case WM_QUIT:
			{
				PostQuitMessage(0);
			}

			case WM_KEYDOWN:
			{
				//g_pEventReg->push_event(qevent(wParam, Event::KEY_DOWN));
				keys[wParam] = 1;
				break;
			}

			case WM_KEYUP:
			{
				keys[wParam] = 0;
				break;
			}
	
		case WM_SYSCOMMAND:
		{
			if(wParam == SC_SCREENSAVE || wParam == SC_MONITORPOWER)
				return 0;
			
			break;
		}
		
		case WM_CLOSE:
		{
			PostQuitMessage(0);
			return 0;
		}


		default:
		{
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
			break;
		}	
	}
	
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
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
		"void main(float x, float y, float z)				"
		"{							"
		//"	cam.SetCamera( tx, 7*sinf(dt/1.3f), tz, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f);"*/
		"	cam.MoveCameraRelative(x, y, z);"
		"	cam.Apply();		"
		"}							";


	g_pScriptModule = g_pScriptEngine->pGetScriptModule("script");

	// TODO : fix the from file loader..
    //if(mod->addSectionFromFile("test.as") < 0) exit(-1);
    if(g_pScriptModule->addSection(script) < 0) exit(-1);
	if(g_pScriptModule->buildScript() < 0) exit(-1);

	g_pScriptExec = g_pScriptEngine->pGetScriptExec("script", "void main(float x, float y, float z)");

	int w = g_pApp->GetWindowWidth();
	int h = g_pApp->GetWindowHeight();

	g_pApp->SetMousePosition( 1400/2, 1900/2 );

	g_pCamera->SetCamera( 0.0f, 0.0f, -10.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f );
	g_pCamera->CreatePerspective( QMATH_DEG2RAD( 75.0f ), (float)w / (float)h, 0.8f, 500.0f );
	g_pCamera->Apply();

	g_pModelManager->SetTexturePath("Media/Textures/");
	g_hModelHandle = g_pModelManager->AddModel( "glock18c.3DS", "Media/Models/" );
	g_hEffectHandle = g_pRender->AddEffect( "Phong.fx", "Media/Effects/" );
	
	mat4 id;
	QMATH_MATRIX_LOADIDENTITY( id );
	CModelObject* mdl = g_pModelManager->GetModel( "glock18c.3DS", "Media/Models/", g_hModelHandle );
	mdl->SetModelPos( vec3f( 0.0f, 0.0f, 0.0f ) );
	mdl->SetModelOrientation( id );
	mdl->SetModelScale( vec3f( 1.0f, 1.0f, 1.0f ) );
	mdl->BindDiffuseTexture(0);
	mdl->BindNormalmapTexture( -1 );
}

void PlayUpdate();

static void PlayRender()
{
	processKeys();
	PlayUpdate();

	CModelObject* mdl = g_pModelManager->GetModel( "glock18c.3DS", "Media/Models/", g_hModelHandle );
	
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

	g_pCamera->RotateByMouse( mx, my, 1400 / 2, 900 / 2 );
	g_pCamera->Apply();

	g_pApp->SetMousePosition( 1400/2, 900/2 );
}


static void PlayDestroy()
{
	QMem_SafeDelete( g_pCamera );
	QMem_SafeDelete( g_pModelManager );
}

#endif