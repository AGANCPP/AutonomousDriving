
#include "StdAfx.h"
#include "ComDebug.h"
#include <windows.h>


int OpenLogFile(COMM_LOG_FILE* pFile)
{
    if ((NULL == pFile) || (NULL != pFile->m_pFile))
        return (-1);
    int fileLen = strlen(pFile->m_fileName);
    if ((fileLen <= 0) || (fileLen >= COMM_LOG_FILE::MAX_FILE_NAME))
        return (-1);
    pFile->m_pFile = fopen(pFile->m_fileName, "w+t");
    if (NULL == pFile->m_pFile)
        return (-1);
    return (0);
}

int WriteToLogFile(COMM_LOG_FILE* pFile, const char* pInfo)
{
    if ((NULL == pFile) || (NULL == pInfo) || (NULL == pFile->m_pFile))
        return (-1);
    return (fputs(pInfo, pFile->m_pFile));
}

int CloseLogFile(COMM_LOG_FILE* pFile)
{
    if (NULL == pFile)
        return (-1);
    if (NULL != pFile->m_pFile)
    {
        fclose(pFile->m_pFile);
        pFile->m_pFile = NULL;
    }
    return (0);
}



typedef struct
{
    HANDLE m_hMutex;
    DWORD m_timeout;
} TRACE_MUTEX;

BOOL TraceMutexInit(TRACE_MUTEX* pMutex, TCHAR* pName)
{
    if (NULL == pMutex)
        return (FALSE);
    pMutex->m_hMutex = CreateMutex(NULL, FALSE, pName);
    if (NULL == pMutex->m_hMutex)
        return (FALSE);
    return (TRUE);
}

BOOL TraceMutexDestroy(TRACE_MUTEX* pMutex)
{
    if (NULL == pMutex)
        return (FALSE);
    if (NULL != pMutex->m_hMutex)
        CloseHandle(pMutex->m_hMutex);
    return (TRUE);
}

BOOL TraceMutexLock(TRACE_MUTEX* pMutex)
{
    BOOL bRetVal = FALSE;
    DWORD  dwRet = 0;
    if (NULL == pMutex)
        return (FALSE);
    if (NULL == pMutex->m_hMutex)
        return (FALSE);
    dwRet = WaitForSingleObject(pMutex->m_hMutex, pMutex->m_timeout);
    switch (dwRet)
    {
        case WAIT_OBJECT_0:
        case WAIT_ABANDONED:    // mutex only
            bRetVal = TRUE;
            break;
        case WAIT_TIMEOUT:
        case WAIT_FAILED:
        default:
            bRetVal = FALSE;
            break;
    };
    return (bRetVal);
}

BOOL TraceMutexUnLock(TRACE_MUTEX* pMutex)
{
    if (NULL == pMutex)
        return (FALSE);
    if (NULL == pMutex->m_hMutex)
        return (FALSE);
    ReleaseMutex(pMutex->m_hMutex);
    return (TRUE);
}


static TRACE_MUTEX s_MyDebugOutputlocker = {0, 30000};

void DebugOutputLock(void)
{
    TraceMutexLock(&s_MyDebugOutputlocker);
}

void DebugOutputUnlock(void)
{
    TraceMutexUnLock(&s_MyDebugOutputlocker);
}

//Output to terminal
#define COMM_LOG_OUTPUT_FILE 1
#define COMM_LOG_USE_LOCK 1

#define TEST_BUFFER_SIZE 512
static BOOL b_traceInit = FALSE;
static char s_szBuffer[TEST_BUFFER_SIZE];
COMM_LOG_FILE s_commLogFile = {0};
static BOOL b_outToFile = FALSE;
int g_nLogLevel = 3;

int OpenLogRecord(const char* path_)
{
    s_commLogFile.m_pFile = NULL;
    if (NULL == path_)
    {
        printf("[ERR]CAM: OpenLogRecord(), path is null\n");
        return (-1);
    }
#if (COMM_LOG_OUTPUT_FILE)
    int fileLen = strlen(path_);
    if (fileLen >= COMM_LOG_FILE::MAX_FILE_NAME - 64)
    {
        printf("[ERR]CAM: OpenLogRecord(), path too long\n");
        return (-1);
    }
    if (NULL != s_commLogFile.m_pFile)
    {
        printf("[ERR]CAM: OpenLogRecord(), can not set path agin\n");
        return (-1);
    }
    SYSTEMTIME st;
    GetLocalTime(&st);
    sprintf_s(s_commLogFile.m_fileName, COMM_LOG_FILE::MAX_FILE_NAME - 1,
              "%s\\cam_%04d%02d%02d%02d%02d%02d.log", path_, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
    //strcpy(s_commLogFile.m_fileName, "TestLog.log");
    if (0 != OpenLogFile(&s_commLogFile))
    {
        printf("[ERR]CAM: OpenLogRecord(), open file err\n");
        return (-1);
    }
#endif
    return (0);
}

int EnableLogRecord(int enable_)
{
#if (COMM_LOG_OUTPUT_FILE)
    if (NULL != s_commLogFile.m_pFile)
        b_outToFile = enable_;
    else
        printf("[ERR]CAM: StartLogRecord(), should set log path first\n");
#endif
    return (0);
}

int CloseLogRecord(void)
{
#if (COMM_LOG_OUTPUT_FILE)
    b_outToFile = FALSE;
    CloseLogFile(&s_commLogFile);
#endif
    return (0);
}

void ToTerminal(const char* pszText)
{
    //    OutputDebugStringA(pszText);
    printf(pszText);
#if (COMM_LOG_OUTPUT_FILE)
    // write to file
    if (b_outToFile)
        WriteToLogFile(&s_commLogFile, pszText);
#endif // COMM_LOG_OUTPUT_FILE
}

void PrintToTerminal(const char* fmt, ...)
{
    va_list argptr;
    //char* szBuffer = (char*)malloc(TEST_BUFFER_SIZE);
    char* szBuffer = s_szBuffer;
    if (NULL == fmt)
        return;
    if (FALSE == b_traceInit)
    {
#if (COMM_LOG_USE_LOCK)
        TraceMutexInit(&s_MyDebugOutputlocker, TEXT("MY_CAMERA_LOG_DEBUG_OUTPUT_LOCKER"));
#endif
        b_traceInit = TRUE;
    }
#if (COMM_LOG_USE_LOCK)
    TraceMutexLock(&s_MyDebugOutputlocker);
#endif // COMM_LOG_USE_LOCK
    SYSTEMTIME st;
    GetLocalTime(&st);
    sprintf(&szBuffer[0], "%04d-%02d-%02d %02d:%02d:%02d:%03d ",
            st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    va_start(argptr, fmt);
    _vsnprintf(&szBuffer[24], TEST_BUFFER_SIZE - 24, fmt, argptr);
    va_end(argptr);
    szBuffer[TEST_BUFFER_SIZE - 1] = 0;
    ToTerminal(szBuffer);
#if (COMM_LOG_USE_LOCK)
    TraceMutexUnLock(&s_MyDebugOutputlocker);
#endif
    //free(szBuffer);
}




