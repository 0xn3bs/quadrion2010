////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// HDRPIPELINE.H
//
// Written by Shawn Simonson for Quadrion Engine 05/2008
//
// This file implements a basic HDR pipeline in a manner that is very easy for the client to
// work with and implement. It involves initializing the HDR pipeline, giving it a base texture
// from which to send down the pipeline and finally presents the HDR asset to the backbuffer
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////



#ifndef __HDRPIPELINE_H_
#define __HDRPIPELINE_H_


#include "qrender.h"
#include "qmath.h"
#include "qtimer.h"
#include "debug.h"



////////////////////////////////////////////////////////////////////////////
// cHDRPipeline
// Primary point of interaction for HDR rendering
class CHDRPipeline
{
	public:
		
		CHDRPipeline();
		~CHDRPipeline();
	
		// initHDRPipeline- must be called prior to any subsequent operation //
		BOOL		Initialize();
		
		// explicit destructor //
		VOID		Destroy();

		
		// setSceneTexture- must be called prior to pipeline rendering //
		// Present the pipeline with the full scene rendered to a floating point render target //
		// So the handle presented must be in a FP16 format //
		const inline VOID		SetDepthTarget( const int& depthid ) { m_depthTarget = depthid; }
		
		// Run the pipeline and present the HDR asset to the backbuffer //
		VOID			Render( const CQuadrionRenderTarget* dest = NULL );
		
		
		// is the pipeline initialized? //
		const inline BOOL			IsInitialized() { return m_isInitialized; }
		const inline bool			IsEncoded() { return m_bUsingLogLuv; }
		const inline double			GetDeltaTime() { return m_deltaTime; }
		const inline int			GetHDRSurface() { return m_sceneHDR; }
		
		const inline void			SetMiddleGrey( const float& mg ) { m_middleGrey = mg; }
		const inline void			SetBloomScale( const float& sc ) { m_bloomScale = sc; }
		const inline void			SetBrightnessThreshold( const float& thresh ) { m_brightnessThreshold = thresh; }
		const inline void			SetBrightnessOffset( const float& offset ) { m_brightnessOffset = offset; }
		const inline void			SetAdaptationFactor( const float& adap ) { m_adaptationFactor = adap; }
		const inline void			SetFinalGrey( const float& fg ) { m_finalGrey = fg; }
	
	protected:
	
	private:
	
		void			ClearTargets();
		VOID			MeasureLuminance();				// measure overall luminance
		VOID			BrightPass();					// perform full scene bright pass	
		VOID			ApplyBloom();					// create bloom source
		VOID			RenderBloom();					// render bloom source
		VOID			CalculateAdaptation();			// calculate light adaptation
		VOID			RenderSoftenFilter();			// an alternative to bloom
	
		float			m_middleGrey;
		float			m_bloomScale;
		float			m_brightnessThreshold;
		float			m_brightnessOffset;
		float			m_adaptationFactor;
		float			m_finalGrey;
	
		// Texture assets //
		INT			m_depthTarget;
		INT			m_sceneHDR;							// full resolution floating point texture of the scene
		INT			m_scaledHDRScene;					// scaled floating point texture of the scene
		INT			m_luminance[4];						// progressively scaled array of average luminance textures
		INT			m_brightPass;						// overall bright pass texture
		INT			m_bloom;							// bloom source texture
		INT			m_tempBloom[4];						// vertical and horizontal +/- bloom textures to calculate final bloom
		INT			m_lumAdaptCur;						// adapted current luminance
		INT			m_lumAdaptLast;						// adapted last luminance
		INT			m_intermediateBloom;				// temp bloom texture
		
		INT			m_effect;							// effect handle
		
		INT			m_cropWidth;						// texture scale factor in U
		INT			m_cropHeight;						// texture scale factor in V
		
		BOOL		m_bUsingLogLuv;
		BOOL			m_isInitialized;				// is the pipeline initialized?
//		BOOL			m_isFPSupported;				// are floating point render targets supported?
		
		ETexturePixelFormat		m_luminanceFormat;			// greatest luminance format  I16F or I32F
//		ETexturePixelFormat		m_floatingPointFormat;		// greatest floating point texture format
		
		CTimer		m_adaptedTimer;						// timer for adaptive luminance calculation
		DOUBLE		m_deltaTime;						// change in time from last frame
};





#endif