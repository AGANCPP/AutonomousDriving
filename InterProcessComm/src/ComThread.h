

#ifndef COMMON_THREAD_H
#define COMMON_THREAD_H

#include <windows.h>
#include <process.h>
#include "ComTools.h"
#include "ComDataPool.h"

//typedef unsigned int size_t;

typedef unsigned int (__stdcall* THREAD_PROC)(void* pArg_);

inline HANDLE ComCreateThread(void* pSecurity_, unsigned int nStackSize_, THREAD_PROC pThreadProc_, void* pArgList_, unsigned int nInitFlag_, unsigned int* pThreadAddr_)
{
    return (HANDLE)(_beginthreadex(pSecurity_, nStackSize_, pThreadProc_, pArgList_, nInitFlag_, pThreadAddr_));
}

inline DWORD ComResumeThread(HANDLE hThread_)
{
    return (ResumeThread(hThread_));
}

inline DWORD ComSuspendThread(HANDLE hThread_)
{
    return (SuspendThread(hThread_));
}

inline BOOL ComTerminateThread(HANDLE hThread_, DWORD nExitCode_)
{
    return (TerminateThread(hThread_, nExitCode_));
}

class CComThread
{
public:
    CComThread();
    virtual ~CComThread();

    HANDLE GetThreadHandle(void)
    {
        return m_hThread;
    }
    int GetPriority(void)
    {
        return (m_nPriority);
    }
    void SetPriority(int nPrio_)
    {
        m_nPriority = nPrio_;
    }
    BOOL ResumeThread(void);
    BOOL SuspendThread(void);
    BOOL TerminateThisThread(void)
    {
        return (::TerminateThread(m_hThread, 1));
    }

protected:
    BOOL Create(void);
    virtual unsigned int Run(void);
    int WaitThreadExit(int nCount = 2, int nTimeout_ = 30000);
    BOOL CloseThreadHandle(void);

private:
    static unsigned int __stdcall ThreadProcedure(void* pArg_);

private:
    HANDLE m_hThread;
    DWORD m_nThreadId;
    int m_nPriority;
};

class CThreadLock
{
public:
    CThreadLock()
    {
        ::InitializeCriticalSection(&m_tSection);
    }
    ~CThreadLock()
    {
        ::DeleteCriticalSection(&m_tSection);
    }

    void Lock(void)
    {
        ::EnterCriticalSection(&m_tSection);
    }
    void Unlock(void)
    {
        ::LeaveCriticalSection(&m_tSection);
    }

private:
    CRITICAL_SECTION m_tSection;
};

class ComWork : public CComThread
{
public:
    enum
    {
        WORK_NG = -1,
        WORK_OK = 0,
        WORK_EXIT,
    };

    ComWork(char* pName_);
    ~ComWork();

    BOOL StartWork();
    BOOL EndWork();
    char* GetModuleName(void)
    {
        return (m_moduleName);
    }
    unsigned int Run(void);

    BOOL SendWorkEvent(void)
    {
        if ((NULL != m_hWorkEvent) && (FALSE == GetPauseFlag()))
        {
            SetEvent(m_hWorkEvent);
            return (TRUE);
        }
        return (FALSE);
    }

    BOOL GetPauseFlag(void)
    {
        return (m_bPauseFlag);
    }
    void SetPauseFlag(BOOL bPause_)
    {
        m_bPauseFlag = bPause_;
        COM_DEBUG(DEBUG_LEVEL1, ("CAM: %s: set pause flag [%d]\n",  GetModuleName(), m_bPauseFlag));
    }

    virtual unsigned int HandleMessage(void);
    virtual BOOL BeforeRunning(void);
    virtual void AfterRunning(void);

private:
    enum { MAX_NAME_LENGTH = 64 };
    volatile long m_nRunningFlag;
    HANDLE m_hWorkEvent;
    BOOL m_bPauseFlag;
    char m_moduleName[MAX_NAME_LENGTH];
};

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class CDataPool
{
public:
    typedef char*        pointer;
    typedef const char*  const_pointer;
    typedef unsigned int size_type;
    typedef ptrdiff_t    defference_type;

public:
    CDataPool(size_type dataSize_, size_type maxAmount_);
    ~CDataPool();
    void initialize(void);

    pointer allocate();
    bool deallocate(void* p);
    bool is_dup_dealloc(void* p);
    bool is_valid_ptr(void* p);

    pointer list_first_buff(void);
    pointer list_next_buff(void* p);
    void debug();

private:
    CDataPool(CDataPool&);
    void operator=(const CDataPool&);

private:
    pointer m_pFree;
    size_type pool_data_size;
    size_type max_amount;
    size_type pool_size;
    enum { HEAD_SIZE = sizeof(pointer) };
    enum { DATA_ALIGN = sizeof(pointer) };
    char* m_pDataPool;
    bool* m_bMap;
};

template <typename DATA_TYPE>
class CDataPoolEx
{
public:
    typedef struct _DATA_NODE
    {
        int        ref_count;
        DATA_TYPE  data;
    } DATA_NODE;

public:
    typedef DATA_TYPE         value_type;
    typedef value_type*       value_ptr;
    typedef DATA_NODE         node_type;
    typedef node_type*        node_ptr;
    typedef unsigned int      size_type;

public:
    CDataPoolEx(size_type maxAmount_) : node_pool(sizeof(node_type), maxAmount_) {}

    value_ptr allocate();
    bool deallocate(value_ptr p);
    value_ptr cpy_ref_from(value_ptr p);
    value_ptr list_first_buff();
    value_ptr list_next_buff(value_ptr p);

private:
    typedef CThreadLock POOL_LOCK;
    CDataPool node_pool;
    POOL_LOCK m_lock;
};

template <typename DATA_TYPE>
typename CDataPoolEx<DATA_TYPE>::value_ptr
CDataPoolEx<DATA_TYPE>::allocate()
{
    node_ptr node = NULL;
    m_lock.Lock();
    node = (node_ptr)node_pool.allocate();
    m_lock.Unlock();
    if (NULL == node)
    {
        //printf("CDataPoolEx::allocate: node==NULL\n");
        return NULL;
    }
    node->ref_count = 1;
    //printf("CDataPoolEx::allocate: data=%p ref=%d node=%p\n", &(node->data), node->ref_count, node);
    return (&(node->data));
}

template <typename DATA_TYPE>
bool CDataPoolEx<DATA_TYPE>::deallocate(value_ptr p)
{
    bool bRet = false;
    if (NULL == p)
    {
        //printf("CDataPoolEx::deallocate: p==NULL\n");
        return (bRet);
    }
    node_ptr node = c_container_of(p, node_type, data);
    bRet = true;
    m_lock.Lock();
    if (--(node->ref_count) <= 0)
    {
        node->ref_count = 0;
        bRet = node_pool.deallocate(node);
        //printf("CDataPoolEx::deallocate: deallocate from buff\n");
    }
    m_lock.Unlock();
    //printf("CDataPoolEx::deallocate: data=%p ref=%d node=%p\n", &(node->data), node->ref_count, node);
    return (bRet);
}

template <typename DATA_TYPE>
typename CDataPoolEx<DATA_TYPE>::value_ptr
CDataPoolEx<DATA_TYPE>::cpy_ref_from(value_ptr p)
{
    value_ptr ret = NULL;
    if (NULL == p)
    {
        //printf("CDataPoolEx::cpy_ref_from: p==NULL\n");
        return (ret);
    }
    node_ptr node = c_container_of(p, node_type, data);
    m_lock.Lock();
    if (node_pool.is_valid_ptr(node))
    {
        node->ref_count++;
        ret = p;
        //printf("CDataPoolEx::cpy_ref_from: data=%p ref=%d node=%p\n", &(node->data), node->ref_count, node);
    }
    else
    {
        //printf("CDataPoolEx::cpy_ref_from: p is invalid\n");
    }
    m_lock.Unlock();
    return (ret);
}

template <typename DATA_TYPE>
typename CDataPoolEx<DATA_TYPE>::value_ptr
CDataPoolEx<DATA_TYPE>::list_first_buff()
{
    node_ptr node = NULL;
    m_lock.Lock();
    node = (node_ptr)node_pool.list_first_buff();
    m_lock.Unlock();
    if (NULL == node)
        return NULL;
    return (&(node->data));
}

template <typename DATA_TYPE>
typename CDataPoolEx<DATA_TYPE>::value_ptr
CDataPoolEx<DATA_TYPE>::list_next_buff(value_ptr p)
{
    if (NULL == p)
        return (NULL);
    node_ptr node = c_container_of(p, node_type, data);
    m_lock.Lock();
    node = (node_ptr)node_pool.list_next_buff(node);
    m_lock.Unlock();
    if (NULL == node)
        return NULL;
    return (&(node->data));
}

//Ring list ex (override expired data)
template <typename DATA_TYPE>
class COeRingListEx
{
private:
    typedef struct _LIST_NODE
    {
        c_list_head  data_list;
        DATA_TYPE* data;
    } LIST_NODE;

    typedef LIST_NODE     node_type;
    typedef node_type*    node_ptr;
    typedef DATA_TYPE     data_type;
    typedef data_type*    data_ptr;
    typedef unsigned int  size_type;

public:
    COeRingListEx(CDataPoolEx<DATA_TYPE>& dataPool_, size_type maxAmount_);
    ~COeRingListEx();

    data_ptr GetSpace(void);
    data_ptr CopyRefFrom(data_ptr p)
    {
        return (m_dataPool.cpy_ref_from(p));
    }
    void PushBack(data_ptr pData_);
    data_ptr GetFront(void);
    void PopFront(void);
    void FreeSpace(data_ptr pData_);

    data_ptr list_first_buff()
    {
        return m_dataPool.list_first_buff();
    }
    data_ptr list_next_buff(data_ptr p)
    {
        return m_dataPool.list_next_buff(p);
    }

private:
    node_ptr AllocNode(void);

private:
    typedef CThreadLock LIST_LOCK;
    size_type m_nMaxAmount;
    int m_nDataCount;
    c_list_head m_dataListHead;
    c_list_head m_frontNode;
    LIST_LOCK m_lock;
    CDataPoolEx<DATA_TYPE>& m_dataPool;
    CDataPool m_nodePool;
};

template <typename DATA_TYPE>
COeRingListEx<DATA_TYPE>::COeRingListEx(CDataPoolEx<DATA_TYPE>& dataPool_, size_type maxAmount_) :
    m_dataPool(dataPool_),
    m_nodePool(sizeof(LIST_NODE), maxAmount_),
    m_nMaxAmount(maxAmount_),
    m_nDataCount(0)
{
    INIT_C_LIST_HEAD(&(m_dataListHead));
    INIT_C_LIST_HEAD(&(m_frontNode));
}

template <typename DATA_TYPE>
COeRingListEx<DATA_TYPE>::~COeRingListEx()
{
}

template <typename DATA_TYPE>
typename COeRingListEx<DATA_TYPE>::data_ptr
COeRingListEx<DATA_TYPE>::GetSpace(void)
{
    data_ptr pData = NULL;
    m_lock.Lock();
    pData = m_dataPool.allocate();
    m_lock.Unlock();
    //printf("COeRingListEx::GetSpace: pData=%p\n", pData);
    return (pData);
}

template <typename DATA_TYPE>
typename COeRingListEx<DATA_TYPE>::node_ptr
COeRingListEx<DATA_TYPE>::AllocNode(void)
{
    node_ptr node = NULL;
    if (m_nDataCount >= m_nMaxAmount)
    {
        if (!c_list_empty(&(m_dataListHead)))
        {
            node = c_list_first_entry(&(m_dataListHead), node_type, data_list);
            c_list_del_init(&(node->data_list));
            m_dataPool.deallocate(node->data);
            m_nDataCount--;
            if (m_nDataCount < 0)
                m_nDataCount = 0;
            //printf("COeRingListEx::AllocNode: get expired node=%p m_nDataCount=%d\n", node, m_nDataCount);
            return (node);
        }
        //printf("COeRingListEx::AllocNode: list err, m_nDataCount=%d\n", m_nDataCount);
        return (NULL);
    }
    node = (node_ptr)m_nodePool.allocate();
    //printf("COeRingListEx::AllocNode: allocate node=%p\n", node);
    return (node);
}

template <typename DATA_TYPE>
void COeRingListEx<DATA_TYPE>::PushBack(data_ptr pData_)
{
    if (NULL == pData_)
    {
        //printf("COeRingListEx::PushBack: pData_ is NULL\n");
        return;
    }
    m_lock.Lock();
    node_ptr node = AllocNode();
    if (NULL != node)
    {
        node->data = pData_;
        c_list_add_tail(&(node->data_list), &(m_dataListHead));
        m_nDataCount++;
        //printf("COeRingListEx::PushBack: pData_=%p node=%p m_nDataCount=%d\n", pData_, node, m_nDataCount);
    }
    else
    {
        //printf("COeRingListEx::PushBack: node allocate fail\n");
        m_dataPool.deallocate(pData_);
    }
    m_lock.Unlock();
    return;
}

template <typename DATA_TYPE>
typename COeRingListEx<DATA_TYPE>::data_ptr
COeRingListEx<DATA_TYPE>::GetFront(void)
{
    node_ptr node = NULL;
    m_lock.Lock();
    if (!c_list_empty(&(m_frontNode)))
    {
        node = c_list_first_entry(&(m_frontNode), node_type, data_list);
        //printf("COeRingListEx::GetFront: get node from m_frontNode, node=%p\n", node);
    }
    else
    {
        if (!c_list_empty(&(m_dataListHead)))
        {
            node = c_list_first_entry(&(m_dataListHead), node_type, data_list);
            c_list_del_init(&(node->data_list));
            c_list_add_tail(&(node->data_list), &(m_frontNode));
            //printf("COeRingListEx::GetFront: get node from m_dataListHead, node=%p\n", node);
        }
    }
    m_lock.Unlock();
    if (NULL == node)
    {
        //printf("COeRingListEx::GetFront: get node is null\n");
        return (NULL);
    }
    //printf("COeRingListEx::GetFront: get data=%p\n", node->data);
    return (node->data);
}

template <typename DATA_TYPE>
void COeRingListEx<DATA_TYPE>::PopFront(void)
{
    node_ptr node = NULL;
    m_lock.Lock();
    if (!c_list_empty(&(m_frontNode)))
    {
        node = c_list_first_entry(&(m_frontNode), node_type, data_list);
        c_list_del_init(&(node->data_list));
        //printf("COeRingListEx::PopFront: pop node from m_frontNode, node=%p\n", node);
    }
    else
    {
        if (!c_list_empty(&(m_dataListHead)))
        {
            node = c_list_first_entry(&(m_dataListHead), node_type, data_list);
            c_list_del_init(&(node->data_list));
            m_dataPool.deallocate(node->data);
            //printf("COeRingListEx::PopFront: pop node from m_dataListHead, node=%p\n", node);
        }
    }
    m_nDataCount--;
    if (m_nDataCount < 0)
        m_nDataCount = 0;
    m_nodePool.deallocate(node);
    //printf("COeRingListEx::PopFront: deallocate node=%p m_nDataCount=%d\n", node, m_nDataCount);
    m_lock.Unlock();
}

template <typename DATA_TYPE>
void COeRingListEx<DATA_TYPE>::FreeSpace(DATA_TYPE* pData_)
{
    if (NULL == pData_)
    {
        //printf("COeRingListEx::FreeSpace: pData_ is null\n", pData_);
        return;
    }
    m_lock.Lock();
    m_dataPool.deallocate(pData_);
    m_lock.Unlock();
    //printf("COeRingListEx::FreeSpace: pData_=%p\n", pData_);
}

#endif



