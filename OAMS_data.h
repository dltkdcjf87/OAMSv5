#ifndef __CDATAINFO_H__
#define __CDATAINFO_H__

#include <mutex>
#include "OAMS_define.h"

typedef struct {
    char name[20];
    int use;
} TABLESPACE_INFO;

typedef struct {
    char name[20];
    int status;
} NET_STATUS;

typedef struct {
    int fanStatus[MAX_HW];
    int powerStatus[MAX_HW];
    int diskStatus[MAX_HW];
    NET_STATUS netStatus[MAX_LAN_PORT];
} HW;

typedef struct {
    int cpu;
    int memory;
    int network;
    int disk;
    TABLESPACE_INFO tablespace[MAX_TS];
    int temperature;
    int dbSession;
    int ntp;
    int channel_count;
    int channel_rate;
    int sync_count;
} PERFORMANCE;

typedef struct {
    int ha;
    int status;
} HaStatusInfo;

typedef struct {
    int asId;
    char asName[AS_NAME_LEN];
    char asDesc[50];
    int block;
    int overload;
    int connected;
} AS_INFO;

typedef struct {
    int chassisId;
    int asId;
    int rackId;
    int chassisType;
    char chassisName[30];
    bool use;
    HW hwData;
    int switchStatus[MAX_HW];
    HaStatusInfo oaStatus[MAX_HW];
} CHASSIS_INFO;

typedef struct {
    int moduleId;
    int serverCode;
    int serverId;
    char asName[AS_NAME_LEN];
    char moduleName[MODULE_NAME_LEN];
    char serverName[SERVER_NAME_LEN];
    int status;
    int ha;
    int critical_count;
    int major_count;
    int minor_count;
    int warning_count;
} MODULE_INFO;

typedef struct {
    int serverId;
    int asId;
    char asName[AS_NAME_LEN];
    int chassisId;
    char serverName[SERVER_NAME_LEN];
    int serverType;
    int serverCode;
    char ip[IP_LEN];
    bool use;
    HaStatusInfo serverStatus;
    int block;
    int rebuild;
    HW hwData;
    int critical_count;
    int major_count;
    int minor_count;
    int warning_count;
} SERVER_INFO;

typedef struct {
	int total;
	int count;
    int min;
    int max;
    int avg;
    char peaktime[20];
} STAT_SUMMERY;

typedef struct {
    char service_key[SVC_KEY_LEN];
    int  tot_ch;
    int  use_ch;
    int  total_usage;
    int  total_success;
} SERVICE_STAT;

typedef struct {
    int cps;
    STAT_SUMMERY CpsSummery;
    int discardChannel;
    int sumBusy;
    int sumTotal;
    int sum_usage;
    int sum_success;
    SERVICE_STAT serviceStat[MAX_SVC_USE];
} AS_STATS;

typedef struct {
    STAT_SUMMERY cpu_summery;
    STAT_SUMMERY mem_summery;
    STAT_SUMMERY net_summery;
    STAT_SUMMERY disk_summery;
    STAT_SUMMERY temperature_summery;
} SYSTEM_RESOURCE_STAT;

typedef struct {
    int cpu[4];           // CPU 임계치
    int memory[4];        // MEMORY 임계치
    int network[4];       // NETWORK 임계치
    int disk[4];          // DISK 임계치
    int channel[4];       // Channel 임계치
    int ntp[4];           // NTP 임계치
    int tablespace[4];    // 테이블스페이스 임계치
    int temperature[4];   // 온도 임계치
    int session[4];       // 세션 임계치
} THRESHOLD_INFO;

typedef struct {
    int asId;
    int index;
    char systemName[SYSTEM_NAME_LEN];
    char ip[IP_LEN];
    char port[PORT_LEN];
    int status;
    int block;
    int monitor;
} SSW_CONFIG_INFO;

typedef struct {
    int asId;
    int index;
    char systemName[SYSTEM_NAME_LEN];
    char ip[IP_LEN];
    char port[PORT_LEN];
    bool block_flag;
    bool alive_flag;
    bool state_flag;
    int type;
} MS_CONFIG_INFO;

typedef struct {
    int asId;
    char systemName[SYSTEM_NAME_LEN];
    char ip[IP_LEN];
    char port[PORT_LEN];
    int status;
    int system_type;
	int ConnSvrID;
	char system[SYSTEM_NAME_LEN];
} EXTERNAL_IF_INFO;

typedef struct {
    int asId;
	int serverID;
    int groupId;
    int groupSeq;
    char systemName[SYSTEM_NAME_LEN];
    char ip[IP_LEN];
    char port[PORT_LEN];
    int status;
    int use_flag;
	int completeRate;
} CDB_STATUS_INFO;

typedef struct {
    char asName[AS_NAME_LEN];
    char systemName[SYSTEM_NAME_LEN];
	int  count;
} EXTERNAL_COUNT_INFO;

typedef struct {
    int AlarmCode;
    int AlarmType;
    int AlarmLevel;
    char AlarmDesc[128];
    char AlarmRepair[128];
    int ext_send;
} ALARM_CONFIG;

/*
typedef struct {
    char ServiceName[64];
    int ThreshholdCallCount;
    int SuccessRate;
} SERVICE_LIMIT;
*/


struct stAsModId{
    int asId;
    int moduleId;

    bool operator<(const stAsModId& other) const {
        if (asId != other.asId)
            return asId < other.asId;
        return moduleId < other.moduleId;
    }
};

struct stChassisServerId{
    int chassisId;
    int serverId;
};

class AsInfoManager {
private:
    mutable std::mutex Mutex;
    AS_INFO asInfo[MAX_AS_EMS];

public:
    void Init();
    void setASInfo(int index, const char *asName, const char *asDesc);
    void setOverLoadASInfo(int index, int overload);
    void setBlockASInfo(int index, int block);
    void setConnectedASInfo(int index, int connected);
    void getASInfo(int index, AS_INFO& info) const;
    int  FindAsIdByName(const char* asName);
    void printASInfo() const;
};
class ChassisInfoManager {
private:
    mutable std::mutex Mutex;
    CHASSIS_INFO chassisInfo[MAX_CHASSIS];

public:
    void Init();
    void setChassisInfo(int index, int asId, int rackId, int chassisType, const char *chassisName, bool use);
    void setHwDataChassisInfo(int index, int hwindex, int fanStatus, int powerStatus, int diskStatus, const char *netName,  int netStatus);
    void setSwitchStatusChassisInfo(int index, int hwindex, int switchStatus);
    void getChassisInfo(int index, CHASSIS_INFO& info) const;
    void printChassisInfo() const;
};

class ServerInfoManager {
private:
    mutable std::mutex Mutex;
    SERVER_INFO serverInfo[MAX_SERVER];

public:
    void Init();
    void setServerInfo(int index, int serverId, int asId, const char *asName, int chassisId, const char *serverName, int serverType, int serverCode, const char *ip);
    void setBlockServerInfo(int index, int block);
    void setHaServerInfo(int index, int ha);
    void setRebuildAndHWServerInfo(int index, int rebuild, const HW& hw);
    void plusAlarmCountServerInfo(int index, int type);
    void minusAlarmCountServerInfo(int index, int type);
    void getServerInfo(int index, SERVER_INFO& info) const;
	int getServerASID(int index);
	int getServerCode(int index);
    int  FindServerIdByName(const char* serverName);
    int  FindServerHaByName(const char* serverName);
    std::string FindServerNameByHa(const char* serverName, int ha);

    void printServerInfo() const;
};

class ModuleInfoManager {
private:
    mutable std::mutex Mutex;
    MODULE_INFO moduleInfo[MAX_AS_EMS][MAX_MODULE];

public:
    void Init();
    void setModuleInfo(int asindex, int moindex, int moduleId, int serverCode, int serverId, const char *asName, const char *moduleName, const char *serverName);
    void setStatusModuleInfo(int asindex, int moindex, int state);
    void setHaModuleInfo(int asindex, int moindex, int ha);
    void setHaServerModuleInfo(int asindex, int serverId, int ha);
    void setHaEmsModuleInfo(int side, int result);
    void plusAlarmCountModuleInfo(int asindex, int moindex, int type);
    void minusAlarmCountModuleInfo(int asindex, int moindex, int type);
    void getModuleInfo(int asindex, int moindex,  MODULE_INFO& info) const;
    int  FindModuleIdByName(const char* moduleName);
    int  FindSvrIdByModuleId(int moduleId);
    std::string FindModuleNameByModuleId(int moduleId);
    void printModuleInfo() const;
};
class AsStatsInfoManager {
private:
    mutable std::mutex Mutex;
    AS_STATS asstatsInfo[MAX_AS_EMS];

public:
    void Init();
	void InitCpsStat();
    void setAsStatsInfo(int index, const char *service_key);
    void setChAsStatsInfo(int index, int cmdNo, void *arg);
    void setSvcAsStatsInfo(int index, const char *service_key, int tot_ch, int use_ch, int total_usage, int total_success);
    void getAsStats(int index, AS_STATS& info) const;
    void printAsStats() const;
};
class SystemResourceStatManager {
private:
    mutable std::mutex Mutex;
    SYSTEM_RESOURCE_STAT systemresourceStat[MAX_SERVER];

public:
    void Init();
    void setSystemResourceStat(int index, const char *type, int value);
    void getSystemResourceStat(int index, SYSTEM_RESOURCE_STAT& info) const;

};
class PerformanceManager {
private:
    mutable std::mutex Mutex; 
    PERFORMANCE performance[MAX_SERVER];

public:
    void Init();
    void setResourcePerformance(int index, int cpu, int memory, int network, int disk);
    void setChPerformance(int index, int cmdNo, void *arg);
    void setNtpPerformance(int index, int ntp);
    void setTableSpacePerformance(int serveridx, int tsIdx, const char* tsName, int use);
    void setDBSessionPerformance(int serveridx, int use);
    void setTemperaturePerformance(int serveridx, int temperature);
    void getPerformance(int index, PERFORMANCE& info) const;
    void printPerformance() const;
};
class ThresholdInfoManager {
private:
    mutable std::mutex Mutex;
    THRESHOLD_INFO thresholdInfo;
//	SERVICE_LIMIT  svcholdInfo[MAX_SVC_USE];

public:
    void Init();
    void setThresHoldInfo(const char *type, int critical, int major, int minor, int warning);
    void getThresHoldInfo(THRESHOLD_INFO& info) const;
//	void setServiceHoldInfo(int index, SERVICE_LIMIT info) const;
//	void getServiceHoldInfo(const char* service, SERVICE_LIMIT& info) const;
    void printThresholdInfo() const;

};
class SswConfigInfoManager {
private:
    mutable std::mutex Mutex;
    SSW_CONFIG_INFO sswconfigInfo[MAX_SSW];
  
public:
    void Init();
    void setSswConfigInfo(int index, const SSW_CONFIG_INFO& info);
    void getSswConfigInfo(int index, SSW_CONFIG_INFO& info) const;

	void setSswStatusInfo(int index, int status);
};
class MsConfigInfoManager {
private:
    mutable std::mutex Mutex;
    MS_CONFIG_INFO msconfigInfo[MAX_MS];

public:
    void Init();
    void setMsConfigInfo(int index, const MS_CONFIG_INFO& info);
    void getMsConfigInfo(int index, MS_CONFIG_INFO& info) const;
};
class ExternalIfInfoManager {
private:
    mutable std::mutex Mutex;
    EXTERNAL_IF_INFO externalifInfo[MAX_EXTERNAL];

public:
    void Init();
    void setExternalIfInfo(int index, const EXTERNAL_IF_INFO& info);
    void getExternalIfInfo(int index, EXTERNAL_IF_INFO& info) const;
};
class CdbStatusInfoManager {
private:
    mutable std::mutex Mutex;
    CDB_STATUS_INFO cdbstatusInfo[MAX_CDB]; 

public:
    void Init();
    void setCdbStatusInfo(int index, const CDB_STATUS_INFO& info);
    void getCdbStatusInfo(int index, CDB_STATUS_INFO& info) const;
};

class AlarmConfigManager {
private:
    mutable std::mutex Mutex;
    ALARM_CONFIG alarmConfig[MAX_ALARM];

public:
    void Init();
    void setAlarmConfig(int index, const ALARM_CONFIG& info);
    void getAlarmConfig(int index, ALARM_CONFIG& info) const;
    void printAlarmConfig() const;
};

#endif // __CDATAINFO_H__
