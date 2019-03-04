
#ifndef COMMON_PROCESS_H
#define COMMON_PROCESS_H
#include <windows.h>
#include "ComList.h"


//ComMutex
class ComMutex
{
public:
    ComMutex(BOOL bInitialOwner_, LPCTSTR name_);
    ~ComMutex();

    DWORD Lock(DWORD msec_ = INFINITE);
    BOOL Unlock(void);
private:
    HANDLE m_hMutex;
};

//ComEvent
class ComEvent
{
public:
    ComEvent(BOOL bManualReset_, BOOL bInitialState_, LPCTSTR name_);
    ~ComEvent();

    DWORD Wait(DWORD msec_ = INFINITE);
    BOOL Send(void);
private:
    HANDLE m_hEvent;
};

//ComSemaphore
class ComSemaphore
{
public:
    ComSemaphore(long lInitialCount_, long lMaximumCount_, LPCTSTR name_);
    ~ComSemaphore();

    DWORD Wait(DWORD msec_ = INFINITE);
    BOOL Post(void);

private:
    HANDLE m_hSemaphore;
};

//ComShareMem
class ComShareMem
{
public:
    enum { MAX_MEM_NEME_LEN = 128 };
public:
    ComShareMem(DWORD memSize_, LPCTSTR name_);
    ~ComShareMem(void);

    char* GetShareMemAddr(BOOL& isAlreadyExist_);
private:
    void Destroy(void);
private:
    HANDLE m_hFileMap;
    char* m_pView;
    DWORD m_memSize;
    TCHAR m_memName[MAX_MEM_NEME_LEN];
};

//typedef struct _RESPONSE_TYPE
//{
//    int messageId;
//}RESPONSE_TYPE;

template <typename KEY_TYPE, typename DATA_TYPE>
class c_map_type
{
public:
    typedef DATA_TYPE  data_type;
    typedef KEY_TYPE   key_type;

    data_type& getData(void)
    {
        return (data);
    }
    key_type& getKey(void)
    {
        return (key);
    }
    data_type* operator->(void)
    {
        return &(data);
    }
    //bool operator==(c_map_type& op2) { return (key == op2.key); }
private:
    key_type key;
    data_type data;
};

//ComInterProcessMsgQueue
template <typename DATA_TYPE, int COUNT, typename RES_TYPE = c_map_type<int, int> >
class ComInterProcessMsgQueue
{
public:
    enum { MAX_QUEUE_LEN = 64 };
    typedef DATA_TYPE  data_type;
    typedef data_type* data_ptr;
    typedef data_type& data_ref;
    typedef size_t     size_type;
    typedef RES_TYPE   res_type;
    typedef res_type*  res_ptr;
    typedef ComDataPoolList02<DATA_TYPE, COUNT> data_list_type;
    enum { RES_LIST_SIZE = COUNT };
    typedef ComDataPoolList02<RES_TYPE, RES_LIST_SIZE> res_list_type;
    typedef typename RES_TYPE::key_type res_key_type;

public:
    ComInterProcessMsgQueue(bool bOverrideExpiredData_, LPCTSTR name_);
    ~ComInterProcessMsgQueue();
    bool Create(bool createRespLis_ = false);
    // be careful, can not be called when the queue is using in communication
    bool Clear(void);
    bool EnableResp(bool enable_)
    {
        m_bUseResList = enable_;
    }
    bool isUsingRespList(void)
    {
        return ((m_bUseResList) && (m_bInitFlag));
    }

    data_ptr AllocSpace(DWORD msec_ = INFINITE);
    bool PushBack(data_ptr ptr_);
    data_ptr GetFront(DWORD msec_ = INFINITE);
    void FreeSpace(data_ptr ptr_);

    res_ptr AllocResSpace(DWORD msec_ = INFINITE);
    bool SendRes(res_ptr res_);
    res_ptr WaitRes(res_key_type key_, DWORD msec_ = INFINITE);
    void FreeResSpace(res_ptr ptr_);

    void Debug(void);

private:
    void Destroy_(void);
    res_ptr FindAndRemoveFromResList(res_key_type key_);

private:
    bool m_bOverrideFlag;    // override expired data ?
    bool m_bInitFlag;
    bool m_bUseResList;
    UINT32 m_nMaxMsg;
    ComEvent* m_pQueueItemEvent;
    ComEvent* m_pQueueSpaceEvent;
    ComMutex* m_pMutex;
    ComShareMem* m_pShareMem;
    data_list_type* m_pDataList;
    res_list_type* m_pResList;
    ComEvent* m_pResQueueItemEvent;
    ComEvent* m_pResQueueSpaceEvent;
    ComMutex* m_pResMutex;
    TCHAR m_queueName[MAX_QUEUE_LEN];
};

template <typename DATA_TYPE, int COUNT, typename RES_TYPE>
ComInterProcessMsgQueue<DATA_TYPE, COUNT, RES_TYPE>::ComInterProcessMsgQueue(bool bOverrideExpiredData_, LPCTSTR name_)
{
    m_bOverrideFlag = bOverrideExpiredData_;
    m_bInitFlag = false;
    m_nMaxMsg = COUNT;
    m_pQueueItemEvent = NULL;
    m_pQueueSpaceEvent = NULL;
    m_pMutex = NULL;
    m_pShareMem = NULL;
    m_pDataList = NULL;
    m_pResList = NULL;
    m_pResQueueItemEvent = NULL;
    m_pResQueueSpaceEvent = NULL;
    m_pResMutex = NULL;
    m_bUseResList = false;
    memset(m_queueName, 0, sizeof(m_queueName));
    if (NULL == name_)
    {
        COM_ERR("[P 0x%08x][ERR]ComInterProcessMsgQueue name is null\n", GetCurrentProcessId());
        return;
    }
    DWORD nNameLen = lstrlen(name_);
    if ((nNameLen <= 0) || (nNameLen > MAX_QUEUE_LEN - 1))
    {
        COM_ERR("[P 0x%08x][ERR]ComInterProcessMsgQueue name length (%d) not good\n", GetCurrentProcessId(), nNameLen);
        return;
    }
    lstrcpyn(m_queueName, name_, MAX_QUEUE_LEN - 1);
}

template <typename DATA_TYPE, int COUNT, typename RES_TYPE>
ComInterProcessMsgQueue<DATA_TYPE, COUNT, RES_TYPE>::~ComInterProcessMsgQueue()
{
    if (NULL != m_pMutex)
    {
        m_pMutex->Lock();
        Destroy_();
        m_pMutex->Unlock();
        delete m_pMutex;
        m_pMutex = NULL;
    }
}

template <typename DATA_TYPE, int COUNT, typename RES_TYPE>
bool ComInterProcessMsgQueue<DATA_TYPE, COUNT, RES_TYPE>::Clear(void)
{
    if (NULL != m_pMutex)
    {
        //if (NULL != m_pQueueItemEvent) {
        //    m_pQueueItemEvent->Send();
        //}
        //if (NULL != m_pQueueSpaceEvent) {
        //    m_pQueueSpaceEvent->Send()
        //}
        //Sleep(50);
        m_pMutex->Lock();
        if (NULL != m_pDataList)
            m_pDataList->clear();
        m_pMutex->Unlock();
        //m_pQueueItemEvent->Wait(50);
        //m_pQueueSpaceEvent->Wait(50);
    }
    if (NULL != m_pResMutex)
    {
        //if (NULL != m_pResQueueItemEvent) {
        //    m_pResQueueItemEvent->Send();
        //}
        //if (NULL != m_pResQueueSpaceEvent) {
        //    m_pResQueueSpaceEvent->Send();
        //}
        //Sleep(50);
        m_pResMutex->Lock();
        if (NULL != m_pResList)
            m_pResList->clear();
        m_pResMutex->Unlock();
        //m_pResQueueItemEvent->Wait(50);
        //m_pResQueueSpaceEvent->Wait(50);
    }
}

template <typename DATA_TYPE, int COUNT, typename RES_TYPE>
bool ComInterProcessMsgQueue<DATA_TYPE, COUNT, RES_TYPE>::Create(bool createRespLis_)
{
    if (m_bInitFlag)
    {
        COM_WARN("[P 0x%08x][WARN] ComInterProcessMsgQueue::Create Already created!\n", GetCurrentProcessId());
        return (true);
    }
    DWORD nNameLen = lstrlen(m_queueName);
    if ((nNameLen <= 0) || (nNameLen > MAX_QUEUE_LEN - 1))
    {
        COM_ERR("[P 0x%08x][ERR]ComInterProcessMsgQueue::Create name length (%d) not good\n", GetCurrentProcessId(), nNameLen);
        return (false);
    }
    DWORD  dwRet = 0;
    BOOL bRetVal = FALSE;
    TCHAR szTempName[128] = {0};
    lstrcpy(szTempName, m_queueName);
    lstrcat(szTempName, TEXT("_MUTEX"));
    m_pMutex = new ComMutex(FALSE, szTempName);
    if (NULL == m_pMutex)
    {
        COM_ERR("[P 0x%08x][ERR]ComInterProcessMsgQueue::Create mutex err\n", GetCurrentProcessId());
        return (false);
    }
    class TempLock
    {
    public:
        TempLock(ComMutex& lock_) : m_lock(lock_)
        {
            m_lock.Lock();
        }
        ~TempLock(void)
        {
            m_lock.Unlock();
        }
    private:
        ComMutex& m_lock;
    };
    TempLock myLock(*m_pMutex);
    lstrcpy(szTempName, m_queueName);
    lstrcat(szTempName, TEXT("_QUEUE_ITEM_EVENT"));
    m_pQueueItemEvent = new ComEvent(FALSE, FALSE, szTempName);
    if (NULL == m_pQueueItemEvent)
    {
        Destroy_();
        COM_ERR("[P 0x%08x][ERR]ComInterProcessMsgQueue::Create create item event err\n", GetCurrentProcessId());
        return (false);
    }
    lstrcpy(szTempName, m_queueName);
    lstrcat(szTempName, TEXT("_QUEUE_SPACE_EVENT"));
    m_pQueueSpaceEvent = new ComEvent(FALSE, FALSE, szTempName);
    if (NULL == m_pQueueSpaceEvent)
    {
        Destroy_();
        COM_ERR("[P 0x%08x][ERR]ComInterProcessMsgQueue::Create create space event err\n", GetCurrentProcessId());
        return (false);
    }
    size_t memSize = sizeof(data_list_type);
    if (createRespLis_)
    {
        lstrcpy(szTempName, m_queueName);
        lstrcat(szTempName, TEXT("_RES_LIST_MUTEX"));
        m_pResMutex = new ComMutex(FALSE, szTempName);
        if (NULL == m_pResMutex)
        {
            COM_ERR("[P 0x%08x][ERR]ComInterProcessMsgQueue::Create response list mutex err\n", GetCurrentProcessId());
            return (false);
        }
        lstrcpy(szTempName, m_queueName);
        lstrcat(szTempName, TEXT("_RES_LIST_ITEM_EVENT"));
        m_pResQueueItemEvent = new ComEvent(FALSE, FALSE, szTempName);
        if (NULL == m_pResQueueItemEvent)
        {
            Destroy_();
            COM_ERR("[P 0x%08x][ERR]ComInterProcessMsgQueue::Create create response list item event err\n", GetCurrentProcessId());
            return (false);
        }
        lstrcpy(szTempName, m_queueName);
        lstrcat(szTempName, TEXT("_RES_LIST_SPACE_EVENT"));
        m_pResQueueSpaceEvent = new ComEvent(FALSE, FALSE, szTempName);
        if (NULL == m_pResQueueSpaceEvent)
        {
            Destroy_();
            COM_ERR("[P 0x%08x][ERR]ComInterProcessMsgQueue::Create create space event err\n", GetCurrentProcessId());
            return (false);
        }
        memSize = sizeof(data_list_type) + sizeof(res_list_type);
    }
    lstrcpy(szTempName, m_queueName);
    lstrcat(szTempName, TEXT("_FILE_MAP"));
    m_pShareMem = new ComShareMem(memSize, szTempName);
    if (NULL == m_pShareMem)
    {
        Destroy_();
        COM_ERR("[P 0x%08x][ERR]ComInterProcessMsgQueue::Create create share memery err\n", GetCurrentProcessId());
        return (false);
    }
    BOOL bAlreadyExist = FALSE;
    char* memAddr = m_pShareMem->GetShareMemAddr(bAlreadyExist);
    if (NULL == memAddr)
    {
        Destroy_();
        COM_ERR("[P 0x%08x][ERR]ComInterProcessMsgQueue::Create get share memery address err\n", GetCurrentProcessId());
        return (false);
    }
    if (FALSE == bAlreadyExist)
    {
        COM_DEBUG(DEBUG_LEVEL3, ("[P 0x%08x]ComInterProcessMsgQueue::Create New data list at addr(%p)\n", GetCurrentProcessId(), memAddr));
        m_pDataList = new(memAddr) data_list_type(m_bOverrideFlag);
        if (createRespLis_)
        {
            m_pResList = new(memAddr + sizeof(data_list_type)) res_list_type(true);
            m_bUseResList = true;
        }
    }
    else
    {
        m_pDataList = (data_list_type*)memAddr;
        if (createRespLis_)
        {
            m_pResList = (res_list_type*)(memAddr + sizeof(data_list_type));
            m_bUseResList = true;
        }
    }
    m_bInitFlag = true;
    return (true);
}

template <typename DATA_TYPE, int COUNT, typename RES_TYPE>
void ComInterProcessMsgQueue<DATA_TYPE, COUNT, RES_TYPE>::Destroy_(void)
{
    m_bInitFlag = false;
    if (NULL != m_pQueueItemEvent)
    {
        delete(m_pQueueItemEvent);
        m_pQueueItemEvent = NULL;
    }
    if (NULL != m_pQueueSpaceEvent)
    {
        delete(m_pQueueSpaceEvent);
        m_pQueueSpaceEvent = NULL;
    }
    if (NULL != m_pShareMem)
    {
        delete(m_pShareMem);
        m_pShareMem = NULL;
    }
    // m_pDataList not need delete
    // m_pResList not need delete
    if (NULL != m_pResQueueItemEvent)
    {
        delete(m_pResQueueItemEvent);
        m_pResQueueItemEvent = NULL;
    }
    if (NULL != m_pResQueueSpaceEvent)
    {
        delete(m_pResQueueSpaceEvent);
        m_pResQueueSpaceEvent = NULL;
    }
    if (NULL != m_pResMutex)
    {
        delete(m_pResMutex);
        m_pResMutex = NULL;
    }
}

template <typename DATA_TYPE, int COUNT, typename RES_TYPE>
typename ComInterProcessMsgQueue<DATA_TYPE, COUNT, RES_TYPE>::data_ptr
ComInterProcessMsgQueue<DATA_TYPE, COUNT, RES_TYPE>::AllocSpace(DWORD msec_)
{
    COM_ASSERT_RET(true == m_bInitFlag, NULL);
    DWORD dwRet = 0;
    data_ptr ret;
    if (!m_bOverrideFlag)
    {
        do
        {
            m_pMutex->Lock();
            ret = m_pDataList->allocSpace();
            m_pMutex->Unlock();
            if (NULL == ret)
            {
                dwRet = m_pQueueSpaceEvent->Wait(msec_);
                if (WAIT_OBJECT_0 != dwRet)
                {
                    COM_DEBUG(DEBUG_LEVEL2, ("[P 0x%08x] ComInterProcessMsgQueue::AllocSpace waiting m_pQueueSpaceEvent timeout\n", GetCurrentProcessId()));
                    break;
                }
            }
        }
        while (!ret);
    }
    else
    {
        m_pMutex->Lock();
        ret = m_pDataList->allocSpace();
        m_pMutex->Unlock();
    }
    return (ret);
}

template <typename DATA_TYPE, int COUNT, typename RES_TYPE>
bool ComInterProcessMsgQueue<DATA_TYPE, COUNT, RES_TYPE>::PushBack(data_ptr ptr_)
{
    COM_ASSERT_RET(true == m_bInitFlag, false);
    m_pMutex->Lock();
    bool ret = m_pDataList->pushBack(ptr_);
    m_pMutex->Unlock();
    if (ret)
        m_pQueueItemEvent->Send();
    return (ret);
}

template <typename DATA_TYPE, int COUNT, typename RES_TYPE>
typename ComInterProcessMsgQueue<DATA_TYPE, COUNT, RES_TYPE>::data_ptr
ComInterProcessMsgQueue<DATA_TYPE, COUNT, RES_TYPE>::GetFront(DWORD msec_)
{
    COM_ASSERT_RET(true == m_bInitFlag, NULL);
    DWORD dwRet = 0;
    data_ptr ret = NULL;
    do
    {
        m_pMutex->Lock();
        ret = m_pDataList->getFront();
        m_pMutex->Unlock();
        if (NULL == ret)
        {
            dwRet = m_pQueueItemEvent->Wait(msec_);
            if (WAIT_OBJECT_0 != dwRet)
            {
                COM_DEBUG(DEBUG_LEVEL2, ("[P 0x%08x] ComInterProcessMsgQueue::GetFront waiting m_pQueueItemEvent timeout\n", GetCurrentProcessId()));
                break;
            }
        }
    }
    while (!ret);
    return (ret);
}

template <typename DATA_TYPE, int COUNT, typename RES_TYPE>
void ComInterProcessMsgQueue<DATA_TYPE, COUNT, RES_TYPE>::FreeSpace(data_ptr ptr_)
{
    COM_ASSERT(true == m_bInitFlag);
    m_pMutex->Lock();
    bool ret = m_pDataList->freeSpace(ptr_);
    m_pMutex->Unlock();
    if (ret)
    {
        if (!m_bOverrideFlag)
        {
            //printf("m_pQueueEmptys->Post\n");
            m_pQueueSpaceEvent->Send();
        }
    }
    return;
}

template <typename DATA_TYPE, int COUNT, typename RES_TYPE>
typename ComInterProcessMsgQueue<DATA_TYPE, COUNT, RES_TYPE>::res_ptr
ComInterProcessMsgQueue<DATA_TYPE, COUNT, RES_TYPE>::AllocResSpace(DWORD msec_)
{
    COM_ASSERT_RET(true == m_bInitFlag, NULL);
    DWORD dwRet = 0;
    res_ptr ret = NULL;
    if (m_bUseResList)
    {
        if (NULL != m_pResList)
        {
            do
            {
                m_pResMutex->Lock();
                ret = m_pResList->allocSpace();
                m_pResMutex->Unlock();
                if (NULL == ret)
                {
                    dwRet = m_pResQueueSpaceEvent->Wait(msec_);
                    if (WAIT_OBJECT_0 != dwRet)
                    {
                        COM_DEBUG(DEBUG_LEVEL2, ("[P 0x%08x] ComInterProcessMsgQueue::AllocResSpace waiting m_pResQueueSpaceEvent timeout\n", GetCurrentProcessId()));
                        break;
                    }
                }
            }
            while (!ret);
        }
    }
    return (ret);
}

template <typename DATA_TYPE, int COUNT, typename RES_TYPE>
bool ComInterProcessMsgQueue<DATA_TYPE, COUNT, RES_TYPE>::SendRes(res_ptr res_)
{
    COM_ASSERT_RET(true == m_bInitFlag, false);
    bool ret = true;
    if (m_bUseResList)
    {
        if (NULL != m_pResList)
        {
            m_pResMutex->Lock();
            ret = m_pResList->pushBack(res_);
            m_pResMutex->Unlock();
            if (ret)
                m_pResQueueItemEvent->Send();
        }
    }
    return (ret);
}

template <typename DATA_TYPE, int COUNT, typename RES_TYPE>
typename ComInterProcessMsgQueue<DATA_TYPE, COUNT, RES_TYPE>::res_ptr
ComInterProcessMsgQueue<DATA_TYPE, COUNT, RES_TYPE>::WaitRes(res_key_type key_, DWORD msec_)
{
    COM_ASSERT_RET(true == m_bInitFlag, NULL);
    DWORD dwRet = 0;
    res_ptr ret = NULL;
    if (m_bUseResList)
    {
        if (NULL != m_pResList)
        {
            do
            {
                m_pResMutex->Lock();
                ret = FindAndRemoveFromResList(key_);
                m_pResMutex->Unlock();
                if (NULL == ret)
                {
                    dwRet = m_pResQueueItemEvent->Wait(msec_);
                    if (WAIT_OBJECT_0 != dwRet)
                    {
                        COM_DEBUG(DEBUG_LEVEL2, ("[P 0x%08x] ComInterProcessMsgQueue::WaitRes waiting m_pResQueueItemEvent timeout\n", GetCurrentProcessId()));
                        break;
                    }
                }
            }
            while (!ret);
        }
    }
    return (ret);
}

template <typename DATA_TYPE, int COUNT, typename RES_TYPE>
void ComInterProcessMsgQueue<DATA_TYPE, COUNT, RES_TYPE>::FreeResSpace(res_ptr ptr_)
{
    COM_ASSERT(true == m_bInitFlag);
    if (NULL != m_pResList)
    {
        m_pResMutex->Lock();
        bool ret = m_pResList->freeSpace(ptr_);
        m_pResMutex->Unlock();
        if (ret)
            m_pResQueueSpaceEvent->Send();
    }
    return;
}

template <typename DATA_TYPE, int COUNT, typename RES_TYPE>
typename ComInterProcessMsgQueue<DATA_TYPE, COUNT, RES_TYPE>::res_ptr
ComInterProcessMsgQueue<DATA_TYPE, COUNT, RES_TYPE>::FindAndRemoveFromResList(res_key_type key_)
{
    typename res_list_type::iterator it = m_pResList->begin();
    typename res_list_type::iterator itEnd = m_pResList->end();
    for (; it != itEnd; ++it)
    {
        if (key_ == it->getKey())
        {
            it.remove();
            break;
        }
    }
    return (it.current());
}

#endif


