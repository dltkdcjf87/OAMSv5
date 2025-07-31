#include "OAMS_common.h"

void AsInfoManager::Init() {
    Log.printf(LOG_LV2, "[AsInfoManager] Init START ");

    std::lock_guard<std::mutex> lock(Mutex);
    for (int i = 0; i < MAX_AS_EMS; ++i) {
        asInfo[i].asId = -1;
        memset(asInfo[i].asName, 0x00, sizeof(asInfo[i].asName));
        memset(asInfo[i].asDesc, 0x00, sizeof(asInfo[i].asDesc));
        asInfo[i].block = -1;
        asInfo[i].overload = 0;
        asInfo[i].connected = -1;
    }
    Log.printf(LOG_LV2, "[AsInfoManager] Init END ");
}
void ServerInfoManager::Init() {
    Log.printf(LOG_LV2, "[ServerInfoManager] Init START ");
    std::lock_guard<std::mutex> lock(Mutex);
    for (int i = 0; i < MAX_SERVER; ++i) {
        serverInfo[i].serverId = -1;
        serverInfo[i].asId = -1;
        memset(serverInfo[i].asName, 0x00, sizeof(serverInfo[i].asName));
        serverInfo[i].chassisId = -1;
        memset(serverInfo[i].serverName, 0x00, sizeof(serverInfo[i].serverName));
        serverInfo[i].serverType = -1;
        serverInfo[i].serverCode = -1;
        memset(serverInfo[i].ip, 0x00, sizeof(serverInfo[i].ip));
        serverInfo[i].use = false;
        serverInfo[i].serverStatus = { -1, -1 };
        serverInfo[i].block = -1;
        serverInfo[i].rebuild = 0;
        for (int j = 0; j  < MAX_HW; j++) {
            serverInfo[i].hwData.fanStatus[j] = 1;
            serverInfo[i].hwData.powerStatus[j] = 1;
            serverInfo[i].hwData.diskStatus[j] = 1;
        }
        for (int j = 0; j < MAX_LAN_PORT; j++) {
            memset(serverInfo[i].hwData.netStatus[j].name, 0x00, sizeof(serverInfo[i].hwData.netStatus[j].name));
            serverInfo[i].hwData.netStatus[j].status = 1;
        }
        serverInfo[i].critical_count = 0;
        serverInfo[i].major_count = 0;
        serverInfo[i].minor_count = 0;
        serverInfo[i].warning_count = 0;
    }
    Log.printf(LOG_LV2, "[ServerInfoManager] Init END ");
}
void ModuleInfoManager::Init() {
    Log.printf(LOG_LV2, "[ModuleInfoManager] Init START ");
    std::lock_guard<std::mutex> lock(Mutex);
    for (int i = 0; i < MAX_AS_EMS; ++i) {
        for (int j = 0; j < MAX_MODULE; ++j) {
            moduleInfo[i][j].moduleId = -1;
            moduleInfo[i][j].serverCode = -1;
            moduleInfo[i][j].serverId = -1;
            memset(moduleInfo[i][j].asName, 0x00, sizeof(moduleInfo[i][j].asName));
            memset(moduleInfo[i][j].moduleName, 0x00, sizeof(moduleInfo[i][j].moduleName));
            memset(moduleInfo[i][j].serverName, 0x00, sizeof(moduleInfo[i][j].serverName));
            moduleInfo[i][j].ha = -1;
            moduleInfo[i][j].status = -1;
            moduleInfo[i][j].critical_count = 0;
            moduleInfo[i][j].major_count = 0;
            moduleInfo[i][j].minor_count = 0;
            moduleInfo[i][j].warning_count = 0;
        }
    }
    Log.printf(LOG_LV2, "[ModuleInfoManager] Init END ");
}

void AsStatsInfoManager::Init() {
    Log.printf(LOG_LV2, "[AsStatsInfoManager] Init START ");
    std::lock_guard<std::mutex> lock(Mutex);
    for (int i = 0; i < MAX_AS_EMS; ++i) {
        asstatsInfo[i].cps = -1;
        asstatsInfo[i].CpsSummery.min = 999;
        asstatsInfo[i].CpsSummery.max = -1;
        asstatsInfo[i].CpsSummery.avg = -1;
		asstatsInfo[i].CpsSummery.count = 0;
		asstatsInfo[i].CpsSummery.total = 0;
        memset(asstatsInfo[i].CpsSummery.peaktime, 0x00, sizeof(asstatsInfo[i].CpsSummery.peaktime));
        asstatsInfo[i].discardChannel = 0;
        asstatsInfo[i].sumBusy = 0;
        asstatsInfo[i].sumTotal = 0;
        asstatsInfo[i].sum_usage = 0;
        asstatsInfo[i].sum_success = 0;
        for (int j = 0; j < MAX_SVC_USE; ++j) {
            memset(asstatsInfo[i].serviceStat[j].service_key, 0x00, sizeof(asstatsInfo[i].serviceStat[j].service_key));
            asstatsInfo[i].serviceStat[j].tot_ch = 0;
            asstatsInfo[i].serviceStat[j].use_ch = 0;
            asstatsInfo[i].serviceStat[j].total_usage = 0;
            asstatsInfo[i].serviceStat[j].total_success = 0;
        }
    }
    Log.printf(LOG_LV2, "[AsStatsInfoManager] Init END ");
}
void SystemResourceStatManager::Init() {
    Log.printf(LOG_LV2, "[SystemResourceStatManager] Init START");
    std::lock_guard<std::mutex> lock(Mutex);
    
    for (int i = 0; i < MAX_SERVER; ++i) {
        // 모든 리소스 통계에 대해 동일한 초기값 설정
        const STAT_SUMMERY defaultSummery = {0, 0, 999, -1, -1, 0}; // total, count, min, max, avg, peaktime
        
        systemresourceStat[i].cpu_summery = defaultSummery;
        systemresourceStat[i].mem_summery = defaultSummery;
        systemresourceStat[i].net_summery = defaultSummery;
        systemresourceStat[i].disk_summery = defaultSummery;
        systemresourceStat[i].temperature_summery = defaultSummery;
    }
    
    Log.printf(LOG_LV2, "[SystemResourceStatManager] Init END");
}

void ChassisInfoManager::Init() {
    Log.printf(LOG_LV2, "[ChassisInfoManager] Init START ");
    std::lock_guard<std::mutex> lock(Mutex);
    for (int i = 0; i < MAX_CHASSIS; ++i) {
        chassisInfo[i].chassisId = -1;
        chassisInfo[i].asId = -1;
        chassisInfo[i].rackId = -1;
        chassisInfo[i].chassisType = -1;
        memset(chassisInfo[i].chassisName, 0x00, sizeof(chassisInfo[i].chassisName));
        chassisInfo[i].use = false;
        memset(&chassisInfo[i].hwData, 0x00, sizeof(HW));
        memset(chassisInfo[i].switchStatus, 0x00, sizeof(chassisInfo[i].switchStatus));
        for (int j = 0; j < MAX_HW; ++j) {
            chassisInfo[i].oaStatus[j] = { -1, -1 };
        }
    }
    Log.printf(LOG_LV2, "[ChassisInfoManager] Init END ");
}
void PerformanceManager::Init() {
    Log.printf(LOG_LV2, "[PerformanceManager] Init START ");
    std::lock_guard<std::mutex> lock(Mutex);
    for (int i = 0; i < MAX_SERVER; ++i) {
        performance[i].cpu = -1;
        performance[i].memory = -1;
        performance[i].network = -1;
        performance[i].disk = -1;
        performance[i].temperature = -1;
        performance[i].dbSession = -1;
        performance[i].ntp = -1;
        performance[i].channel_count = -1;
        performance[i].channel_rate = -1;
        performance[i].sync_count = -1;
        for (int j = 0; j < MAX_TS; ++j) {
            memset(performance[i].tablespace[j].name, 0x00, sizeof(performance[i].tablespace[j].name));
            performance[i].tablespace[j].use = -1;
        }
    }
    Log.printf(LOG_LV2, "[PerformanceManager] Init END ");
}
void ThresholdInfoManager::Init() {
    Log.printf(LOG_LV2, "[ThresholdInfoManager] Init START ");

    std::lock_guard<std::mutex> lock(Mutex);
    for(int i=0;i<4;i++)
    { 
        thresholdInfo.cpu[i] = -1;
        thresholdInfo.memory[i] = -1;
        thresholdInfo.network[i] = -1;
        thresholdInfo.disk[i] = -1;
        thresholdInfo.channel[i] = -1;
        thresholdInfo.ntp[i] = -1;
        thresholdInfo.tablespace[i] = -1;
        thresholdInfo.temperature[i] = -1;
        thresholdInfo.session[i] = -1;
    }
    Log.printf(LOG_LV2, "[ThresholdInfoManager] Init END ");
}

void SswConfigInfoManager::Init() {
    Log.printf(LOG_LV2, "[SswConfigInfoManager] Init START ");
    std::lock_guard<std::mutex> lock(Mutex);
    for (int i = 0; i < MAX_SSW; ++i) {
        sswconfigInfo[i].asId = -1;
        sswconfigInfo[i].index = -1;
        memset(sswconfigInfo[i].systemName, 0x00, sizeof(sswconfigInfo[i].systemName));
        memset(sswconfigInfo[i].ip, 0x00, sizeof(sswconfigInfo[i].ip));
        memset(sswconfigInfo[i].port, 0x00, sizeof(sswconfigInfo[i].port));
        sswconfigInfo[i].status = -1;
        sswconfigInfo[i].block = -1;
        sswconfigInfo[i].monitor = -1;
    }
    Log.printf(LOG_LV2, "[SswConfigInfoManager] Init END ");
}
void MsConfigInfoManager::Init() {
    Log.printf(LOG_LV2, "[MsConfigInfoManager] Init START ");
    std::lock_guard<std::mutex> lock(Mutex);
    for (int i = 0; i < MAX_MS; ++i) {
        msconfigInfo[i].asId = -1;
        msconfigInfo[i].index = -1;
        memset(msconfigInfo[i].systemName, 0x00, sizeof(msconfigInfo[i].systemName));
        memset(msconfigInfo[i].ip, 0x00, sizeof(msconfigInfo[i].ip));
        memset(msconfigInfo[i].port, 0x00, sizeof(msconfigInfo[i].port));
        msconfigInfo[i].block_flag = -1;
        msconfigInfo[i].alive_flag = -1;
        msconfigInfo[i].state_flag = -1;
        msconfigInfo[i].type = -1;
    }
    Log.printf(LOG_LV2, "[MsConfigInfoManager] Init END ");
}
void ExternalIfInfoManager::Init() {
    Log.printf(LOG_LV2, "[ExternalIfInfoManager] Init START ");
    std::lock_guard<std::mutex> lock(Mutex);
    for (int i = 0; i < MAX_EXTERNAL; ++i) {
        externalifInfo[i].asId = -1;
        memset(externalifInfo[i].systemName, 0x00, sizeof(externalifInfo[i].systemName));
        memset(externalifInfo[i].ip, 0x00, sizeof(externalifInfo[i].ip));
        memset(externalifInfo[i].port, 0x00, sizeof(externalifInfo[i].port));
        externalifInfo[i].status = -1;
        externalifInfo[i].system_type = -1;
    }
    Log.printf(LOG_LV2, "[ExternalIfInfoManager] Init END ");
}
void CdbStatusInfoManager::Init() {
    Log.printf(LOG_LV2, "[CdbStatusInfoManager] Init START ");
    std::lock_guard<std::mutex> lock(Mutex);
    for (int i = 0; i < MAX_CDB; ++i) {
        cdbstatusInfo[i].asId = -1;
        cdbstatusInfo[i].groupId = -1;
        cdbstatusInfo[i].groupSeq = -1;
        memset(cdbstatusInfo[i].ip, 0x00, sizeof(cdbstatusInfo[i].ip));
        memset(cdbstatusInfo[i].port, 0x00, sizeof(cdbstatusInfo[i].port));
        cdbstatusInfo[i].status = -1; 
        cdbstatusInfo[i].use_flag = -1; 
    }
    Log.printf(LOG_LV2, "[CdbStatusInfoManager] Init END ");
}

void AlarmConfigManager::Init() {
    Log.printf(LOG_LV2, "[AlarmConfigManager] Init START ");
    std::lock_guard<std::mutex> lock2(Mutex);
    for (int i = 0; i < MAX_ALARM; ++i) {
        alarmConfig[i].AlarmCode = -1;
        alarmConfig[i].AlarmType = -1;
        alarmConfig[i].AlarmLevel = -1;
        memset(alarmConfig[i].AlarmDesc, 0, sizeof(alarmConfig[i].AlarmDesc));
        memset(alarmConfig[i].AlarmRepair, 0, sizeof(alarmConfig[i].AlarmRepair));
        alarmConfig[i].ext_send = 0;
    }
    Log.printf(LOG_LV2, "[AlarmConfigManager] Init END ");
}

void AsInfoManager::setASInfo(int index, const char *asName, const char *asDesc) {
    char tmpAsName[10]; // 크기 충분히 설정
    strncpy(tmpAsName, asName, sizeof(tmpAsName) - 1);
    tmpAsName[sizeof(tmpAsName) - 1] = '\0';
    ToUpperCase(tmpAsName);

    Log.printf(LOG_LV2, "[AsInfoManager] setASInfo START ");
    Log.printf(LOG_LV1, "[AsInfoManager] setASInfo index[%d] asName[%s] asDesc[%s] ", index, tmpAsName, asDesc);

    std::lock_guard<std::mutex> lock(Mutex);
    if (index >= 0 && index < MAX_AS_EMS) {
        asInfo[index].asId = index;
        strncpy(asInfo[index].asName, tmpAsName, sizeof(asInfo[index].asName));
        strncpy(asInfo[index].asDesc, asDesc, sizeof(asInfo[index].asDesc));
    }
    Log.printf(LOG_LV2, "[AsInfoManager] setASInfo END ");
}
void AsInfoManager::setOverLoadASInfo(int index, int overload){
    Log.printf(LOG_LV2, "[AsInfoManager] setOverLoadASInfo START ");
    Log.printf(LOG_LV1, "[AsInfoManager] setOverLoadASInfo index[%d] overload[%d]", index, overload);

    std::lock_guard<std::mutex> lock(Mutex);
    if (index >= 0 && index < MAX_AS_EMS) {
        asInfo[index].overload = overload;
    }
    Log.printf(LOG_LV2, "[AsInfoManager] setOverLoadASInfo END ");
}
void AsInfoManager::setBlockASInfo(int index, int block){
    Log.printf(LOG_LV2, "[AsInfoManager] setBlockASInfo START ");
    Log.printf(LOG_LV1, "[AsInfoManager] setBlockASInfo index[%d] block[%d]", index, block);

    std::lock_guard<std::mutex> lock(Mutex);
    if (index >= 0 && index < MAX_AS_EMS) {
        asInfo[index].block = block;
    }
    Log.printf(LOG_LV2, "[AsInfoManager] setBlockASInfo END ");
}
void AsInfoManager::setConnectedASInfo(int index, int connected){
    Log.printf(LOG_LV2, "[AsInfoManager] setConnectedASInfo START ");
    Log.printf(LOG_LV1, "[AsInfoManager] setConnectedASInfo index[%d] connected[%d]", index, connected);

    std::lock_guard<std::mutex> lock(Mutex);
    if (index >= 0 && index < MAX_AS_EMS) {
        asInfo[index].connected = connected;
    }
    Log.printf(LOG_LV2, "[AsInfoManager] setConnectedASInfo END ");
}

void ChassisInfoManager::setChassisInfo(int index, int asId, int rackId, int chassisType, const char *chassisName, bool use)
{
    Log.printf(LOG_LV2, "[ChassisInfoManager] setChassisInfo START ");
    Log.printf(LOG_LV1, "[ChassisInfoManager] setChassisInfo index[%d] asId[%d] rackId[%d] chassisType[%d] chassisName[%s] use[%d] ",
               index, asId, rackId, chassisType, chassisName, use);
    std::lock_guard<std::mutex> lock(Mutex);
    if (index >= 0 && index < MAX_CHASSIS) {
        chassisInfo[index].chassisId = index;
        chassisInfo[index].asId = asId;
        chassisInfo[index].rackId = rackId;
        chassisInfo[index].chassisType = chassisType;
        strncpy(chassisInfo[index].chassisName, chassisName, sizeof(chassisInfo[index].chassisName));
        chassisInfo[index].use = use;
    }
    Log.printf(LOG_LV2, "[ChassisInfoManager] setChassisInfo END ");
}
void ChassisInfoManager::setHwDataChassisInfo(int index, int hwindex, int fanStatus, int powerStatus, int diskStatus, const char *netName,  int netStatus)
{
    Log.printf(LOG_LV2, "[ChassisInfoManager] setHwDataChassisInfo START ");
    Log.printf(LOG_LV1, "[ChassisInfoManager] setHwDataChassisInfo Id[%d] hwindex[%d] fanStatus[%d] powerStatus[%d] diskStatus[%d] netName[%s] netStatus[%s] ",
               index, hwindex, fanStatus, powerStatus, diskStatus, netName, netStatus);
    std::lock_guard<std::mutex> lock(Mutex);
    if (index >= 0 && index < MAX_CHASSIS &&  hwindex >= 0 && hwindex < MAX_HW) {
        chassisInfo[index].hwData.fanStatus[hwindex] = fanStatus;
        chassisInfo[index].hwData.powerStatus[hwindex] = powerStatus;
        chassisInfo[index].hwData.diskStatus[hwindex] = diskStatus;
        strncpy(chassisInfo[index].hwData.netStatus[hwindex].name, netName, sizeof(chassisInfo[index].hwData.netStatus[hwindex].name));
        chassisInfo[index].hwData.netStatus[hwindex].status = netStatus;
    }
    Log.printf(LOG_LV2, "[ChassisInfoManager] setHwDataChassisInfo END ");
}
void ChassisInfoManager::setSwitchStatusChassisInfo(int index, int hwindex, int switchStatus)
{
    Log.printf(LOG_LV2, "[ChassisInfoManager] setSwitchStatusChassisInfo START ");
    Log.printf(LOG_LV1, "[ChassisInfoManager] setSwitchStatusChassisInfo Id[%d] switchStatus[%d] ", index, switchStatus);
    std::lock_guard<std::mutex> lock(Mutex);
    if (index >= 0 && index < MAX_CHASSIS && hwindex >= 0 && hwindex < MAX_HW) {
        chassisInfo[index].switchStatus[hwindex] = switchStatus;
    }
    Log.printf(LOG_LV2, "[ChassisInfoManager] setSwitchStatusChassisInfo END ");

}

void ServerInfoManager::setServerInfo(int index, int serverId, int asId, const char *asName, int chassisId, const char *serverName, int serverType, int serverCode, const char *ip)
{
    char tmpAsName[10]; // 크기 충분히 설정
    char tmpServerName[10]; // 크기 충분히 설정

    strncpy(tmpAsName, asName, sizeof(tmpAsName) - 1);
    strncpy(tmpServerName, serverName, sizeof(tmpServerName) - 1);
    tmpAsName[sizeof(tmpAsName) - 1] = '\0';
    tmpServerName[sizeof(tmpServerName) - 1] = '\0';
    ToUpperCase(tmpAsName);
    ToUpperCase(tmpServerName);
    std::string result = tmpServerName;
    result.erase(std::remove(result.begin(), result.end(), '_'), result.end());

    Log.printf(LOG_LV2, "[ServerInfoManager] setServerInfo START ");
    Log.printf(LOG_LV1, "[ServerInfoManager] setServerInfo asId[%d] serverId[%d] asName[%s] chassisId[%d] serverName[%s] serverType[%d] serverCode[%d] ip[%s] ",
               asId, serverId, tmpAsName, chassisId, result.c_str(), serverType, serverCode, ip);
    std::lock_guard<std::mutex> lock(Mutex);
    if (index >= 0 && index < MAX_SERVER) {
        serverInfo[index].serverId = serverId;
        serverInfo[index].asId = asId;
        strncpy(serverInfo[index].asName, tmpAsName, sizeof(serverInfo[index].asName));
        serverInfo[index].chassisId = chassisId;
        strncpy(serverInfo[index].serverName, result.c_str(), sizeof(serverInfo[index].serverName));
        serverInfo[index].serverType = serverType;
        serverInfo[index].serverCode = serverCode;
        serverInfo[index].serverStatus.status = 1;
        strncpy(serverInfo[index].ip, ip, sizeof(serverInfo[index].ip));
        serverInfo[index].use = true;
    }
    Log.printf(LOG_LV2, "[ServerInfoManager] setServerInfo END ");
}
void ServerInfoManager::setBlockServerInfo(int index, int block)
{
    Log.printf(LOG_LV2, "[ServerInfoManager] setBlockServerInfo START ");
    Log.printf(LOG_LV1, "[ServerInfoManager] setBlockServerInfo serverId[%d] block[%d] ", index, block);
    std::lock_guard<std::mutex> lock(Mutex);
    if (index >= 0 && index < MAX_SERVER) {
        serverInfo[index].block = block;
    }
    Log.printf(LOG_LV2, "[ServerInfoManager] setBlockServerInfo END ");
}
void ServerInfoManager::setHaServerInfo(int index, int ha) {
    Log.printf(LOG_LV2, "[ServerInfoManager] setHaServerInfo START ");
    std::lock_guard<std::mutex> lock(Mutex);
    if (index >= 0 && index < MAX_SERVER) {
        if(ha == 3) serverInfo[index].serverStatus.ha = 2;
        else if(ha == 0) serverInfo[index].serverStatus.ha = 0;
        else serverInfo[index].serverStatus.ha = 3;
        if (strstr(serverInfo[index].serverName, "SES") != nullptr && ha !=0) serverInfo[index].serverStatus.ha = 2;

        Log.printf(LOG_LV1, "[ServerInfoManager] setHaServerInfo serverId[%d] ha[%d] ", index, serverInfo[index].serverStatus.ha);
    }
    Log.printf(LOG_LV2, "[ServerInfoManager] setHaServerInfo END ");
}
void ServerInfoManager::setRebuildAndHWServerInfo(int index, int rebuild, const HW& hw) {
    Log.printf(LOG_LV2, "[ServerInfoManager] setRebuildAndHWServerInfo START ");
    std::lock_guard<std::mutex> lock(Mutex);
    if (index < 0 || index >= MAX_SERVER) return;

    serverInfo[index].rebuild = rebuild;
    serverInfo[index].hwData = hw;
    Log.printf(LOG_LV1, "[ServerInfoManager] setRebuildAndHWServerInfo serverId[%d] rebuild[%d] ", index, serverInfo[index].rebuild);
    Log.printf(LOG_LV2, "[ServerInfoManager] setRebuildAndHWServerInfo END ");
}
void ServerInfoManager::plusAlarmCountServerInfo(int index, int type) {
    Log.printf(LOG_LV2, "[ServerInfoManager] plusAlarmCountServerInfo START ");
    Log.printf(LOG_LV1, "[ServerInfoManager] plusAlarmCountServerInfo serverId[%d] type[%d] ", index, type);
    std::lock_guard<std::mutex> lock(Mutex);
    if (index >= 0 && index < MAX_SERVER) {
        if(type == ALM_LEVEL_CRITICAL) {
            serverInfo[index].critical_count++;
        } else if(type == ALM_LEVEL_MAJOR) {
            serverInfo[index].major_count++;
        } else if(type == ALM_LEVEL_MINOR) {
            serverInfo[index].minor_count++;
        } else if(type == ALM_LEVEL_WARN) {
            serverInfo[index].warning_count++;
        }
    }
    Log.printf(LOG_LV2, "[ServerInfoManager] plusAlarmCountServerInfo END ");
}
void ServerInfoManager::minusAlarmCountServerInfo(int index, int type) {
    Log.printf(LOG_LV2, "[ServerInfoManager] minusAlarmCountServerInfo START ");
    Log.printf(LOG_LV1, "[ServerInfoManager] minusAlarmCountServerInfo serverId[%d] ", index);
    std::lock_guard<std::mutex> lock(Mutex);
    if (index >= 0 && index < MAX_SERVER) {
        if(type == ALM_LEVEL_CRITICAL) {
            serverInfo[index].critical_count--;
        } else if(type == ALM_LEVEL_MAJOR) {
            serverInfo[index].major_count--;
        } else if(type == ALM_LEVEL_MINOR) {
            serverInfo[index].minor_count--;
        } else if(type == ALM_LEVEL_WARN) {
            serverInfo[index].warning_count--;
        }
    }
    Log.printf(LOG_LV2, "[ServerInfoManager] minusAlarmCountServerInfo END ");
}
void ModuleInfoManager::setModuleInfo(int asindex, int moindex, int moduleId, int serverCode, int serverId, const char *asName, const char *moduleName, const char *serverName)
{
    char tmpAsName[10];
    char tmpServerName[10];
    char tmpModuleName[10];

    strncpy(tmpAsName, asName, sizeof(tmpAsName) - 1);
    strncpy(tmpServerName, serverName, sizeof(tmpServerName) - 1);
    strncpy(tmpModuleName, moduleName, sizeof(tmpModuleName) - 1);
    tmpAsName[sizeof(tmpAsName) - 1] = '\0';
    tmpServerName[sizeof(tmpServerName) - 1] = '\0';
    tmpModuleName[sizeof(tmpModuleName) - 1] = '\0';
    ToUpperCase(tmpAsName);
    ToUpperCase(tmpServerName);
    ToUpperCase(tmpModuleName);

    std::string result = tmpServerName;
    result.erase(std::remove(result.begin(), result.end(), '_'), result.end());

    Log.printf(LOG_LV2, "[ModuleInfoManager] setModuleInfo START ");
    Log.printf(LOG_LV1, "[ModuleInfoManager] setModuleInfo asindex[%d] moindex[%d] moduleId[%d] serverCode[%d] serverId[%d] asName[%s] moduleName[%s] serverName[%s] ",
               asindex, moindex, moduleId, serverCode, serverId, tmpAsName, tmpModuleName, result.c_str());
    std::lock_guard<std::mutex> lock(Mutex);
    if (asindex >= 0 && asindex < MAX_AS_EMS && moindex >= 0 && moindex < MAX_MODULE ) {
        moduleInfo[asindex][moindex].moduleId = moduleId;
        moduleInfo[asindex][moindex].serverCode = serverCode;
        moduleInfo[asindex][moindex].serverId = serverId;
        strncpy(moduleInfo[asindex][moindex].asName, tmpAsName, sizeof(moduleInfo[asindex][moindex].asName));
        strncpy(moduleInfo[asindex][moindex].moduleName, moduleName, sizeof(moduleInfo[asindex][moindex].moduleName));
        strncpy(moduleInfo[asindex][moindex].serverName, result.c_str(), sizeof(moduleInfo[asindex][moindex].serverName));
    }
    Log.printf(LOG_LV2, "[ModuleInfoManager] setModuleInfo END ");
}
void ModuleInfoManager::setStatusModuleInfo(int asindex, int moindex, int state) {
    Log.printf(LOG_LV2, "[ModuleInfoManager] setStatusModuleInfo START ");
    std::lock_guard<std::mutex> lock(Mutex);
    if (asindex >= 0 && asindex < MAX_AS_EMS && moindex >= 0 && moindex < MAX_MODULE ) {
        moduleInfo[asindex][moindex].status = state;
        Log.printf(LOG_LV1, "[ModuleInfoManager] setStatusModuleInfo asindex[%d] moindex[%d] status[%d] ", asindex, moindex, moduleInfo[asindex][moindex].status);
    }
    Log.printf(LOG_LV2, "[ModuleInfoManager] setStatusModuleInfo END ");
}
void ModuleInfoManager::setHaModuleInfo(int asindex, int moindex, int ha) {
    Log.printf(LOG_LV2, "[ModuleInfoManager] setHaModuleInfo START ");
    std::lock_guard<std::mutex> lock(Mutex);
    if (asindex >= 0 && asindex < MAX_AS_EMS && moindex >= 0 && moindex < MAX_MODULE ) {
        if(ha == 3) moduleInfo[asindex][moindex].ha = 2;
        else if(ha == 0) moduleInfo[asindex][moindex].ha = 0;
        else moduleInfo[asindex][moindex].ha = 3;
        if (strstr(moduleInfo[asindex][moindex].serverName, "SES") != nullptr && ha !=0) moduleInfo[asindex][moindex].ha = 2;
        Log.printf(LOG_LV1, "[ModuleInfoManager] setHaModuleInfo asindex[%d] moindex[%d] ha[%d] ", asindex, moindex, moduleInfo[asindex][moindex].ha);
    }
    Log.printf(LOG_LV2, "[ModuleInfoManager] setHaModuleInfo END ");
}
void ModuleInfoManager::setHaServerModuleInfo(int asindex, int serverId, int ha) {
    Log.printf(LOG_LV2, "[ModuleInfoManager] setHaServerModuleInfo START ");
    std::lock_guard<std::mutex> lock(Mutex);
    if (asindex >= 0 && asindex < MAX_AS_EMS && serverId >=0 && serverId < MAX_SERVER  ) {
        for(int moindex = 0 ; moindex < MAX_MODULE ; moindex++)
        {
            if(moduleInfo[asindex][moindex].serverId != serverId) continue;
            if(ha == 3) moduleInfo[asindex][moindex].ha = 2;
            else if(ha == 0) moduleInfo[asindex][moindex].ha = 0;
            else moduleInfo[asindex][moindex].ha = 3;
            if (strstr(moduleInfo[asindex][moindex].serverName, "SES") != nullptr && ha !=0) moduleInfo[asindex][moindex].ha = 2;
            Log.printf(LOG_LV1, "[ModuleInfoManager] setHaServerModuleInfo asindex[%d] moindex[%d] ha[%d] ", asindex, moindex, moduleInfo[asindex][moindex].ha);
        }
    }
    Log.printf(LOG_LV2, "[ModuleInfoManager] setHaServerModuleInfo END ");
}

void ModuleInfoManager::setHaEmsModuleInfo(int side, int result) {
    int haA=0 , haB=0;

    if(result == 1) {
        if(side ==0){ 
            haA = 2;
            haB = 3;
        } else {
            haA = 3;
            haB = 2;
        }
    }

    Log.printf(LOG_LV2, "[ModuleInfoManager] setHaEmsModuleInfo START ");

    std::lock_guard<std::mutex> lock(Mutex);
    for(int i=0;i<MAX_MODULE;i++) { 
        if(moduleInfo[MAX_AS][i].moduleId == -1) continue;
        if(moduleInfo[MAX_AS][i].serverName[3] == 'A') moduleInfo[MAX_AS][i].ha = haA; 
        else if(moduleInfo[MAX_AS][i].serverName[3] == 'B') moduleInfo[MAX_AS][i].ha = haB; 
        Log.printf(LOG_LV1, "[ModuleInfoManager] setHaEmsModuleInfo moduleName[%s] ha[%d] ", moduleInfo[MAX_AS][i].moduleName, moduleInfo[MAX_AS][i].ha);
    }
    Log.printf(LOG_LV2, "[ModuleInfoManager] setHaEmsModuleInfo END ");
}
void ModuleInfoManager::plusAlarmCountModuleInfo(int asindex, int moindex, int type) {
    Log.printf(LOG_LV2, "[ModuleInfoManager] plusAlarmCountModuleInfo START ");
    Log.printf(LOG_LV1, "[ModuleInfoManager] plusAlarmCountModuleInfo ASId[%d] moId[%d] type[%d] ", asindex, moindex, type);
    if(moindex < 0 ) return;
    std::lock_guard<std::mutex> lock(Mutex);
    if ((asindex >= 0 && asindex < MAX_AS_EMS) && (moindex >=0 && moindex < MAX_MODULE)) {
        if(type == ALM_LEVEL_CRITICAL) {
            moduleInfo[asindex][moindex].critical_count++;
        } else if(type == ALM_LEVEL_MAJOR) {
            moduleInfo[asindex][moindex].major_count++;
        } else if(type == ALM_LEVEL_MINOR) {
            moduleInfo[asindex][moindex].minor_count++;
        } else if(type == ALM_LEVEL_WARN) {
            moduleInfo[asindex][moindex].warning_count++;
        }
    }
    Log.printf(LOG_LV1, "[ModuleInfoManager] plusAlarmCountModuleInfo critical_count[%d] major_count[%d] minor_count[%d] warning_count[%d] ",
               moduleInfo[asindex][moindex].critical_count, moduleInfo[asindex][moindex].major_count, moduleInfo[asindex][moindex].minor_count, moduleInfo[asindex][moindex].warning_count);
    Log.printf(LOG_LV2, "[ModuleInfoManager] plusAlarmCountModuleInfo END ");
}
void ModuleInfoManager::minusAlarmCountModuleInfo(int asindex, int moindex, int type) {
    Log.printf(LOG_LV2, "[ModuleInfoManager] minusAlarmCountModuleInfo START ");
    Log.printf(LOG_LV1, "[ModuleInfoManager] minusAlarmCountModuleInfo serverId[%d] moId[%d] type[%d] ", asindex, moindex, type);
    std::lock_guard<std::mutex> lock(Mutex);
    if ((asindex >= 0 && asindex < MAX_AS_EMS) && (moindex >=0 && moindex < MAX_MODULE)) {
        if(type == ALM_LEVEL_CRITICAL) {
            moduleInfo[asindex][moindex].critical_count--;
        } else if(type == ALM_LEVEL_MAJOR) {
            moduleInfo[asindex][moindex].major_count--;
        } else if(type == ALM_LEVEL_MINOR) {
            moduleInfo[asindex][moindex].minor_count--;
        } else if(type == ALM_LEVEL_WARN) {
            moduleInfo[asindex][moindex].warning_count--;
        }
    }
    Log.printf(LOG_LV1, "[ModuleInfoManager] minusAlarmCountModuleInfo critical_count[%d] major_count[%d] minor_count[%d] warning_count[%d] ",
               moduleInfo[asindex][moindex].critical_count, moduleInfo[asindex][moindex].major_count, moduleInfo[asindex][moindex].minor_count, moduleInfo[asindex][moindex].warning_count);
    Log.printf(LOG_LV2, "[ModuleInfoManager] minusAlarmCountModuleInfo END ");
}

void AsStatsInfoManager::InitCpsStat() {
	Log.printf(LOG_LV2, "[AsStatsInfoManager] CPS STAT Init START");
	std::lock_guard<std::mutex> lock(Mutex);

	for (int i = 0; i < MAX_AS_EMS; ++i) {
        asstatsInfo[i].CpsSummery.min = 999;
        asstatsInfo[i].CpsSummery.max = -1;
        asstatsInfo[i].CpsSummery.avg = -1;
		asstatsInfo[i].CpsSummery.count = 0;
		asstatsInfo[i].CpsSummery.total = 0;
        memset(asstatsInfo[i].CpsSummery.peaktime, 0x00, sizeof(asstatsInfo[i].CpsSummery.peaktime));
    }
}


void AsStatsInfoManager::setAsStatsInfo(int index, const char *service_key) {
    Log.printf(LOG_LV2, "[AsStatsInfoManager] setAsStatsInfo START ");
    Log.printf(LOG_LV1, "[AsStatsInfoManager] setAsStatsInfo index[%d] service_key[%s] ", index, service_key);

    std::lock_guard<std::mutex> lock(Mutex);

    if (strcmp(service_key, "") == 0)
        return;

    // service_key 중복 체크 및 등록
    bool exists = false;
    int emptySlot = -1;

    for (int i = 0; i < MAX_SVC_USE; ++i) {
        if (strcmp(asstatsInfo[index].serviceStat[i].service_key, service_key) == 0) {
            exists = true;
            break;
        }
        if (emptySlot == -1 && strcmp(asstatsInfo[index].serviceStat[i].service_key, "") == 0) {
            emptySlot = i;
        }
    }

    if (!exists && emptySlot != -1) {
        strncpy(asstatsInfo[index].serviceStat[emptySlot].service_key, service_key, sizeof(asstatsInfo[index].serviceStat[emptySlot].service_key));
    }

    Log.printf(LOG_LV2, "[AsStatsInfoManager] setAsStatsInfo END ");
}
void AsStatsInfoManager::setSvcAsStatsInfo(int index, const char *service_key,  int tot_ch, int use_ch, int total_usage, int total_success)
{
    Log.printf(LOG_LV2, "[AsStatsInfoManager] setSvcAsStatsInfo START ");
    Log.printf(LOG_LV1, "[AsStatsInfoManager] setSvcAsStatsInfo index[%d] service_key[%s] tot_ch[%d] use_ch[%d] total_usage[%d] total_success[%d] ",
               index, service_key, tot_ch, use_ch, total_usage, total_success);
    std::lock_guard<std::mutex> lock(Mutex);
    if (index >= 0 && index < MAX_AS_EMS) {
        asstatsInfo[index].sum_usage = 0 ;
        asstatsInfo[index].sum_success = 0 ;
        for (int i = 0; i < MAX_SVC_USE; ++i) {
            if(strcmp(asstatsInfo[index].serviceStat[i].service_key, service_key)==0) { 
                asstatsInfo[index].serviceStat[i].tot_ch = tot_ch;
                asstatsInfo[index].serviceStat[i].use_ch = use_ch;
                asstatsInfo[index].serviceStat[i].total_usage = total_usage;
                asstatsInfo[index].serviceStat[i].total_success = total_success;
                asstatsInfo[index].sum_usage += total_usage;
                asstatsInfo[index].sum_success += total_success;
                break;
            }
        }
    }
    Log.printf(LOG_LV2, "[AsStatsInfoManager] setSvcAsStatsInfo END ");
}
void SystemResourceStatManager::setSystemResourceStat(int index, const char *type, int value) {
    int peak_check = 0;
    time_t now = time(NULL);
    struct tm t;
	localtime_r(&now, &t);

    char buffer[15];
    strftime(buffer, sizeof(buffer), "%Y%m%d%H%M%S", &t);

    Log.printf(LOG_LV2, "[SystemResourceStatManager] setSystemResourceStat START ");

    std::lock_guard<std::mutex> lock(Mutex);
    if (index >= 0 && index < MAX_SERVER) {
        if(strcmp(type, "CPU")==0) {
			//CPU MIN
            if(systemresourceStat[index].cpu_summery.min > value) {
                systemresourceStat[index].cpu_summery.min = value;
            }

			//CPU MAX
            if(systemresourceStat[index].cpu_summery.max < value) {
                systemresourceStat[index].cpu_summery.max = value;
                peak_check = 1;
            }

			//CPU AVG
			systemresourceStat[index].cpu_summery.count++;
			systemresourceStat[index].cpu_summery.total+=value;

			//CPU PEAKTIME
            if(peak_check == 1) strcpy(systemresourceStat[index].cpu_summery.peaktime, buffer);
            Log.printf(LOG_LV1, "[SystemResourceStatManager] setSystemResourceStat index[%d] CPU max[%d] min[%d] peak[%s] ", 
                       index, systemresourceStat[index].cpu_summery.max, systemresourceStat[index].cpu_summery.min, systemresourceStat[index].cpu_summery.peaktime);
        } else if(strcmp(type, "MEM")==0) {
			//MEM MIN
            if(systemresourceStat[index].mem_summery.min > value) {
                systemresourceStat[index].mem_summery.min = value;
            }
			
			//MEM MAX
            if(systemresourceStat[index].mem_summery.max < value) {
                systemresourceStat[index].mem_summery.max = value;
                peak_check = 1;
            }

			//MEM AVG
			systemresourceStat[index].mem_summery.count++;
			systemresourceStat[index].mem_summery.total+=value;

			//MEM PEAKTIME
            if(peak_check == 1) strcpy(systemresourceStat[index].mem_summery.peaktime, buffer);
            Log.printf(LOG_LV1, "[SystemResourceStatManager] setSystemResourceStat index[%d] MEM max[%d] min[%d] peak[%s] ", 
                       index, systemresourceStat[index].mem_summery.max, systemresourceStat[index].mem_summery.min, systemresourceStat[index].mem_summery.peaktime);
        } else if(strcmp(type, "NET")==0) {
			//NET MIN
            if(systemresourceStat[index].net_summery.min > value) {
                systemresourceStat[index].net_summery.min = value;
            }

			//NET MAX
            if(systemresourceStat[index].net_summery.max < value) {
                systemresourceStat[index].net_summery.max = value;
                peak_check = 1;
            }

			//NET AVG
			systemresourceStat[index].net_summery.count++;
			systemresourceStat[index].net_summery.total+=value;

			//NET PEAKTIME
            if(peak_check == 1) strcpy(systemresourceStat[index].net_summery.peaktime, buffer);
            Log.printf(LOG_LV1, "[SystemResourceStatManager] setSystemResourceStat index[%d] NET max[%d] min[%d] peak[%s] ", 
                       index, systemresourceStat[index].net_summery.max, systemresourceStat[index].net_summery.min, systemresourceStat[index].net_summery.peaktime);

        } else if(strcmp(type, "DISK")==0) {
			//DISK MIN
            if(systemresourceStat[index].disk_summery.min > value) {
                systemresourceStat[index].disk_summery.min = value;
            }

			//DISK MAX
            if(systemresourceStat[index].disk_summery.max < value) {
                systemresourceStat[index].disk_summery.max = value;
                peak_check = 1;
            }

			//DISK AVG
			systemresourceStat[index].disk_summery.count++;
			systemresourceStat[index].disk_summery.total+=value;

			//DISK PEAKTIME
            if(peak_check == 1) strcpy(systemresourceStat[index].disk_summery.peaktime, buffer);
            Log.printf(LOG_LV1, "[SystemResourceStatManager] setSystemResourceStat index[%d] NET max[%d] min[%d] peak[%s] ", 
                       index, systemresourceStat[index].disk_summery.max, systemresourceStat[index].disk_summery.min, systemresourceStat[index].disk_summery.peaktime);
        } else if(strcmp(type, "TEMPERATURE")==0) {
			//TEMPERATURE MIN
            if(systemresourceStat[index].temperature_summery.min > value) {
                systemresourceStat[index].temperature_summery.min = value;
            }

			//TEMPERATURE MAX
            if(systemresourceStat[index].temperature_summery.max < value) {
                systemresourceStat[index].temperature_summery.max = value;
                peak_check = 1;
            }

			//TEMPERATURE AVG
			systemresourceStat[index].temperature_summery.count++;
			systemresourceStat[index].temperature_summery.total+=value;

			//TMEPERATURE PEAKTIME
            if(peak_check == 1) strcpy(systemresourceStat[index].temperature_summery.peaktime, buffer);
            Log.printf(LOG_LV1, "[SystemResourceStatManager] setSystemResourceStat index[%d] TEMPERATURE max[%d] min[%d] peak[%s] ", 
                       index, systemresourceStat[index].temperature_summery.max, systemresourceStat[index].temperature_summery.min, systemresourceStat[index].temperature_summery.peaktime);
        } 
    }
    Log.printf(LOG_LV2, "[SystemResourceStatManager] setSystemResourceStat END ");
}


void AsStatsInfoManager::setChAsStatsInfo(int index, int cmdNo, void *arg)
{
    int ch_percent = 0;
    int peak_check = 0;
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    BD_USE    *rcv_v1;
    BD_USE_V2 *rcv_v2;
    BD_USE_V3 *rcv_v3;
    BD_USE_V3 trans_rcv;
    char buffer[15];
    strftime(buffer, sizeof(buffer), "%Y%m%d%H%M%S", t);


    Log.printf(LOG_LV2, "[AsStatsInfoManager] setChAsStatsInfo START ");
    Log.printf(LOG_LV1, "[AsStatsInfoManager] setChAsStatsInfo index[%d] cmdNo[%d] ", index, cmdNo);

    if (arg == nullptr) {
        Log.printf(LOG_ERR, "[AsStatsInfoManager] setChAsStatsInfo arg is null! index:%d cmdNo:%d", index, cmdNo);
        return;
    }

    memset (&trans_rcv, 0, sizeof(BD_USE_V3));

    std::lock_guard<std::mutex> lock(Mutex);
    if (index >= 0 && index < MAX_AS_EMS) {
        if(cmdNo == CNS_BoardUseRP){
            rcv_v1 = (BD_USE *)arg;
            trans_rcv.count  = rcv_v1->count;
            trans_rcv.cCPS   = rcv_v1->cCPS;
            trans_rcv.cReady = rcv_v1->cReady;
        } else if(cmdNo == CNS_BoardUseRPL){
            rcv_v2 = (BD_USE_V2 *)arg;
            trans_rcv.count  = rcv_v2->count;
            trans_rcv.cCPS   = rcv_v2->cCPS;
            trans_rcv.cReady = rcv_v2->cReady;
            asstatsInfo[index].discardChannel = rcv_v2->nOverloadCount;
        } else if(cmdNo == CNS_BoardUseRPB){
            rcv_v3 = (BD_USE_V3 *)arg;
            trans_rcv.count  = rcv_v3->count;
            trans_rcv.cCPS   = rcv_v3->cCPS;
            trans_rcv.cReady = rcv_v3->cReady;
            asstatsInfo[index].discardChannel = rcv_v3->nOverloadCount;
        }
        Log.printf(LOG_LV1, "[AsStatsInfoManager] setChAsStatsInfo trans_rcv.count[%d] cps[%d] cReady[%d] ", trans_rcv.count, trans_rcv.cCPS, trans_rcv.cCPS);
        if (asstatsInfo[index].discardChannel > 0 &&
            asstatsInfo[index].discardChannel < 5)
        {
            asstatsInfo[index].discardChannel = 1 ;
        }
        else if (asstatsInfo[index].discardChannel >= 5)
        {
            asstatsInfo[index].discardChannel = asstatsInfo[index].discardChannel / 5;
        }
        else
        {
            asstatsInfo[index].discardChannel = 0;
        }


        asstatsInfo[index].cps   = trans_rcv.cCPS;
        asstatsInfo[index].sumBusy  = 0;
        asstatsInfo[index].sumTotal = 0;

		//CPS MAX
        if(asstatsInfo[index].CpsSummery.min > trans_rcv.cCPS) asstatsInfo[index].CpsSummery.min = trans_rcv.cCPS;
        if(asstatsInfo[index].CpsSummery.max < trans_rcv.cCPS) {
            asstatsInfo[index].CpsSummery.max = trans_rcv.cCPS;
            peak_check = 1;
        }

		//CPS PEAKTIME
        if(peak_check == 1) strcpy(asstatsInfo[index].CpsSummery.peaktime, buffer);

		//CPS MIN
        if(asstatsInfo[index].CpsSummery.min > trans_rcv.cCPS) asstatsInfo[index].CpsSummery.min = trans_rcv.cCPS;

        Log.printf(LOG_LV2, "[AsStatsInfoManager] setChAsStatsInfo index[%d] cps[%d] discardChannel[%d] count[%d] ", 
                  index, asstatsInfo[index].cps, asstatsInfo[index].discardChannel, trans_rcv.count);

        if(trans_rcv.count < 0 ) { 
            Log.printf(LOG_ERR, "[AsStatsInfoManager] setChAsStatsInfo count[%d] < 0 ", trans_rcv.count);
            return;
        }

		//CPS AVG
		asstatsInfo[index].CpsSummery.total+=trans_rcv.cCPS;
		asstatsInfo[index].CpsSummery.count++;


        for(int i = 0 ; i < trans_rcv.count ; i++)  // as   //SES5 .jhcho 
        {
            if(cmdNo == CNS_BoardUseRP){
                rcv_v1 = (BD_USE *)arg;
                trans_rcv.item[i].TotalCh = rcv_v1->item[i].TotalCh;
                trans_rcv.item[i].BusyCh  = rcv_v1->item[i].BusyCh;
                trans_rcv.item[i].Flag    = rcv_v1->item[i].Flag;
            } else if(cmdNo == CNS_BoardUseRPL){
                rcv_v2 = (BD_USE_V2 *)arg;
                trans_rcv.item[i].TotalCh = rcv_v2->item[i].TotalCh;
                trans_rcv.item[i].BusyCh  = rcv_v2->item[i].BusyCh;
                trans_rcv.item[i].Flag    = rcv_v2->item[i].Flag;
            } else if(cmdNo == CNS_BoardUseRPB){
                rcv_v3 = (BD_USE_V3 *)arg;
                trans_rcv.item[i].TotalCh    = rcv_v3->item[i].TotalCh;
                trans_rcv.item[i].BusyCh     = rcv_v3->item[i].BusyCh;
                trans_rcv.item[i].Flag       = rcv_v3->item[i].Flag;
                trans_rcv.item[i].nSyncCount = rcv_v3->item[i].nSyncCount;
            }

            asstatsInfo[index].sumBusy  += trans_rcv.item[i].BusyCh;
            asstatsInfo[index].sumTotal += trans_rcv.item[i].TotalCh;

            if (cmdNo == CNS_BoardUseRPB)
            {
                asstatsInfo[index].sumBusy += trans_rcv.item[i].nSyncCount;
            }
			Log.printf(LOG_LV2, "[AsStatsInfoManager] setChAsStatsInfo i[%d] Busy[%ld] Total[%ld] Flag[%d] ",
                       i, trans_rcv.item[i].BusyCh, trans_rcv.item[i].TotalCh, trans_rcv.item[i].Flag);
        }
    }
    Log.printf(LOG_LV2, "[AsStatsInfoManager] setChAsStatsInfo sumBusy[%ld] sumTotal[%ld] ", asstatsInfo[index].sumBusy, asstatsInfo[index].sumTotal);
    Log.printf(LOG_LV2, "[AsStatsInfoManager] setChAsStatsInfo END ");
}

void PerformanceManager::setResourcePerformance(int index, int cpu, int memory, int network, int disk) {
    Log.printf(LOG_LV2, "[PerformanceManager] setResourcePerformance START ");
    Log.printf(LOG_LV1, "[PerformanceManager] setResourcePerformance index[%d] cpu[%d] memory[%d] network[%d] disk[%d]",
               index, cpu, memory, network, disk);

    std::lock_guard<std::mutex> lock(Mutex);
    if (index >= 0 && index < MAX_SERVER) {
        performance[index].cpu = cpu;
        performance[index].memory = memory;
        performance[index].network = network;
        performance[index].disk = disk;
    }
    Log.printf(LOG_LV2, "[PerformanceManager] setResourcePerformance END ");
}
void PerformanceManager::setChPerformance(int index, int cmdNo, void *arg) {
    int svrId; 
    int  ch_percent = 0;
    BYTE      xbusId;
    BD_USE    *rcv_v1;
    BD_USE_V2 *rcv_v2;
    BD_USE_V3 *rcv_v3;
    BD_USE_V3 trans_rcv;

    Log.printf(LOG_LV2, "[PerformanceManager] setChPerformance START ");
    Log.printf(LOG_LV1, "[PerformanceManager] setChPerformance index[%d] ", index);

    memset (&trans_rcv, 0, sizeof(BD_USE_V3));

    std::lock_guard<std::mutex> lock(Mutex);
    if (index >= 0 && index < MAX_SERVER) {
        if(cmdNo == CNS_BoardUseRP){
            rcv_v1 = (BD_USE *)arg;
            trans_rcv.count  = rcv_v1->count;
        } else if(cmdNo == CNS_BoardUseRPL){
            rcv_v2 = (BD_USE_V2 *)arg;
            trans_rcv.count  = rcv_v2->count;
        } else if(cmdNo == CNS_BoardUseRPB){
            rcv_v3 = (BD_USE_V3 *)arg;
            trans_rcv.count  = rcv_v3->count;
        }

        for(int i = 0 ; i < trans_rcv.count ; i++)  // as   //SES5 .jhcho 
        {
        #ifdef ONESERVER_MODE
            if(i == 0) svrId = 2;
            else svrId = 3;
        #else
            xbusId = (0x90+i);
            if (!GetServerIdFromXbus(index, xbusId, svrId)) {
                return;
            }
        #endif

            if(cmdNo == CNS_BoardUseRP){
                rcv_v1 = (BD_USE *)arg;
                trans_rcv.item[i].TotalCh = rcv_v1->item[i].TotalCh;
                trans_rcv.item[i].BusyCh  = rcv_v1->item[i].BusyCh;
            } else if(cmdNo == CNS_BoardUseRPL){
                rcv_v2 = (BD_USE_V2 *)arg;
                trans_rcv.item[i].TotalCh = rcv_v2->item[i].TotalCh;
                trans_rcv.item[i].BusyCh  = rcv_v2->item[i].BusyCh;
            } else if(cmdNo == CNS_BoardUseRPB){
                rcv_v3 = (BD_USE_V3 *)arg;
                trans_rcv.item[i].TotalCh    = rcv_v3->item[i].TotalCh;
                trans_rcv.item[i].BusyCh     = rcv_v3->item[i].BusyCh;
                performance[svrId].sync_count = rcv_v3->item[i].nSyncCount;
            }

            performance[svrId].channel_count = trans_rcv.item[i].BusyCh;
            if (trans_rcv.item[i].TotalCh ==0 || trans_rcv.item[i].BusyCh == 0)
            {
                performance[svrId].channel_rate = 0;

            } else {
                ch_percent = (int)((float)(trans_rcv.item[i].BusyCh)*100.00/(float)(trans_rcv.item[i].TotalCh));
                performance[svrId].channel_rate = ch_percent;

            }
            Log.printf(LOG_LV2, "[PerformanceManager] setChPerformance i[%d] svrId[%d] channel_count[%d], sync_count[%d]  channel_rate[%d]",
                                 i, svrId, performance[svrId].channel_count, performance[svrId].sync_count,  performance[svrId].channel_rate);
        }
    }
    Log.printf(LOG_LV2, "[PerformanceManager] setChPerformance END ");
}

void PerformanceManager::setNtpPerformance(int index, int ntp) {
    Log.printf(LOG_LV2, "[PerformanceManager] setNtpPerformance START ");
    Log.printf(LOG_LV1, "[PerformanceManager] setNtpPerformance index[%d] ntp[%d]", index, ntp);

    std::lock_guard<std::mutex> lock(Mutex);
    if (index >= 0 && index < MAX_SERVER) {
        performance[index].ntp = ntp;
    }
    Log.printf(LOG_LV2, "[PerformanceManager] setNtpPerformance END ");
}

void PerformanceManager::setTableSpacePerformance(int serveridx, int tsIdx, const char* tsName, int use) {
    Log.printf(LOG_LV2, "[PerformanceManager] setTableSpacePerformance START ");
    Log.printf(LOG_LV1, "[PerformanceManager] setTableSpacePerformance index[%d] use[%d]", tsIdx, use);

    std::lock_guard<std::mutex> lock(Mutex);
    if (serveridx >= 0 && serveridx < MAX_SERVER) {
        performance[serveridx].tablespace[tsIdx].use = use;
		strncpy(performance[serveridx].tablespace[tsIdx].name, tsName, sizeof(performance[serveridx].tablespace[tsIdx].name));
    }
    Log.printf(LOG_LV2, "[PerformanceManager] setTableSpacePerformance END ");
}

void PerformanceManager::setDBSessionPerformance(int serveridx, int use) {
    Log.printf(LOG_LV2, "[PerformanceManager] setDBSessionPerformance START \n");
    Log.printf(LOG_LV1, "[PerformanceManager] setDBSessionPerformance index[%d], use[%d]\n", serveridx, use);

    std::lock_guard<std::mutex> lock(Mutex);
    if (serveridx >= 0 && serveridx < MAX_SERVER) {
        performance[serveridx].dbSession = use;
    }
    Log.printf(LOG_LV2, "[PerformanceManager] setDBSessionPerformance END \n");
}
void PerformanceManager::setTemperaturePerformance(int serveridx, int temperature) {
    Log.printf(LOG_LV2, "[PerformanceManager] setTemperaturePerformance START \n");
    Log.printf(LOG_LV1, "[PerformanceManager] setTemperaturePerformance index[%d], temperature[%d]\n", serveridx, temperature);

    std::lock_guard<std::mutex> lock(Mutex);
    if (serveridx >= 0 && serveridx < MAX_SERVER) {
        performance[serveridx].temperature = temperature;
    }
    Log.printf(LOG_LV2, "[PerformanceManager] setTemperaturePerformance END \n");
}

void ThresholdInfoManager::setThresHoldInfo(const char *type, int critical, int major, int minor, int warning) {
    Log.printf(LOG_LV2, "[ThresholdInfoManager] setThresHoldInfo START ");
    Log.printf(LOG_LV1, "[ThresholdInfoManager] setThresHoldInfo type[%s] critical[%d] major[%d] minor[%d] warning[%d] ", type, critical, major, minor, warning);
    std::lock_guard<std::mutex> lock(Mutex);
    
    if(strcmp(type, "cpu")==0) {
        thresholdInfo.cpu[0] = critical;
        thresholdInfo.cpu[1] = major;
        thresholdInfo.cpu[2] = minor;
        thresholdInfo.cpu[3] = warning;
    } else if(strcmp(type, "memory")==0) {
        thresholdInfo.memory[0] = critical;
        thresholdInfo.memory[1] = major;
        thresholdInfo.memory[2] = minor;
        thresholdInfo.memory[3] = warning;
    } else if(strcmp(type, "network")==0) {
        thresholdInfo.network[0] = critical;
        thresholdInfo.network[1] = major;
        thresholdInfo.network[2] = minor;
        thresholdInfo.network[3] = warning;
    } else if(strcmp(type, "disk")==0) {
        thresholdInfo.disk[0] = critical;
        thresholdInfo.disk[1] = major;
        thresholdInfo.disk[2] = minor;
        thresholdInfo.disk[3] = warning;
    } else if(strcmp(type, "channel")==0) {
        thresholdInfo.channel[0] = critical;
        thresholdInfo.channel[1] = major;
        thresholdInfo.channel[2] = minor;
        thresholdInfo.channel[3] = warning;
    } else if(strcmp(type, "ntp")==0) {
        thresholdInfo.ntp[0] = critical;
        thresholdInfo.ntp[1] = major;
        thresholdInfo.ntp[2] = minor;
        thresholdInfo.ntp[3] = warning;
    } else if(strcmp(type, "tablespace")==0) {
        thresholdInfo.tablespace[0] = critical;
        thresholdInfo.tablespace[1] = major;
        thresholdInfo.tablespace[2] = minor;
        thresholdInfo.tablespace[3] = warning;
    } else if(strcmp(type, "temperature")==0) {
        thresholdInfo.temperature[0] = critical;
        thresholdInfo.temperature[1] = major;
        thresholdInfo.temperature[2] = minor;
        thresholdInfo.temperature[3] = warning;
    } else if(strcmp(type, "session")==0) {
        thresholdInfo.session[0] = critical;
        thresholdInfo.session[1] = major;
        thresholdInfo.session[2] = minor;
        thresholdInfo.session[3] = warning;
    } 
    Log.printf(LOG_LV2, "[ThresholdInfoManager] setThresHoldInfo END ");
}
void SswConfigInfoManager::setSswConfigInfo(int index, const SSW_CONFIG_INFO& info) {
    Log.printf(LOG_LV2, "[SswConfigInfoManager] setSswConfigInfo START ");
    Log.printf(LOG_LV1, "[SswConfigInfoManager] asId[%d] index[%d] systemName[%s] ip[%s] status[%d] block[%d] monitor[%d] ",
              info.asId, info.index, info.systemName, info.ip, info.status, info.block, info.monitor);
    std::lock_guard<std::mutex> lock(Mutex);
    if (index >= 0 && index < MAX_SSW) {
        sswconfigInfo[index].asId = info.asId;
        sswconfigInfo[index].index = info.index;
        strncpy(sswconfigInfo[index].systemName, info.systemName, sizeof(sswconfigInfo[index].systemName));
        strncpy(sswconfigInfo[index].ip, info.ip, sizeof(sswconfigInfo[index].ip));
        strncpy(sswconfigInfo[index].port, info.port, sizeof(sswconfigInfo[index].port));
        sswconfigInfo[index].status = info.status;
        sswconfigInfo[index].block = info.block;
        sswconfigInfo[index].monitor = info.monitor;
    }
    Log.printf(LOG_LV2, "[SswConfigInfoManager] setSswConfigInfo END ");
}

void SswConfigInfoManager::setSswStatusInfo(int index, int status) {
    Log.printf(LOG_LV2, "[SswConfigInfoManager] setSswStatusInfo START ");
    Log.printf(LOG_LV1, "[SswConfigInfoManager] setSswStatusInfo status[%d] ", status);
    std::lock_guard<std::mutex> lock(Mutex);
    sswconfigInfo[index].status = status;
    Log.printf(LOG_LV2, "[SswConfigInfoManager] setSswStatusInfo END ");
}

void MsConfigInfoManager::setMsConfigInfo(int index, const MS_CONFIG_INFO& info) {
    Log.printf(LOG_LV2, "[MsConfigInfoManager] setMsConfigInfo START ");
    std::lock_guard<std::mutex> lock(Mutex);
    if (index >= 0 && index < MAX_MS) {
        msconfigInfo[index].asId = info.asId;
        msconfigInfo[index].index = info.index;
        strncpy(msconfigInfo[index].systemName, info.systemName, sizeof(msconfigInfo[index].systemName));
        strncpy(msconfigInfo[index].ip, info.ip, sizeof(msconfigInfo[index].ip));
        strncpy(msconfigInfo[index].port, info.port, sizeof(msconfigInfo[index].port));
        msconfigInfo[index].block_flag = info.block_flag;
        msconfigInfo[index].alive_flag = info.alive_flag;
        msconfigInfo[index].state_flag = info.state_flag;
        msconfigInfo[index].type = info.type;
    }
    Log.printf(LOG_LV2, "[MsConfigInfoManager] setMsConfigInfo END ");
}
void ExternalIfInfoManager::setExternalIfInfo(int index, const EXTERNAL_IF_INFO& info) {
    Log.printf(LOG_LV2, "[ExternalIfInfoManager] setExternalIfInfo START ");
    std::lock_guard<std::mutex> lock(Mutex);
    if (index >= 0 && index < MAX_EXTERNAL) {
        externalifInfo[index].asId = info.asId;
        strncpy(externalifInfo[index].systemName, info.systemName, sizeof(externalifInfo[index].systemName));
        strncpy(externalifInfo[index].ip, info.ip, sizeof(externalifInfo[index].ip));
        strncpy(externalifInfo[index].port, info.port, sizeof(externalifInfo[index].port));
        externalifInfo[index].status = info.status;
        externalifInfo[index].system_type = info.system_type;
		externalifInfo[index].ConnSvrID = info.ConnSvrID;
		strncpy(externalifInfo[index].system, info.system, sizeof(externalifInfo[index].system));
    }
    Log.printf(LOG_LV2, "[ExternalIfInfoManager] setExternalIfInfo END ");
}
void CdbStatusInfoManager::setCdbStatusInfo(int index, const CDB_STATUS_INFO& info) {
    Log.printf(LOG_LV2, "[CdbStatusInfoManager] setCdbStatusInfo START ");
    std::lock_guard<std::mutex> lock(Mutex);
    if (index >= 0 && index < MAX_CDB) {
        cdbstatusInfo[index].asId = info.asId;
		cdbstatusInfo[index].serverID = info.serverID;
        cdbstatusInfo[index].groupId = info.groupId;
        cdbstatusInfo[index].groupSeq = info.groupSeq;
        strncpy(cdbstatusInfo[index].systemName, info.systemName, sizeof(cdbstatusInfo[index].systemName));
        strncpy(cdbstatusInfo[index].ip, info.ip, sizeof(cdbstatusInfo[index].ip));
        strncpy(cdbstatusInfo[index].port, info.port, sizeof(cdbstatusInfo[index].port));
        cdbstatusInfo[index].status = info.status;
        cdbstatusInfo[index].use_flag = info.use_flag;
		cdbstatusInfo[index].completeRate = info.completeRate;
        Log.printf(LOG_LV1, "[CdbStatusInfoManager] index[%d], serverID[%d], systemName[%s], ip[%s] port[%s] ",
                             index, cdbstatusInfo[index].serverID, cdbstatusInfo[index].systemName, cdbstatusInfo[index].ip, cdbstatusInfo[index].port );
    }
    Log.printf(LOG_LV2, "[CdbStatusInfoManager] setCdbStatusInfo END ");
}

void AlarmConfigManager::setAlarmConfig(int index, const ALARM_CONFIG& info) {
    std::lock_guard<std::mutex> lock(Mutex);
    if (index >= 0 && index < MAX_ALARM) {
        if (info.AlarmCode != -1) alarmConfig[index].AlarmCode = info.AlarmCode;
        if (info.AlarmType != -1) alarmConfig[index].AlarmType = info.AlarmType;
        if (info.AlarmLevel != -1) alarmConfig[index].AlarmLevel = info.AlarmLevel;
        if (info.AlarmDesc[0] != '\0') strncpy(alarmConfig[index].AlarmDesc, info.AlarmDesc, sizeof(alarmConfig[index].AlarmDesc));
        if (info.ext_send != -1) alarmConfig[index].ext_send = info.ext_send;
    }
}
void AsInfoManager::getASInfo(int index, AS_INFO& info) const {
    //Log.printf(LOG_LV2, "[AsInfoManager] getASInfo START ");
    std::lock_guard<std::mutex> lock(Mutex);
    if (index >= 0 && index < MAX_AS_EMS) {
        info = asInfo[index];
    }
    //Log.printf(LOG_LV2, "[AsInfoManager] getASInfo END ");
}
int AsInfoManager::FindAsIdByName(const char* asName) {
    for (int index = 0; index < MAX_AS_EMS; ++index) {
        if (strcmp(asInfo[index].asName, asName) == 0) {
            Log.printf(LOG_LV2, "[AsInfoManager ] FindAsIdByName asName[%s] asId[%d] ", asName, asInfo[index].asId);
            return asInfo[index].asId;
        }
    }
    Log.printf(LOG_LV2, "[AsInfoManager ] FindAsIdByName asName[%s] Not Found ", asName);
    return -1;
}

void ServerInfoManager::getServerInfo(int index, SERVER_INFO& info) const {
    //Log.printf(LOG_LV2, "[ServerInfoManager] getServerInfo START ");
    std::lock_guard<std::mutex> lock(Mutex);
    if (index >= 0 && index < MAX_SERVER) {
        info = serverInfo[index];
    }
    //Log.printf(LOG_LV2, "[ServerInfoManager] getServerInfo END ");
}

int ServerInfoManager::getServerCode(int index){
	SERVER_INFO svrData;
	std::lock_guard<std::mutex> lock(Mutex);
	if (index >= 0 && index < MAX_SERVER) {
    	svrData = serverInfo[index];
		return svrData.serverCode;
    }
	else	return -1;
}


int ServerInfoManager::getServerASID(int index){
    SERVER_INFO svrData;
    std::lock_guard<std::mutex> lock(Mutex);
    if (index >= 0 && index < MAX_SERVER) {
        svrData = serverInfo[index];
        return svrData.asId;
    }
    else    return -1;
}

int ServerInfoManager::FindServerIdByName(const char* serverName) {
    for (int index = 0; index < MAX_SERVER; ++index) {
        if (strcmp(serverInfo[index].serverName, serverName) == 0) {
            Log.printf(LOG_LV2, "[ServerInfoManager ] FindServerIdByName serverName[%s] serverId[%d] ", serverName, serverInfo[index].serverId);
            return serverInfo[index].serverId;
        }
    }
    Log.printf(LOG_LV2, "[ServerInfoManager ] FindServerIdByName serverName[%s] Not Found ", serverName);
    return -1;
}
int ServerInfoManager::FindServerHaByName(const char* serverName) {
    for (int index = 0; index < MAX_SERVER; ++index) {
        if (strcmp(serverInfo[index].serverName, serverName) == 0) {
            Log.printf(LOG_LV2, "[ServerInfoManager ] FindServerHaByName serverName[%s] serverHa[%d] ", serverName, serverInfo[index].serverStatus.ha);
            return serverInfo[index].serverStatus.ha;
        }
    }
    Log.printf(LOG_LV2, "[ServerInfoManager ] FindServerHaByName serverName[%s] Not Found ", serverName);
    return -1;
}
std::string ServerInfoManager::FindServerNameByHa(const char* serverName, int ha)
{
    for (int index = 0; index < MAX_SERVER; ++index) {
        if (strstr(serverInfo[index].serverName, serverName) != nullptr && serverInfo[index].serverStatus.ha == ha) {
            Log.printf(LOG_LV2, "[ServerInfoManager ] FindServerNameByHa serverName[%s] serverHa[%d] ", serverName, serverInfo[index].serverStatus.ha);
            return serverInfo[index].serverName;
        }
    }
    Log.printf(LOG_LV2, "[ServerInfoManager ] FindServerNameByHa serverName[%s] Not Found ", serverName);
    return "";
}

void AsStatsInfoManager::getAsStats(int index, AS_STATS& info) const {
    //Log.printf(LOG_LV2, "[AsStatsInfoManager] getAsStats START ");
    std::lock_guard<std::mutex> lock(Mutex);
    if (index >= 0 && index < MAX_AS_EMS) {
        info = asstatsInfo[index];
    }
    //Log.printf(LOG_LV2, "[AsStatsInfoManager] getAsStats END ");
}
void SystemResourceStatManager::getSystemResourceStat(int index, SYSTEM_RESOURCE_STAT& info) const {
    //Log.printf(LOG_LV2, "[SystemResourceStatManager] getSystemResourceStat START ");
    std::lock_guard<std::mutex> lock(Mutex);
    if (index >= 0 && index < MAX_SERVER) {
        info = systemresourceStat[index];
    }
    //Log.printf(LOG_LV2, "[SystemResourceStatManager] getSystemResourceStat END ");
}
void ModuleInfoManager::getModuleInfo(int asindex, int moindex, MODULE_INFO& info) const {
    //Log.printf(LOG_LV2, "[ModuleInfoManager ] getModuleInfo START ");
    std::lock_guard<std::mutex> lock(Mutex);
    if (asindex >= 0 && asindex < MAX_AS_EMS && moindex >= 0 && moindex < MAX_MODULE ) {
        info = moduleInfo[asindex][moindex];
    }
    //Log.printf(LOG_LV2, "[ModuleInfoManager ] getModuleInfo END ");
}
int ModuleInfoManager::FindModuleIdByName(const char* moduleName) {
    for (int as = 0; as < MAX_AS_EMS; ++as) {
        for (int mod = 0; mod < MAX_MODULE; ++mod) {
            if (strcmp(moduleInfo[as][mod].moduleName, moduleName) == 0) {
                Log.printf(LOG_LV2, "[ModuleInfoManager ] FindModuleIdByName moduleName[%s] moduleId[%d] ", moduleName, moduleInfo[as][mod].moduleId);
                return moduleInfo[as][mod].moduleId;
            }
        }
    }
    Log.printf(LOG_LV2, "[ModuleInfoManager ] FindModuleIdByName moduleName[%s] Not Found ", moduleName);
    return -1;
}
int ModuleInfoManager::FindSvrIdByModuleId(int moduleId) {
    for (int as = 0; as < MAX_AS_EMS; ++as) {
        for (int mod = 0; mod < MAX_MODULE; ++mod) {
            if (moduleInfo[as][mod].moduleId == moduleId) {
                Log.printf(LOG_LV2, "[ModuleInfoManager ] FindSvrIdByModuleId moduleId[%d] serverId[%d] ", moduleInfo[as][mod].moduleId, moduleInfo[as][mod].serverId);
                return moduleInfo[as][mod].serverId;
            }
        }
    }
    Log.printf(LOG_LV2, "[ModuleInfoManager ] FindSvrIdByModuleId moduleId[%d] Not Found ", moduleId);
    return -1;
}
std::string ModuleInfoManager::FindModuleNameByModuleId(int moduleId){
    for (int as = 0; as < MAX_AS_EMS; ++as) {
        for (int mod = 0; mod < MAX_MODULE; ++mod) {
            if (moduleInfo[as][mod].moduleId == moduleId) {
                Log.printf(LOG_LV2, "[ModuleInfoManager ] FindModuleNameByModuleId moduleId[%d] serverId[%d] ", moduleInfo[as][mod].moduleId, moduleInfo[as][mod].serverId);
                return moduleInfo[as][mod].moduleName;
            }
        }
    }
    Log.printf(LOG_LV2, "[ModuleInfoManager ] FindModuleNameByModuleId moduleId[%d] Not Found ", moduleId);
    return "";
}
void PerformanceManager::getPerformance(int index, PERFORMANCE& info) const {
    //Log.printf(LOG_LV2, "[PerformanceManager] getPerformance START ");
    std::lock_guard<std::mutex> lock(Mutex);
    if (index >= 0 && index < MAX_SERVER) {
        info = performance[index];
    }
    //Log.printf(LOG_LV2, "[PerformanceManager] getPerformance END ");
}
void ChassisInfoManager::getChassisInfo(int index, CHASSIS_INFO& info) const {
    //Log.printf(LOG_LV2, "[ChassisInfoManager] getChassisInfo START ");
    std::lock_guard<std::mutex> lock(Mutex);
    if (index >= 0 && index < MAX_SERVER) {
        info = chassisInfo[index];
    }
    //Log.printf(LOG_LV2, "[ChassisInfoManager] getChassisInfo END ");
}
void ThresholdInfoManager::getThresHoldInfo(THRESHOLD_INFO& info) const {
    //Log.printf(LOG_LV2, "[ThresholdInfoManager] getThresHoldInfo START ");
    std::lock_guard<std::mutex> lock(Mutex);
    info = thresholdInfo;
    //Log.printf(LOG_LV2, "[ThresholdInfoManager] getThresHoldInfo END ");
}
/*
void ThresholdInfoManager::setServiceHoldInfo(int index, SERVICE_LIMIT info) const {
    std::lock_guard<std::mutex> lock(Mutex);
	strcpy(svcholdInfo[index].ServiceName, info.ServiceName);
	svcholdInfo[index].ThreshholdCallCount = info.ThreshholdCallCount;
	svcholdInfo[index].SuccessRate = info.SuccessRate;
   
}   

void ThresholdInfoManager::getServiceHoldInfo(const char* service, SERVICE_LIMIT& info) const {
    std::lock_guard<std::mutex> lock(Mutex);
	int INDEX = -1;

	for(int i =0, i<MAX_SVC_USE, i++)
	{
		if(!strcmp(svcholdInfo[i].ServiceName, service)) continue;
		else
		{
			INDEX = i;
			break;
		}
	}
	
	if(INDEX >= 0)
	{
		info = svcholdInfo[i];
	}
	else if(find == true)
	{
		info = NULL;
	}
}   
*/
void SswConfigInfoManager::getSswConfigInfo(int index, SSW_CONFIG_INFO& info) const {
    //Log.printf(LOG_LV2, "[SswConfigInfoManager] getSswConfigInfo START ");
    std::lock_guard<std::mutex> lock(Mutex);
    if (index >= 0 && index < MAX_SSW) {
        info = sswconfigInfo[index];
    }
    //Log.printf(LOG_LV2, "[SswConfigInfoManager] getSswConfigInfo END ");
}
void MsConfigInfoManager::getMsConfigInfo(int index, MS_CONFIG_INFO& info) const {
    //Log.printf(LOG_LV2, "[MsConfigInfoManager] getMsConfigInfo START ");
    std::lock_guard<std::mutex> lock(Mutex);
    if (index >= 0 && index < MAX_MS) {
        info = msconfigInfo[index];
    }
    //Log.printf(LOG_LV2, "[MsConfigInfoManager] getMsConfigInfo END ");
}
void ExternalIfInfoManager::getExternalIfInfo(int index, EXTERNAL_IF_INFO& info) const {
    //Log.printf(LOG_LV2, "[ExternalIfInfoManager] getExternalIfInfo START ");
    std::lock_guard<std::mutex> lock(Mutex);
    if (index >= 0 && index < MAX_EXTERNAL) {
        info = externalifInfo[index];
    }
    //Log.printf(LOG_LV2, "[ExternalIfInfoManager] getExternalIfInfo END ");
}
void CdbStatusInfoManager::getCdbStatusInfo(int index, CDB_STATUS_INFO& info) const {
    //Log.printf(LOG_LV2, "[CdbStatusInfoManager] getCdbStatusInfo START ");
    std::lock_guard<std::mutex> lock(Mutex);
    if (index >= 0 && index < MAX_CDB) {
        info = cdbstatusInfo[index];
    }
    //Log.printf(LOG_LV2, "[CdbStatusInfoManager] getCdbStatusInfo END ");
}

void AlarmConfigManager::getAlarmConfig(int index, ALARM_CONFIG& info) const {
    std::lock_guard<std::mutex> lock(Mutex);
    if (index >= 0 && index < MAX_ALARM) {
        info = alarmConfig[index];
    }
}
void AsInfoManager::printASInfo() const {
    std::lock_guard<std::mutex> lock(Mutex);
    for(int index = 0;index < MAX_AS_EMS;++index) { 
        const AS_INFO& info = asInfo[index];
        if(info.asId == -1) continue;
        printf("[AS_INFO] asId: %d, asName: %s, asDesc: %s", info.asId, info.asName, info.asDesc);
    }
}
void ChassisInfoManager::printChassisInfo() const {
    std::lock_guard<std::mutex> lock(Mutex);
    for(int index = 0;index < MAX_CHASSIS;++index) { 
        const CHASSIS_INFO& info = chassisInfo[index];
        if(info.asId == -1) continue;
        printf("[CHASSIS_INFO] chassisId: %d, asId: %d, rackId: %d, chassisType: %d, chassisName: %s, use: %d",
               info.chassisId, info.asId, info.rackId, info.chassisType, info.chassisName, info.use);
        for (int i = 0; i < MAX_HW; ++i) {
            if(info.hwData.fanStatus[i] == -1) continue;
            printf("  - HW[%d]: fan: %d, power: %d, disk: %d, netName: %s, netStatus: %d",
                   i,
                   info.hwData.fanStatus[i],
                   info.hwData.powerStatus[i],
                   info.hwData.diskStatus[i],
                   info.hwData.netStatus[i].name,
                   info.hwData.netStatus[i].status);
        }
    }
}

void ServerInfoManager::printServerInfo() const {
    std::lock_guard<std::mutex> lock(Mutex);
    for(int index = 0;index < MAX_SERVER;++index) { 
        const SERVER_INFO& info = serverInfo[index];
        if(info.serverId == -1) continue;
        printf("[SERVER_INFO] index: %d, serverId: %d, asId: %d, asName: %s, chassisId: %d",
               index, info.serverId, info.asId, info.asName, info.chassisId);
        printf("  serverName: %s, serverType: %d, serverCode: %d, ip: %s, use: %d",
               info.serverName, info.serverType, info.serverCode, info.ip, info.use);
        printf("  status: (ha=%d, status=%d), block: %d, rebuild: %d",
               info.serverStatus.ha, info.serverStatus.status, info.block, info.rebuild);
		printf(" alarm crit[%d], major[%d], minor[%d], warr[%d]\n", info.critical_count, info.major_count, info.minor_count, info.warning_count);
    }
}

void ModuleInfoManager::printModuleInfo() const {
    std::lock_guard<std::mutex> lock(Mutex);
    for(int asi = 0;asi < MAX_AS_EMS;++asi) { 
        for(int moi = 0;moi < MAX_MODULE;++moi) {
            const MODULE_INFO& info = moduleInfo[asi][moi];
            if(info.moduleId == -1) continue;
            printf("[MODULE_INFO] asi: %d, moi: %d, moduleId: %d, serverCode: %d, serverId: %d",
                   asi, moi, info.moduleId, info.serverCode, info.serverId);
            printf("  asName: %s, moduleName: %s, serverName: %s, ha: %d",
                   info.asName, info.moduleName, info.serverName, info.ha);
        }
    }
}

void AsStatsInfoManager::printAsStats() const {
    std::lock_guard<std::mutex> lock(Mutex);
    for(int index = 0;index < MAX_AS_EMS;++index) { 
        const AS_STATS& info = asstatsInfo[index];
        if(info.cps == -1) continue;
        printf("[AS_STATS] index: %d, cps: %d, discardChannel: %d",
               index, info.cps, info.discardChannel);
        printf("  CpsSummery(min=%d, max=%d, avg=%d, peak=%s)",
               info.CpsSummery.min, info.CpsSummery.max, info.CpsSummery.avg, info.CpsSummery.peaktime);
    }
}

void PerformanceManager::printPerformance() const {
    std::lock_guard<std::mutex> lock(Mutex);
    for(int index = 0;index < MAX_SERVER;++index) { 
        const PERFORMANCE& info = performance[index];
        if(info.cpu == -1) continue;
        printf("[PERFORMANCE] index: %d ", index);
        printf("  CPU: %d, MEM: %d, NET: %d, DISK: %d, TEMP: %d, Session: %d, NTP: %d",
               info.cpu, info.memory, info.network, info.disk, info.temperature, info.dbSession, info.ntp);
        printf("  Channels: %d, ChannelRate: %d, SyncCount: %d",
               info.channel_count, info.channel_rate, info.sync_count);
    }
}

void ThresholdInfoManager::printThresholdInfo() const {
    std::lock_guard<std::mutex> lock(Mutex);
    const int* fields[] = { thresholdInfo.cpu, thresholdInfo.memory, thresholdInfo.network, thresholdInfo.disk,
                            thresholdInfo.channel, thresholdInfo.ntp, thresholdInfo.tablespace, thresholdInfo.temperature, thresholdInfo.session };
    const char* names[] = { "CPU", "MEMORY", "NETWORK", "DISK", "CHANNEL", "NTP", "TABLESPACE", "TEMPERATURE", "SESSION" };

    for (int i = 0; i < 9; ++i) {
        if(fields[i][0] == -1) continue;
        printf("[THRESHOLD_INFO]  %s: CRI[%d] MAJ[%d] MIN[%d] WAR[%d]", names[i], fields[i][0], fields[i][1], fields[i][2], fields[i][3]);
    }
}

void AlarmConfigManager::printAlarmConfig() const {
    std::lock_guard<std::mutex> lock(Mutex);
    for (int i = 0; i < MAX_ALARM; ++i) {
        const ALARM_CONFIG& config = alarmConfig[i];
        if (config.AlarmCode != -1) {
            printf("[ALARM_CONFIG] index: %d, Code: %d, Type: %d, Level: %d, Desc: %s, Repair: %s",
                   i, config.AlarmCode, config.AlarmType, config.AlarmLevel,
                   config.AlarmDesc, config.AlarmRepair);
        }
    }
}
