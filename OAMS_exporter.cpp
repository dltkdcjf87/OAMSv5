#include "OAMS_exporter.h"
#include "OAMS_common.h"

struct ServerArgs {
    CExporter* exporter;
    const char* ip;
    const char* port;
};
extern "C" {
    const char* SetMetric(const char* name, const char* labels, const char* value, const char* ip, const char* port);
    const char* DeleteMetricWrapper(const char* name, const char* labels, const char* ip, const char* port);
    const char* DeleteMetricByNameWrapper(const char* name, const char* ip, const char* port);
    const char* StartServer(const char* ip, const char* port);
    const char* checkServerHealth(const char* ip, const char* port);
}

CExporter::CExporter() {}
CExporter::~CExporter() {}

std::string LabelListToString(const LabelList* labelList) {
    std::string result;
    for (int i = 0; i < labelList->count; ++i) {
        result += labelList->labels[i].key;
        result += "=";
        result += labelList->labels[i].value;
        if (i < labelList->count - 1) result += ",";
    }
    return result;
}
template <typename T>
std::pair<const char*, std::string> MakeLabel(const char* key, T value) {
    return { key, std::to_string(value) };  // 숫자형일 때
}

// char* 혹은 const char*일 경우를 위한 오버로드 추가
inline std::pair<const char*, std::string> MakeLabel(const char* key, const char* value) {
    return { key, std::string(value) };
}

inline std::pair<const char*, std::string> MakeLabel(const char* key, char* value) {
    return { key, std::string(value) };
}

void FillCommonLabels(LabelList& labels, std::initializer_list<std::pair<const char*, std::string>> keyValues) {
    memset(&labels, 0x00, sizeof(LabelList));

    int i = 0;
    for (const auto& kv : keyValues) {
        if (i >= MAX_LABEL_COUNT) break;
        strncpy(labels.labels[i].key, kv.first, MAX_LABEL_KEY_LEN);
        strncpy(labels.labels[i].value, kv.second.c_str(), MAX_LABEL_VALUE_LEN);
        ++i;
    }
    labels.count = i;
}

void GenerateASLabels(LabelList& labelList) {
    AS_INFO as_metric;
    labelList.count = 0;
    for (int i = 0; i < MAX_AS_EMS; ++i) {
        asInfoManager.getASInfo(i, as_metric);

        if (as_metric.asId == -1) continue;

        char labelKey[MAX_LABEL_KEY_LEN];
        snprintf(labelKey, sizeof(labelKey), "AS%d", as_metric.asId);

        Label label;
        strncpy(label.key, labelKey, MAX_LABEL_KEY_LEN - 1);
        label.key[MAX_LABEL_KEY_LEN - 1] = '\0';

        strncpy(label.value, as_metric.asName, MAX_LABEL_VALUE_LEN - 1);
        label.value[MAX_LABEL_VALUE_LEN - 1] = '\0';

        if (labelList.count < MAX_AS_EMS) {
            labelList.labels[labelList.count++] = label;
        }
    }
}
void CExporter::PushAlarmMessage(int type, ALM_Q_MSG* msg) {
    ALM_Q_MSG* newMsg = new ALM_Q_MSG;
    std::lock_guard<std::mutex> lock(alarmQueueMutex);
    std::memcpy(newMsg, msg, sizeof(ALM_Q_MSG));

    AlarmMessage* alarm = new AlarmMessage();
    alarm->type = type;           // type 할당
    alarm->msg = newMsg;          // msg 할당

    alarmQueue.push(alarm);      // 큐에 AlarmMessage 구조체 삽입

}
void CExporter::PopAndProcessAlarmMessage(const char* ip, const char* port) {
    std::queue<AlarmMessage*> tempQueue;

    {
        std::lock_guard<std::mutex> lock(alarmQueueMutex);
        std::swap(tempQueue, alarmQueue);  // alarmQueue의 내용을 tempQueue로 이동
    }

    while (!tempQueue.empty()) {
        AlarmMessage* alarm = tempQueue.front();
        tempQueue.pop();

        if (alarm->msg) {
            GetAlarmInfo(ip, port, *alarm);

            delete alarm->msg;  // msg 메모리 직접 해제
            alarm->msg = nullptr;
        }

        delete alarm;  // AlarmMessage 구조체 메모리 해제
    }
}

bool CExporter::startPrometheusServer(const char* ip, const char* port) {
    const char* errorMessage = StartServer(ip, port);
    if (strcmp(errorMessage, "OK")==0) {
        Log.printf(LOG_INF, "[CExporter][startPrometheusServer] SUCCESS ip:%s  port:%s ", ip, port);
        return true;
    } else {
        Log.printf(LOG_ERR, "[CExporter][startPrometheusServer] Failed ip:%s port:%s errorMessage:%s ", ip,  port, errorMessage);
        return false;
    }
}

bool CExporter::SetMetricValue(const char* name, const LabelList* labels, double value, const char* ip, const char* port) {
    std::string labelStr = LabelListToString(labels);
    char valueStr[64];
    snprintf(valueStr, sizeof(valueStr), "%.2f", value);

    const char* errorMessage = SetMetric(name, labelStr.c_str(), valueStr, ip, port);

    if (errorMessage != nullptr && strlen(errorMessage) > 0) {
        Log.printf(LOG_ERR, "[CExporter][SetMetricValue] %s{%s}%s Fail : %s ", name, labelStr.c_str(), valueStr, errorMessage);
        return false;
    } else { 
        Log.printf(LOG_LV3, "[CExporter][SetMetricValue] %s{%s}%s Succ", name, labelStr.c_str(), valueStr);
        return true;
    }
}

bool CExporter::RemoveMetric(const char* name, const char* labelStr, const char* ip, const char* port) {
    const char* errorMessage = DeleteMetricWrapper(name, labelStr, ip, port);

    if (errorMessage != nullptr && strlen(errorMessage) > 0) {
        Log.printf(LOG_LV3, "[CExporter][RemoveMetric] %s{%s} Fail : %s ", name, labelStr, errorMessage);
        return false;
    } else { 
        Log.printf(LOG_LV3, "[CExporter][RemoveMetric] %s{%s} Succ", name, labelStr, errorMessage);
        return true;
    }
}
bool CExporter::RemoveMetricName(const char* name, const char* ip, const char* port) {
    const char* errorMessage = DeleteMetricByNameWrapper(name, ip, port);

    if (errorMessage != nullptr && strlen(errorMessage) > 0) {
        Log.printf(LOG_LV3, "[CExporter][RemoveMetricName] %s Fail : %s ", name, errorMessage);
        return false;
    } else {
        Log.printf(LOG_LV3, "[CExporter][RemoveMetricName] %s Succ", name, errorMessage);
        return true;
    }
}
void CExporter::RemoveMetricByPartialLabel(const char* name, const char* key, const char* value, const char* ip, const char* port) {
    std::string target = std::string(key) + "=" + value;

    std::lock_guard<std::mutex> lock(labelListMutex);

    Log.printf(LOG_LV2, "[CExporter][RemoveMetricByPartialLabel] name: %s key: %s value: %s", name, key, value);
    for (auto it = labelListOnly.begin(); it != labelListOnly.end(); ) {
        const std::string& labelStr = *it;

        if (labelStr.find(target) != std::string::npos) {
            if (RemoveMetric(name, labelStr.c_str(), ip, port)) {
                Log.printf(LOG_LV2, "[CExporter][RemoveMetricByPartialLabel] Deleted : %s (matched: %s)", labelStr.c_str(), target.c_str());
            }
            it = labelListOnly.erase(it);
        } else {
            ++it;
        }
    }
}

void CExporter::monitorServerHealth(const char* ip, const char* port) {
	sleep(5);
    while (true) {
        sleep(5);
        const char* errorMessage = checkServerHealth(ip, port);

        if (errorMessage != nullptr && strlen(errorMessage) > 0) {
            Log.printf(LOG_ERR, "[CExporter][monitorServerHealth] Fail ip:%s port:%s error : %s ", ip, port, errorMessage);

            if (!startPrometheusServer(ip, port)) {
                Log.printf(LOG_ERR, "[CExporter][monitorServerHealth] Restart Fail ip:%s  port:%s ", ip, port);
            }
        } else { 
            Log.printf(LOG_LV3, "[CExporter][monitorServerHealth] OK ip:%s port:%s ", ip, port);
        }
    }
}
void CExporter::startAndMonitorServer(const char* ip, const char* port) {
    if (!startPrometheusServer(ip, port)) {
        Log.printf(LOG_ERR, "[CExporter][startAndMonitorServer] Failed to start ip:%s port:%s ", ip,  port);
        //shut_down(10);
    }
    monitorServerHealth(ip, port);
}
void CExporter::AddLabel(const std::string& labelStr) {
    std::lock_guard<std::mutex> lock(labelQueueMutex);
    auto now = std::chrono::system_clock::now();
    labelQueue.emplace_back(now, labelStr);

    // 시간 변환
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    char timeStr[32];
    std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", std::localtime(&now_c));

    Log.printf(LOG_LV1, "[CExporter][AddLabel] time:%s labelStr:%s", timeStr, labelStr.c_str());
}
void CExporter::AddLabelOnly(const std::string& labelStr) {
    std::lock_guard<std::mutex> lock(labelListMutex);
    labelListOnly.emplace_back(labelStr);

    Log.printf(LOG_LV1, "[CExporter][AddLabelOnly] labelStr:%s", labelStr.c_str());
}

void CExporter::DelLabel(const std::string& labelStr) {
    std::lock_guard<std::mutex> lock(labelQueueMutex);

    for (auto it = labelQueue.begin(); it != labelQueue.end(); ) {
        if (it->second == labelStr) {
            Log.printf(LOG_LV3, "[CExporter][DelLabel] labelStr:%s ", labelStr.c_str());
            it = labelQueue.erase(it); 
        } else {
            ++it; 
        }
    }
}
void CExporter::DelLabelOnly(const std::string& labelStr) {
    std::lock_guard<std::mutex> lock(labelListMutex);

    for (auto it = labelListOnly.begin(); it != labelListOnly.end(); ) {
        if (*it == labelStr) {
            Log.printf(LOG_LV3, "[CExporter][DelLabelOnly] labelStr:%s ", labelStr.c_str());
            it = labelListOnly.erase(it);
        } else {
            ++it;
        }
    }
}


void CExporter::GetStateServerInfo(const char* ip, const char* port) {
    int status = 1;
    AS_INFO as_metric;
    SERVER_INFO server_metric;
    LabelList labels;

    Log.printf(LOG_LV2, "[CExporter][GetStateServerInfo] START ");

    for (int i = 0; i < MAX_SERVER; ++i) {
        memset(&server_metric, 0x00, sizeof(SERVER_INFO));
        serverInfoManager.getServerInfo(i, server_metric);
        asInfoManager.getASInfo(server_metric.asId, as_metric);

        if (server_metric.asId == -1) continue; 

        memset(&labels, 0x00, sizeof(LabelList));
        FillCommonLabels(labels, {
            MakeLabel("AS", server_metric.asName),
            MakeLabel("SERVER", server_metric.serverName)
        });

        if (server_metric.serverStatus.ha == -1) {
            continue; 
        }
     
        if(as_metric.block == 1) status = 8; //AS BLOCK
        else if(server_metric.block == 2) status = 10; // SES M-BLOCK 
        else if(server_metric.block == 1) status = 9;  // SES P-BLOCK 
        else if(server_metric.block == 3) status = 11; // SES D-BLOCK 
        else if(server_metric.critical_count > 0) status = 7;
        else if(server_metric.major_count > 0) status = 6;
        else if(server_metric.minor_count > 0) status = 5;
        else if(server_metric.warning_count > 0) status = 4;
        else status = server_metric.serverStatus.ha;

        Log.printf(LOG_LV2, "[CExporter][GetStateServerInfo] server[%s] status[%d] ", server_metric.serverName, status);
        SetMetricValue("get_state_server_info", &labels, status, ip, port);
    }
    Log.printf(LOG_LV2, "[CExporter][GetStateServerInfo] END ");
}
void CExporter::GetHaServerInfo(const char* ip, const char* port) {
    int status = 1;
    SERVER_INFO server_metric;
    LabelList labels;
    
    Log.printf(LOG_LV2, "[CExporter][GetHaServerInfo] START ");
    
    for (int i = 0; i < MAX_SERVER; ++i) {
        memset(&server_metric, 0x00, sizeof(SERVER_INFO));
        serverInfoManager.getServerInfo(i, server_metric);
        
        if (server_metric.asId == -1 || server_metric.serverStatus.ha == -1) {
            continue;
        } 
        
        memset(&labels, 0x00, sizeof(LabelList));
        
        FillCommonLabels(labels, {
            MakeLabel("AS", server_metric.asName),
            MakeLabel("SERVER", server_metric.serverName)
        }); 
        status = server_metric.serverStatus.ha;
        SetMetricValue("get_ha_server_info", &labels, status, ip, port);
    }   
    
    Log.printf(LOG_LV2, "[CExporter][GetHaServerInfo] END ");
} 
void CExporter::GetStateModuleInfo(const char* ip, const char* port) {
    MODULE_INFO module_metric;
    LabelList labels;
    int status = 1;
    int ha = 0;
    char baseName[20] = {0};

    Log.printf(LOG_LV2, "[CExporter][GetStateModuleInfo] START ");

    for (int i = 0; i < MAX_AS_EMS; ++i) {
        for (int j = 0; j < MAX_MODULE; ++j) {
            memset(&module_metric, 0x00, sizeof(MODULE_INFO));
            moduleInfoManager.getModuleInfo(i, j, module_metric);
       
            if(module_metric.moduleId == -1 || module_metric.ha == -1) continue; 

            Log.printf(LOG_LV1, "[CExporter][GetStateModuleInfo] moduleId[%d] status[%d] critical[%d] major[%d] minor[%d] warning[%d] ha[%d] ",
                      module_metric.moduleId, module_metric.status, module_metric.critical_count, module_metric.major_count, module_metric.minor_count,
                      module_metric.warning_count, module_metric.ha);
            if(module_metric.critical_count > 0) status = 7;
            else if(module_metric.major_count > 0) status = 6;
            else if(module_metric.minor_count > 0) status = 5;
            else if(module_metric.warning_count > 0) status = 4;
            else status = module_metric.ha;


            extractModuleBaseName(module_metric.moduleName, baseName, sizeof(baseName));
            memset(&labels, 0x00, sizeof(LabelList));
            FillCommonLabels(labels, {
                {"AS", module_metric.asName},
                {"SERVER", module_metric.serverName},
                {"PROCESS", baseName}
            });

            SetMetricValue("get_state_module_info", &labels, status, ip, port);
        }
    }

    Log.printf(LOG_LV2, "[CExporter][GetStateModuleInfo] END ");
}

void CExporter::GetAsStatInfo(const char* ip, const char* port) {
    AS_INFO as_metric;
    AS_STATS asstats_metric;
    LabelList incall_labels;
    LabelList discard_labels;
    LabelList use_labels;
    LabelList total_labels;

    Log.printf(LOG_LV2, "[CExporter][GetAsStatInfo] START ");

    for (int i = 0; i < MAX_AS_EMS; ++i) {
        memset(&as_metric, 0x00, sizeof(AS_INFO));
        memset(&asstats_metric, 0x00, sizeof(AS_STATS));
        asInfoManager.getASInfo(i, as_metric);
        asstatsInfoManager.getAsStats(i, asstats_metric);

        if(asstats_metric.cps == -1) continue; 

        memset(&incall_labels,   0x00, sizeof(LabelList));
        memset(&discard_labels, 0x00, sizeof(LabelList));
        memset(&use_labels, 0x00, sizeof(LabelList));
        memset(&total_labels, 0x00, sizeof(LabelList));

        //get_statistic_graph_info incall
        FillCommonLabels(incall_labels, {
            {"AS", as_metric.asName},
            {"TYPE", "INCALL"}
        });
        
        SetMetricValue("get_statistic_info", &incall_labels, asstats_metric.cps, ip, port);

        //get_statistic_graph_info discard_call
        FillCommonLabels(discard_labels, {
            {"AS", as_metric.asName},
            {"TYPE", "DISCARDCALL"}
        }); 
        
        SetMetricValue("get_statistic_info", &discard_labels, asstats_metric.discardChannel, ip, port);

        //get_statistic_graph_info use_ch
        FillCommonLabels(use_labels, {
            {"AS", as_metric.asName},
            {"TYPE", "USECALL"}
        });

        SetMetricValue("get_statistic_info", &use_labels, asstats_metric.sumBusy, ip, port);

        //get_statistic_graph_info tot_ch
        FillCommonLabels(total_labels, {
            {"AS", as_metric.asName},
            {"TYPE", "TOTALCHANNEL"}
        });

        SetMetricValue("get_statistic_info", &total_labels, asstats_metric.sumTotal, ip, port);
    }
    Log.printf(LOG_LV2, "[CExporter][GetAsStatInfo] END ");
}
void CExporter::GetHadwareInfo(const char* ip, const char* port) {
    PERFORMANCE performance_metric;
    SERVER_INFO server_metric;
    LabelList cpu_labels;
    LabelList mem_labels;
    LabelList disk_labels;
    LabelList db_labels;
    LabelList table_labels;
    int check_tablespace = 0, total_tablespace = 0;
    char dbtype[5];

    Log.printf(LOG_LV2, "[CExporter][GetHadwareInfo] START ");

    for (int i = 0; i < MAX_SERVER; ++i) {
        memset(&performance_metric, 0x00, sizeof(PERFORMANCE));
        performanceManager.getPerformance(i, performance_metric);

        serverInfoManager.getServerInfo(i, server_metric);
        if(strcmp(server_metric.asName, "")==0) continue;

        memset(&cpu_labels,  0x00, sizeof(LabelList));
        memset(&mem_labels,  0x00, sizeof(LabelList));
        memset(&disk_labels, 0x00, sizeof(LabelList));
        memset(&db_labels, 0x00, sizeof(LabelList));
        memset(&table_labels, 0x00, sizeof(LabelList));

        if(performance_metric.cpu != -1) {
            //get_hardware_info cpu
            FillCommonLabels(cpu_labels, {
                {"AS", server_metric.asName},
                {"SERVER", server_metric.serverName},
                {"TYPE", "CPU"}
            });

            SetMetricValue("get_hardware_info", &cpu_labels, performance_metric.cpu, ip, port);
        }
        if(performance_metric.memory != -1) {
            //get_hardware_info mem
            FillCommonLabels(mem_labels, {
                {"AS", server_metric.asName},
                {"SERVER", server_metric.serverName},
                {"TYPE", "MEM"}
            });

            SetMetricValue("get_hardware_info", &mem_labels, performance_metric.memory, ip, port);
        }
        if(performance_metric.disk != -1) {
            //get_hardware_info disk
            FillCommonLabels(disk_labels, {
                {"AS", server_metric.asName},
                {"SERVER", server_metric.serverName},
                {"TYPE", "DISK"}
            });

            SetMetricValue("get_hardware_info", &disk_labels, performance_metric.disk, ip, port);
        }
        if(performance_metric.dbSession != -1) {
            //get_hardware_info dbsession
            FillCommonLabels(db_labels, {
                {"AS", server_metric.asName}
            });


            SetMetricValue("get_db_session_info", &db_labels, performance_metric.dbSession, ip, port);
        }

        //get_hardware_info tablespace
        strncpy(dbtype, server_metric.serverName, 3);
        for(int j=0;j<MAX_TS;++j){ 
            if(strcmp(performance_metric.tablespace[j].name, "")==0) continue;
            if(performance_metric.tablespace[j].use == -1) continue;
            if(performance_metric.tablespace[j].name[0] == '\0') continue;

            FillCommonLabels(table_labels, {
                {"TYPE", dbtype},
                {"DBNAME", performance_metric.tablespace[j].name}
            });
            total_tablespace += performance_metric.tablespace[j].use;

            SetMetricValue("get_db_tablespace_info", &table_labels, performance_metric.tablespace[j].use, ip, port);
            check_tablespace = 1;

        }
        if(check_tablespace == 1) { 
            FillCommonLabels(table_labels, {
                {"TYPE", dbtype},
                {"DBNAME", "TOTAL"}
            });

            SetMetricValue("get_db_tablespace_info", &table_labels, total_tablespace, ip, port);
        }
        check_tablespace = 0;
        total_tablespace = 0;
    }
    Log.printf(LOG_LV2, "[CExporter][GetHadwareInfo] END ");
}
void CExporter::GetChannelUsage(const char* ip, const char* port) {
    PERFORMANCE performance_metric;
    SERVER_INFO server_metric;
    LabelList sync_labels;
    LabelList normal_labels;
    LabelList normalrate_labels;

    Log.printf(LOG_LV2, "[CExporter][GetChannelUsage] START ");

    for (int i = 0; i < MAX_SERVER; ++i) {
        memset(&performance_metric, 0x00, sizeof(PERFORMANCE));
        memset(&sync_labels, 0x00, sizeof(LabelList));
        memset(&normal_labels, 0x00, sizeof(LabelList));
        memset(&normalrate_labels, 0x00, sizeof(LabelList));

        performanceManager.getPerformance(i, performance_metric);
        serverInfoManager.getServerInfo(i, server_metric);

        if(strcmp(server_metric.asName, "")==0 || strcmp(server_metric.serverName, "")==0) continue; 

        if(performance_metric.channel_count == -1 || performance_metric.channel_rate == -1) continue; 

        //get_channel_usage sync_ch
        if(performance_metric.sync_count != -1) { 
            FillCommonLabels(sync_labels, {
                {"AS", server_metric.asName},
                {"SERVER", server_metric.serverName},
                {"TYPE", "SYNC"}
            });

            SetMetricValue("get_channel_usage", &sync_labels, performance_metric.sync_count, ip, port);
        } 
        //get_channel_usage busy_ch
        FillCommonLabels(normal_labels, {
            {"AS", server_metric.asName},
            {"SERVER", server_metric.serverName},
            {"TYPE", "NORMAL"}
        });

        SetMetricValue("get_channel_usage", &normal_labels, performance_metric.channel_count, ip, port);

        //get_channel_usage channel_rate
        FillCommonLabels(normalrate_labels, {
            {"AS", server_metric.asName},
            {"SERVER", server_metric.serverName},
            {"TYPE", "NORMALRATE"}
        });

        SetMetricValue("get_channel_usage", &normalrate_labels, performance_metric.channel_rate, ip, port);
    }

    Log.printf(LOG_LV2, "[CExporter][GetChannelUsage] END ");
}
void CExporter::GetConfigAsInfo(const char* ip, const char* port) {
    Label label;
    LabelList labels;
    AS_INFO as_metric;
    char labelKey[MAX_LABEL_KEY_LEN];

    Log.printf(LOG_LV2, "[CExporter][GetConfigAsInfo] START ");

    memset(&labels, 0x00, sizeof(LabelList));

    GenerateASLabels(labels);
    labels.count = 0;
    for (int i = 0; i < MAX_AS_EMS; ++i) {
        memset(&label, 0x00, sizeof(Label));
        memset(labelKey, 0x00, sizeof(labelKey));
        asInfoManager.getASInfo(i, as_metric);

        if (as_metric.asId == -1) {
            continue;
        }
        if (strstr(as_metric.asName, "EMS") != nullptr) {
            continue;
        }

        snprintf(labelKey, sizeof(labelKey), "AS%d", as_metric.asId);

        Label label;
        strncpy(label.key, labelKey, MAX_LABEL_KEY_LEN - 1);
        label.key[MAX_LABEL_KEY_LEN - 1] = '\0';

        strncpy(label.value, as_metric.asName, MAX_LABEL_VALUE_LEN - 1);
        label.value[MAX_LABEL_VALUE_LEN - 1] = '\0';

        if (labels.count < MAX_AS_EMS) {
            labels.labels[labels.count++] = label;
        }
    }

    SetMetricValue("get_config_as_info", &labels, 1, ip, port);

    Log.printf(LOG_LV2, "[CExporter][GetConfigAsInfo] END ");
}
void CExporter::GetConfigAsListInfo(const char* ip, const char* port) {
    AS_INFO as_metric;
    LabelList labels;

    Log.printf(LOG_LV2, "[CExporter][GetConfigAsListInfo] START ");

    for (int i = 0; i < MAX_AS_EMS; ++i) {

        memset(&as_metric, 0x00, sizeof(AS_INFO));
        memset(&labels, 0x00, sizeof(LabelList));
        asInfoManager.getASInfo(i, as_metric);

        if (as_metric.asId == -1) continue;
        if (strstr(as_metric.asName, "EMS") != nullptr) continue;

        //get_config_as_info
        FillCommonLabels(labels, {
            MakeLabel("AS", as_metric.asName)
        });

        SetMetricValue("get_config_as_list_info", &labels, 1, ip, port);
    }

    Log.printf(LOG_LV2, "[CExporter][GetConfigAsListInfo] END ");
}
void CExporter::GetOverloadInfo(const char* ip, const char* port) {
    AS_INFO as_metric;
    LabelList labels;
    int overload;

    Log.printf(LOG_LV2, "[CExporter][GetOverloadInfo] START ");

    for (int i = 0; i < MAX_AS_EMS; ++i) {

        memset(&as_metric, 0x00, sizeof(AS_INFO));
        asInfoManager.getASInfo(i, as_metric);

        if (as_metric.asId == -1) continue;
        if (strstr(as_metric.asName, "EMS") != nullptr) continue;

        memset(&labels, 0x00, sizeof(LabelList));

        FillCommonLabels(labels, {
            MakeLabel("AS", as_metric.asName)
        });
        if(as_metric.connected == 1) overload = 9;
        else if(as_metric.block == 4) overload = 8;
        else if(as_metric.overload == 0) overload = 1;
        else if(as_metric.overload == 1) overload = 4;
        else if(as_metric.overload == 2) overload = 5;
        else if(as_metric.overload == 3) overload = 6;
        else if(as_metric.overload == 4) overload = 7;
        Log.printf(LOG_LV2, "[CExporter][GetOverloadInfo] asindex[%d] overload[%d] ", i, overload);

        SetMetricValue("get_overload_info", &labels, overload, ip, port);

    }

    Log.printf(LOG_LV2, "[CExporter][GetOverloadInfo] END ");
}

void CExporter::GetConfigThresholdInfo(const char* ip, const char* port) {
    THRESHOLD_INFO threshold_metric;
    LabelList cpulabels[4];
    LabelList memlabels[4];
    LabelList networklabels[4];
    LabelList disklabels[4];
    LabelList channellabels[4];
    LabelList ntplabels[4];
    LabelList tablespacelabels[4];
    LabelList temperaturelabels[4];
    LabelList sessionlabels[4];
    char szLevel[10 + 1];

    Log.printf(LOG_LV2, "[CExporter][GetConfigThresholdInfo] START ");

    memset(&threshold_metric, 0x00, sizeof(THRESHOLD_INFO));
    memset(szLevel, 0x00, sizeof(szLevel));
    thresholdInfoManager.getThresHoldInfo(threshold_metric);

    for(int i=0;i<4;++i) {
        memset(&cpulabels[i], 0x00, sizeof(LabelList));
        memset(&memlabels[i], 0x00, sizeof(LabelList));
        memset(&networklabels[i], 0x00, sizeof(LabelList));
        memset(&disklabels[i], 0x00, sizeof(LabelList));
        memset(&channellabels[i], 0x00, sizeof(LabelList));
        memset(&ntplabels[i], 0x00, sizeof(LabelList));
        memset(&tablespacelabels[i], 0x00, sizeof(LabelList));
        memset(&temperaturelabels[i], 0x00, sizeof(LabelList));
        memset(&sessionlabels[i], 0x00, sizeof(LabelList));

        if(i == 0) strcpy(szLevel, "CRITICAL");
        if(i == 1) strcpy(szLevel, "MAJOR");
        if(i == 2) strcpy(szLevel, "MINOR");
        if(i == 3) strcpy(szLevel, "WARNING");

        if(threshold_metric.cpu[i] == -1)  continue;
        //cpu 
        FillCommonLabels(cpulabels[i], {
            MakeLabel("TYPE", "CPU"),
            MakeLabel("LEVEL", szLevel)
        });


        SetMetricValue("get_config_threshold_info", &cpulabels[i], threshold_metric.cpu[i], ip, port);

        //memory
        FillCommonLabels(memlabels[i], {
            MakeLabel("TYPE", "MEM"),
            MakeLabel("LEVEL", szLevel)
        });
        
        SetMetricValue("get_config_threshold_info", &memlabels[i], threshold_metric.memory[i], ip, port);

        //network
        FillCommonLabels(networklabels[i], {
            MakeLabel("TYPE", "NETWORK"),
            MakeLabel("LEVEL", szLevel)
        });

        SetMetricValue("get_config_threshold_info", &networklabels[i], threshold_metric.network[i], ip, port);

        //disk
        FillCommonLabels(disklabels[i], {
            MakeLabel("TYPE", "DISK"),
            MakeLabel("LEVEL", szLevel)
        });

        SetMetricValue("get_config_threshold_info", &disklabels[i], threshold_metric.disk[i], ip, port);

        //channel
        FillCommonLabels(channellabels[i], {
            MakeLabel("TYPE", "CHANNEL"),
            MakeLabel("LEVEL", szLevel)
        });

        SetMetricValue("get_config_threshold_info", &channellabels[i], threshold_metric.channel[i], ip, port);

        //ntp
        FillCommonLabels(ntplabels[i], {
            {"TYPE", "NTP"},
            {"LEVEL", szLevel}
        });

        SetMetricValue("get_config_threshold_info", &ntplabels[i], threshold_metric.ntp[i], ip, port);

        //tablespace
        FillCommonLabels(tablespacelabels[i], {
            MakeLabel("TYPE", "TABLESPACE"),
            MakeLabel("LEVEL", szLevel)
        });

        SetMetricValue("get_config_threshold_info", &tablespacelabels[i], threshold_metric.tablespace[i], ip, port);

        //temperature
        FillCommonLabels(temperaturelabels[i], {
            MakeLabel("TYPE", "TEMPERATURE"),
            MakeLabel("LEVEL", szLevel)
        });

        SetMetricValue("get_config_threshold_info", &temperaturelabels[i], threshold_metric.temperature[i], ip, port);

        //session
        FillCommonLabels(sessionlabels[i], {
            MakeLabel("TYPE", "SESSION"),
            MakeLabel("LEVEL", szLevel)
        });

        SetMetricValue("get_config_threshold_info", &sessionlabels[i], threshold_metric.session[i], ip, port);

    }

    Log.printf(LOG_LV2, "[CExporter][GetConfigThresholdInfo] END ");
}
void CExporter::GetExternServerListInfo(const char* ip, const char* port) {
    int status;
    char external_type[10]={0};
    char szDesc[32]={0};
    std::string szCMSName = serverInfoManager.FindServerNameByHa("CMS", 2);
    AS_INFO as_metric;
    SSW_CONFIG_INFO ssw_metric;
    MS_CONFIG_INFO ms_metric;
    EXTERNAL_IF_INFO extern_metric;
    CDB_STATUS_INFO cdb_metric;
    SERVER_INFO serverData;

    LabelList ssw_labels;
    LabelList ms_labels;
    LabelList extern_labels;
    LabelList cdb_labels;

    Log.printf(LOG_LV2, "[CExporter][GetExternServerListInfo] START ");

    RemoveMetricName("get_extern_server_list_info", "", port);

    for (int i = 0; i < MAX_SSW; ++i) {
        memset(&as_metric, 0x00, sizeof(AS_INFO));

        sswconfigInfoManager.getSswConfigInfo(i, ssw_metric);
        asInfoManager.getASInfo(ssw_metric.asId, as_metric);

        if (strcmp(ssw_metric.systemName, "")==0) continue;
        if (ssw_metric.systemName[0] == '\0') continue;

        memset(&ssw_labels, 0x00, sizeof(LabelList));
        memset(external_type, 0x00, sizeof(external_type));
        g_cfg.GetConfigString("EXPORTER.SSW_TYPE", external_type);


        FillCommonLabels(ssw_labels, {
            MakeLabel("TYPE", external_type),
            MakeLabel("AS", as_metric.asName),
            MakeLabel("SERVER", szCMSName.c_str()),
            MakeLabel("IP", ssw_metric.ip),
            MakeLabel("PORT", ssw_metric.port),
            MakeLabel("DESC", ssw_metric.systemName)
        });

        if(ssw_metric.status == 1) status = 2;
        else status = 0;

        SetMetricValue("get_extern_server_list_info", &ssw_labels, status, ip, port);
    }

    for (int i = 0; i < MAX_MS; ++i) {
        memset(&ms_metric, 0x00, sizeof(MS_CONFIG_INFO));

        msconfigInfoManager.getMsConfigInfo(i, ms_metric);
        asInfoManager.getASInfo(ms_metric.asId, as_metric);

        if (ms_metric.asId == -1 || strcmp(ms_metric.systemName, "")==0) continue;
        if(ms_metric.systemName[0] == '\0') continue;

        memset(&ms_labels, 0x00, sizeof(LabelList));
        memset(external_type, 0x00, sizeof(external_type));
        g_cfg.GetConfigString("EXPORTER.MS_TYPE", external_type);

        FillCommonLabels(ms_labels, {
            MakeLabel("TYPE", external_type),
            MakeLabel("AS", as_metric.asName),
            MakeLabel("SERVER", szCMSName.c_str()),
            MakeLabel("IP", ms_metric.ip),
            MakeLabel("PORT", ms_metric.port),
            MakeLabel("DESC", ms_metric.systemName)
        });

        if(ms_metric.block_flag > 0) status = 8;
        else if(ms_metric.state_flag == 1 && ms_metric.alive_flag == 1) status = 2;
        else if(ms_metric.state_flag == 0 && ms_metric.alive_flag == 1) status = 4;
        else status = 0;

        SetMetricValue("get_extern_server_list_info", &ms_labels, status, ip, port);
    }
    for (int i = 0; i < MAX_EXTERNAL; ++i) {
        memset(&extern_labels, 0x00, sizeof(EXTERNAL_IF_INFO));

        externalIfInfoManager.getExternalIfInfo(i, extern_metric);
        asInfoManager.getASInfo(extern_metric.asId, as_metric);
        serverInfoManager.getServerInfo(extern_metric.ConnSvrID, serverData);

        if(extern_metric.asId == -1 || strcmp(extern_metric.systemName, "")==0) continue;
        if(extern_metric.systemName[0] == '\0') continue;

        memset(&extern_labels, 0x00, sizeof(LabelList));

        FillCommonLabels(extern_labels, {
            MakeLabel("TYPE", extern_metric.system),
            MakeLabel("AS", as_metric.asName),
            MakeLabel("SERVER", serverData.serverName),
            MakeLabel("IP", extern_metric.ip),
            MakeLabel("PORT", extern_metric.port),
            MakeLabel("DESC", extern_metric.systemName)
        });

        if(extern_metric.status == 1) status = 2;
        else status = 0;

        SetMetricValue("get_extern_server_list_info", &extern_labels, status, ip, port);
    }
    for (int i = 0; i < MAX_CDB; ++i) {
        memset(&cdb_labels, 0x00, sizeof(CDB_STATUS_INFO));

        cdbstatusInfoManager.getCdbStatusInfo(i, cdb_metric);
        asInfoManager.getASInfo(cdb_metric.asId, as_metric);
        serverInfoManager.getServerInfo(cdb_metric.serverID, serverData);

        if (cdb_metric.asId == -1) continue;

        memset(&cdb_labels, 0x00, sizeof(LabelList));
        memset(external_type, 0x00, sizeof(external_type));
        g_cfg.GetConfigString("EXPORTER.CDB_TYPE", external_type);
        sprintf(szDesc, "%s %d", cdb_metric.systemName, cdb_metric.groupSeq);

        FillCommonLabels(cdb_labels, {
            MakeLabel("TYPE", external_type),
            MakeLabel("AS", as_metric.asName),
            MakeLabel("SERVER", serverData.serverName),
            MakeLabel("IP", cdb_metric.ip),
            MakeLabel("PORT", cdb_metric.port),
            MakeLabel("DESC", szDesc)
        });

        if(cdb_metric.status == 1) status = 2;
        else status = 0;

        SetMetricValue("get_extern_server_list_info", &cdb_labels, status, ip, port);
    }

    Log.printf(LOG_LV2, "[CExporter][GetExternServerListInfo] END ");
}
void CExporter::GetServiceGraphInfo(const char* ip, const char* port) {
    AS_INFO as_metric;
    AS_STATS asstats_metric;
    LabelList totalcall_labels;
    LabelList totalcomplete_labels;;
    LabelList incall_labels;
    LabelList complete_labels;

    Log.printf(LOG_LV2, "[CExporter][GetServiceGraphInfo] START ");

    for (int i = 0; i < MAX_AS_EMS; ++i) {
        memset(&as_metric, 0x00, sizeof(AS_INFO));
        memset(&asstats_metric, 0x00, sizeof(AS_STATS));
        memset(&totalcall_labels, 0x00, sizeof(LabelList));
        memset(&totalcomplete_labels, 0x00, sizeof(LabelList));

        asInfoManager.getASInfo(i, as_metric);
        asstatsInfoManager.getAsStats(i, asstats_metric);

        if(asstats_metric.cps==-1) continue;

        //get_service_graph_info totalcall
        FillCommonLabels(totalcall_labels, {
            MakeLabel("AS", as_metric.asName),
            MakeLabel("TYPE", "TOTALCALL"),
            MakeLabel("SERVICE", "TOTAL")
        });
        SetMetricValue("get_service_graph_info", &totalcall_labels, asstats_metric.sum_usage, ip, port);

        //get_service_graph_info totalcomplete
        FillCommonLabels(totalcomplete_labels, {
            MakeLabel("AS", as_metric.asName),
            MakeLabel("TYPE", "TOTALCOMPLETE"),
            MakeLabel("SERVICE", "TOTAL")
        }); 
        SetMetricValue("get_service_graph_info", &totalcomplete_labels, asstats_metric.sum_success, ip, port);

        for(int j = 0; j <MAX_SVC_USE; ++j) {
            memset(&incall_labels, 0x00, sizeof(LabelList));
            memset(&complete_labels, 0x00, sizeof(LabelList));

            if(strcmp(asstats_metric.serviceStat[j].service_key, "")==0) continue;
            if(asstats_metric.serviceStat[j].service_key[0] == '\0') continue;

            //get_service_graph_info incall
            FillCommonLabels(incall_labels, {
                MakeLabel("AS", as_metric.asName),
                MakeLabel("TYPE", "INCALL"),
                MakeLabel("SERVICE", asstats_metric.serviceStat[j].service_key)
            });

            SetMetricValue("get_service_graph_info", &incall_labels, asstats_metric.serviceStat[j].use_ch, ip, port);

            //get_service_graph_info total_success
            FillCommonLabels(complete_labels, {
                MakeLabel("AS", as_metric.asName),
                MakeLabel("TYPE", "COMPLETE"),
                MakeLabel("SERVICE", asstats_metric.serviceStat[j].service_key)
            });

            SetMetricValue("get_service_graph_info", &complete_labels, asstats_metric.serviceStat[j].total_success, ip, port);

        }
    }

    Log.printf(LOG_LV2, "[CExporter][GetServiceGraphInfo] END ");
}
void CExporter::GetExternServerInfo(const char* ip, const char* port) {
    AS_INFO as_metric;
    SSW_CONFIG_INFO ssw_metric;
    MS_CONFIG_INFO ms_metric;
    EXTERNAL_IF_INFO extern_metric;
    CDB_STATUS_INFO cdb_metric;
    LabelList labels;
    int sswTotCnt=0, sswAliveCnt=0, sswErrCnt=0, sswThresholdCnt=0;
    int msTotCnt=0, msAliveCnt=0, msErrCnt=0, msThresholdCnt=0;
    int cdbTotCnt=0, cdbAliveCnt=0, cdbErrCnt=0, cdbThresholdCnt=0;
    char external_type[10]={0};
    std::string szCMSName = serverInfoManager.FindServerNameByHa("CMS", 2);
    struct ServerStatusCounter {
        int total = 0;
        int alive = 0;
        int err = 0;
        int threshold = 0;
    };
    std::map<std::string, ServerStatusCounter> ExternCountMap;
    
    Log.printf(LOG_LV2, "[CExporter][GetExternServerInfo] START ");

    memset(&as_metric, 0x00, sizeof(AS_INFO));
    asInfoManager.getASInfo(ssw_metric.asId, as_metric);

    for (int i = 0; i < MAX_SSW; ++i) {
        sswconfigInfoManager.getSswConfigInfo(i, ssw_metric);

        if (ssw_metric.asId == -1) continue; 
        if (ssw_metric.status == 0 ) sswErrCnt++;
        sswTotCnt++; 
        sswAliveCnt = sswTotCnt - sswErrCnt;
        sswThresholdCnt = sswTotCnt - 1;
    }

    memset(external_type, 0x00, sizeof(external_type));
    g_cfg.GetConfigString("EXPORTER.SSW_TYPE", external_type);

    memset(&labels, 0x00, sizeof(LabelList));
    FillCommonLabels(labels, {
        MakeLabel("TYPE", external_type),
        MakeLabel("COUNT_TYPE", "TOTAL"),
        MakeLabel("AS", as_metric.asName)
    });

    SetMetricValue("get_extern_server_info", &labels, sswTotCnt, ip, port);

    memset(&labels, 0x00, sizeof(LabelList));
    FillCommonLabels(labels, {
        MakeLabel("TYPE", external_type),
        MakeLabel("COUNT_TYPE", "ALIVE"),
        MakeLabel("AS", as_metric.asName)
    });

    SetMetricValue("get_extern_server_info", &labels, sswAliveCnt, ip, port);

    memset(&labels, 0x00, sizeof(LabelList));
    FillCommonLabels(labels, {
        MakeLabel("TYPE", external_type),
        MakeLabel("COUNT_TYPE", "THRESHOLD"),
        MakeLabel("AS", as_metric.asName)
    });

    SetMetricValue("get_extern_server_info", &labels, sswThresholdCnt, ip, port);


    for (int i = 0; i < MAX_MS; ++i) {
        msconfigInfoManager.getMsConfigInfo(i, ms_metric);

        if (ms_metric.asId == -1) continue;
        if (ms_metric.state_flag == 0 && ms_metric.alive_flag == 1 ) msErrCnt++;
        msTotCnt++;
        msAliveCnt = msTotCnt - msErrCnt;
        msThresholdCnt = msTotCnt - 1;

    }

    memset(external_type, 0x00, sizeof(external_type));
    g_cfg.GetConfigString("EXPORTER.MS_TYPE", external_type);

    memset(&labels, 0x00, sizeof(LabelList));
    FillCommonLabels(labels, {
        MakeLabel("TYPE", external_type),
        MakeLabel("COUNT_TYPE", "TOTAL"),
        MakeLabel("AS", as_metric.asName)
    });

    SetMetricValue("get_extern_server_info", &labels, msTotCnt, ip, port);

    memset(&labels, 0x00, sizeof(LabelList));
    FillCommonLabels(labels, {
        MakeLabel("TYPE", external_type),
        MakeLabel("COUNT_TYPE", "ALIVE"),
        MakeLabel("AS", as_metric.asName)
    });

    SetMetricValue("get_extern_server_info", &labels, msAliveCnt, ip, port);

    memset(&labels, 0x00, sizeof(LabelList));
    FillCommonLabels(labels, {
        MakeLabel("TYPE", external_type),
        MakeLabel("COUNT_TYPE", "THRESHOLD"),
        MakeLabel("AS", as_metric.asName)
    });

    SetMetricValue("get_extern_server_info", &labels, msThresholdCnt, ip, port);


    for (int i = 0; i < MAX_CDB; ++i) {
        cdbstatusInfoManager.getCdbStatusInfo(i, cdb_metric);

        if (cdb_metric.asId == -1) continue;
        if (cdb_metric.status == 0 ) cdbErrCnt++;
        cdbTotCnt++;
        cdbAliveCnt = cdbTotCnt - cdbErrCnt;
        cdbThresholdCnt = cdbTotCnt - 1;
    }

    memset(external_type, 0x00, sizeof(external_type));
    g_cfg.GetConfigString("EXPORTER.CDB_TYPE", external_type);

    memset(&labels, 0x00, sizeof(LabelList));
    FillCommonLabels(labels, {
        MakeLabel("TYPE", external_type),
        MakeLabel("COUNT_TYPE", "TOTAL"),
        MakeLabel("AS", as_metric.asName)
    });

    SetMetricValue("get_extern_server_info", &labels, cdbTotCnt, ip, port);

    memset(&labels, 0x00, sizeof(LabelList));
    FillCommonLabels(labels, {
        MakeLabel("TYPE", external_type),
        MakeLabel("COUNT_TYPE", "ALIVE"),
        MakeLabel("AS", as_metric.asName)
    });

    SetMetricValue("get_extern_server_info", &labels, cdbAliveCnt, ip, port);

    memset(&labels, 0x00, sizeof(LabelList));
    FillCommonLabels(labels, {
        MakeLabel("TYPE", external_type),
        MakeLabel("COUNT_TYPE", "THRESHOLD"),
        MakeLabel("AS", as_metric.asName)
    });

    SetMetricValue("get_extern_server_info", &labels, cdbThresholdCnt, ip, port);

    for (int i = 0; i < MAX_EXTERNAL; ++i) {
        externalIfInfoManager.getExternalIfInfo(i, extern_metric);

        if (extern_metric.asId == -1) continue;

        std::string name(extern_metric.system);
        auto& counter = ExternCountMap[name];

        counter.total++;
        if (extern_metric.status == 0) {
            counter.err++;
        }
        counter.alive = counter.total - counter.err;
        counter.threshold = counter.total - 1;
        memset(&labels, 0x00, sizeof(LabelList));

        FillCommonLabels(labels, {
            MakeLabel("TYPE", name.c_str()),
            MakeLabel("COUNT_TYPE", "TOTAL"),
            MakeLabel("AS", as_metric.asName)
        });
        SetMetricValue("get_extern_server_info", &labels, counter.total, ip, port);

        memset(&labels, 0x00, sizeof(LabelList));
        FillCommonLabels(labels, {
            MakeLabel("TYPE", name.c_str()),
            MakeLabel("COUNT_TYPE", "ALIVE"),
            MakeLabel("AS", as_metric.asName)
        });
        SetMetricValue("get_extern_server_info", &labels, counter.alive, ip, port);

        memset(&labels, 0x00, sizeof(LabelList));
        FillCommonLabels(labels, {
            MakeLabel("TYPE", name.c_str()),
            MakeLabel("COUNT_TYPE", "THREHOLD"),
            MakeLabel("AS", as_metric.asName)
        });
        SetMetricValue("get_extern_server_info", &labels, counter.threshold, ip, port);

    }
}
void CExporter::GetAlarmInfo(const char* ip, const char* port, const AlarmMessage& alarmMessage) {
    int  alarmLevel = 0;
    char module_name[MODULE_NAME_LEN] = {0}; 
    char outdate[25];

    LabelList labels, down_labels;
    SERVER_INFO server_metric;
    MODULE_INFO module_metric;

    Log.printf(LOG_LV2, "[CExporter][GetAlarmInfo] START type : %d ", alarmMessage.type);

    if (alarmMessage.msg == nullptr) {
        Log.printf(LOG_ERR, "[CExporter][GetAlarmInfo] alarmMessage.msg is nullptr");
        return;
    }

    Log.printf(LOG_LV1, "[CExporter][GetAlarmInfo] serverId : %d asId : %d alarmId : %d moduleId : %d level : %d  comment : %s",
               alarmMessage.msg->serverId, alarmMessage.msg->asId, alarmMessage.msg->alarmId, alarmMessage.msg->moduleId, alarmMessage.msg->alarmLevel, alarmMessage.msg->comment);  

    memset(&labels, 0x00, sizeof(LabelList));
    memset(&server_metric, 0x00, sizeof(SERVER_INFO));
    memset(&module_metric, 0x00, sizeof(MODULE_INFO));
    memset(module_name, 0x00, sizeof(module_name));

    serverInfoManager.getServerInfo(alarmMessage.msg->serverId, server_metric);
    moduleInfoManager.getModuleInfo(alarmMessage.msg->asId, alarmMessage.msg->moduleId, module_metric);

    if(alarmMessage.msg->asId == -1 || alarmMessage.msg->alarmId == -1) {
        Log.printf(LOG_ERR, "[CExporter][GetAlarmInfo] GetAlarmHistoryInfo asId [%d] alarmId[%d] is NULL ", alarmMessage.msg->asId, alarmMessage.msg->alarmId);
        return;
    }
    if(strcmp(server_metric.asName, "")==0 || strcmp(server_metric.serverName, "")==0) {
        Log.printf(LOG_ERR, "[CExporter][GetAlarmInfo] GetAlarmHistoryInfo asName [%s] serverName[%s] is NULL ", server_metric.asName, server_metric.serverName);
        return;
    }
    if(alarmMessage.msg->alarmLevel == -1){ 
        Log.printf(LOG_ERR, "[CExporter][GetAlarmInfo] GetAlarmHistoryInfo alarmLevel[%d] Invalid ", alarmMessage.msg->alarmLevel);
        return;
    }
    if(strcmp(module_metric.moduleName, "")==0) strcpy(module_name, "-");
    else extractModuleBaseName(module_metric.moduleName, module_name, sizeof(module_name)); 

    formatDateTime(alarmMessage.msg->alarmTime, outdate, sizeof(outdate));

    FillCommonLabels(labels, {
        MakeLabel("AS", server_metric.asName),
        MakeLabel("SERVER", server_metric.serverName),
        MakeLabel("MODULE", module_name),
        MakeLabel("ALERT_TIME", outdate),
        MakeLabel("ALARM_KEY", alarmMessage.msg->alarmKey),
        MakeLabel("DESC", alarmMessage.msg->comment)
    });
    std::string labelStr = LabelListToString(&labels);
    
    if(alarmMessage.type == EXPORTER_ALARM_ERR) {
        if(alarmMessage.msg->alarmLevel == 0) {
            Log.printf(LOG_LV2, "[CExporter][GetAlarmInfo] EXPORTER_ALARM_ERR ALARM OFF ", alarmMessage.type);
            RemoveMetricByPartialLabel("get_alarm_list_info", "ALARM_KEY", alarmMessage.msg->alarmKey, ip, port);
            SetMetricValue("get_alarm_history_info", &labels, alarmMessage.msg->alarmLevel, ip, port);
            AddLabel(labelStr);
        } else {
            RemoveMetricByPartialLabel("get_alarm_list_info", "ALARM_KEY", alarmMessage.msg->alarmKey, ip, port);
            Log.printf(LOG_LV2, "[CExporter][GetAlarmInfo] EXPORTER_ALARM_ERR ALARM ON ", alarmMessage.type);
            SetMetricValue("get_alarm_list_info", &labels, alarmMessage.msg->alarmLevel, ip, port);
            SetMetricValue("get_alarm_history_info", &labels, alarmMessage.msg->alarmLevel, ip, port);
            AddLabel(labelStr);
            AddLabelOnly(labelStr);
        }
    } else{
        Log.printf(LOG_LV2, "[CExporter][GetAlarmInfo] EXPORTER_ALARM_FAULT or EXPORTER_ALARM_RELAY [%d] ", alarmMessage.type);
        if(alarmMessage.type == EXPORTER_ALARM_FAULT) alarmLevel = 12;
        else if(alarmMessage.type == EXPORTER_ALARM_RELAY) alarmLevel = alarmMessage.msg->alarmLevel;

        SetMetricValue("get_alarm_history_info", &labels, alarmLevel, ip, port);
        AddLabel(labelStr);
        AddLabelOnly(labelStr);
    }

    Log.printf(LOG_LV2, "[CExporter][GetAlarmInfo] END ");
}

void CExporter::SetFixMetricData(const char* ip, const char* port) {
    while (true) {
        sleep(3);

        GetConfigAsInfo(ip, port);
        GetConfigAsListInfo(ip, port);
        GetOverloadInfo(ip, port);
        GetStateServerInfo(ip, port);
        GetHaServerInfo(ip, port);
        GetStateModuleInfo(ip, port);
        GetAsStatInfo(ip, port);
        GetHadwareInfo(ip, port);
        GetConfigThresholdInfo(ip, port);
        GetExternServerListInfo(ip, port);
        GetServiceGraphInfo(ip, port);
        GetChannelUsage(ip, port);
        GetExternServerInfo(ip, port);
    }
}

void CExporter::CheckDeleteLabels(const char* ip, const char* port) {
    while (true) {
        sleep(3);  // 3초 대기
        auto now = std::chrono::system_clock::now();
        std::deque<std::pair<std::chrono::system_clock::time_point, std::string>> expired;  // 만료된 레이블을 저장할 deque

        {
            std::lock_guard<std::mutex> lock(labelQueueMutex);  // 큐에 접근할 때 Mutex로 보호
            while (!labelQueue.empty()) {
                // 시간 차이를 초 단위로 계산
                auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - labelQueue.front().first).count();
                if (elapsed >= 10) {  // 10초 이상 경과된 항목들
                    expired.push_back(labelQueue.front());  // 만료된 항목을 expired 큐에 추가
                    labelQueue.pop_front();  // 큐에서 제거
                } else {
                    break;  // 더 이상 만료된 항목이 없으면 종료
                }
            }
        }

        // 만료된 항목들을 처리
        for (const auto& entry : expired) {
            const std::string& labelStr = entry.second;  // 만료된 labelStr 가져오기
            if (RemoveMetric("get_alarm_history_info", labelStr.c_str(), ip, port)) {
                Log.printf(LOG_LV2, "[CExporter][CheckDeleteLabels] get_alarm_history_info map delete[%s]", labelStr.c_str());
                DelLabel(labelStr);
            }
        }
    }
}
void CExporter::SetAlarmMetricData(const char* ip, const char* port) {
    while (true) {
        // 3초마다 큐를 확인
        sleep(3);
        PopAndProcessAlarmMessage(ip, port);
    }
}
void* startAndMonitorServerWrapper(void* arg) {
    auto* args = static_cast<ServerArgs*>(arg);
    args->exporter->startAndMonitorServer(args->ip, args->port);
    delete args;  // 동적 할당 해제
    return nullptr;
}
void* setFixMetricDataWrapper(void* arg) {
    auto* args = static_cast<ServerArgs*>(arg);
    args->exporter->SetFixMetricData(args->ip, args->port);
    delete args;  // 동적 할당 해제
    return nullptr;
}
void* setAlarmMetricDataWrapper(void* arg) {
    auto* args = static_cast<ServerArgs*>(arg);
    args->exporter->SetAlarmMetricData(args->ip, args->port);
    delete args;  // 동적 할당 해제
    return nullptr;
}
void* CheckDeleteLabelsWrapper(void* arg) {
    auto* args = static_cast<ServerArgs*>(arg);
    args->exporter->CheckDeleteLabels(args->ip, args->port);
    delete args;  // 동적 할당 해제
    return nullptr;
}

void CExporter::startFixMetrics(const char* ip, const char* port) {
    Log.printf(LOG_INF, "[CExporter] startFixMetrics START ");

    pthread_t serverMonitorThread;
    pthread_t metricThread;

    auto* args1 = new ServerArgs{this, ip, port};  // for server monitor
    auto* args2 = new ServerArgs{this, ip, port};  // for fix metric

    if (pthread_create(&serverMonitorThread, nullptr, startAndMonitorServerWrapper, args1)) {
        Log.printf(LOG_ERR, "[CExporter] Failed to create serverMonitorThread");
        shut_down(11);
    }

    if (pthread_create(&metricThread, nullptr, setFixMetricDataWrapper, args2)) {
        Log.printf(LOG_ERR, "[CExporter] Failed to create metricThread");
        shut_down(12);
    }
    pthread_detach(serverMonitorThread);
    pthread_detach(metricThread);
}
void CExporter::startAlarmMetrics(const char* ip, const char* port) {
    Log.printf(LOG_INF, "[CExporter] startAlarmMetrics START ");

    pthread_t serverMonitorThread;
    pthread_t metricThread;
    pthread_t checkerThread;

    auto* args1 = new ServerArgs{this, ip, port};  // for server monitor
    auto* args2 = new ServerArgs{this, ip, port};  // for fix metric
    auto* args3 = new ServerArgs{this, ip, port};  // for fix metric

    if (pthread_create(&serverMonitorThread, nullptr, startAndMonitorServerWrapper, args1)) {
        Log.printf(LOG_ERR, "[CExporter] Failed to create serverMonitorThread");
        shut_down(13);
    }

    if (pthread_create(&metricThread, nullptr, setAlarmMetricDataWrapper, args2)) {
        Log.printf(LOG_ERR, "[CExporter] Failed to create metricThread");
        shut_down(14);
    }

    if (pthread_create(&checkerThread, nullptr, CheckDeleteLabelsWrapper, args3)) {
        Log.printf(LOG_ERR, "[CExporter] Failed to create checkerThread");
        shut_down(15);
    }
    pthread_detach(serverMonitorThread);
    pthread_detach(metricThread);
    pthread_detach(checkerThread);
}
