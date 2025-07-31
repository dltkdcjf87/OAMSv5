#ifndef OAMS_MONITOR_H
#define OAMS_MONITOR_H

#include "OAMS_define.h"
#include "OAMS_common.h"

// 함수 프로토타입
int getMccsState();
BYTE getRemoteMccsState();
BYTE CheckProcRunning(const char* command, const char* moduleName);
//BYTE GetFdMccsProc(FILE* fp, BYTE moduleId);
BYTE GetFdMccsProc(FILE *fp, const char*  moduleName);
BYTE GetMccsProc(const char* moduleName);
void updateEmsModule(const char* moduleName);
void chkEmsModuleState(void);
void *MonitorMccs(void *arg);
void startEmsCheckMonitor();

#endif // OAMS_MONITOR_H
