

#include "stdafx.h"
#include "ComTools.h"

int GetAllFiles(TCHAR* path_, std::vector<WIN32_FIND_DATA>& vResult_)
{
    int nCount = 0;
    BOOL bRet = FALSE;
    HANDLE hfd;
    WIN32_FIND_DATA wfd;
    TCHAR cdir[MAX_PATH];
    //printf("sizeof(WIN32_FIND_DATA) = %d\n", sizeof(WIN32_FIND_DATA));
    GetCurrentDirectory(MAX_PATH, cdir);
    SetCurrentDirectory(path_);
    hfd = FindFirstFile(TEXT("*.*"), &wfd);
    if (INVALID_HANDLE_VALUE != hfd)
    {
        do
        {
            if (FILE_ATTRIBUTE_DIRECTORY != (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                ++nCount;
                //#ifdef UNICODE
                //wprintf(TEXT("[%03d]file: %s\n"), nCount, wfd.cFileName);
                //#else
                //printf("[%03d]file: %s\n", nCount, wfd.cFileName);
                //#endif
                vResult_.push_back(wfd);
            }
            bRet = FindNextFile(hfd, &wfd);
        }
        while (FALSE != bRet);
    }
    SetCurrentDirectory(cdir);
    return (nCount);
}

int GetCurrWorkPath(char* buff_, size_t size_)
{
    return (GetCurrentDirectoryA(size_, buff_));
}

int GetModuleLocation(char* buff_, size_t size_)
{
    char full_name[MAX_PATH] = { 0 };
    int size = 0;
    char* pCut = NULL;
    size = GetModuleFileNameA(NULL, buff_, size_);
    //printf("full module name = %s\n", full_name);
    if (pCut = strrchr(buff_, '\\'))
    {
        *(pCut) = 0;
        size = strlen(buff_);
    }
    else
        size = 0;
    return (size);
}

BOOL SetCurrProcessDir(char* dir_)
{
    return (SetCurrentDirectoryA(dir_));
}

//Timing
__int64 FileTimeToQuadWord(PFILETIME pft)
{
    return (Int64ShllMod32(pft->dwHighDateTime, 32) | pft->dwLowDateTime);
}

unsigned int GetCpuTick(void)
{
    return (unsigned int)GetTickCount();
}

unsigned int GetThreadTimeMs(void)
{
    FILETIME ftKernelTime, ftUserTime, ftDummy;
    GetThreadTimes(GetCurrentThread(), &ftDummy, &ftDummy, &ftKernelTime, &ftUserTime);
    return (unsigned int)((FileTimeToQuadWord(&ftKernelTime) + FileTimeToQuadWord(&ftUserTime)) / 10000);
}

unsigned int CalcElapsedTime(unsigned int nStart_, unsigned int nEnd_)
{
    if (nEnd_ >= nStart_)
        return (nEnd_ - nStart_);
    else
        return (0xFFFFFFFF - nStart_ + nEnd_);
}

__time64_t GetCurrGMTime()
{
    union FT
    {
        unsigned __int64 ft_scalar;
        FILETIME ft_struct;
    } nt_time;
    GetSystemTimeAsFileTime(&(nt_time.ft_struct));
    __time64_t tim = (__time64_t)((nt_time.ft_scalar - 116444736000000000i64) / 10000i64);
    return tim;
}