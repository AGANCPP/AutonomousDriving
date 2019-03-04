

#include "stdafx.h"
#include "ComThread.h."
#include "ComDebug.h"

unsigned int __stdcall CComThread::ThreadProcedure(void* pArg_)
{
    int nRet = 0;
    CComThread* pComThread = (CComThread*)pArg_;
    if (NULL != pComThread)
        nRet = pComThread->Run();
    return (nRet);
}

CComThread::CComThread()
{
    m_hThread = 0;
    m_nThreadId = 0;
    m_nPriority = THREAD_PRIORITY_NORMAL;
}

CComThread::~CComThread()
{
}

BOOL CComThread::Create(void)
{
    unsigned int initFlag = CREATE_SUSPENDED;
    m_hThread = ComCreateThread(NULL, 0, CComThread::ThreadProcedure, this, initFlag, (unsigned int*)&m_nThreadId);
    if (NULL == m_hThread)
        return (FALSE);
    SetThreadPriority(m_hThread, m_nPriority);
    return (TRUE);
}

BOOL CComThread::ResumeThread(void)
{
    if (NULL != m_hThread)
    {
        ComResumeThread(m_hThread);
        return (TRUE);
    }
    return (FALSE);
}

BOOL CComThread::SuspendThread(void)
{
    if (NULL != m_hThread)
    {
        ComSuspendThread(m_hThread);
        return (TRUE);
    }
    return (FALSE);
}

unsigned int CComThread::Run(void)
{
    return (0);
}

int CComThread::WaitThreadExit(int nCount, int nTimeout_)
{
    int nRet = 0;
    if ((nCount < 1) || (nTimeout_ < 100))
    {
        nCount = 1;
        nTimeout_ = 100;
    }
    BOOL bExitFlag = FALSE;
    DWORD dwRetValue = 0;
    for (int i = 0; (i < 2) && (FALSE == bExitFlag); i++)
    {
        dwRetValue = WaitForSingleObject(m_hThread, 10000);
        switch (dwRetValue)
        {
            case WAIT_OBJECT_0:
                bExitFlag = TRUE;
                break;
            case WAIT_TIMEOUT:
            case WAIT_FAILED:
                nRet = -1;
                ::TerminateThread(m_hThread, 1);
                COM_ERR("[ERR]Waiting thread Error!\n");
            default:
                nRet = -1;
                break;
        };
    }
    CloseHandle(m_hThread);
    m_hThread = 0;
    return (nRet);
}

BOOL CComThread::CloseThreadHandle(void)
{
    return (CloseHandle(m_hThread));
}


ComWork::ComWork(char* pName_)
{
    if (NULL != pName_)
        strncpy(m_moduleName, pName_, MAX_NAME_LENGTH);
    m_hWorkEvent = 0;
    m_bPauseFlag = FALSE;
    m_nRunningFlag = FALSE;
}

ComWork::~ComWork()
{
}

BOOL ComWork::StartWork()
{
    if (TRUE == m_nRunningFlag)
    {
        COM_WARN("[WARN]CAM: %s, thread alread created\n", GetModuleName());
        return (FALSE);
    }
    InterlockedExchange(&m_nRunningFlag, TRUE);
    m_hWorkEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (NULL == m_hWorkEvent)
    {
        COM_ERR("[ERR]CAM: %s: CreateEvent error\n", GetModuleName());
        goto FAIL01;
    }
    COM_DEBUG(DEBUG_LEVEL3, ("CAM: Create %s thread\n", GetModuleName()));
    BOOL bRet = Create();
    if (FALSE == bRet)
    {
        COM_ERR("[ERR]CAM: %s: Create thread error\n", GetModuleName());
        goto FAIL02;
    }
    ResumeThread();
    COM_DEBUG(DEBUG_LEVEL1, ("CAM: \"%s\" start working\n", GetModuleName()));
    return (TRUE);
FAIL02:
    CloseHandle(m_hWorkEvent);
FAIL01:
    InterlockedExchange(&m_nRunningFlag, FALSE);
    return (FALSE);
}

BOOL ComWork::EndWork(void)
{
    if (FALSE == m_nRunningFlag)
    {
        COM_WARN("[WARN]CAM: %s, thread alread stopped\n", GetModuleName());
        return (FALSE);
    }
    COM_DEBUG(DEBUG_LEVEL3, ("CAM: %s: End thread\n", GetModuleName()));
    InterlockedExchange(&m_nRunningFlag, FALSE);
    SetEvent(m_hWorkEvent);
    WaitThreadExit();
    CloseHandle(m_hWorkEvent);
    m_hWorkEvent = 0;
    COM_DEBUG(DEBUG_LEVEL1, ("CAM: \"%s\" end working\n", GetModuleName()));
    return (TRUE);
}

unsigned int ComWork::Run(void)
{
    DWORD dwResult = 0;
    unsigned int nRet = -1;
    COM_DEBUG(DEBUG_LEVEL3, ("CAM: %s thread running\n", GetModuleName()));
    if (FALSE == BeforeRunning())
    {
        COM_ERR("[ERR]CAM: %s: BeforeRunning error\n", GetModuleName());
        return (-1);
    }
    while (InterlockedCompareExchange(&m_nRunningFlag, TRUE, TRUE))
    {
        nRet = HandleMessage();
        if (WORK_EXIT == nRet)
            break;
    }
    AfterRunning();
    EndWork();
    return (0);
}

unsigned int ComWork::HandleMessage(void)
{
    COM_WARN("[WARN]CAM: %s: handle messsage in base class\n", GetModuleName());
    return (WORK_EXIT);
}

BOOL ComWork::BeforeRunning(void)
{
    return (TRUE);
}

void ComWork::AfterRunning(void)
{
    return;
}



CDataPool::CDataPool(size_type dataSize_, size_type maxAmount_)
{
    if ((dataSize_) <= 0 || (maxAmount_ <= 0))
    {
        throw std::exception("CDataPool parameter error!");
        return;
    }
    pool_data_size = HEAD_SIZE + ((dataSize_ + DATA_ALIGN - 1) & ~(DATA_ALIGN - 1));
    max_amount = maxAmount_;
    pool_size = pool_data_size * max_amount;
    m_pDataPool = new char[pool_size];
    if (NULL == m_pDataPool)
    {
        throw std::exception("CDataPool new pool error!");
        return;
    }
    m_bMap = new bool[max_amount];
    if (NULL == m_bMap)
    {
        delete [] m_pDataPool;
        throw std::exception("CDataPool new pool map error!");
        return;
    }
    initialize();
}

CDataPool::~CDataPool()
{
    delete [] m_pDataPool;
    delete [] m_bMap;
}

void CDataPool::initialize(void)
{
    char* pNextNode = (char*)m_pDataPool;
    size_type nDataSize = pool_data_size;
    for (int i = 0; i < max_amount - 1; ++i)
    {
        *((pointer*)pNextNode) = (pointer)(pNextNode + nDataSize);
        pNextNode += nDataSize;
        m_bMap[i] = false;
    }
    m_bMap[max_amount - 1] = false;
    *((pointer*)pNextNode) = NULL;
    m_pFree = (pointer)m_pDataPool;
}

CDataPool::pointer CDataPool::allocate()
{
    pointer pRetVal = m_pFree;
    if (NULL != m_pFree)
    {
        size_type nIndex = ((size_type)pRetVal - (size_type)m_pDataPool) / pool_data_size;
        m_bMap[nIndex] = true;
        m_pFree = *((pointer*)m_pFree);
    }
    if (NULL != pRetVal)
        pRetVal = (pointer)((size_type)pRetVal + HEAD_SIZE);
    return (pointer)pRetVal;
}

bool  CDataPool::deallocate(void* p)
{
    if (NULL == p)
        return (false);
    size_type nBytes = (size_type)p - HEAD_SIZE - (size_type)m_pDataPool;
    size_type nIndex = nBytes / pool_data_size;
    if ((nIndex >= max_amount) ||
        (false == m_bMap[nIndex]))
        return (false);
    m_bMap[nIndex] = false;
    pointer pAddr = (pointer)((char*)m_pDataPool + nIndex * pool_data_size);
    *((pointer*)pAddr) = m_pFree;
    m_pFree = pAddr;
    return (true);
}

bool CDataPool::is_dup_dealloc(void* p)
{
    if (NULL == p)
        return (true);
    size_type nBytes = (size_type)p - HEAD_SIZE - (size_type)m_pDataPool;
    size_type nIndex = nBytes / pool_data_size;
    if ((nIndex >= max_amount) ||
        (false == m_bMap[nIndex]))
        return (true);
    return (false);
}

bool CDataPool::is_valid_ptr(void* p)
{
    if (NULL == p)
        return (false);
    size_type nBytes = (size_type)p - HEAD_SIZE - (size_type)m_pDataPool;
    size_type nIndex = nBytes / pool_data_size;
    if ((nIndex >= max_amount) ||
        (false == m_bMap[nIndex]))
        return (false);
    return (true);
}

CDataPool::pointer CDataPool::list_first_buff(void)
{
    //pointer pRetVal = m_pFree;
    pointer pRetVal = m_pDataPool;
    //if (NULL != m_pFree) {
    //    size_type nIndex = ((size_type)pRetVal - (size_type)m_pDataPool) / pool_data_size;
    //    m_bMap[nIndex] = true;
    //    m_pFree = *((pointer*)m_pFree);
    //}
    if (NULL != pRetVal)
        pRetVal = (pointer)((size_type)pRetVal + HEAD_SIZE);
    return (pointer)pRetVal;
}

CDataPool::pointer CDataPool::list_next_buff(void* p)
{
    if (NULL == p)
        return (NULL);
    size_type nBytes = (size_type)p - HEAD_SIZE - (size_type)m_pDataPool;
    size_type nIndex = nBytes / pool_data_size;
    //if ((nIndex >= max_amount) ||
    //    (true == m_bMap[nIndex])) {
    //    return (NULL);
    //}
    if (nIndex >= max_amount)
        return (NULL);
    pointer pAddr = (pointer)((char*)m_pDataPool + nIndex * pool_data_size);
    //pAddr = *((pointer*)pAddr);
    if (nIndex + 1 < max_amount)
        pAddr = (pointer)((size_type)pAddr + pool_data_size);
    else
        pAddr = NULL;
    if (NULL != pAddr)
        pAddr = (pointer)((size_type)pAddr + HEAD_SIZE);
    return (pAddr);
}

void CDataPool::debug()
{
    char* pNextNode = (char*)m_pDataPool;
    size_type nDataSize = pool_data_size;
    for (int i = 0; i < max_amount; ++i)
    {
        printf("No. %d : %p\n", i, (pointer*)pNextNode);
        pNextNode += nDataSize;
    }
}
