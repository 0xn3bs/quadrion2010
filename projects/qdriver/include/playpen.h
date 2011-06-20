#ifndef __PLAYPEN_H_
#define __PLAYPEN_H_


#include <iostream>
#include "qcamera.h"
#include "qrender.h"
#include "qmodelobject.h"
#include "qmem.h"
#include "app.h"
#include "qtimer.h"
#include "qmath.h"
#include <math.h>

#include "qscriptengine.h"
#include "qscriptexec.h"
#include "qscriptmodule.h"
#include "angelscript.h"

#include "qeventregistry.h"
#include "qevent.h"
#include "qeventlistener.h"
#include "qeventhandler.h"
#include "qkeyeventlistener.h"
#include "qscripteventhandler.h"
#include "qobject.h"

#include "qphysics.h"
#include "btBulletDynamicsCommon.h"

#include "add_ons/scriptstring/scriptstring.h"
#include "add_ons/scriptany/scriptany.h"

qscriptengine *g_pScriptEngine;
qscriptexec *g_pScriptExec;
qscriptexec *event_script;
qscriptexec *cam;
qscriptmodule *g_pScriptModule;
qeventregistry *g_pEventRegistry;
qphysicsengine *g_pPhysicsWorld;

btRigidBody *handle;

static CCamera* g_pCamera = NULL;
static CModelManager* g_pModelManager = NULL;

static int g_hModelHandle = QRENDER_INVALID_HANDLE;
static int g_hEffectHandle = QRENDER_INVALID_HANDLE;

char keys[256];

short VK_KEY(CScriptString *str)
{
	return VkKeyScan(str->buffer.at(0));
}

void processKeys()
{
	for(int a = 0;a < 256;a++)
	{
		if(keys[a] == 1)
		{
			g_pEventRegistry->push_event(qevent(a, KEY_DOWN, EVENT_KEY));
		}
	}

	int mx = 0;
				int my = 0;
				g_pApp->GetMousePosition(mx, my);
				g_pEventRegistry->push_event(qevent(mx, my, EVENT_MOUSE));
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
				//g_pEventRegistry->push_event(qevent(wParam, KEY_DOWN, EVENT_KEY));
				keys[wParam] = 1;
				break;
			}

			case WM_KEYUP:
			{
				keys[wParam] = 0;
				break;
			}

			case WM_MOUSEMOVE:
			{
				//POINTS curs;
				//MAKEPOINTS(lParam);

				//g_pEventRegistry->push_event(qevent(curs.x, curs.y, EVENT_MOUSE));
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

vec3f lightPos;

static void PlayInit()
{
	g_pCamera = new CCamera;
	g_pModelManager = new CModelManager;

	// scripting init //
	g_pScriptEngine = new qscriptengine();
	g_pScriptEngine->registerScriptable<qobject>();
	g_pScriptEngine->registerScriptable<qevent>();
	g_pScriptEngine->registerScriptable<CCamera>();
	g_pScriptEngine->registerScriptable<CTimer>();

	// event system //
	g_pEventRegistry = new qeventregistry();

	// physics init //
	g_pPhysicsWorld = new qphysicsengine();
	g_pPhysicsWorld->setGravity(0.0f, -9.8f, 0.0f);

	// TODO : write overloaded wrappers to AS
	int r = g_pScriptEngine->getEngine()->RegisterGlobalFunction("float cosf(float)",  asFUNCTIONPR(cos, (float), float), asCALL_CDECL); assert( r >= 0);
	r = g_pScriptEngine->getEngine()->RegisterGlobalFunction("float sinf(float)",  asFUNCTIONPR(sin, (float), float), asCALL_CDECL); assert( r >= 0);
	//r = g_pScriptEngine->getEngine()->RegisterGlobalFunction("uint16 VK_KEY(uint8)",  asFUNCTIONPR(VkKeyScan, (float), float), asCALL_CDECL); assert( r >= 0);
	REGISTER_GLOBAL_FUNCTION(g_pScriptEngine, "uint16 VK_KEY(string &in)", VK_KEY);
	g_pScriptEngine->getEngine()->RegisterGlobalProperty("CCamera@ cam", &g_pCamera);

	char *on_event = 
		"CTimer time;										"
		"void ON_EVENT(qevent @evt, qobject @obj)			"
		"{												"
		"	time.Start();"
		"	double dt = time.GetElapsedSec();		"
		"	CCamera @view = cast<CCamera>(@obj);		"
		"	switch(evt.type())							"
		"	{											"
		"		case EVENT_KEY :						"
		"		{										"
		"			switch(evt.get_key_code())			"
		"			{									"
		"				case VK_A :						"
		"				{								"
		"					view.MoveCameraRelative(-50000.0f*dt, 0.0f, 0.0f);	"
		"					break;						"
		"				}								"
		"				case VK_D :						"
		"				{								"
		"					view.MoveCameraRelative(50000.0f*dt, 0.0f, 0.0f);	"
		"					break;						"
		"				}								"
		"				case VK_W :						"
		"				{								"
		"					view.MoveCameraRelative(0.0f, 0.0f, 50000.0f*dt);	"
		"					break;						"
		"				}								"
		"				case VK_S :						"
		"				{								"
		"					view.MoveCameraRelative(0.0f, 0.0f, -50000.0f*dt);	"
		"					break;						"
		"				}								"
		"			}									"
		"			break;								"
		"		}										"
		"		case EVENT_MOUSE :						"
		"		{										"
		"			view.RotateByMouse( evt.get_mouse_x(), evt.get_mouse_y(), 1400 / 2, 900 / 2 );"	
		"			break;								"
		"		}										"
		"	}											"
		"	view.Apply();								"
		"	time.Reset();								"
		"}												";

	g_pScriptModule = g_pScriptEngine->pGetScriptModule("script");

	// TODO : fix the from file loader..
    //if(g_pScriptModule->addSectionFromFile("C:\Users\avansc\Desktop\quad2010\Media\Scriptstest.as") < 0) exit(-1);
	if(g_pScriptModule->addSection(on_event) < 0) exit(-1);
	if(g_pScriptModule->buildScript() < 0) exit(-1);

	event_script = g_pScriptEngine->pGetScriptExec("script", "void ON_EVENT(qevent @evt, qobject @obj)");

	qeventlistener *L = new qkeyeventlistener();
	L->set_key(KEY_DOWN);
	qeventhandler *H = new qscripteventhandler();
	((qscripteventhandler*)H)->set_script_exe(event_script);

	g_pEventRegistry->register_pair(L, H, g_pCamera);

	int w = g_pApp->GetWindowWidth();
	int h = g_pApp->GetWindowHeight();

	g_pApp->SetMousePosition( 1400/2, 1900/2 );

	g_pCamera->SetCamera( 0.0f, 10.0f, -40.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f );
	g_pCamera->CreatePerspective( QMATH_DEG2RAD( 75.0f ), (float)w / (float)h, 0.8f, 500.0f );
	g_pCamera->Apply();

	g_pModelManager->SetTexturePath("Media/Textures/");
	g_hModelHandle = g_pModelManager->AddModel( "glock18c.3DS", "Media/Models/" );
	g_hEffectHandle = g_pRender->AddEffect( "Phong.fx", "Media/Effects/" );
	
	mat4 id;
	QMATH_MATRIX_LOADIDENTITY( id );
	CModelObject* mdl = g_pModelManager->GetModel( "glock18c.3DS", "Media/Models/", g_hModelHandle );
	mdl->SetModelPos( vec3f( 0.0f, 5.0f, 0.0f ) );
	mdl->SetModelOrientation( id );
	mdl->SetModelScale( vec3f( 1.0f, 1.0f, 1.0f ) );
	mdl->BindDiffuseTexture(0);
	mdl->BindNormalmapTexture( -1 );

	lightPos = vec3f(500.0f, 500.0f, 500.0f);

	handle = g_pPhysicsWorld->addCube(10.0f, vec3f(0.0f, 10.0f, 0.0f), mdl);
}

void PlayUpdate();

void RenderGrid()
{


	for(int x = -100;x < 100;x++)
	{
		//for(int y = -100;y < 100;y++)
		//{
			g_pRender->RenderLine(vec3f(x*2,0,-100*2), vec3f(x*2,0,100*2), QRENDER_MAKE_ARGB(0xFF, 10,100,10));
			
			g_pRender->RenderLine(vec3f(-100*2, 0, x*2), vec3f(100*2, 0, x*2), QRENDER_MAKE_ARGB(0xFF, 10,100,10));
		//}
	}
	//g_pRender->RenderLine(vec3f(0,0,0), vec3f(1000,1000,1000), QRENDER_MAKE_ARGB(0xFF, 255,255,255));
}

void GLtoDX(float *dxm, float *glm)
{
	for(int u = 0;u < 4;u++)
		for(int v = 0;v < 4;v++)
			dxm[u*4+v] = glm[v*4+u];
}

static void PlayRender()
{
	g_pPhysicsWorld->step(1/60.0f);
	processKeys();
	g_pEventRegistry->process_events();
	PlayUpdate();

	CModelObject* mdl = g_pModelManager->GetModel( "glock18c.3DS", "Media/Models/", g_hModelHandle );
	

	CQuadrionEffect* fx = g_pRender->GetEffect( g_hEffectHandle );
	unsigned int mat = QRENDER_MATRIX_MODELVIEWPROJECTION;
	mat4 modelMat, prev;
	vec3f camPos = g_pCamera->GetPosition();

	btTransform trans;
    handle->getMotionState()->getWorldTransform(trans);

	//mdl->SetModelPos(vec3f(trans.getOrigin().getX(), trans.getOrigin().getY(), trans.getOrigin().getZ()));

	btScalar m[16];
	trans.getOpenGLMatrix(&m[0]);
	mat4 id;
	//GLtoDX(id, m);
	QMATH_MATRIX_COPY(id, *((mat4*)m));
	//QMATH_MATRIX_TRANSPOSE(id);
	mdl->SetModelOrientation( id );
	///////////////


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

	g_pRender->RenderBox(vec3f(-1,-1,-1), vec3f(1,1,1), QRENDER_MAKE_ARGB(0xFF, 255,255,255));

	//g_pRender->GetMatrix( QRENDER_MATRIX_MODEL, prev );
	//g_pRender->MulMatrix( QRENDER_MATRIX_MODEL, modelMat );

	// attempt to render a green line.
	RenderGrid();
	//g_pRender->RenderLine(fromVec, toVec, QRENDER_MAKE_ARGB(0xFF, fromR, fromG, fromB)
	//g_pRender->RenderLine(vec3f(0,0,0), vec3f(1000,1000,1000), QRENDER_MAKE_ARGB(0xFF, 255,255,255));
	/*fx->BeginEffect( "Phong" );
	fx->UploadParameters( "g_mMVP", QEFFECT_VARIABLE_STATE_MATRIX, 1, &mat );
	fx->UploadParameters( "g_lightPos", QEFFECT_VARIABLE_FLOAT_ARRAY, 3, &camPos );
	fx->UploadParameters( "g_camPos", QEFFECT_VARIABLE_FLOAT_ARRAY, 3, &camPos );
	fx->RenderEffect( 0 );
	mdl->RenderModel();
	fx->EndRender( 0 );
	fx->EndEffect();*/

	//g_pRender->SetMatrix( QRENDER_MATRIX_MODEL, prev );
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