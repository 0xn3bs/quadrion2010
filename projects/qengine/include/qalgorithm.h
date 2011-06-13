#ifndef __QALGORITHM_H_
#define __QALGORITHM_H_

#include <vector>

#ifdef QRENDER_EXPORTS
#define QALGORITHMEXPORT_API		__declspec(dllexport)
#else
#define QALGORITHMEXPORT_API		__declspec(dllimport)
#endif

template <class T>
class QALGORITHMEXPORT_API CTreeNode
{
public:
	CTreeNode()
	{

	}

	CTreeNode(T* pData)
	{
		m_pData = pData;
	}

	~CTreeNode()
	{
		m_vChildNodes.clear();
	}

	void AppendChildNode(CTreeNode<T> pChild)
	{
		m_vChildNodes.push_back(pChild);
	}

	void Clear()
	{
		for(unsigned int i = 0; i < m_vChildNodes.size(); ++i)
			m_vChildNodes[i].Clear();
		m_vChildNodes.clear();
	}

	T* GetData()
	{
		return m_pData;
	}

	std::vector<CTreeNode<T>>* GetChildNodes()
	{
		return &m_vChildNodes;
	}

private:
	std::vector<CTreeNode<T>> m_vChildNodes;
	T* m_pData;
};

template <class T>
class QALGORITHMEXPORT_API CTree
{
public:
	CTree()
	{

	}

	~CTree()
	{
		Clear();
	}

	void AppendRootNode(CTreeNode<T> pRoot)
	{
		m_pRootNodes.push_back(pRoot);
	}
	
	void Clear()
	{
		for(unsigned int i = 0; i < m_pRootNodes.size(); ++i)
			m_pRootNodes[i].Clear();
		m_pRootNodes.clear();
	}

	std::vector<CTreeNode<T>>* GetRootNodes()
	{
		return &m_pRootNodes;
	}

	std::vector<CTreeNode<T>> m_pRootNodes;
};

#endif /*__QALGORITHM_H_*/
