// VehControl.h
#ifndef VEHICLE_CONTROL_H
#define VEHICLE_CONTROL_H

#include <windows.h>

enum
{
    VEH_STATUS_IDLE,
    VEH_STATUS_STARTING,
    VEH_STATUS_RUNNING,
    VEH_STATUS_ENDING,
};

class VehControl
{
public:
    VehControl();
    int GetStatus(void);
    void ChangeStatus(int status_);

    bool StartVeh(void);
    bool EndVeh(void);
    // speed range 0 - 127
    bool ReqTurnWheel(int direction_, int speed_);
    // strength range 0x0000 - 0x0500
    bool ReqBrake(int strength_);

private:
    int m_nStatus;
    TCHAR m_szProcess[128];
    STARTUPINFO m_siProcess;
    PROCESS_INFORMATION m_piProcess;
};

#endif // VEHICLE_CONTROL_H

