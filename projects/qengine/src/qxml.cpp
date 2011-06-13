#include "qxml.h"

#include <fstream>
#include <cassert>
#include <iostream>
#include <sstream>

const CXMLNode* CXMLNodeHandler::GetNode(const std::string &name, const unsigned int instance) const
{
	unsigned int count = -1;

	for(unsigned int i = 0; i < m_vNodes.size(); ++i)
	{
		if(m_vNodes[i]->GetName().compare(name)==0)
			++count;

		if(count == instance)
			return m_vNodes[i];
	}

	return NULL;
}

const unsigned int CXMLNodeHandler::GetNumNodes(const std::string& name) const
{
	unsigned int count = 0;

	for(unsigned int i = 0; i < m_vNodes.size(); ++i)
	{
		if(m_vNodes[i]->GetName().compare(name)==0)
			++count;
	}

	return count;
}

void CXMLNodeHandler::AddNode(CXMLNode* pNode)
{
	if(pNode)
		m_vNodes.push_back(pNode);
}

CXML::CXML()
{
	m_pCurrentNode = NULL;
}

CXML::CXML(std::string path)
{
	m_pCurrentNode = NULL;
	this->Read(path.c_str());
	this->Parse();
}

CXML::CXML(const char* path)
{
	m_pCurrentNode = NULL;
	this->Read(path);
	this->Parse();
}

CXML::~CXML()
{
	m_pCurrentNode = NULL;
	if(m_vNodes.size() > 0)
		m_vNodes.clear();
}

bool CXML::IsInQuotes(std::vector<size_t> &quoteList, size_t index)
{
	size_t fq, sq;
	fq = sq = std::string::npos;

	for(size_t i = 0; i < quoteList.size(); ++i)
	{
		if(quoteList[i] < index)
			fq = i+1;

		if(quoteList[i] > index)
		{
			sq = i+1;
			break;
		}
	}

	if(fq != std::string::npos && sq != std::string::npos && fq % 2 != 0 && sq % 2 == 0)
		return true;

	return false;
}

std::vector<size_t> CXML::BuildQuoteList(std::string &line)
{
	std::vector<size_t> vecQuoteList;

	size_t iqPos = line.find('"');

	while(iqPos != std::string::npos)
	{
		vecQuoteList.push_back(iqPos);
		iqPos = line.find('"', iqPos+1);
	}

	return vecQuoteList;
}

const unsigned int CXML::Read(const char *path)
{
	std::fstream fsFile(path, std::fstream::in);

	if(!fsFile.is_open())
	{
		m_bIsValidFile = false;
		return QXML_LOAD_FAIL;
	}

	std::string strTmpLine;

	while(std::getline(fsFile, strTmpLine))
		if(strTmpLine.size()>0)
			m_vRawData.push_back(strTmpLine);

	fsFile.close();

	//	Trim leading whitespace characters.
	for(unsigned int i = 0; i < m_vRawData.size(); ++i)
	{
		size_t iWhiteSpace = m_vRawData[i].find_first_of(' ');

		if(iWhiteSpace != 0)
			iWhiteSpace = m_vRawData[i].find_first_of('\t');

		if(iWhiteSpace == 0)
		{
			m_vRawData[i] = m_vRawData[i].substr(iWhiteSpace+1, m_vRawData[i].size()-iWhiteSpace-1);
			--i;
		}
	}

	for(unsigned int i = 0; i < m_vRawData.size(); ++i)
	{
		std::vector<size_t> vecQuoteList = BuildQuoteList(m_vRawData[i]);

		size_t iRightArrow = m_vRawData[i].find_first_of('>');

		while(IsInQuotes(vecQuoteList, iRightArrow))
			iRightArrow = m_vRawData[i].find_first_of('>', iRightArrow+1);

		if(iRightArrow != m_vRawData[i].size()-1 && iRightArrow != -1)
		{
			std::string strToken1 = m_vRawData[i].substr(0, iRightArrow+1);
			std::string strToken2 = m_vRawData[i].substr(iRightArrow+1, m_vRawData[i].size()-iRightArrow-1);

			m_vRawData[i] = strToken1;

			if(i+1 < m_vRawData.size())
				m_vRawData.insert(m_vRawData.begin()+i+1, strToken2);
			else
				m_vRawData.push_back(strToken2);
			--i;
		}

		vecQuoteList = BuildQuoteList(m_vRawData[i]);
		size_t larrow = m_vRawData[i].find_first_of('<');

		while(IsInQuotes(vecQuoteList, larrow))
			larrow = m_vRawData[i].find_first_of('<', larrow+1);

		if(larrow > 0 && larrow != std::string::npos)
		{
			std::string strToken1 = m_vRawData[i].substr(0, larrow);
			std::string strToken2 = m_vRawData[i].substr(larrow, m_vRawData[i].size()-larrow);

			m_vRawData[i] = strToken1;

			if(i+1 < m_vRawData.size())
				m_vRawData.insert(m_vRawData.begin()+i+1, strToken2);
			else
				m_vRawData.push_back(strToken2);
			--i;
		}
	}

	return QXML_LOAD_OK;
}

const unsigned int CXML::Parse()
{
	//	Get elements
	bool bLastBlockClosed = false;
	for(unsigned int i = 0; i < m_vRawData.size(); ++i)
	{
		//	Don't worry about XML header info for now.
		if(i == 0)
		{
			size_t iHeader = m_vRawData[i].find("<\?");
			if(iHeader == 0)
				continue;
		}
		
		//	Is this a beginning/ending of a block?
		size_t iStartingBlock = m_vRawData[i].find("<");
		size_t iEndingBlock = m_vRawData[i].find("</");
		size_t iStartingAndEndingBlock = m_vRawData[i].find("/>");

		std::string strElementName;

		//	Get the element's name.
		//	Starting block
		if(iStartingBlock == 0 && iEndingBlock == std::string::npos && iStartingAndEndingBlock == std::string::npos)
		{
			size_t iFirstSpace = m_vRawData[i].find_first_not_of(' ', 1);
			size_t iSecondSpace = m_vRawData[i].find(' ', m_vRawData[i].find_first_not_of(' ', 1));

			if(iFirstSpace < iSecondSpace)
			{
				if(iFirstSpace != std::string::npos && iSecondSpace != std::string::npos)
					strElementName = m_vRawData[i].substr(iFirstSpace, iSecondSpace-iFirstSpace);
			}

			if(iSecondSpace == std::string::npos)
				strElementName = m_vRawData[i].substr(iFirstSpace, m_vRawData[i].find_last_not_of(' ') - 1);

			CXMLNode* pNode = new CXMLNode();
			pNode->SetStartLine(i);
			pNode->SetEndLine(0);
			pNode->SetName(strElementName);

			if(m_pCurrentNode==NULL)
			{
				pNode->SetParentNode(NULL);
				m_vNodes.push_back(pNode);
				m_pCurrentNode = pNode;
			}
			else
			{
				if(bLastBlockClosed)
				{
					if(m_pCurrentNode->GetParentNode())
					{
						pNode->SetParentNode(m_pCurrentNode->GetParentNode());
						m_pCurrentNode->GetParentNode()->m_vNodes.push_back(pNode);
					}
					else
					{
						pNode->SetParentNode(NULL);
						m_vNodes.push_back(pNode);
					}
				}
				else
				{
					pNode->SetParentNode(m_pCurrentNode);
					m_pCurrentNode->m_vNodes.push_back(pNode);
				}

				m_pCurrentNode = pNode;
			}
			
			bLastBlockClosed = false;

			continue;
		}

		//	Starting block and ending block
		if(iStartingBlock == 0 && iStartingAndEndingBlock != std::string::npos)
		{
			size_t iFirstSpace = m_vRawData[i].find_first_not_of(' ', 1);
			size_t iSecondSpace = m_vRawData[i].find(' ', m_vRawData[i].find_first_not_of(' ', 1));

			if(iFirstSpace < iSecondSpace)
				if(iFirstSpace != std::string::npos && iSecondSpace != std::string::npos)
					strElementName = m_vRawData[i].substr(iFirstSpace, iSecondSpace-iFirstSpace);

			if(iSecondSpace == std::string::npos)
				strElementName = m_vRawData[i].substr(iFirstSpace, m_vRawData[i].find_last_not_of(' ') - 1);

			CXMLNode* pNode = new CXMLNode();
			pNode->SetStartLine(i);
			pNode->SetEndLine(i);
			pNode->SetName(strElementName);

			if(m_pCurrentNode == 0)
			{
				pNode->SetParentNode(NULL);
				m_vNodes.push_back(pNode);
				m_pCurrentNode = pNode;
			}
			else
			{
				pNode->SetParentNode(m_pCurrentNode);

				if(bLastBlockClosed)
				{
					pNode->SetParentNode(m_pCurrentNode->GetParentNode());
					m_pCurrentNode->GetParentNode()->m_vNodes.push_back(pNode);
				}
				else
				{
					pNode->SetParentNode(m_pCurrentNode);
					m_pCurrentNode->m_vNodes.push_back(pNode);
				}

				m_pCurrentNode = pNode;
			}

			bLastBlockClosed = true;

			continue;
		}

		//	Ending block
		if(iEndingBlock == 0 && iStartingAndEndingBlock == std::string::npos)
		{
			size_t iFirstSpace = m_vRawData[i].find_first_not_of(' ', 2);
			size_t iSecondSpace = m_vRawData[i].find(' ', m_vRawData[i].find_first_not_of(' ', 2));

			if(iFirstSpace < iSecondSpace)
				if(iFirstSpace != std::string::npos && iSecondSpace != std::string::npos)
					strElementName = m_vRawData[i].substr(iFirstSpace, iSecondSpace-iFirstSpace);
			
			if(iSecondSpace == std::string::npos)
				strElementName = m_vRawData[i].substr(iFirstSpace, m_vRawData[i].find_last_not_of(' ') - 2);

			if(m_pCurrentNode->GetParentNode() != NULL && bLastBlockClosed)
				m_pCurrentNode->GetParentNode()->SetEndLine(i);
			else
			if(!bLastBlockClosed)
				m_pCurrentNode->SetEndLine(i);

			if(bLastBlockClosed)
				m_pCurrentNode=m_pCurrentNode->GetParentNode();

			if(bLastBlockClosed && m_pCurrentNode->GetParentNode() == NULL)
				m_pCurrentNode=NULL;

			bLastBlockClosed = true;
			continue;
		}
	}

	for(unsigned int i = 0; i < m_vNodes.size(); ++i)
		PopulateNodeContent(m_vNodes[i]);

	for(unsigned int i = 0; i < m_vNodes.size(); ++i)
		ParseAttributes(m_vNodes[i]);

	m_vRawData.clear();

	return QXML_OK;
}

void CXMLNode::TabFromLevels(std::fstream &output_ss, unsigned int levelsDeep)
{
	while(levelsDeep > 0)
		output_ss << "\t", levelsDeep--;
}

void CXMLNode::TraverseNodes(std::vector<CXMLNode*> &nodes, std::fstream &output_ss, unsigned int &levelsDeep)
{
	levelsDeep++;
	for(unsigned int i = 0; i < m_vNodes.size(); ++i)
	{
		int nodeCount = m_vNodes[i]->m_vNodes.size();

		TabFromLevels(output_ss, levelsDeep);
		output_ss << "<" + m_vNodes[i]->GetName();

		HashMapStrings& attributes = m_vNodes[i]->GetAttributes();

		for(HashMapStrings::iterator iter = attributes.begin(); iter != attributes.end(); ++iter) 
			output_ss << " " << iter->first << "=\"" << iter->second << "\"";

		std::vector<std::string> &contents = m_vNodes[i]->GetContents();

		if(nodeCount <= 0 && contents.size() <= 0)
			output_ss << " />\n";
		else
			output_ss << ">\n";

		if(contents.size() > 0)
			for(unsigned int j = 0; j < contents.size(); ++j)
				output_ss << contents[j] << std::endl;

		if(nodeCount > 0)
			m_vNodes[i]->TraverseNodes(m_vNodes[i]->m_vNodes, output_ss, levelsDeep);

		if(nodeCount > 0 || contents.size() > 0)
		{
			TabFromLevels(output_ss, levelsDeep);
			output_ss << "</" << m_vNodes[i]->GetName() << ">" << std::endl;
		}
	}
	levelsDeep--;
}

const unsigned int CXML::Write(const char* path)
{
	m_uiLevelsDeep = 0;

	std::fstream fsFile(path, std::fstream::out);

	if(!fsFile.is_open())
	{
		m_bIsValidFile = false;
		return QXML_LOAD_FAIL;
	}

	for(unsigned int i = 0; i < m_vNodes.size(); ++i)
	{
		fsFile << "<" << m_vNodes[i]->GetName() << ">" << std::endl;
		m_vNodes[i]->TraverseNodes(m_vNodes[i]->m_vNodes, fsFile, m_uiLevelsDeep);
		fsFile << "</" << m_vNodes[i]->GetName() << ">" << std::endl;
	}

	return QXML_OK;
}

CXMLNode* CXML::ResolveURI(std::string &identifier)
{
	if(identifier[0] == '#')
		identifier = identifier.substr(1, identifier.size()-1);
	std::map<std::string,CXMLNode*>::iterator it = m_mURIs.find(identifier);
	return (*it).second;
}

std::string CXML::ReplaceAll(std::string &str, std::string delimiter, std::string val)
{
	if(delimiter.compare(val) == 0)
		return str;

	std::string strResult = str;
	size_t iDelim = strResult.find(delimiter);
	while(iDelim != std::string::npos)
	{
		if(val.size() > 0)
			if(val.size()>delimiter.size())
				strResult.replace(iDelim,val.size(),val);
			else
				strResult.replace(iDelim,delimiter.size(),val);
		else
			strResult.erase(iDelim, 1);
		iDelim = strResult.find(delimiter);
	}

	return strResult;
}

void CXML::PopulateNodeContent(CXMLNode* node)
{
	if(node->m_vNodes.size() == 0)
		for(size_t i = node->GetStartLine()+1; i < node->GetEndLine(); ++i)
			node->GetContents().push_back(m_vRawData[i]);
	else
	{
		for(size_t i = 0; i < node->m_vNodes.size(); ++i)
		{
			std::vector<CXMLNode*>* pNode = &node->m_vNodes;
			PopulateNodeContent(pNode->at(i));
		}
	}
}

const unsigned int CXML::CountChar(std::string &str, const char chr)
{
	int num = 0;
	for(unsigned int i = 0; i < str.size(); ++i)
		if(str[i]==chr)
			++num;

	return num;
}

void CXML::ParseAttributes(CXMLNode* node)
{
	std::string strLine = m_vRawData[node->GetStartLine()];
	strLine = strLine.substr(node->GetName().size()+2, strLine.size() - node->GetName().size()-3);

	for(unsigned int i = 0; i < node->m_vNodes.size(); ++i)
		ParseAttributes(node->m_vNodes.at(i));

	if(strLine.size()==0)
		return;

	size_t iEqualSign = strLine.find('=');

	if(CountChar(strLine, '\"') % 2 != 0 || CountChar(strLine, '\"') == 0)
	{
		std::cerr << "Invalid XML detected, misplaced greater-than sign." << std::endl;
		return;
	}

	while(iEqualSign != std::string::npos)
	{
		if(iEqualSign > strLine.find('"'))
			continue;

		std::string strAttributeName;
		strAttributeName = strLine.substr(0, iEqualSign);
		strAttributeName = ReplaceAll(strAttributeName, " ", "");
		strAttributeName = ReplaceAll(strAttributeName, "\t", "");
		
		std::string strAttributeValue;
		unsigned int nq  = 0;
		unsigned int sqi = 0;

		for(sqi = 0; sqi < strLine.size(); ++sqi)
		{
			if(strLine[sqi] == '"')
				++nq;

			if(nq==2)
				break;
		}

		strAttributeValue = strLine.substr(iEqualSign+2, sqi-iEqualSign-2);

		if(strAttributeName.compare("id") == 0)
			m_mURIs.insert(std::pair<std::string, CXMLNode*>(strAttributeValue, node));
		
		node->GetAttributes().insert(std::pair<std::string, std::string>(strAttributeName, strAttributeValue));

		if(sqi<strLine.size()-1)
			strLine = strLine.substr(sqi+2, strLine.size()-sqi-2);
		else
			strLine = "";

		iEqualSign = strLine.find('=');

		if(strLine.find("\"")<iEqualSign)
		{
			std::cerr << "Invalid XML detected, misplaced quote." << std::endl;
			break;
		}
	}
}	
