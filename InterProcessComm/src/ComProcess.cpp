
#include "stdafx.h"
#include <exception>
#include "ComProcess.h"
#include "ComDebug.h"

ComMutex::ComMutex(BOOL bInitialOwner_, LPCTSTR name_)
{
    m_hMutex = CreateMutex(NULL, bInitialOwner_, name_);
    if (NULL == m_hMutex)
    {
        COM_ERR("[P 0x%08x][ERR]ComMutex::ComMutex::Create mutex err\n", GetCurrentProcessId());
        throw std::exception("ComMutex::ComMutex::Create mutex err");
        return;
    }
}

ComMutex::~ComMutex()
{
    ReleaseMutex(m_hMutex);
    CloseHandle(m_hMutex);
}

DWORD ComMutex::Lock(DWORD msec_)
{
    DWORD dwRet = 0;
    dwRet = WaitForSingleObject(m_hMutex, msec_);
    switch (dwRet)
    {
        case WAIT_OBJECT_0:
        case WAIT_ABANDONED:    // mutex only
            break;
        case WAIT_TIMEOUT:
            COM_DEBUG(DEBUG_LEVEL2, ("[P 0x%08x] ComMutex::Lock waiting mutex timeout\n", GetCurrentProcessId()));
            break;
        case WAIT_FAILED:
        default:
            COM_ERR("[P 0x%08x][ERR]ComMutex::Lock waiting mutex err\n", GetCurrentProcessId());
            throw std::exception("ComMutex::Lock waiting mutex err");
            break;
    };
    return (dwRet);
}

BOOL ComMutex::Unlock()
{
    BOOL bRetVal = FALSE;
    bRetVal = ReleaseMutex(m_hMutex);
    if (FALSE == bRetVal)
    {
        COM_ERR("[P 0x%08x][ERR]ComMutex::Unlock release mutex err\n", GetCurrentProcessId());
        throw std::exception("ComMutex::Unlock release mutex err");
    }
    return (bRetVal);
}


ComEvent::ComEvent(BOOL bManualReset_, BOOL bInitialState_, LPCTSTR name_)
{
    m_hEvent = CreateEvent(NULL, bManualReset_, bInitialState_, name_);
    if (NULL == m_hEvent)
    {
        COM_ERR("[P 0x%08x][ERR]ComEvent::ComEvent::Create event err\n", GetCurrentProcessId());
        throw std::exception("ComEvent::ComEvent::Create event err");
        return;
    }
}

ComEvent::~ComEvent()
{
    SetEvent(m_hEvent);
    CloseHandle(m_hEvent);
}

DWORD ComEvent::Wait(DWORD msec_)
{
    DWORD dwRet = 0;
    dwRet = WaitForSingleObject(m_hEvent, msec_);
    switch (dwRet)
    {
        case WAIT_OBJECT_0:
            break;
        case WAIT_TIMEOUT:
            COM_DEBUG(DEBUG_LEVEL2, ("[P 0x%08x] ComEvent::Wait waiting event timeout\n", GetCurrentProcessId()));
            break;
        case WAIT_FAILED:
        default:
            COM_ERR("[P 0x%08x][ERR]ComEvent::Wait waiting event err\n", GetCurrentProcessId());
            throw std::exception("ComEvent::Wait waiting event err");
            break;
    };
    return (dwRet);
}

BOOL ComEvent::Send(void)
{
    BOOL bRetVal = FALSE;
    bRetVal = SetEvent(m_hEvent);
    if (FALSE == bRetVal)
    {
        COM_ERR("[P 0x%08x][ERR]ComEvent::Send set event err\n", GetCurrentProcessId());
        throw std::exception("ComEvent::Send set event err");
    }
    return (bRetVal);
}


ComSemaphore::ComSemaphore(long lInitialCount_, long lMaximumCount_, LPCTSTR name_)
{
    m_hSemaphore = CreateSemaphore(NULL, lInitialCount_, lMaximumCount_, name_);
    if (NULL == m_hSemaphore)
    {
        COM_ERR("[P 0x%08x][ERR]ComSemaphore::ComSemaphore Create semaphore err\n", GetCurrentProcessId());
        throw std::exception("ComSemaphore::ComSemaphore Create semaphore err");
        return;
    }
}

ComSemaphore::~ComSemaphore()
{
    //ReleaseSemaphore(m_hSemaphore, 1, NULL);
    CloseHandle(m_hSemaphore);
}

DWORD ComSemaphore::Wait(DWORD msec_)
{
    DWORD dwRet = 0;
    dwRet = WaitForSingleObject(m_hSemaphore, msec_);
    switch (dwRet)
    {
        case WAIT_OBJECT_0:
            break;
        case WAIT_TIMEOUT:
            COM_DEBUG(DEBUG_LEVEL2, ("[P 0x%08x] ComSemaphore::Wait waiting semaphore timeout\n", GetCurrentProcessId()));
            break;
        case WAIT_FAILED:
        default:
            COM_ERR("[P 0x%08x][ERR]ComSemaphore::Wait waiting semaphore err\n", GetCurrentProcessId());
            throw std::exception("ComSemaphore::Wait waiting semaphore err");
            break;
    };
    return (dwRet);
}

BOOL ComSemaphore::Post(void)
{
    BOOL bRetVal = FALSE;
    bRetVal = ReleaseSemaphore(m_hSemaphore, 1, NULL);
    if (FALSE == bRetVal)
    {
        COM_ERR("[P 0x%08x][ERR]ComSemaphore::Post release semaphore err\n", GetCurrentProcessId());
        throw std::exception("ComSemaphore::Post release semaphore err");
    }
    return (bRetVal);
}


ComShareMem::ComShareMem(DWORD memSize_, LPCTSTR name_)
{
    m_memSize = 0;
    m_hFileMap = NULL;
    m_pView = NULL;
    memset(m_memName, 0, sizeof(m_memName));
    if (NULL == name_)
    {
        COM_ERR("[P 0x%08x][ERR]ComShareMem::ComShareMem name is null\n", GetCurrentProcessId());
        return;
    }
    DWORD nNameLen = lstrlen(name_);
    if ((nNameLen <= 0) || (nNameLen > MAX_MEM_NEME_LEN - 1))
    {
        COM_ERR("[P 0x%08x][ERR]ComShareMem::ComShareMem name length (%d) not good\n", GetCurrentProcessId(), nNameLen);
        return;
    }
    m_memSize = memSize_;
    lstrcpyn(m_memName, name_, MAX_MEM_NEME_LEN - 1);
}

ComShareMem::~ComShareMem(void)
{
    Destroy();
}

char* ComShareMem::GetShareMemAddr(BOOL& isAlreadyExist_)
{
    isAlreadyExist_ = FALSE;
    if (NULL != m_pView)
        return (m_pView);
    if (NULL != m_hFileMap)
    {
        COM_ERR("[P 0x%08x][ERR]ComShareMem::GetShareMemAddr create share memory err\n", GetCurrentProcessId());
        return (NULL);
    }
    DWORD nNameLen = lstrlen(m_memName);
    if ((nNameLen <= 0) || (nNameLen > MAX_MEM_NEME_LEN - 1))
    {
        COM_ERR("[P 0x%08x][ERR]ComShareMem::GetShareMemAddr name length (%d) not good\n", GetCurrentProcessId(), nNameLen);
        return (NULL);
    }
    COM_DEBUG(DEBUG_LEVEL3, ("[P 0x%08x] ComShareMem::GetShareMemAddr create share memery start\n", GetCurrentProcessId()));
    m_hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, m_memSize, m_memName);
    if (NULL == m_hFileMap)
    {
        COM_ERR("[P 0x%08x][ERR]ComShareMem::GetShareMemAddr CreateFileMapping err\n", GetCurrentProcessId());
        goto FAIL01;
    }
    //printf("\n");
    //wprintf(m_memName);
    //printf("\n");
    if (ERROR_ALREADY_EXISTS == GetLastError())
    {
        isAlreadyExist_ = TRUE;
        COM_DEBUG(DEBUG_LEVEL3, ("[P 0x%08x] ComShareMem::GetShareMemAddr share memery alread exist\n", GetCurrentProcessId()));
    }
    m_pView = (char*)MapViewOfFile(m_hFileMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
    if (INVALID_HANDLE_VALUE == m_pView)
    {
        COM_ERR("[P 0x%08x][ERR]ComShareMem::GetShareMemAddr MapViewOfFile err\n", GetCurrentProcessId());
        goto FAIL02;
    }
    COM_DEBUG(DEBUG_LEVEL3, ("[P 0x%08x] ComShareMem::GetShareMemAddr create share memery end (OK)\n", GetCurrentProcessId()));
    return (m_pView);
FAIL02:
    CloseHandle(m_hFileMap);
FAIL01:
    // return null;
    return (NULL);
}

void ComShareMem::Destroy(void)
{
    if (NULL != m_pView)
    {
        UnmapViewOfFile(m_pView);
        m_pView = NULL;
    }
    if (NULL != m_hFileMap)
    {
        CloseHandle(m_hFileMap);
        m_hFileMap = NULL;
    }
}

