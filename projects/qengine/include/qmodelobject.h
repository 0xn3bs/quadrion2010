
#include <map>
#include <string>
#include "q3dsmodel.h"
#include "qmd3.h"
#include "qmath.h"
#include "qeffect.h"

#ifndef __MODELOBJECT_H_
#define __MODELOBJECT_H_


#define	MAX_MODEL_INSTANCES		48

#ifdef QRENDER_EXPORTS
#define QMODELOBJECTEXPORT_API		__declspec(dllexport)
#else
#define QMODELOBJECTEXPORT_API		__declspec(dllimport)
#endif

class QMODELOBJECTEXPORT_API CModelObject
{
	public:
	
		CModelObject(const unsigned int handle, const std::string& name, const std::string& path = "./");
		virtual ~CModelObject();
		
		
		virtual bool		LoadModel( bool loadNormalmaps ) {return true;}   
		virtual void		RenderModel(); 
		virtual void		GetAABB(vec3f& mins, vec3f& maxs) {} 
		virtual void		GetBoundingSphere(vec3f& center, float& rad) {}
		virtual void		GetModelCenter(vec3f& center) {}
		virtual bool		IsInstance() { return false; }

		bool				LoadEffect(const std::string& fxName, const std::string& fxPath = "./");
		
		void				CreateFinalTransform(mat4& M);
		
		const inline void	MakeInvisible() { m_bIsRenderable = false; }
		const inline void	MakeVisible() { m_bIsRenderable = true; }
		const inline bool	IsVisible() { return m_bIsRenderable; }
		const inline bool	IsLoaded() { return m_bIsLoaded; }
		
		const inline void	SetModelScale(const vec3f& new_scale) { m_modelScale.set(new_scale); }
		const inline vec3f  GetModelScale() { return m_modelScale; }
		
		const inline void	SetModelPos(const vec3f& new_pos) { m_worldPos.set(new_pos); }
		const inline vec3f	GetModelPos() { return m_worldPos; }
		const inline void	SetFilePath( const std::string& path ) { m_filePath = path; }
		
		const inline void	SetModelOrientation(const mat4& new_pose) { QMATH_MATRIX_COPY(m_modelPose, new_pose); }
		const inline void	GetModelOrientation(mat4& out_pose) { QMATH_MATRIX_COPY(out_pose, m_modelPose); }

		
		const inline std::string	GetFileName() { return m_fileName; }
		const inline int			GetEffectHandle() { return m_effectHandle; }
		
		// Set the diffuse texture's sampler location: all diffuse textures upon rendering will be bound to this unit //
		// Pass QRENDER_INVALID_HANDLE to not bind any textures for rendering //
		void		BindDiffuseTexture( const int& texUnit );
		void		BindNormalmapTexture( const int& texUnit );

	
	protected:
	
		friend class CModelManager;

		
		vec3f				m_modelCenter;
		vec3f				m_worldPos;
		vec3f				m_modelScale;
		mat4				m_modelPose;
	
		std::string			m_fileName;
		std::string			m_filePath;
		bool				m_bIsLoaded;
		bool				m_bIsRenderable;
		bool				m_bHasNormalmaps;
		
		int					m_effectHandle;
		
		int					m_handle;
		
		int				m_diffuseBindPoint;			// Diffuse texture's current sampler unit
		int				m_normalmapBindPoint;		// The Normalmap's current sampler unit
	
	private:

};


class QMODELOBJECTEXPORT_API CModelObjectInstance : public CModelObject
{
	public:
	
		CModelObjectInstance(const unsigned int handle, const std::string& name, const std::string& path = "./");
		~CModelObjectInstance();
		
		
		void		RenderModel();
		
		bool		IsInstance() { return true; }
	
	protected:
	
	private:
	
		friend class		CModelManager;
	
		CModelObject*		m_pRootModel;
};


struct QMODELOBJECTEXPORT_API ltstr
{
	bool operator()(std::string s1, std::string s2) const
	{
		return s1.compare(s2) < 0;
	}
};


class QMODELOBJECTEXPORT_API CModelManager
{
	public:
	
		CModelManager();
		~CModelManager();
		
		int		AddModel( const std::string& name, const std::string& path, bool loadNormalmaps = false );
		void	RemoveModel(const std::string& name, const std::string& path, const int& handle);
		
		static void	RenderVisibleModelsBSPCallback(void* self);
		void			RenderVisibleModelsBSP();

		
		CModelObject*			GetModel(const std::string& name, const std::string& path, const int& handle);
		
		const inline void		SetEffectPath( const std::string& path ) { m_effectPath = path; }
		const inline void		BindDiffuseTexture( const int& texUnit ) { m_diffuseBindPoint = texUnit; }
		const inline void		BindNormalmapTexture( const int& texUnit ) { m_normalmapBindPoint = texUnit; }
	
	private:
	
		std::map <std::string, std::vector <CModelObject*>, ltstr>	m_modelMap;
		std::string		m_effectPath;

		int				m_diffuseBindPoint;
		int				m_normalmapBindPoint;
};


#endif
