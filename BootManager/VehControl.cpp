

//#include "stdafx.h"
#include "VehControl.h"
////#include "IpcAccess.h"

#define COM_DEBUG
#define COM_ERR
#define COM_WARN
#define DEBUG_LEVEL3 1
VehControl::VehControl()
{
    m_nStatus = VEH_STATUS_IDLE;
    lstrcpy(m_szProcess, TEXT("..//..//VehicleControl//Debug//VehicleControl.exe"));
    m_siProcess.cb = sizeof(m_siProcess);
}

int VehControl::GetStatus(void)
{
    return (m_nStatus);
}

void VehControl::ChangeStatus(int status_)
{
    COM_DEBUG(DEBUG_LEVEL3, ("Change veh status from (%d) to (%d)\n", m_nStatus, status_));
    m_nStatus = status_;
}

bool VehControl::StartVeh(void)
{
    COM_DEBUG(DEBUG_LEVEL3, ("Func : %s\n", __FUNCTION__));
    BOOL bRetValue = FALSE;
    DWORD dwRetValue = 0;
    int status = GetStatus();
    if (VEH_STATUS_IDLE != status)
    {
        COM_WARN("[WARN] veh status error (%d)\n", status);
        return (false);
    }
    ChangeStatus(VEH_STATUS_STARTING);
    // create vehicle control process
    bRetValue = CreateProcess(NULL, m_szProcess, NULL, NULL, FALSE, 0, NULL, NULL, &m_siProcess, &m_piProcess);
    if (FALSE == bRetValue)
    {
        ChangeStatus(VEH_STATUS_IDLE);
        COM_ERR("Create vehicle control process error!\n");
        return (false);
    }
    COM_DEBUG(DEBUG_LEVEL3, ("Func : %s, create vehicle control process ok\n", __FUNCTION__));
    return (true);
}

bool VehControl::EndVeh(void)
{
    COM_DEBUG(DEBUG_LEVEL3, ("Func : %s\n", __FUNCTION__));
    DWORD dwRetValue = 0;
    int status = GetStatus();
    if (VEH_STATUS_RUNNING != status)
    {
        COM_WARN("[WARN] veh status error (%d)\n", status);
        return (false);
    }
    ChangeStatus(VEH_STATUS_ENDING);
    COM_DEBUG(DEBUG_LEVEL3, ("Send end work message to vehicle control...\n"));
    ////ReqVehEndWork();
    COM_DEBUG(DEBUG_LEVEL3, ("Waiting vehicle control process exit...\n"));
    dwRetValue = WaitForSingleObject(m_piProcess.hProcess, 20000);
    switch (dwRetValue)
    {
        case WAIT_OBJECT_0:
            break;
        case WAIT_TIMEOUT:
        case WAIT_FAILED:
            COM_ERR("Waiting vehicle control process exit...error\n");
        default:
            break;
    };
    ChangeStatus(VEH_STATUS_IDLE);
    COM_DEBUG(DEBUG_LEVEL3, ("Waiting vehicle control process exit...ok\n"));
    return (true);
}

// speed range 0 - 127
bool VehControl::ReqTurnWheel(int direction_, int speed_)
{
    COM_DEBUG(DEBUG_LEVEL3, ("CAM: %s (%d, %d)\n", __FUNCTION__, direction_, speed_));
    ////ReqVehTurnWheel(direction_, speed_);
    return (true);
}

// strength range 0x0000 - 0x0500
bool VehControl::ReqBrake(int strength_)
{
    COM_DEBUG(DEBUG_LEVEL3, ("CAM: %s (%d, %d)\n", __FUNCTION__, strength_));
    ////ReqVehBrake(strength_);
    return (true);
}
