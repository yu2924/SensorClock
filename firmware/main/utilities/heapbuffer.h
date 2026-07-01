//
//  heapbuffer.h
//  SensorClock
//
//  created by yu2924 on 2026-05-24
//

#pragma once

#include <cstdlib>
#include <cstdint>
#include <cstring>
#include "utilities/xheap.h"

template<typename T> struct HeapBufferT
{
	T* ptrBuffer = nullptr;
	size_t lenBuffer = 0;
	size_t lenCont = 0;
	HeapBufferT(const HeapBufferT&) = delete;
	HeapBufferT& operator=(const HeapBufferT&) = delete;
	HeapBufferT() = default;
	~HeapBufferT() { xheap::memFree(ptrBuffer); }
	size_t size() const { return lenCont; }
	const T* data() const { return ptrBuffer; }
	T* data() { return ptrBuffer; }
	const T& operator[](size_t i) const { return ptrBuffer[i]; }
	T& operator[](size_t i) { return ptrBuffer[i]; }
	const T* begin() const { return ptrBuffer; }
	T* begin() { return ptrBuffer; }
	const T* end() const { return ptrBuffer + lenCont; }
	T* end() { return ptrBuffer + lenCont; }
	bool empty() const { return size() == 0; }
	void clear() { resize(0); }
	bool reserve(size_t ct)
	{
		if(ct <= lenBuffer) { return true; }
		T* pt = (T*)xheap::memRealloc(ptrBuffer, ct * sizeof(T));
		if(!pt) { return false; }
		ptrBuffer = pt;
		lenBuffer = ct;
		return true;
	}
	void shrinkToFit()
	{
		if(0 < lenCont)
		{
			if(T* pt = (T*)xheap::memRealloc(ptrBuffer, lenCont * sizeof(T)))
			{
				ptrBuffer = pt;
				lenBuffer = lenCont;
			}
		}
		else if(ptrBuffer)
		{
			xheap::memFree(ptrBuffer);
			ptrBuffer = nullptr;
			lenBuffer = 0;
		}
	}
	bool resize(size_t ct)
	{
		if(!reserve(ct)) { return false; }
		lenCont = ct;
		return true;
	}
	bool append(const T* pt, size_t ct)
	{
		size_t ctprev = size();
		if(!resize(ctprev + ct)) { return false; }
		memcpy(data() + ctprev, pt, ct * sizeof(T));
		return true;
	}
};
