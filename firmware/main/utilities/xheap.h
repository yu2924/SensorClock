//
//  xheap.h
//  SensorClock
//
//  created by yu2924 on 2025-05-14
//

#pragma once

#include <memory>

namespace xheap
{
	void* memAlloc(size_t size);
	void* memRealloc(void* p, size_t new_size);
	void memFree(void* p);
	template<typename T> struct objDeleter
	{
		void operator()(T* p) const
		{
			p->~T();
			memFree(p);
		}
	};
	template<typename T> using uniquePtr = std::unique_ptr<T, objDeleter<T>>;
	template<typename T, typename... Args> uniquePtr<T> makeUnique(Args&&... args)
	{
		void* p = memAlloc(sizeof(T));
		T* pt = new(p) T(args...);
		return uniquePtr<T>(pt);
	}
	template<typename T, typename TImpl, typename... Args> uniquePtr<T> makeUnique(Args&&... args)
	{
		void* p = memAlloc(sizeof(TImpl));
		TImpl* pt = new(p) TImpl(args...);
		return uniquePtr<T>(pt);
	}
}
