#ifdef QRENDER_EXPORTS
	#define QFILEEXPORT_API __declspec(dllexport)
#else
	#define QFILEEXPORT_API __declspec(dllimport)
#endif


#ifndef __QFILE_H_
#define __QFILE_H_

#include <string>
#include "qmem.h"
#include "qhash.h"
#include "qtext.h"



#define QFILE_TYPE_UNKNOWN		0x00000001
#define QFILE_TYPE_BINARY		0x00000002
#define QFILE_TYPE_PLAINTEXT	0x00000004
#define QFILE_READ				0x00000008
#define QFILE_WRITE				0x00000010
#define QFILE_READWRITE			(QFILE_READ | QFILE_WRITE)
#define QFILE_APPEND			0x00000020







class QFILEEXPORT_API CFile
{
	public:
		
		CFile();
		CFile(const std::string& fName, unsigned int flags = 0);
		virtual ~CFile();


		virtual bool OpenFile(unsigned int access = 0);
		void CloseFile();
		bool CacheFile();


		void				SetFileName(const std::string& fName);
		const inline bool	IsOpen() { return isOpen; }
		const inline bool	IsCached() { return isCached; }
		const inline long	GetCacheSize() { return cacheSize; }

	protected:

		unsigned int	fileFlags;
		unsigned int	fileType;

		std::string		fileName;
		FILE*			file;

		bool			isOpen;
		bool			isCached;

		long			cacheSize;
		char*			fileCache;

	private:
		
};



class QFILEEXPORT_API CConfigFile : public CFile
{
	public:

		CConfigFile();
		CConfigFile(const std::string& fName);
		~CConfigFile();


		bool QueryBool(char* varName);
		void WriteValue(char* varName, char* val);
		void WriteValue(char* varName, bool val);
		void WriteValue(char* varName, int val);

		bool OpenFile();
		bool SaveChanges();

	protected:

	private:
		
		CHashTable <std::string>	configVars;
};


#endif