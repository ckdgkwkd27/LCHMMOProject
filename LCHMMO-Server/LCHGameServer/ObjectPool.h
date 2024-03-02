#pragma once
#define MAX_POOL_SIZE 4096

template<typename T>
class ObjectPool
{
public:
	ObjectPool(uint32 _maxSize = MAX_POOL_SIZE);
	~ObjectPool();

	T* BorrowObject();
	void ReturnObject(T* _object);
	void ExpandPool();

private:
	RecursiveMutex poolLock;
	std::stack<T*> pool;
	uint32 maxSize;
};

template<typename T>
inline ObjectPool<T>::ObjectPool(uint32 _maxSize)
{
	maxSize = _maxSize;

	for (uint32 i = 0; i < maxSize; i++)
	{
		T* _object = new T();
		pool.push(_object);
	}
}

template<typename T>
inline ObjectPool<T>::~ObjectPool()
{
	RecursiveLockGuard poolGuard(poolLock);
	
	while (!pool.empty())
	{
		T* _object = pool.top();
		pool.pop();
		delete _object;
	}

	maxSize = 0;
}

template<typename T>
T* ObjectPool<T>::BorrowObject()
{
	RecursiveLockGuard poolGuard(poolLock);

	if (pool.empty())
	{
		ExpandPool();
	}
	
	T* _object = pool.top();
	pool.pop();
	return _object;
}

template<typename T>
inline void ObjectPool<T>::ReturnObject(T* _object)
{
	RecursiveLockGuard poolGuard(poolLock);
	pool.push(_object);
}

template<typename T>
inline void ObjectPool<T>::ExpandPool()
{
	RecursiveLockGuard poolGuard(poolLock);

	for (uint32 i = 0; i < maxSize; i++)
	{
		T* _object = new T();
		pool.push(_object);
	}

	maxSize *= 2;
}
