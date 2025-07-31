#ifndef __OAMS_COMMON_H__
#define __OAMS_COMMON_H__

#include "LIB_nsmlog.h"
#include "libsmcom.h"
#include "CQueueBase.h"
#include "OAMS_define.h"
#include "OAMS_config.h"
#include "OAMS_data.h"
#include "OAMS_mgm.h"
#include "OAMS_monitor.h"
#include "OAMS_xbus.h"
#include "OAMS_exporter.h"
#include "OAMS_db.h"
#include "OAMS_limit.h"
#include "OAMS_alarm.h"
#include "OAMS_extern.h"
#include "OAMS_stat.h"
#include "OAMS_hwmon.h"

extern NSM_LOG  Log;
extern AsInfoManager asInfoManager;
extern ChassisInfoManager chassisInfoManager;
extern ServerInfoManager  serverInfoManager;
extern ModuleInfoManager  moduleInfoManager;
extern AsStatsInfoManager asstatsInfoManager;
extern SystemResourceStatManager  systemresourcestatManager;
extern PerformanceManager performanceManager;
extern ThresholdInfoManager thresholdInfoManager;
extern SswConfigInfoManager sswconfigInfoManager;
extern MsConfigInfoManager  msconfigInfoManager;
extern ExternalIfInfoManager externalIfInfoManager;
extern CdbStatusInfoManager  cdbstatusInfoManager;
extern AlarmConfigManager    alarmconfigManager;

class StatisticManager;
extern StatisticManager    statisticManager;

class ExternServerManager;
extern ExternServerManager externServerManager;

extern CConfig     g_cfg;
extern CExporter fixMetric;
extern CExporter alarmMetric;
extern ClimitProcess limitProc;
extern uint8_t	MY_SIDE;
extern uint8_t	OTHER_SIDE;
extern bool     g_bDEBUG_LOGFILE;
extern int      g_nDebug_level;
extern char     gMccsUse[6];
extern char     gMccsUse[6];
extern int      gSvrEmsA;
extern int      gSvrEmsB;
extern char     gMccsEmsA[16];
extern char     gMccsEmsB[16];

extern std::vector<std::pair<std::string, std::string>>  gMccsProcess;
extern std::vector<std::pair<std::string, std::string>>  gEmsProcess;

class AlarmManager;
extern AlarmManager alarmMNG;

extern CHWMonitor  hwMonitorManager;

extern char    as_ip_a[MAX_AS][IPADDR_LEN+1];
extern char    as_ip_b[MAX_AS][IPADDR_LEN+1];
extern int     r_buf;
extern int     gMgmFd[MAX_AS_EMS];
extern LibQ    g_mgmQue[MAX_AS];
extern bool    g_mgmTimeOut[MAX_AS];
extern char    gFixPort[6];
extern char    gAlarmPort[6];
extern int     g_curCntClients;
extern int     g_NMS_Alarm_Sendflag;    // 0 : off, 1 : on
extern int     g_Alarm_Flag;

extern bool	   dbConnect;

extern std::map<stAsModId, stChassisServerId*>    xbusid_Map;

int GetServerIdFromXbus(int asId, int xbusId, int& outServerId);
bool convertLimitName(int alarmField, const char*& LimitName);
void formatDateTime(const char* input, char* output, size_t output_size);
void make_strTime(char strTime[15], long t);
void shut_down(int reason); 
void trim(char* str);
void ToUpperCase(char* str);
int ConverAlarmCode(int code);
void clean_str(char* str);

#endif //__OAMS_COMMON_H__

