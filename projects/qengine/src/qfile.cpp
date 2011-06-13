#include "qfile.h"

static bool GetFileAccessMode(unsigned int access, std::string& accessModeStr)
{
	accessModeStr = "";

	if((access & QFILE_READ) && (!(access & QFILE_WRITE)))
		accessModeStr.append("r");
	else if(!(access & QFILE_READ) && (access & QFILE_WRITE))
		accessModeStr.append("w");
	else if(access & QFILE_READWRITE)
		accessModeStr.append("r+");
	else if(access & QFILE_APPEND)
		accessModeStr.append("a");
	else
		return false;

	if((access & QFILE_TYPE_BINARY) && (!(access & QFILE_TYPE_PLAINTEXT)))
		accessModeStr.append("b");
	else if(!(access & QFILE_TYPE_BINARY) && (access & QFILE_TYPE_PLAINTEXT))
		accessModeStr.append("t");
	else if(access & QFILE_TYPE_UNKNOWN)
		return false;
	else 
		return false;
}

static unsigned int GetFileType(unsigned int flags)
{
	if(flags & QFILE_TYPE_UNKNOWN) return QFILE_TYPE_UNKNOWN;
	else if(flags & QFILE_TYPE_BINARY) return QFILE_TYPE_BINARY;
	else if(flags & QFILE_TYPE_PLAINTEXT) return QFILE_TYPE_PLAINTEXT;
	else return QFILE_TYPE_UNKNOWN;
}


CFile::CFile()
{
	fileFlags = 0;
	cacheSize = 0;
	file = NULL;
	isOpen = false;
	fileCache = NULL;
	isCached = false;
	fileType = QFILE_TYPE_UNKNOWN;
}

CFile::CFile(const std::string& fName, unsigned int flags)
{
	fileName = fName;
	cacheSize = 0;
	fileFlags = flags;
	fileCache = NULL;
	file = NULL;
	isCached = false;
	isOpen = false;
	fileType = GetFileType(flags);
}


CFile::~CFile()
{
	if(isOpen)
	{
		fclose(file);
		isOpen = false;
		file = NULL;
	}

	QMem_SafeDeleteArray(fileCache);
	cacheSize = 0;
	fileName.clear();

	isCached = false;
}

void CFile::SetFileName(const std::string& fName)
{
	fileName = fName;
}


bool CFile::OpenFile(unsigned int access)
{
	if(!access)
		return false;

	std::string accessModeStr;
	if(!GetFileAccessMode(access, accessModeStr))
		return false;

	if((file = fopen(fileName.c_str(), accessModeStr.c_str())) == NULL)
		return false;

	isOpen = true;
	return true;
}


void CFile::CloseFile()
{
	if(isOpen)
	{
		fclose(file);
		isOpen = false;
	}
}


bool CFile::CacheFile()
{
	// Check that the file is open, if not open it //
	if(!isOpen)
		if(!OpenFile(QFILE_READ | fileType))
			return false;

	// Get the current position pointer and if its not valid, close 
	// the file and bail.
	long pos = ftell(file);
	if(pos < 0)
	{
		CloseFile();
		return false;
	}

	// If its advanced, move it to the beginning of file
	if(pos > 0)
		fseek(file, 0, SEEK_SET);

	// seek to the end of the file and get file size
	pos = fseek(file, 0, SEEK_END);
	cacheSize = ftell(file);
	fileCache = new char[cacheSize];

	// reset position ptr to the beginning of file and read the contents 
	// into memory
	fseek(file, 0, SEEK_SET);
	if(fread(fileCache, cacheSize, 1, file) != 1)
	{
		QMem_SafeDeleteArray(fileCache);
		CloseFile();
		cacheSize = 0;
		return false;
	} 

	// close the file and bail
	CloseFile();
	isCached = true;
	return true;
}





CConfigFile::CConfigFile() : CFile()
{
	
}

CConfigFile::CConfigFile(const std::string& fName) : CFile(fName, QFILE_READ | QFILE_TYPE_BINARY)
{
	
}

CConfigFile::~CConfigFile()
{	
	CFile::~CFile();
}


bool CConfigFile::OpenFile()
{
	if(!isCached)
		if(!CacheFile())
			return false;

	long bytesRead = 0;
	void* start = (void*)fileCache;
	void* end = 0;
	do
	{
		// look for first occurences of new line characters 
		end = memchr(start, '\n', cacheSize - bytesRead);
		int stringLength = (char*)end - (char*)start;	
		if(stringLength < 0)
		{
			end = (char*)fileCache + cacheSize;
			stringLength = (char*)end - (char*)start;
		}
		bytesRead += stringLength + 1;

		// Copy the line into the new string //
		char* str = new char[stringLength + 1];
		memcpy(str, start, stringLength);
		str[stringLength] = '\0';

		// Find the first '=' character, if its not found then continue on
		// we can not cache this value
		char* eqLoc = strchr(str, '=');
		if(!eqLoc)
			continue;
			
		int eqOffs = eqLoc - str;
		std::vector <std::string> lineSplit;
		lineSplit = Split("=", std::string(str), 1);
		std::string var = lineSplit[0];
		std::string value = lineSplit[1];
		configVars[var] = value;

		start = (char*)end + 1;
	}while(bytesRead <= cacheSize);
	return true;
}


bool CConfigFile::QueryBool(char* varName)
{
	char* lCase = (char*)configVars[varName].c_str();
	ToLower(lCase);
	if(strcmp(lCase, "true") == 0)
		return true;
	return false;
}

void CConfigFile::WriteValue(char* varName, char* val)
{
	configVars[varName] = val;
}

void CConfigFile::WriteValue(char* varName, bool val)
{
	if(val)
		configVars[varName] = "true";
	else
		configVars[varName] = "false";
}

void CConfigFile::WriteValue(char* varName, int val)
{
	char num[10];
	itoa(val, num, 10);
	configVars[varName] = num;
}

bool CConfigFile::SaveChanges()
{
	if(!isOpen)
	{
		if(!CFile::OpenFile(QFILE_TYPE_PLAINTEXT | QFILE_WRITE))
			return false;
	}

	else
	{
		CloseFile();
		if(!CFile::OpenFile(QFILE_TYPE_PLAINTEXT | QFILE_WRITE))
			return false;
	}

	for(unsigned i = 0; i < configVars.Size(); ++i)
		fprintf(file, "%s=%s\n", configVars.GetIndexName(i).c_str(), configVars[i].c_str());

	CloseFile();
	return true;
}