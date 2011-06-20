#include "qswf.h"

#include <fstream>

#ifdef _DEBUG
#include <iostream>
#endif

SWF_FILE::SWF_FILE()
{
	m_pFileData		= NULL;
	m_byteOffset	= 0;
	m_bitOffset		= 0;
	m_uiFileSize	= 0;
	m_bLoaded		= false;
}

SWF_FILE::~SWF_FILE()
{
	if(m_pFileData)
	{
		delete m_pFileData;
		m_pFileData = NULL;
	}

	m_bLoaded = false;
}

int SWF_FILE::LoadSWF(const char* path)
{
	std::ifstream fSwfFile;
	fSwfFile.open(path, std::ifstream::in | std::ifstream::binary);

	if(!fSwfFile.is_open())
	{
#ifdef _DEBUG
		std::cerr << "Error - Failed to load SWF: \"" << path << "\"" << std::endl;
#endif
		m_bLoaded = false;
		return -1;
	}

	fSwfFile.seekg (0, std::ios::end);
	m_uiFileSize = fSwfFile.tellg();
	fSwfFile.seekg (0, std::ios::beg);

	m_pFileData = new char[m_uiFileSize];
	fSwfFile.read(m_pFileData, m_uiFileSize);
	fSwfFile.close();

	m_bLoaded = true;

	return 0;
}

SWF::SWF()
{
	m_bIsEnd		= false;

	m_pFile			= NULL;
	m_pHeader		= NULL;
	m_pAttributes	= NULL;
	m_pSceneAndFrameLabelData = NULL;
}

SWF::~SWF()
{
	if(m_pFile)
	{
		delete m_pFile;
		m_pFile = NULL;
	}

	if(m_pHeader)
	{
		if(m_pHeader->rect)
		{
			delete m_pHeader->rect;
			m_pHeader->rect = NULL;
		}
		delete m_pHeader;
		m_pHeader = NULL;
	}

	if(m_pAttributes)
	{
		delete m_pAttributes;
		m_pAttributes = NULL;
	}

	if(m_pSceneAndFrameLabelData)
	{
		delete m_pSceneAndFrameLabelData;
		m_pSceneAndFrameLabelData = NULL;
	}
}

int SWF::LoadSWF(const char* path)
{
	//m_pFile = new SWF_FILE;
	//m_pFile->LoadSWF(path);

	//if(!m_pFile->IsLoaded())
	//	return -1;

	//LoadHeader(m_pFile);

	//while(!m_bIsEnd)
	//	LoadTag(m_pFile);

	return 0;
}