

#ifndef COMMON_LIST_H
#define COMMON_LIST_H

#include "ComDataPool.h"
#include "ComDebug.h"

template <typename DATA_TYPE, int COUNT>
class ComDataPoolList02
{
public:
    typedef DATA_TYPE data_type;
    typedef size_t size_type;

public:
    ComDataPoolList02(bool bOverrideExpiredData_);
    bool clear(void);

    data_type* allocSpace();
    bool pushBack(data_type* pData);
    data_type* getFront();
    bool freeSpace(data_type* pData);

    class iterator;
    friend class iterator;
    class iterator
    {
    public:
        typedef ComDataPoolList02<DATA_TYPE, COUNT> list_type;

    private:
        list_type& m_list;
        size_t m_index;

    public:
        iterator(list_type& list_) : m_list(list_), m_index(m_list.m_list.head) {}
        iterator(list_type& list_, bool) : m_list(list_), m_index(list_type::NULL_PTR) {}
        iterator(const iterator& rv) : m_list(rv.m_list), m_index(rv.m_index) {}
        iterator& operator=(const iterator& rv)
        {
            m_list = rv.m_list;
            m_index = rv.m_index;
            return (*this);
        }
        iterator& operator++()
        {
            if (!m_list.m_pool.isNull(m_index))
            {
                typename list_type::node_type* pddr = m_list.m_pool.getVirtualAddr(m_index);
                m_index = pddr->next;
            }
            return (*this);
        }
        bool operator==(const iterator& rv) const
        {
            return (m_index == rv.m_index);
        }
        bool operator!=(const iterator& rv) const
        {
            return (m_index != rv.m_index);
        }
        typename list_type::data_type* current()
        {
            if (!m_list.m_pool.isNull(m_index))
            {
                typename list_type::node_type* pddr = m_list.m_pool.getVirtualAddr(m_index);
                return (&(pddr->data));
            }
            return (NULL);
        }
        typename list_type::data_type* operator*()
        {
            return current();
        }
        typename list_type::data_type* operator->()
        {
            return current();
        }
        typename list_type::data_type* remove()
        {
            if (!m_list.m_pool.isNull(m_index))
            {
                typename list_type::node_type* pddr = m_list.m_pool.getVirtualAddr(m_index);
                m_list.erease(pddr);
                return (&(pddr->data));
            }
            return (NULL);
        }
    };
    iterator begin()
    {
        return iterator(*this);
    }
    iterator end()
    {
        return iterator(*this, true);
    }

    void debug(void);

public:
    typedef struct _NODE_HEAD
    {
        size_type head;
        size_type tail;
    } node_head;
    typedef struct _NODE_TYPE
    {
        size_type next;
        size_type prev;
        DATA_TYPE data;
    } node_type;

    node_type* allocNode(void);
    bool isFreeNode(node_type* pNode)
    {
        return ((m_list.head != m_pool.getOffset(pNode)) && (NULL_PTR == pNode->next) && (NULL_PTR == pNode->prev));
    }
    bool insert(node_type* pFront, node_type* pNew);
    bool erease(node_type* pDelNode);

public:
    typedef ComDataPool02<node_type, COUNT> pool_type;
    enum { NULL_PTR = ComDataPool02<DATA_TYPE, COUNT>::NULL_PTR };

    bool m_bOverrideFlag;    // override expired data ?
    node_head m_list;
    pool_type m_pool;
};

template <typename DATA_TYPE, int COUNT>
ComDataPoolList02<DATA_TYPE, COUNT>::ComDataPoolList02(bool bOverrideExpiredData_)
{
    m_bOverrideFlag = bOverrideExpiredData_;
    m_list.head = NULL_PTR;
    m_list.tail = NULL_PTR;
}

template <typename DATA_TYPE, int COUNT>
bool ComDataPoolList02<DATA_TYPE, COUNT>::clear(void)
{
    m_list.head = NULL_PTR;
    m_list.tail = NULL_PTR;
    m_pool.clear();
    return (true);
}

template <typename DATA_TYPE, int COUNT>
typename ComDataPoolList02<DATA_TYPE, COUNT>::data_type*
ComDataPoolList02<DATA_TYPE, COUNT>::allocSpace()
{
    node_type* pNode = allocNode();
    if (NULL != pNode)
        return (&(pNode->data));
    return (NULL);
}

template <typename DATA_TYPE, int COUNT>
bool ComDataPoolList02<DATA_TYPE, COUNT>::pushBack(data_type* pData)
{
    if (NULL == pData)
        return (false);
    node_type* pNode = c_container_of(pData, node_type, data);
    COM_ASSERT_RET(isFreeNode(pNode), false);
    COM_ASSERT_RET(!m_pool.isNull(m_pool.getOffset(pNode)), false);
    insert((node_type*)m_pool.getVirtualAddr(m_list.tail), pNode);
    return (true);
}

template <typename DATA_TYPE, int COUNT>
typename ComDataPoolList02<DATA_TYPE, COUNT>::data_type*
ComDataPoolList02<DATA_TYPE, COUNT>::getFront()
{
    if (!m_pool.isNull(m_list.head))
    {
        node_type* pNode = (node_type*)m_pool.getVirtualAddr(m_list.head);
        erease(pNode);
        pNode->next = NULL_PTR;
        pNode->prev = NULL_PTR;
        return (&(pNode->data));
    }
    return (NULL);
}

template <typename DATA_TYPE, int COUNT>
bool ComDataPoolList02<DATA_TYPE, COUNT>::freeSpace(data_type* pData)
{
    if (NULL == pData)
        return (false);
    node_type* pNode = c_container_of(pData, node_type, data);
    COM_ASSERT_RET(isFreeNode(pNode), false);
    return (m_pool.deallocate(m_pool.getOffset(pNode)));
}

template <typename DATA_TYPE, int COUNT>
typename ComDataPoolList02<DATA_TYPE, COUNT>::node_type*
ComDataPoolList02<DATA_TYPE, COUNT>::allocNode(void)
{
    node_type* pNode = NULL;
    size_type newOffset = m_pool.allocate();
    if (m_pool.isNull(newOffset))
    {
        if (m_bOverrideFlag)
        {
            // get node from expired list
            if (!m_pool.isNull(m_list.head))
            {
                //printf("1\n");
                pNode = (node_type*)m_pool.getVirtualAddr(m_list.head);
                erease(pNode);
                pNode->next = NULL_PTR;
                pNode->prev = NULL_PTR;
                return (pNode);
            }
        }
        //printf("2\n");
        return (NULL);
    }
    pNode = (node_type*)m_pool.getVirtualAddr(newOffset);
    pNode->next = NULL_PTR;
    pNode->prev = NULL_PTR;
    //printf("3\n");
    return (pNode);
}

template <typename DATA_TYPE, int COUNT>
bool ComDataPoolList02<DATA_TYPE, COUNT>::insert(node_type* pFront, node_type* pNew)
{
    size_type newOffset = m_pool.getOffset(pNew);
    if (m_pool.isNull(m_list.head))
    {
        //printf("1\n");
        m_list.head = newOffset;
        m_list.tail = newOffset;
        pNew->prev = NULL_PTR;
        pNew->next = NULL_PTR;
    }
    else if (NULL != pFront)
    {
        //printf("2\n");
        // next->prev = newOne;
        size_type nextOffset = pFront->next;
        if (!m_pool.isNull(nextOffset))
        {
            node_type* nextAddr = (node_type*)m_pool.getVirtualAddr(nextOffset);
            nextAddr->prev = newOffset;
        }
        // newOne->next = next;
        pNew->next = nextOffset;
        // newOne->prev = prev;
        size_type prevOffset = m_pool.getOffset(pFront);
        pNew->prev = prevOffset;
        // prev->next = newOne;
        pFront->next = newOffset;
        if (m_list.tail == prevOffset)
            m_list.tail = newOffset;
    }
    else
    {
        //printf("3\n");
        // next->prev = newOne;
        size_type nextOffset = m_list.head;
        if (!m_pool.isNull(nextOffset))
        {
            node_type* nextAddr = (node_type*)m_pool.getVirtualAddr(nextOffset);
            nextAddr->prev = newOffset;
        }
        // newOne->next = next;
        pNew->next = nextOffset;
        // newOne->prev = prev;
        size_type prevOffset = NULL_PTR;
        pNew->prev = prevOffset;
        // prev->next = newOne;
        m_list.head = newOffset;
    }
    return (true);
}

template <typename DATA_TYPE, int COUNT>
bool ComDataPoolList02<DATA_TYPE, COUNT>::erease(node_type* pDelNode)
{
    //next->prev = prev;
    size_type nextOffset = pDelNode->next;
    size_type prevOffset = pDelNode->prev;
    if (!m_pool.isNull(nextOffset))
    {
        //printf("1\n");
        node_type* nextAddr = (node_type*)m_pool.getVirtualAddr(nextOffset);
        nextAddr->prev = prevOffset;
    }
    else
    {
        //printf("2\n");
        m_list.tail = prevOffset;
    }
    //prev->next = next;
    if (!m_pool.isNull(prevOffset))
    {
        //printf("3\n");
        node_type* prevAddr = (node_type*)m_pool.getVirtualAddr(prevOffset);
        prevAddr->next = nextOffset;
    }
    else
    {
        //printf("4\n");
        m_list.head = nextOffset;
    }
    return (true);
}

template <typename DATA_TYPE, int COUNT>
void ComDataPoolList02<DATA_TYPE, COUNT>::debug(void)
{
    COM_DEBUG(DEBUG_LEVEL3, ("**ComDataPoolList01<DATA_TYPE, COUNT>::Debug enter**\n"));
    COM_DEBUG(DEBUG_LEVEL3, ("****** show node pool start ******\n"));
    m_pool.debug();
    COM_DEBUG(DEBUG_LEVEL3, ("****** show node pool end   ******\n"));
    COM_DEBUG(DEBUG_LEVEL3, ("**m_list.head=%p m_list.tail=%p\n", m_list.head, m_list.tail));
    COM_DEBUG(DEBUG_LEVEL3, ("**node list (head->tail) :\n"));
    size_type offset = m_list.head;
    node_type* pddr = (node_type*)m_pool.getVirtualAddr(offset);
    int i = 0;
    while (!m_pool.isNull(offset))
    {
        COM_DEBUG(DEBUG_LEVEL3, ("**[%d] node addr(%p), current (%p), next (%p), prev (%p)\n",
                                 i++, pddr, offset, pddr->next, pddr->prev));
        offset = pddr->next;
        pddr = (node_type*)m_pool.getVirtualAddr(offset);
    }
    COM_DEBUG(DEBUG_LEVEL3, ("**node list (tail->head) :\n"));
    offset = m_list.tail;
    pddr = (node_type*)m_pool.getVirtualAddr(offset);
    i = 0;
    while (!m_pool.isNull(offset))
    {
        COM_DEBUG(DEBUG_LEVEL3, ("**[%d] node addr(%p), current (%p), next (%p), prev (%p)\n",
                                 i++, pddr, offset, pddr->next, pddr->prev));
        offset = pddr->prev;
        pddr = (node_type*)m_pool.getVirtualAddr(offset);
    }
    COM_DEBUG(DEBUG_LEVEL3, ("**ComDataPoolList01<DATA_TYPE, COUNT>::Debug leave**\n"));
}


#endif

