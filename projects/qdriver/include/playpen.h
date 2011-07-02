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
#include <list>

#include "qscriptengine.h"
#include "qscriptexec.h"
#include "qscriptmodule.h"
#include "angelscript.h"
//#include "../../../angelscript/include/angelscript.h"

#include "qerrorlog.h"
#include "qeventregistry.h"
#include "qevent.h"
#include "qeventlistener.h"
#include "qeventhandler.h"
#include "qkeyeventlistener.h"
#include "qscripteventhandler.h"
#include "qobject.h"

#include "qphysics.h"
#include "qphysicsmesh.h"
#include "btBulletDynamicsCommon.h"

// new physics interface //
#include "qphysicsengine.h"
#include "qRigidBody.h"

// PhysX physics implementation //
#include "qPhysXEngine.h"
#include "qRigidBodyPhysX.h"

qRigidBody *box;

#include "add_ons/scriptstring/scriptstring.h"
#include "add_ons/scriptany/scriptany.h"

#include "qswf.h"

#include "qfont.h"

qscriptengine *g_pScriptEngine;
qscriptexec *g_pScriptExec;
qscriptexec *event_script;
qscriptexec *cam;
qscriptmodule *g_pScriptModule;
qeventregistry *g_pEventRegistry;
qphysicsengine *g_pPhysicsWorld;

qscriptexec*	camMovement;
qscriptexec*	keysUpdate;

qPhysicsEngine *PE;

CFont *font;

qPhysicsMesh *convex_mesh;
SWF	*g_pSWF;

CTimer *timer;

btRigidBody *handle;

static CCamera* g_pCamera = NULL;
static CModelManager* g_pModelManager = NULL;

static int g_hModelHandle = QRENDER_INVALID_HANDLE;
static int g_hEffectHandle = QRENDER_INVALID_HANDLE;

unsigned char keys[256];

std::vector<int> sky_box_handles;


//// completely temporary solution ///

struct glockObject
{
	qRigidBody*		bodyHandle;
	int				modelHandle;
};

std::vector<glockObject> glockObjectList;

//// completely temporary solution ///


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
				if(keys[VkKeyScan('n')])
				{
				//phymod *t = new phymod();
				//model_list.push_back(t);
				}

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

void killApp()
{
	exit(1);
}

bool isKeyDown(unsigned char key)
{
	return (keys[key] == 1);
}

void GetMousePosition( int& x, int& y )
{
	POINT pt;
	if( GetCursorPos( &pt ) )
	{
		x = pt.x;
		y = pt.y;
		return;
	}

	else 
		return;
}
static void PlayInit()
{
	for(int a = 0;a < 256;a++)
		keys[a] = 0;

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
	g_pPhysicsWorld->setGravity(0.0f, -32.2f, 0.0f);

	// new physics init //
	PE = new qPhysXEngine();
	PE->init();

	//box = PE->addRigidBody(10.0f, NULL, qPhysicsShape());

	// TODO : write overloaded wrappers to AS
	int r = g_pScriptEngine->getEngine()->RegisterGlobalFunction("float cosf(float)",  asFUNCTIONPR(cos, (float), float), asCALL_CDECL); assert( r >= 0);
	r = g_pScriptEngine->getEngine()->RegisterGlobalFunction("float sinf(float)",  asFUNCTIONPR(sin, (float), float), asCALL_CDECL); assert( r >= 0);
	//r = g_pScriptEngine->getEngine()->RegisterGlobalFunction("uint16 VK_KEY(uint8)",  asFUNCTIONPR(VkKeyScan, (float), float), asCALL_CDECL); assert( r >= 0);
	REGISTER_GLOBAL_FUNCTION(g_pScriptEngine, "uint16 VK_KEY(string &in)", VK_KEY);
	REGISTER_GLOBAL_FUNCTION(g_pScriptEngine, "void killApp()", killApp);
	REGISTER_GLOBAL_FUNCTION(g_pScriptEngine, "bool isKeyDown(uint8 key)", isKeyDown);
	REGISTER_GLOBAL_FUNCTION(g_pScriptEngine, "void GetMousePosition(int &out, int &out)", GetMousePosition);
	r = g_pScriptEngine->getEngine()->RegisterGlobalProperty("CCamera@ cam", &g_pCamera); assert( r >= 0);
	

	g_pScriptModule = g_pScriptEngine->pGetScriptModule("script");

	// TODO : fix the from file loader..
    //if(g_pScriptModule->addSectionFromFile("C:\Users\avansc\Desktop\quad2010\Media\Scriptstest.as") < 0) exit(-1);
	//if(g_pScriptModule->addSection(on_event) < 0) exit(-1);

	if(g_pScriptModule->addSectionFromFile("main.as", "Media/Scripts/") < 0) exit(-1);

	if(g_pScriptModule->buildScript() < 0) exit(-1);

	keysUpdate	= g_pScriptEngine->pGetScriptExec("script", "void updateKeyboard()");
	camMovement	= g_pScriptEngine->pGetScriptExec("script", "void updateMovement(CCamera @cam)");

	event_script = g_pScriptEngine->pGetScriptExec("script", "void ON_EVENT(qevent @evt, qobject @obj)");

	qeventlistener *L = new qkeyeventlistener();
	L->set_key(KEY_DOWN);
	qeventhandler *H = new qscripteventhandler();
	((qscripteventhandler*)H)->set_script_exe(event_script);

	g_pEventRegistry->register_pair(L, H, g_pCamera);

	int w = g_pApp->GetWindowWidth();
	int h = g_pApp->GetWindowHeight();

	g_pApp->SetMousePosition( 1400/2, 1900/2 );

	g_pCamera->SetCamera( 60.0f, 20.0f, 60.0f, 0.0f, 100.0f, 0.0f, 0.0f, 1.0f, 0.0f );
	g_pCamera->CreatePerspective( QMATH_DEG2RAD( 55.0f ), (float)w / (float)h, 2.0f, 1000.0f );
	g_pCamera->Apply();

	g_pModelManager->SetTexturePath("Media/Textures/");

	// adding a root model and one instance model;
	//g_hModelHandle = g_pModelManager->AddModel( "glock18c.3DS", "Media/Models/" );
	int mHandle = g_pModelManager->AddModel( "glock18c.3DS", "Media/Models/" );
	CModelObject* mdl = g_pModelManager->GetModel( "glock18c.3DS", "Media/Models/", mHandle );

	vec3f min, max, center;
	mdl->GetAABB(min, max);
	mdl->GetModelCenter(center);
	/// physics convex decomposition mesh ///
	//convex_mesh = new qPhysicsMesh3DS(mdl);
	//convex_mesh->processMesh();

	mat4 id;
	QMATH_MATRIX_LOADIDENTITY( id );
	mdl->SetModelPos( vec3f( 0.0f, 50.0f, 0.0f ) );
	mdl->SetModelScale( vec3f( 1.0f, 1.0f, 1.0f ) );
	mdl->SetModelOrientation( id );
	mdl->BindDiffuseTexture(0);
	mdl->BindNormalmapTexture( -1 );
	mdl->CreateFinalTransform(id);

	//btRigidBody* bHandle = g_pPhysicsWorld->addRigidBody(20.0f, mdl, convex_mesh->getCollisionShape());
	//btRigidBody* bHandle = g_pPhysicsWorld->addBox(20.0f, vec3f(0.0f, 60.0f, 0.0f), max - center, mdl);
	qRigidBody* bHandle = PE->addRigidBody(10.0f, mdl, qPhysicsShape());
	glockObject GO = {bHandle, mHandle};
	glockObjectList.push_back(GO);

	for(int a = 0;a < 100;a++)
	{
		mHandle = g_pModelManager->AddModel( "glock18c.3DS", "Media/Models/" );

		mdl = g_pModelManager->GetModel( "glock18c.3DS", "Media/Models/", mHandle );

		mat4 id;
		QMATH_MATRIX_LOADIDENTITY( id );
		mdl->SetModelPos( vec3f( rand()%500-250, rand()%100+100.0f, rand()%500-250 ) );
		mdl->SetModelScale( vec3f( 1.0f, 1.0f, 1.0f ) );
		mdl->SetModelOrientation( id );
		mdl->BindDiffuseTexture(0);
		mdl->BindNormalmapTexture( -1 );
		mdl->CreateFinalTransform(id);

		//g_pModelManager->UpdateModelOrientation("glock18c.3DS", "Media/Models/", mHandle, id);

		//bHandle = g_pPhysicsWorld->addRigidBody(20.0f, mdl, convex_mesh->getCollisionShape());
		//bHandle = g_pPhysicsWorld->addBox(20.0f, vec3f(0.0f, 60.0f, 0.0f), max - center, mdl);
		 bHandle = PE->addRigidBody(10.0f, mdl, qPhysicsShape());
		//bHandle->applyTorqueImpulse(btVector3(rand()%400-200, rand()%400-200, rand()%400-200));

		glockObject GO = {bHandle, mHandle};
		glockObjectList.push_back(GO);
	}
	//g_pModelManager->PushInstances("glock18c.3DS", "Media/Models/");

	
	/*mat4 id;
	QMATH_MATRIX_LOADIDENTITY( id );
	mdl->SetModelPos( vec3f( 0.0f, 50.0f, 0.0f ) );
	mdl->SetModelScale( vec3f( 1.0f, 1.0f, 1.0f ) );
	mdl->SetModelOrientation( id );
	mdl->BindDiffuseTexture(0);
	mdl->BindNormalmapTexture( -1 );
	mdl->CreateFinalTransform(id);*/


	g_hEffectHandle = g_pRender->AddEffect( "Phong.fx", "Media/Effects/" );

	
	//handle = g_pPhysicsWorld->addRigidBody(20.0f, vec3f(0.0f, 10.0f, 0.0f), max - center, convex_mesh->getCollisionShape());

	// btRigidBody *addRigidBody(float mass, CModelObject *model, btCollisionShape *shape);
	//handle = g_pPhysicsWorld->addRigidBody(20.0f, mdl, convex_mesh->getCollisionShape());
	
	//handle = g_pPhysicsWorld->addBox(20.0f, vec3f(0.0f, 60.0f, 0.0f), max - center, mdl);
	//handle->applyTorqueImpulse(btVector3(100,100,-50));
	
	g_pSWF = new SWF;
	g_pSWF->LoadSWF("Media/SWF/console.swf");

	timer = new CTimer();
	timer->Start();

	font = new CFont();
	if(!font->LoadFont("arial.tga", "Media/Textures/", false))
		QUIT_ERROR("Could not load font!", "Font Loading Error!");

	unsigned int cock = QTEXTURE_FILTER_LINEAR;

	sky_box_handles.push_back(g_pRender->AddTextureObject(cock, "skybox_right1.jpg", "Media/Textures/skybox/space/purple_nebula_complex/"));
	sky_box_handles.push_back(g_pRender->AddTextureObject(cock, "skybox_left2.jpg", "Media/Textures/skybox/space/purple_nebula_complex/"));
	sky_box_handles.push_back(g_pRender->AddTextureObject(cock, "skybox_top3.jpg", "Media/Textures/skybox/space/purple_nebula_complex/"));
	sky_box_handles.push_back(g_pRender->AddTextureObject(cock, "skybox_bottom4.jpg", "Media/Textures/skybox/space/purple_nebula_complex/"));
	sky_box_handles.push_back(g_pRender->AddTextureObject(cock, "skybox_front5.jpg", "Media/Textures/skybox/space/purple_nebula_complex/"));
	sky_box_handles.push_back(g_pRender->AddTextureObject(cock, "skybox_back6.jpg", "Media/Textures/skybox/space/purple_nebula_complex/"));
}

void PlayUpdate();

void RenderGrid(vec3f pos)
{
	pos.y = 0;
	//pos.x = (int)(pos.x/5);
	//pos.z = (int)(pos.z/5);
	int tx = ((int)(pos.x))%5;
	pos.x = pos.x/1 + tx;
	int tz = ((int)(pos.z))%5;
	pos.z = pos.z/1 + tz;

	int size = 500;
	for(int x = -size;x < size;x++)
	{
		//for(int y = -100;y < 100;y++)
		//{
			g_pRender->RenderLine(vec3f(x*5,0,-size*5), vec3f(x*5,0,size*5), QRENDER_MAKE_ARGB(0x80, 10,100,10));
			
			g_pRender->RenderLine(vec3f(-size*5, 0, x*5), vec3f(size*5, 0, x*5), QRENDER_MAKE_ARGB(0x80, 10,100,10));
		//}
	}
	//g_pRender->RenderLine(vec3f(0,0,0), vec3f(1000,1000,1000), QRENDER_MAKE_ARGB(0xFF, 255,255,255));
}

void RenderBox(vec3f min, vec3f max, unsigned int color)
{
	vec3f minx(min.x,0,0);
	vec3f miny(0,min.y,0);
	vec3f minz(0,0,min.z);

	vec3f maxx(max.x,0,0);
	vec3f maxy(0,max.y,0);
	vec3f maxz(0,0,max.z);

	//for(int a = 0;a < 8;a++)
	//g_pRender->RenderLine(minx - maxx/2, maxx - maxx/2, color);
	//g_pRender->RenderLine(minx - maxy/2, maxx - maxy/2, color);

	//X-AXIS
	/*g_pRender->RenderLine(minx - maxy - maxz/2, maxx - maxy - maxz/2, color);
	g_pRender->RenderLine(minx + maxy + maxz/2, maxx + maxy + maxz/2, color);

	g_pRender->RenderLine(minx - maxz/2, maxx - maxz/2, color);
	g_pRender->RenderLine(minx + maxz/2, maxx + maxz/2, color);

	//g_pRender->RenderLine(miny, maxy, color);
	//g_pRender->RenderLine(minz, maxz, color);*/

	/*g_pRender->RenderLine(x-x/2, -x, color);
	g_pRender->RenderLine(y, -y, color);
	g_pRender->RenderLine(z, -z, color);*/
}

void GLtoDX(float *dxm, float *glm)
{
	for(int u = 0;u < 4;u++)
		for(int v = 0;v < 4;v++)
			dxm[u*4+v] = glm[v*4+u];
}

float dx = 0.0f;

static void PlayRender()
{
	g_pRender->ChangeDepthBias(-0.0005f);
	g_pRender->ChangeSlopeBias(1.0f);

	//g_pPhysicsWorld->renderBodies(g_pCamera);
	//g_pPhysicsWorld->step(timer->GetElapsedSec());

	PE->step(timer->GetElapsedMicroSec()/1000000.0f);
	timer->Start();
	//g_pPhysicsWorld->step(1/60.0f);
	
	//timer->Reset();
	
	//keysUpdate->ctx->SetArgObject(0, &keys[0]);
	keysUpdate->exec();
	keysUpdate->reset();

	camMovement->ctx->SetArgObject(0, g_pCamera);
	camMovement->exec();
	camMovement->reset();
	
	//processKeys();
	//g_pEventRegistry->process_events();
	PlayUpdate();

	CModelObject* mdl = g_pModelManager->GetModel( "glock18c.3DS", "Media/Models/", 0 );
	 

//	g_pPhysicsWorld->updateCenterOfMassOffest(handle, mdl);

	CQuadrionEffect* fx = g_pRender->GetEffect( g_hEffectHandle );
	unsigned int mat = QRENDER_MATRIX_MODELVIEWPROJECTION;
	unsigned int worldMat = QRENDER_MATRIX_MODEL;
	unsigned int vp = QRENDER_MATRIX_VIEWPROJECTION;
	mat4 modelMat, prev;
	vec3f p = g_pCamera->GetPosition();
	vec3f camPos = vec3f(p.x, p.y, p.z);
	
	btTransform trans;
	mat4 rot;
	std::vector<glockObject>::iterator it;
	for(it = glockObjectList.begin(); it != glockObjectList.end(); it++)
	{
		//(*it).bodyHandle->getMotionState()->getWorldTransform(trans);
		(*it).bodyHandle->getPose(rot);
		//trans.getOpenGLMatrix(rot);
		//QMATH_MATRIX_TRANSPOSE(rot);
		g_pModelManager->UpdateModelOrientation("glock18c.3DS", "Media/Models/", (*it).modelHandle, rot);
	}

	//mat4 pose;
	//box->getPose(pose);

	//QMATH_MATRIX_LOADIDENTITY(pose);

	//g_pModelManager->UpdateModelOrientation("glock18c.3DS", "Media/Models/", 0, pose);

	//g_pModelManager->PushInstances("glock18c.3DS", "Media/Models/");
	
    /*
	handle->getMotionState()->getWorldTransform(trans);
	trans.getOpenGLMatrix(rot);
	QMATH_MATRIX_TRANSPOSE(rot);
	g_pModelManager->UpdateModelOrientation("glock18c.3DS", "Media/Models/", g_hModelHandle, rot);
	g_pModelManager->PushInstances("glock18c.3DS", "Media/Models/");
	}
	*/
	g_pModelManager->PushInstances("glock18c.3DS", "Media/Models/");

	g_pRender->ChangeDepthMode(QRENDER_ZBUFFER_ENABLEWRITE);
	g_pRender->ChangeDepthMode(QRENDER_ZBUFFER_LEQUAL);
	fx->BeginEffect( "Phong" );
	
	fx->UploadParameters("g_mVP", QEFFECT_VARIABLE_STATE_MATRIX, 1, &vp);
	fx->UploadParameters( "g_mMVP", QEFFECT_VARIABLE_STATE_MATRIX, 1, &mat );
	fx->UploadParameters("g_mWorld", QEFFECT_VARIABLE_STATE_MATRIX, 1, &worldMat);
	fx->UploadParameters( "g_lightPos", QEFFECT_VARIABLE_FLOAT_ARRAY, 3, &camPos );
	fx->UploadParameters( "g_camPos", QEFFECT_VARIABLE_FLOAT_ARRAY, 3, &camPos );
	fx->RenderEffect( 0 );

	mdl->RenderModel();

	fx->EndRender( 0 );
	fx->EndEffect();
	
	g_pRender->ChangeDepthMode(QRENDER_ZBUFFER_DEFAULT);
	g_pRender->EnableColorWrites();

	/*
	//vec3f bound_low, bound_high;
	//mdl->GetAABB(bound_low, bound_high);

	//btVector3 min, max;
	//g_pPhysicsWorld->getLocalAABB(handle, min, max);

	vec3f mint(min.getX(), min.getY(), min.getZ());
	vec3f maxt(max.getX(), max.getY(), max.getZ());*/

	RenderGrid(camPos);

	STexturedQuad skybox;
	vec2f skybox_uv[4];
	vec3f skybox_coord[4];

	skybox_uv[0] = vec2f(0, 0);
	skybox_uv[1] = vec2f(0, 1);
	skybox_uv[2] = vec2f(1, 1);
	skybox_uv[3] = vec2f(1, 0);

	skybox_coord[0] = vec3f(-1024, 1024, 1041.2);
	skybox_coord[1] = vec3f(-1024, -1024, 1041.2);
	skybox_coord[2] = vec3f(1024, -1024, 1041.2);
	skybox_coord[3] = vec3f(1024, 1024, 1041.2);


	skybox.pos[0] = skybox_coord[0] + g_pCamera->GetPosition();
	skybox.pos[1] = skybox_coord[1] + g_pCamera->GetPosition();
	skybox.pos[2] = skybox_coord[2] + g_pCamera->GetPosition();
	skybox.pos[3] = skybox_coord[3] + g_pCamera->GetPosition();

	skybox.texcoords[0] = skybox_uv[0];
	skybox.texcoords[1] = skybox_uv[1];
	skybox.texcoords[2] = skybox_uv[2];
	skybox.texcoords[3] = skybox_uv[3];

	CQuadrionTextureObject* tex = g_pRender->GetTextureObject(sky_box_handles[0]);
	tex->BindTexture();
	g_pRender->RenderQuad(skybox);
	tex->UnbindTexture();

	g_pRender->ChangeDepthMode(QRENDER_ZBUFFER_DISABLE);
	g_pRender->EnableAlphaBlending();
	g_pRender->ChangeAlphaBlendMode(QRENDER_ALPHABLEND_SRCALPHA, QRENDER_ALPHABLEND_ONE);
	font->WriteText("test", vec2f(100,100), vec2f(0,0), FONT_ALIGN_LEFT, QRENDER_MAKE_ARGB(0xFF, 255,0,0));
	//g_pRender->ChangeDepthMode(QRENDER_ZBUFFER_DEFAULT);
	g_pRender->DisableAlphaBlending();
	g_pRender->ChangeDepthMode(QRENDER_ZBUFFER_DEFAULT);	
	g_pRender->ChangeDepthBias(0.0f);
	g_pRender->ChangeSlopeBias(0.0f);
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
	QMem_SafeDelete( g_pSWF );
}

#endif
