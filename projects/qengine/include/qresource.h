#ifndef __QRESOURCE_H_
#define __QRESOURCE_H_




#include <windows.h>
#include <string>
#include <stack>
#include <vector>



const int		QRESOURCE_INVALID_RESOURCE		 = -1;



class CQuadrionResource
{
	public:
		
		CQuadrionResource(const unsigned int handle, const std::string& name, const std::string& path = "./")
		{
			if(!name.empty())
				m_name = name;
	
			if(!path.empty())
				m_path = path;
	
			m_handle = handle;
		
			if((!name.empty()) && (!path.empty()))
				m_filename = path + name;
	
			m_refCount = 1;
		}
		
		virtual ~CQuadrionResource(){}
		
		const inline std::string			GetName()	  { return m_name; }
		const inline std::string			GetPath()	  { return m_path; }
		const inline std::string			GetFilename() { return m_filename; }
		const inline unsigned int		    GetHandle()	  { return m_handle; }
		const inline unsigned long		    GetRefCount() { return m_refCount; }
		
		const inline void			IncrementRef() { m_refCount++; }
		const inline void			DecrementRef() { m_refCount--; }
	
	protected:
	
		std::string			m_filename;
		std::string			m_path;
		std::string			m_name;
		
		unsigned int				m_handle;	
		unsigned long				m_refCount;
		
	private:
		
};


template <class T>
class CQuadrionResourceManager
{
	public:
	
		// Default constructor will optionally accept custom callback for handling resource creation //
		CQuadrionResourceManager( VOID (*CreateResourceFunc)( T** resource, const unsigned int handle, const std::string& name, const std::string& path ) = NULL )
		{
			m_resources = new std::vector<T*>;
			
			CreateResource = CreateResourceFunc;
		}
		
		~CQuadrionResourceManager()
		{
			ClearList();
			delete m_resources;
		}
		
		
		void ClearList()
		{
			if(m_resources->size() <= 0)
				return;
		
			for(std::vector<T*>::iterator iter = m_resources->begin(); iter != m_resources->end(); ++iter)
			{
				if(*iter)
				{
					delete (*iter);
					(*iter) = NULL;
				}
			}
	
			while(!m_handles.empty())
			{
				m_handles.pop();
			}
	
			m_resources->clear();
			m_resources->swap(*m_resources);
		}
		
		const inline std::vector<T*>*		GetResources() { return m_resources; }
		
		T* GetResource(const std::string& name, const std::string& path = "./")
		{
			if(name.empty() || path.empty() || m_resources->empty() || m_resources == NULL)
				return NULL;
	
			for(std::vector<T*>::iterator i = m_resources->begin(); i != m_resources->end(); ++i)
			{
				if((*i))
				{
					if((*i)->GetName() == name)
						if((*i)->GetPath() == path)
							return (*i);
				}
			}
		
			return NULL;
		}
		
		
		T*	GetResource(const unsigned int& handle)
		{
			if(handle < m_resources->size() && handle >= 0)
				return (*m_resources)[handle];
	
			return NULL;
		}
		
		
		void RemoveResource(const unsigned int& handle)
		{
			// Sanity check //
			if(handle < 0 || m_resources == NULL || handle > m_resources->size() || m_resources->size() <= 0 )
				return;
		
			// Get resource handle and decrement its reference count //
			T* resource = (*m_resources)[handle];
			resource->DecrementRef();
		
			// If the resource is no longer used by anything, then it is safe to delete //
			if(resource->GetRefCount() == 0)
			{
				m_handles.push(handle);
				delete resource;
				(*m_resources)[handle] = NULL;
			}
		}
		
		unsigned int AddResource(const std::string& name, const std::string& path = "./")
		{
			if(name.empty() || path.empty() || m_resources == NULL)
			return QRESOURCE_INVALID_RESOURCE;
	
			// Look to see if this resource is already loaded //
			// Return its handle if so //
			T* elem = GetResource(name, path);
			if(elem)
			{
				elem->IncrementRef();
				return elem->GetHandle();
			}
		
			// Otherwise, look for an available handle //
			// Obtain a new one, otherwise //
			bool isHandleAvailable = !m_handles.empty();
			unsigned int handle;
			if(isHandleAvailable)
			{
				handle = m_handles.top();
				m_handles.pop();
			}
			
			else
				handle = m_resources->size();
				
				
			// Create the new resource //
			// If a custom creation function was defined for this object type then use it //
			T* resource = NULL;
			if(CreateResource != NULL)
				CreateResource(&resource, handle, name, path);
			else
				resource = new T(handle, name, path);
		
			// Add resource to manager //
			if(isHandleAvailable)
				(*m_resources)[handle] = resource;
			else
				m_resources->push_back(resource);
	
			// Give handle back //
			return handle;
		
		}
			
		T* operator[] (unsigned int handle)
		{
			if(handle < m_resources->size() && handle >= 0)
				return (*m_resources)[handle];
			return NULL;
		}
		
		const int GetResourceCount() { return m_resources->size(); }
		
	protected:
	
	private:
	
		std::stack<UINT>			m_handles;
		std::vector<T*>*			m_resources;
		
		void (*CreateResource)(T** resource, const unsigned int handle, const std::string& name, const std::string& path);
};









#endif