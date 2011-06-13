////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// XML.H
// 
// Written by Jonathan Bastnagel for Quadrion Engine 12/2008
//
// Used for XML interaction by the engine.
// The COLLADA model implementation is highly dependent on this class.
//
// Known issues/todo:
// TODO: If it's invalid XML don't get stuck in an infinite loop or crash.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __QXMLPARSER_H_
#define __QXMLPARSER_H_

#include <vector>
#include <hash_map>
#include <string>
#include <map>
#include <sstream>

const unsigned int QXML_LOAD_OK				= 0x000000;
const unsigned int QXML_LOAD_FAIL			= 0x000001;
const unsigned int QXML_OK					= 0x000002;
const unsigned int QXML_PARSE_ERROR_UNKNOWN	= 0x000004;

#ifdef QRENDER_EXPORTS
#define QXML_API		__declspec(dllexport)
#else
#define QXML_API		__declspec(dllimport)
#endif

typedef stdext::hash_map <std::string, std::string> HashMapStrings;

//	Forward declaration
class CXMLNode;

class QXML_API CXMLNodeHandler
{
public:
	friend class CXML;
	const CXMLNode* GetNode(const std::string& name, const unsigned int instance = 0) const;
	const unsigned int GetNumNodes(const std::string& name) const;

	void AddNode(CXMLNode* pNode);
protected:
	std::vector<CXMLNode*> m_vNodes;
};

class QXML_API CXMLNode : public CXMLNodeHandler
{
public:
	friend class CXML;

	CXMLNode()
	{
		m_strName = "";
		m_pParentNode = NULL;
		m_iStartLine = 0;
		m_iEndLine = 0;
	}

	~CXMLNode()
	{
		m_strName = "";
		m_pParentNode = NULL;
		m_iStartLine = 0;
		m_iEndLine = 0;
		m_vNodes.clear();
		m_mAttributes.clear();
		m_vContents.clear();
	}

	void SetStartLine(const int startLine){this->m_iStartLine = startLine;}
	void SetEndLine(const int endLine){this->m_iEndLine = endLine;}
	void SetName(std::string name){this->m_strName = name;}
	void SetParentNode(CXMLNode* parentNode){this->m_pParentNode = parentNode;}

	std::string GetAttributeValue(std::string key) const
	{
		HashMapStrings::const_iterator pIter;

		pIter = m_mAttributes.find(key);

		if(pIter == m_mAttributes.end())
			return "";

		return pIter->second;
	}

	const size_t GetStartLine(){return m_iStartLine;}
	const size_t GetEndLine(){return m_iEndLine;}
	std::string& GetName(){return m_strName;}
	CXMLNode* const GetParentNode(){return m_pParentNode;}

	void AddAttribute(std::string name, std::string val){m_mAttributes[name] = val;}
	void AddContents(std::string content){m_vContents.push_back(content);}
	
	HashMapStrings& GetAttributes() const {return (HashMapStrings&)m_mAttributes;}
	std::vector<std::string>& GetContents() const {return (std::vector<std::string>&)m_vContents;}

private:
	std::string m_strName;
	CXMLNode* m_pParentNode;
	size_t m_iStartLine;
	size_t m_iEndLine;
	HashMapStrings m_mAttributes;
	std::vector<std::string> m_vContents;

protected:	
	void TraverseNodes(std::vector<CXMLNode*> &nodes, std::fstream &output_ss, unsigned int &levelsDeep);
	void TabFromLevels(std::fstream &output_ss, unsigned int levelsDeep);
};

class QXML_API CXML : public CXMLNodeHandler
{
public:
	CXML();
	CXML(const char* path);
	CXML(std::string path);
	~CXML();
	
	const unsigned int Read(const char* path);
	const unsigned int Parse();

	const unsigned int Write(const char* path);
	CXMLNode* ResolveURI(std::string &identifier);
	bool IsValidFile(){return m_bIsValidFile;}

private:
	std::vector<std::string> m_vRawData;
	CXMLNode* m_pCurrentNode;
	std::string ReplaceAll(std::string& str, std::string delimiter, std::string val);
	const unsigned int CountChar(std::string& str, const char chr);
	void PopulateNodeContent(CXMLNode* node);
	void ParseAttributes(CXMLNode* node);
	bool IsInQuotes(std::vector<size_t> &quoteList, size_t index);
	std::vector<size_t> BuildQuoteList(std::string &line);
	bool m_bIsValidFile;

	std::map<std::string, CXMLNode*> m_mURIs;

	unsigned int m_uiLevelsDeep;
};

#define XML_QUOTE			"&quot;"
#define XML_LESSTHANSIGN	"&lt;"
#define XML_GREATERTHANSIGN "&gt;"

#endif
