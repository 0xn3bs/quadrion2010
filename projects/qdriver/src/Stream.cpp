// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//							         USER STREAM
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#include <stdio.h>
#include "NxPhysics.h"
#include "Stream.h"

UserStream::UserStream(const char* filename, bool load) : fp(NULL)
{
	fp = fopen(filename, load ? "rb" : "wb");
}

UserStream::~UserStream()
{
	if(fp)	fclose(fp);
}

// Loading API
NxU8 UserStream::readByte() const
{
	NxU8 b;
	size_t r = fread(&b, sizeof(NxU8), 1, fp);
	NX_ASSERT(r);
	return b;
}

NxU16 UserStream::readWord() const
{
	NxU16 w;
	size_t r = fread(&w, sizeof(NxU16), 1, fp);
	NX_ASSERT(r);
	return w;
}

unsigned int UserStream::readDword() const
{
	unsigned int d;
	size_t r = fread(&d, sizeof(unsigned int), 1, fp);
	NX_ASSERT(r);
	return d;
}

float UserStream::readFloat() const
{
	float f;
	size_t r = fread(&f, sizeof(float), 1, fp);
	NX_ASSERT(r);
	return f;
}

double UserStream::readDouble() const
{
	NxF64 f;
	size_t r = fread(&f, sizeof(NxF64), 1, fp);
	NX_ASSERT(r);
	return f;
}

void UserStream::readBuffer(void* buffer, unsigned int size)	const
{
	size_t w = fread(buffer, size, 1, fp);
	NX_ASSERT(w);
}

// Saving API
NxStream& UserStream::storeByte(NxU8 b)
{
	size_t w = fwrite(&b, sizeof(NxU8), 1, fp);
	NX_ASSERT(w);
	return *this;
}

NxStream& UserStream::storeWord(NxU16 w)
{
	size_t ww = fwrite(&w, sizeof(NxU16), 1, fp);
	NX_ASSERT(ww);
	return *this;
}

NxStream& UserStream::storeDword(unsigned int d)
{
	size_t w = fwrite(&d, sizeof(unsigned int), 1, fp);
	NX_ASSERT(w);
	return *this;
}

NxStream& UserStream::storeFloat(float f)
{
	size_t w = fwrite(&f, sizeof(float), 1, fp);
	NX_ASSERT(w);
	return *this;
}

NxStream& UserStream::storeDouble(NxF64 f)
{
	size_t w = fwrite(&f, sizeof(NxF64), 1, fp);
	NX_ASSERT(w);
	return *this;
}

NxStream& UserStream::storeBuffer(const void* buffer, unsigned int size)
{
	size_t w = fwrite(buffer, size, 1, fp);
	NX_ASSERT(w);
	return *this;
}



MemoryWriteBuffer::MemoryWriteBuffer() : currentSize(0), maxSize(0), data(NULL)
{
}

MemoryWriteBuffer::~MemoryWriteBuffer()
{
	NxGetPhysicsSDKAllocator()->free(data);
}

NxStream& MemoryWriteBuffer::storeByte(NxU8 b)
{
	storeBuffer(&b, sizeof(NxU8));
	return *this;
}

NxStream& MemoryWriteBuffer::storeWord(NxU16 w)
{
	storeBuffer(&w, sizeof(NxU16));
	return *this;
}

NxStream& MemoryWriteBuffer::storeDword(unsigned int d)
{
	storeBuffer(&d, sizeof(unsigned int));
	return *this;
}

NxStream& MemoryWriteBuffer::storeFloat(float f)
{
	storeBuffer(&f, sizeof(float));
	return *this;
}

NxStream& MemoryWriteBuffer::storeDouble(NxF64 f)
{
	storeBuffer(&f, sizeof(NxF64));
	return *this;
}

NxStream& MemoryWriteBuffer::storeBuffer(const void* buffer, unsigned int size)
{
	unsigned int expectedSize = currentSize + size;
	if(expectedSize > maxSize)
	{
		maxSize = expectedSize + 4096;

		NxU8* newData = (NxU8*)NxGetPhysicsSDKAllocator()->malloc(maxSize, NX_MEMORY_PERSISTENT);
		if(data)
		{
			memcpy(newData, data, currentSize);
			NxGetPhysicsSDKAllocator()->free(data);
		}
		data = newData;
	}
	memcpy(data+currentSize, buffer, size);
	currentSize += size;
	return *this;
}

MemoryReadBuffer::MemoryReadBuffer(const NxU8* data) : buffer(data)
{
}

MemoryReadBuffer::~MemoryReadBuffer()
{
	// We don't own the data => no delete
}

NxU8 MemoryReadBuffer::readByte() const
{
	NxU8 b;
	memcpy(&b, buffer, sizeof(NxU8));
	buffer += sizeof(NxU8);
	return b;
}

NxU16 MemoryReadBuffer::readWord() const
{
	NxU16 w;
	memcpy(&w, buffer, sizeof(NxU16));
	buffer += sizeof(NxU16);
	return w;
}

unsigned int MemoryReadBuffer::readDword() const
{
	unsigned int d;
	memcpy(&d, buffer, sizeof(unsigned int));
	buffer += sizeof(unsigned int);
	return d;
}

float MemoryReadBuffer::readFloat() const
{
	float f;
	memcpy(&f, buffer, sizeof(float));
	buffer += sizeof(float);
	return f;
}

double MemoryReadBuffer::readDouble() const
{
	double f;
	memcpy(&f, buffer, sizeof(double));
	buffer += sizeof(double);
	return f;
}

void MemoryReadBuffer::readBuffer(void* dest, unsigned int size) const
{
	memcpy(dest, buffer, size);
	buffer += size;
}
