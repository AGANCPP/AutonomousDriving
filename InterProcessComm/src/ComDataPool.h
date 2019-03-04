

#ifndef COMMON_DATA_POOL_H
#define COMMON_DATA_POOL_H

#include "ComTools.h"
#include "ComDebug.h"

//data pool, use offset to point data
template <typename DATA_TYPE, int COUNT>
class ComDataPool02
{
public:
    enum { MAX_POOL_SIZE = 0x00FFFFFF };
    enum { NULL_PTR = 0x7FFFFFFF };
    typedef DATA_TYPE data_type;
    typedef size_t size_type;

public:
    ComDataPool02();
    bool create(void);
    bool clear(void);

    size_type allocate();
    bool deallocate(size_type offset);

    data_type* getVirtualAddr(size_type offset);
    size_type getOffset(data_type* addr);

    bool isNull(size_type offset)
    {
        return ((offset & 0x7FFFFFFF) == 0x7FFFFFFF);
    }
    //bool isValidOffset(size_type offset);
    //bool isValidPtr(type_data_ptr addr);

    void debug();

private:
    bool isUsed(size_type* poffset)
    {
        return (*poffset & 0x80000000);
    }
    bool isUnused(size_type* poffset)
    {
        return (!isUsed(poffset));
    }
    void setUsed(size_type* poffset)
    {
        *poffset |= 0x80000000;
    }
    void setUnused(size_type* poffset)
    {
        *poffset &= 0x7fffffff;
    }
    size_type getIndex(size_type offset)
    {
        return (offset & 0x7fffffff);
    }

private:
    enum { OFFSET_MASK = 0x03 };
    enum { DATA_ALIGN = sizeof(size_type) };
    enum { HEAD_SIZE = sizeof(size_type) };
    static const size_type m_poolDataSize = HEAD_SIZE + ((sizeof(DATA_TYPE) + DATA_ALIGN - 1) & ~(DATA_ALIGN - 1));
    static const size_type m_dataCount = COUNT;
    static const size_type m_poolSize = m_poolDataSize * m_dataCount;
    size_type m_free;
    char m_pool[m_poolSize];
};

//data pool, use offset to point data
template <typename DATA_TYPE, int COUNT>
ComDataPool02<DATA_TYPE, COUNT>::ComDataPool02()
{
    COM_ASSERT(m_poolSize < MAX_POOL_SIZE);
    create();
}

template <typename DATA_TYPE, int COUNT>
bool ComDataPool02<DATA_TYPE, COUNT>::create(void)
{
    char* pNextNode = (char*)m_pool;
    size_type nDataSize = m_poolDataSize;
    for (size_type i = 0; i < m_dataCount - 1; ++i)
    {
        *((size_type**)pNextNode) = (size_type*)((i + 1) * nDataSize);
        setUnused((size_type*)pNextNode);
        pNextNode += nDataSize;
    }
    *((size_type**)pNextNode) = (size_type*)NULL_PTR;
    setUnused((size_type*)pNextNode);
    m_free = 0;
    COM_DEBUG(DEBUG_LEVEL3, ("m_poolDataSize=%d m_dataCount=%d m_poolSize=%d m_pool=%p\n", m_poolDataSize, m_dataCount, m_poolSize, m_pool));
    return (true);
}

template <typename DATA_TYPE, int COUNT>
bool ComDataPool02<DATA_TYPE, COUNT>::clear(void)
{
    this->create();
    return (true);
}

template <typename DATA_TYPE, int COUNT>
typename ComDataPool02<DATA_TYPE, COUNT>::size_type
ComDataPool02<DATA_TYPE, COUNT>::allocate()
{
    size_type offset = m_free;
    if (!isNull(m_free))
    {
        char* pAddr = (char*)((size_type)m_pool + offset);
        m_free = getIndex((size_type) * ((size_type*)pAddr));
        setUsed((size_type*)pAddr);
    }
    return (offset);
}

template <typename DATA_TYPE, int COUNT>
bool ComDataPool02<DATA_TYPE, COUNT>::deallocate(size_type offset)
{
    if (isNull(offset))
        return (false);
    COM_ASSERT_RET((offset < m_poolSize), false);
    COM_ASSERT_RET((offset / m_poolDataSize)*m_poolDataSize == offset, false);
    char* pAddr = (char*)((size_type)m_pool + offset);
    if (isUnused((size_type*)pAddr))
        return (false);
    *((size_type**)pAddr) = (size_type*)m_free;
    setUnused((size_type*)pAddr);
    m_free = offset;
    return (true);
}

template <typename DATA_TYPE, int COUNT>
typename ComDataPool02<DATA_TYPE, COUNT>::data_type*
ComDataPool02<DATA_TYPE, COUNT>::getVirtualAddr(size_type offset)
{
    if (isNull(offset))
        return (NULL);
    COM_ASSERT_RET((offset < m_poolSize), NULL);
    COM_ASSERT_RET((offset / m_poolDataSize)*m_poolDataSize == offset, NULL);
    char* pAddr = (char*)((size_type)m_pool + offset);
    COM_ASSERT_RET(isUsed((size_type*)pAddr), NULL);
    return (data_type*)(pAddr + HEAD_SIZE);
}

template <typename DATA_TYPE, int COUNT>
typename ComDataPool02<DATA_TYPE, COUNT>::size_type
ComDataPool02<DATA_TYPE, COUNT>::getOffset(data_type* addr)
{
    if (NULL == addr)
        return (NULL_PTR);
    size_type offset = (size_type)addr - HEAD_SIZE - (size_type)m_pool;
    COM_ASSERT_RET((offset < m_poolSize), NULL_PTR);
    COM_ASSERT_RET((offset / m_poolDataSize)*m_poolDataSize == offset, NULL_PTR);
    COM_ASSERT_RET(isUsed((size_type*)((size_type)addr - HEAD_SIZE)), NULL_PTR);
    return (offset);
}

template <typename DATA_TYPE, int COUNT>
void ComDataPool02<DATA_TYPE, COUNT>::debug()
{
    COM_ASSERT((m_pool != 0));
    COM_DEBUG(DEBUG_LEVEL3, ("**ComDataPool01::debug+**\n"));
    char* pNextNode = m_pool;
    size_type nDataSize = m_poolDataSize;
    COM_DEBUG(DEBUG_LEVEL3, ("**All :\n"));
    for (size_type i = 0; i < m_dataCount; ++i)
    {
        COM_DEBUG(DEBUG_LEVEL3, ("**No.%d : node addr(%p), data addr(%p), used?(%d), next node index(%p)\n",
                                 i, pNextNode, (char*)((size_type)pNextNode + HEAD_SIZE), isUsed((size_type*)pNextNode), *(size_type**)pNextNode));
        pNextNode += nDataSize;
    }
    COM_DEBUG(DEBUG_LEVEL3, ("**Free :\n"));
    size_type nIndex = m_free;
    int i = 0;
    while (!isNull(nIndex))
    {
        pNextNode = (char*)((size_type)m_pool + nIndex);
        COM_DEBUG(DEBUG_LEVEL3, ("**No.%d : node addr(%p), data addr(%p), used?(%d), next node index(%p)\n",
                                 i, (char*)pNextNode, (char*)((size_type)pNextNode + HEAD_SIZE), isUsed((size_type*)pNextNode), *(size_type**)pNextNode));
        nIndex = getIndex((*(size_type*)pNextNode));
        i++;
    }
    COM_DEBUG(DEBUG_LEVEL3, ("**ComDataPool01::debug-**\n"));
}


#endif



