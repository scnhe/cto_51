#ifndef _MemoryMgr_hpp
#define _MemoryMgr_hpp
#include<stdlib.h>
#include<assert.h>
#include<iostream>
#ifdef _DEBUG
#include<stdio.h>
	#define xPrintf(...) printf(__VA_ARGS__)
#else
	#define xPrintf(...)
#endif // DEBUG

#define MAX_MEMORY_SIZE 64
 class MemoryAlloc;
//内存块，内存池的最小单元
class MemoryBlock
{
public:

	//所属内存池
	MemoryAlloc* pAlloc;
	//下一块地址
	MemoryBlock* pNext;
	//内存块编号
	int nID;
	//引用次数
	int nRef;
	//是否在内存池
	bool bPool;
private:
	//预留
	char c1;
	char c2;
	char c3;
};
//const int MemoryBlockSize = sizeof(MemoryBlock);

//内存池
class MemoryAlloc
{
public:
	MemoryAlloc()
	{
		_pBuf = nullptr;
		_pHeader = nullptr;
		_nSize = 0;
		_nBlockSize = 0;
	}
	~MemoryAlloc()
	{
		if (_pBuf)
			free(_pBuf);
	}

	//申请内存
	void* allocMemory(size_t nSize)
	{
		if (!_pBuf)
		{
			initMemory();
		}
		MemoryBlock * pReturn = nullptr;
		if (nullptr == _pHeader)
		{
			pReturn = (MemoryBlock *)malloc(nSize+sizeof(MemoryBlock));
			pReturn->bPool = false;
			pReturn->nID = -1;
			pReturn->nRef = 1;
			pReturn->pAlloc = nullptr;
			pReturn->pNext = nullptr;
		}
		else {
			pReturn = _pHeader;
			_pHeader = _pHeader->pNext;
			assert(0 == pReturn->nRef);
			pReturn->nRef = 1;
		}
		xPrintf("allocMem: %llx, id=%d, size=%d\n", pReturn, pReturn->nID, nSize);
		return ((char*)pReturn + sizeof(MemoryBlock));
	}
	//释放内存
	void freeMemory(void* pMem)
	{
		MemoryBlock* pBlock = (MemoryBlock*)( (char*)pMem  - sizeof(MemoryBlock));
		assert(1 == pBlock->nRef);
		if (--pBlock->nRef != 0)//多次引用
		{
			return;
		}
		if (pBlock->bPool)//在内存池内
		{
			pBlock->pNext = _pHeader;
			_pHeader = pBlock;
		}
		else//不在内存池
		{
			free(pBlock);
		}
	

	}
	//初始化内存池
	void initMemory()
	{
		assert(nullptr == _pBuf);
		
		if (!_pBuf)
		{
			return;
		}

		//计算内存池的大小
		size_t realSzie = _nSize + sizeof(MemoryBlock);
		size_t bufSize = realSzie*_nBlockSize;
		//向系统申请池的内存
		_pBuf = (char*)malloc(bufSize);

		//初始化内存池
		_pHeader = (MemoryBlock*)_pBuf;
		_pHeader->bPool = true;
		_pHeader->nID = 0;
		_pHeader->nRef = 0;
		_pHeader->pAlloc = this;
		_pHeader->pNext = nullptr;
		//遍历内存块进行初始化
		MemoryBlock* pTemp1 = _pHeader;
		for (size_t n = 1; n < _nBlockSize; n++)
		{
			MemoryBlock* pTemp2 = (MemoryBlock*)(_pBuf + (n* realSzie));
			pTemp2->bPool = true;
			pTemp2->nID = 0;
			pTemp2->nRef = 0;
			pTemp2->pAlloc = this;
			pTemp2->pNext = nullptr;
			pTemp1->pNext = pTemp2;
			pTemp1 = pTemp2;
		}
	}
protected:
	//内存池地址
	char* _pBuf;
	//头部内存单元
	MemoryBlock* _pHeader;
	//内存单元的大小
	size_t _nSize;
	//内存单元的数量
	size_t _nBlockSize;
};
template<size_t nSize,size_t nBlockSize>
class  MemoryAlloctor:public MemoryAlloc
{
public:
	MemoryAlloctor()
	{
		//处理内存对齐
		const size_t n = sizeof(void *);
		_nSize = (nSize / n)*n + (nSize%n ? n : 0);
		_nBlockSize = nBlockSize;
	}

};

 

//内存管理工具
class  MemoryMgr
{
private:
	MemoryMgr()
	{
		init(0, 64, &_mem64);
	}
	~MemoryMgr()
	{}
public:
	//单例模式
	static MemoryMgr &Instance()
	{
		static MemoryMgr mgr;
		return mgr;
	}
	//申请内存
	void* allocMem(size_t nSize)
	{
		if (nSize <= MAX_MEMORY_SIZE)
		{
			return _szAlloc[nSize]->allocMemory(nSize);
		}
		else 
		{
			MemoryBlock* pReturn = (MemoryBlock*)malloc(nSize + sizeof(MemoryBlock));
			pReturn->bPool = false;
			pReturn->nID = -1;
			pReturn->nRef = 1;
			pReturn->pAlloc = nullptr;
			pReturn->pNext = nullptr;
			std::cout << pReturn->nID << " " << nSize << std::endl;
			xPrintf("allocMem: %llx, id=%d, size=%d\n", pReturn , pReturn->nID, nSize);
			return ((char*)pReturn + sizeof(MemoryBlock));
		}

	}
	//释放内存
	void freeMem(void* pMem)
	{
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		xPrintf("freeMem: %llx, id=%d\n", pBlock, pBlock->nID);
		if (pBlock->bPool)
		{
			pBlock->pAlloc->freeMemory(pMem);
		}
		else 
		{
			if (--pBlock->nRef == 0)
				free(pBlock);
		}
	}

	//增加内存块的引用计数
	void addRef(void* pMem)
	{
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		++pBlock->nRef;
	}
private:
	//内存池映射数组初始化
	void init(int nBegin,int nEnd,MemoryAlloc *pMemA)
	{
		for (int n = nBegin; n <= nEnd; n++)
		{
			_szAlloc[n] = pMemA;
		}
		
	}

private:
	MemoryAlloctor<64, 10> _mem64;
	MemoryAlloctor<128, 10> _mem128;
	MemoryAlloc *_szAlloc[MAX_MEMORY_SIZE + 1];
};

 

#endif // !_MemoryMgr_hpp
