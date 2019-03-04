
#include "stdafx.h"
#include <new>
#include "ComDataPool.h"
#include "ComProcess.h"
#include "ComDebug.h"
#include "SerialPortWin32.h"


void TestComInterProcessWithOtherProcess(void)
{
    COM_DEBUG(DEBUG_LEVEL3, ("[P 0x%08x] %s+\n", GetCurrentProcessId(), __FUNCTION__));
    typedef struct _DATA_TYPE
    {
        int a;
        int b;
        int c;
    } DATA_TYPE;
    typedef c_map_type<int, DATA_TYPE> res_type;
    ComInterProcessMsgQueue<DATA_TYPE, 3, res_type> msgQueue(true, TEXT("TestComInterProcessWithOhterProcess"));
    DATA_TYPE* pdata;
    msgQueue.Create(true);
    COM_DEBUG(DEBUG_LEVEL3, ("[P 0x%08x] Send to message queue...\n", GetCurrentProcessId()));
    for (int i = 0; i < 3; i++)
    {
        pdata = msgQueue.AllocSpace(6000);
        if (NULL != pdata)
        {
            int value = i;
            pdata->a = value++;
            pdata->b = value++;
            pdata->c = value++;
            COM_DEBUG(DEBUG_LEVEL3, ("[P 0x%08x] [%d]Send message (%d, %d, %d)\n", GetCurrentProcessId(), i, pdata->a, pdata->b, pdata->c));
            msgQueue.PushBack(pdata);
            COM_DEBUG(DEBUG_LEVEL3, ("[P 0x%08x] [%d]Waiting res (%d) ...\n", GetCurrentProcessId(), i, i));
            res_type* res = msgQueue.WaitRes(i);
            COM_DEBUG(DEBUG_LEVEL3, ("[P 0x%08x] [%d]Wait res (%d) ok\n", GetCurrentProcessId(), i, res->getKey()));
            msgQueue.FreeResSpace(res);
        }
        else
            COM_DEBUG(DEBUG_LEVEL3, ("[P 0x%08x] [%d]Send message error!!!\n", GetCurrentProcessId(), i));
    }
    COM_DEBUG(DEBUG_LEVEL3, ("[P 0x%08x] Send to message queue end!!!\n", GetCurrentProcessId()));
    COM_DEBUG(DEBUG_LEVEL3, ("[P 0x%08x] %s-\n", GetCurrentProcessId(), __FUNCTION__));
    getchar();
}

void TestComInterProcessMsgQueue(void)
{
    COM_DEBUG(DEBUG_LEVEL3, ("%s *** begin***\n", __FUNCTION__));
    typedef struct _DATA_TYPE
    {
        int a;
    } DATA_TYPE;
    typedef c_map_type<int, DATA_TYPE> res_type;
    {
#if 1
        ComInterProcessMsgQueue<DATA_TYPE, 1> msgQueue(false, TEXT("TestComInterProcessMsgQueue"));
        msgQueue.Create();
        DATA_TYPE* pData1 = msgQueue.AllocSpace();
        COM_DEBUG(DEBUG_LEVEL3, ("AllocSpace=%p\n", pData1));
        msgQueue.PushBack(pData1);
        pData1 = msgQueue.GetFront(1000);
        COM_DEBUG(DEBUG_LEVEL3, ("GetFront=%p\n", pData1));
        msgQueue.FreeSpace(pData1);
        pData1 = msgQueue.AllocSpace(1000);
        COM_DEBUG(DEBUG_LEVEL3, ("AllocSpace=%p\n", pData1));
#endif
    }
    {
        ComInterProcessMsgQueue<DATA_TYPE, 1, res_type> msgQueue(false, TEXT("TestComInterProcessMsgQueue"));
        msgQueue.Create(true);
        res_type* res01 = msgQueue.AllocResSpace();
        COM_DEBUG(DEBUG_LEVEL3, ("AllocResSpace=%p\n", res01));
        //res01->key = 1;
        res01->getKey() = 1;
        msgQueue.SendRes(res01);
        res01 = msgQueue.WaitRes(1, 1000);
        COM_DEBUG(DEBUG_LEVEL3, ("WaitRes=%p\n", res01));
        msgQueue.FreeResSpace(res01);
    }
    COM_DEBUG(DEBUG_LEVEL3, ("%s *** end***\n", __FUNCTION__));
}

void TestComInterProcessWithOhterProcess(void)
{
    COM_DEBUG(DEBUG_LEVEL3, ("[P 0x%08x] %s+\n", GetCurrentProcessId(), __FUNCTION__));
    typedef struct _DATA_TYPE
    {
        int a;
        int b;
        int c;
    } DATA_TYPE;
    typedef c_map_type<int, DATA_TYPE> res_type;
    ComInterProcessMsgQueue<DATA_TYPE, 3, res_type> msgQueue(true, TEXT("TestComInterProcessWithOhterProcess"));
    DATA_TYPE* pdata = NULL;
    msgQueue.Create(true);
    COM_DEBUG(DEBUG_LEVEL3, ("[P 0x%08x] Receiving from message queue...\n", GetCurrentProcessId()));
    for (int i = 0; i < 3; i++)
    {
        pdata = msgQueue.GetFront(5000);
        if (NULL != pdata)
        {
            COM_DEBUG(DEBUG_LEVEL3, ("[P 0x%08x] [%d]Receive message (%d, %d, %d)\n", GetCurrentProcessId(), i, pdata->a, pdata->b, pdata->c));
            msgQueue.FreeSpace(pdata);
            res_type* res = msgQueue.AllocResSpace();
            res->getKey() = i;
            COM_DEBUG(DEBUG_LEVEL3, ("[P 0x%08x] [%d]SendRes (%d) ...\n", GetCurrentProcessId(), i, i));
            msgQueue.SendRes(res);
            COM_DEBUG(DEBUG_LEVEL3, ("[P 0x%08x] [%d]SendRes (%d) ok\n", GetCurrentProcessId(), i, i));
        }
        else
            COM_DEBUG(DEBUG_LEVEL3, ("[P 0x%08x] [%d]Receive message error!!!\n", GetCurrentProcessId(), i));
    }
    COM_DEBUG(DEBUG_LEVEL3, ("[P 0x%08x] Receive message end!!!\n", GetCurrentProcessId()));
    COM_DEBUG(DEBUG_LEVEL3, ("[P 0x%08x] %s-\n", GetCurrentProcessId(), __FUNCTION__));
    getchar();
}

void TestComDataPoolList02(void)
{
    COM_DEBUG(DEBUG_LEVEL3, ("[P 0x%08x] %s+\n", GetCurrentProcessId(), __FUNCTION__));
    typedef struct _DATA_TYPE
    {
        int a;
    } DATA_TYPE;
    ComDataPoolList02<DATA_TYPE, 3> list01(true);
    DATA_TYPE* pData01 = list01.allocSpace();
    list01.pushBack(pData01);
    //list01.debug();
    DATA_TYPE* pData02 = list01.allocSpace();
    list01.pushBack(pData02);
    DATA_TYPE* pData03 = list01.allocSpace();
    list01.pushBack(pData03);
    list01.debug();
    typedef ComDataPoolList02<DATA_TYPE, 3>::iterator itr_type01;
    itr_type01 it1 = list01.begin();
    itr_type01 itEnd = list01.end();
    for (int i = 0; it1 != itEnd; ++it1, ++i)
        COM_DEBUG(DEBUG_LEVEL3, ("[%d] current: %p\n", i, it1.current()));
    //it1.remove();
    //list01.debug();
    it1 = list01.begin();
    ++it1;
    ++it1;
    it1.remove();
    list01.debug();
    COM_DEBUG(DEBUG_LEVEL3, ("[P 0x%08x] %s-\n", GetCurrentProcessId(), __FUNCTION__));
}

void TestEnumPortsWdm(void)
{
    COM_DEBUG(DEBUG_LEVEL3, ("[P 0x%08x] %s+\n", GetCurrentProcessId(), __FUNCTION__));
    std::vector<SERIAL_INFO> serialInfo;
    EnumPortsWdm(serialInfo);
    std::vector<SERIAL_INFO>::iterator it = serialInfo.begin();
    COM_DEBUG(DEBUG_LEVEL3, ("serial ports: \n"));
    for (int i = 0; it != serialInfo.end(); ++it, ++i)
    {
        //#ifdef UNICODE
        //wprintf(TEXT("[%d]devPath: %s\n"), i, it->devPath);
        //wprintf(TEXT("[%d]portName: %s\n"), i, it->portName);
        //wprintf(TEXT("[%d]friendlyName: %s\n"), i, it->friendlyName);
        //wprintf(TEXT("[%d]portDesc: %s\n"), i, it->portDesc);
        //wprintf(TEXT("[%d]bUsbDevice: %d\n"), i, it->bUsbDevice);
        //#else
        COM_DEBUG(DEBUG_LEVEL3, ("[%d]devPath: %s\n", i, it->devPath));
        COM_DEBUG(DEBUG_LEVEL3, ("[%d]portName: %s\n", i, it->portName));
        COM_DEBUG(DEBUG_LEVEL3, ("[%d]friendlyName: %s\n", i, it->friendlyName));
        COM_DEBUG(DEBUG_LEVEL3, ("[%d]portDesc: %s\n", i, it->portDesc));
        COM_DEBUG(DEBUG_LEVEL3, ("[%d]bUsbDevice: %d\n", i, it->bUsbDevice));
        //#endif
    }
    COM_DEBUG(DEBUG_LEVEL3, ("[P 0x%08x] %s-\n", GetCurrentProcessId(), __FUNCTION__));
}

void TestCSerialPort(void)
{
    COM_DEBUG(DEBUG_LEVEL3, ("[P 0x%08x] %s+\n", GetCurrentProcessId(), __FUNCTION__));
    unsigned char readBuff[64] = { 0 };
    unsigned char writeBuff[64] = { 0 };
    DWORD readBytes = 0;
    DWORD writeBytes = 0;
    CSerialPort serialPort;
    serialPort.InitPort();
    readBytes = serialPort.ReadData(readBuff, 1, 3000);
    COM_DEBUG(DEBUG_LEVEL3, ("read %d bytes, data: %s\n", readBytes, readBuff));
    readBuff[0] = 0;
    writeBytes = serialPort.WriteData(writeBuff, 60, 3000);
    COM_DEBUG(DEBUG_LEVEL3, ("write %d bytes, data: %s\n", writeBytes, writeBuff));
    writeBytes = serialPort.WriteData(writeBuff, 60, 3000);
    COM_DEBUG(DEBUG_LEVEL3, ("write %d bytes, data: %s\n", writeBytes, writeBuff));
    readBytes = serialPort.ReadData(readBuff, 1, 3000);
    COM_DEBUG(DEBUG_LEVEL3, ("read %d bytes, data: %s\n", readBytes, readBuff));
    readBuff[0] = 0;
    COM_DEBUG(DEBUG_LEVEL3, ("[P 0x%08x] %s-\n", GetCurrentProcessId(), __FUNCTION__));
}

void TestComDataPoolList02_clear(void)
{
    COM_DEBUG(DEBUG_LEVEL3, ("[P 0x%08x] %s+\n", GetCurrentProcessId(), __FUNCTION__));
    typedef struct _DATA_TYPE
    {
        int a;
    } DATA_TYPE;
    ComDataPoolList02<DATA_TYPE, 3> list01(true);
    DATA_TYPE* pData01 = list01.allocSpace();
    list01.pushBack(pData01);
    list01.debug();
    list01.clear();
    list01.debug();
    DATA_TYPE* pData02 = list01.allocSpace();
    list01.pushBack(pData02);
    list01.debug();
}


int main2(int argc, char** argv)
{
    //TestComShareMem();
    //Test_CArrayDataPool_ComShareMem();
    //Test_ComInterProcessList_ComShareMem();
    //TestComInterProcessMsgQueueWithAnotherProcess();
    TestComInterProcessWithOtherProcess();
    getchar();
    return 0;
}

int main(int argc, char** argv)
{
    //TestCArrayDataPool();
    //TestComShareMem();
    //Test_CArrayDataPool_ComShareMem();
    //TestComInterProcessList();
    //Test_ComInterProcessList_ComShareMem();
    //TestComInterProcessMsgQueue();
    //TestComInterProcessMsgQueue();
    TestComInterProcessWithOhterProcess();
    //TestComDataPoolList02();
    //TestEnumPortsWdm();
    //TestCSerialPort();
    //TestComDataPoolList02_clear();
    return 0;
}