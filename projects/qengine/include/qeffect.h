///////////////////////////////////////////////////////////////////////////////////////
//
// QEFFECT.H
//
// Author: SMS
// Date: 12/2008
// Written for Quadrion Engine 2008
//
// Effect and effect management definitions and interface declarations.
// Contains definitions for individual effect objects and effect management objects.
// There should be at least and preferably one effect management object per program which
// is responsible for the use, sequencing, and management of individual effects.
//
// Handles grabbed from the effect manager are to individual effects. 
// 
////////////////////////////////////////////////////////////////////////////////////////////

#include "qtexture.h"

#ifndef __QEFFECT_H_
#define __QEFFECT_H_


#ifdef QRENDER_EXPORTS
	#define QEFFECTEXPORT_API  __declspec(dllexport)
#else
	#define QEFFECTEXPORT_API  __declspec(dllimport)
#endif


#include <string>
#include <vector>
#include <d3d9.h>
#include <d3dx9.h>
#include "qresource.h"
#include "qmath.h"


const unsigned int			QEFFECT_MATRIX_WORLD					= 0x00000001;
const unsigned int			QEFFECT_MATRIX_VIEW						= 0x00000002;	
const unsigned int			QEFFECT_MATRIX_PROJECTION				= 0x00000004;
const unsigned int			QEFFECT_MATRIX_WORLDVIEW				= 0x00000008;
const unsigned int			QEFFECT_MATRIX_VIEWPROJECTION			= 0x00000010;
const unsigned int			QEFFECT_MATRIX_WORLDVIEWPROJECTION		= 0x00000020;
const unsigned int			QEFFECT_MATRIX_INVERSE					= 0x00000040;
const unsigned int			QEFFECT_MATRIX_TRANSPOSE				= 0x00000080;
const unsigned int			QEFFECT_MATRIX_INVERSETRANSPOSE			= 0x00000100;

const unsigned int			QEFFECT_MAX_PARAMETERS					= 64;



enum QEFFECTEXPORT_API EQuadrionEffectVariableType
{
	QEFFECT_VARIABLE_FLOAT				= 0,
	QEFFECT_VARIABLE_INT				= 1,
	QEFFECT_VARIABLE_BOOL				= 2,
	QEFFECT_VARIABLE_MATRIX2X2			= 3,
	QEFFECT_VARIABLE_MATRIX3X3			= 4,
	QEFFECT_VARIABLE_MATRIX4X4			= 5,
	QEFFECT_VARIABLE_STATE_MATRIX		= 6,
	QEFFECT_VARIABLE_FLOAT_ARRAY		= 7,
	QEFFECT_VARIABLE_INT_ARRAY			= 8,
	QEFFECT_VARIABLE_BOOL_ARRAY			= 9,
};



/////////////////////////////////////////////////////////////////////////////////////////////
//
// CQuadrionEffect
//
// Effect object declaration.
// CQuadrionEffect encapsulates a D3DX Effect (.fx) format or extensibly nVidia CGFX (.cgfx) 
// file formats. This makes management of the effect pipeline from creation to rendering to
// destruction more applicable to Quadrion Engine.
//
// Effect handles are obtained from CQuadrionEffectManager
//
//////////////////////////////////////////////////////////////////////////////////////////////
class QEFFECTEXPORT_API CQuadrionEffect : public CQuadrionResource
{
	public:
		
		CQuadrionEffect(const unsigned int handle, const std::string& name, const std::string& path = "./");
		~CQuadrionEffect();
		
		// Constructor(CQuadrionRender*) -- Binds render device to effect at construction time 
		// A suitable device is of type CQuadrionRender
		CQuadrionEffect(const void* pRender, const unsigned int handle, const std::string& name, const std::string& path = "./");
		
		
		// BeginEffect
		// param: technique -- String of the desired technique
		// Commits effect and renderer state, binds the current effect such that
		// all subsequent draw calls are made through this effect via RenderEffect
		bool				BeginEffect(const std::string& technique);
		
		// EndEffect
		// Resets effect and render state to the state before the effect was bound as current.
		void				EndEffect();
		
		// RenderEffect
		// User passes a void callback render routine to the effect 
		// The effect gathers previously stored render states and applies state block and invokes the render
		bool				RenderEffect(void* callbackObj, void(*renderCallback)(void* callbackObj));
		bool				RenderEffect(void(*renderCallback(void)));
		bool				RenderEffect(const cTextureRect& rect, const unsigned int& width, const unsigned int& height);
		bool				RenderEffect(const cTextureRect& rect, const unsigned int& width, const unsigned int& height, const vec3f* V);
		bool				RenderEffect( const int& pass );
		bool				EndRender( const int& pass );
		
		// UploadParameters
		// param: alias-- Name of parameter in effect
		// param: paramType-- One of EQuadrionEffectVariable type describing the type of parameter being uploaded
		// param: nParameters-- Number of parameters in this listing
		// param: val-- pointer to parameter data
		// Applies set of parameters to the effect prior to rendering
		bool				UploadParameters(const std::string& alias, const EQuadrionEffectVariableType& paramType, const unsigned int& nParameters, const void* val);
		
		
		// UploadStateMatrix
		// param: matrix-- Matrix type to upload (combination of QEFFECT_MATRIX_ flags)
		bool				UploadStateMatrix(const unsigned int& matrix);
		
		
		
		// UploadVectorConstant
		// param: alias-- Name of parameter in effect 
		// param: nParameters-- number of vector4 types to upload
		// param: val-- pointer to array of float4's
		bool				UploadVectorConstant(const std::string& alias, const unsigned int& nParameters, const float* val);
		
		// UploadTexture
		// param: paramName-- name of the texture object within the shader
		// param: textureObject-- pointer to a CQuadrionTextureObject to be bound to the shader
		bool				UploadTexture(const std::string& paramName, const CQuadrionTextureObject* textureObject);
		
		
		// IsInitialized -- Returns whether or not a device was successfully bound to the effect object //
		const inline bool	IsInitialized() { return m_bIsInitialized; }
		
		// IsLoaded -- Returns whether or not a shader was loaded and compiled successfully //
		const inline bool   IsLoaded() { return m_bIsShaderLoaded; }
		
		
		// GetEffectName -- Retrieve effect name (includes relative path) //
		void				GetEffectName(std::string& oName);
	
	
	protected:
		
		// DestroyEffect -- Releases effect resources and makes effect invalid //
		void				DestroyEffect();
		
		// CreateEffect -- Takes filename (including relative path).
		// This loads and compiles the effect and offloads it to the GPU //
		// Must be called AFTER device has been bound to effect object //
		bool				CreateEffect(const char* fname);
		bool				CreateEffect(const std::string& name, const std::string& path = "./");
		
		// ChangeRenderDevice -- Bind a different render device to the effect //
		void				ChangeRenderDevice(const void* pRender);
	

	private:
	
		friend class			CQuadrionRender;
		CQuadrionRender*		m_pQuadrionRender;
		

		std::string				m_effectPath;			// Path and filename
		
		bool					m_bIsInitialized;		// Is effect ready?
		bool					m_bIsShaderLoaded;		// Was shader compiled?
		
		int 					m_nPasses;				// Number of passes required to render current technique
		
		void*					m_effectParameterValues[QEFFECT_MAX_PARAMETERS];		// Parameter values listing
		unsigned int			m_effectParameterTypes[QEFFECT_MAX_PARAMETERS];			// Parameter types listing
		unsigned int			m_effectParameterCounts[QEFFECT_MAX_PARAMETERS];		// Parameter counts listing
		std::string				m_effectParameterNames[QEFFECT_MAX_PARAMETERS];			// Parameter names listing
		unsigned int			m_nParameters;											// Effect parameter count
		bool					m_bReloadStateBlock;									// Toggle for reloading the state block

		D3DXHANDLE				m_hStateBlock;			// Effect state block
		D3DXHANDLE				m_hCurrentTechnique;	// Handle to currently bound technique
		LPD3DXEFFECT			m_pEffectHandle;		// Effect Object Interface
		LPDIRECT3DDEVICE9		m_pRenderDevice;		// Render device interface 
};

#endif
