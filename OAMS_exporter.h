#ifndef CEXPORTERLIB_H
#define CEXPORTERLIB_H

#include "OAMS_define.h"

// 라벨 구조 정의
#define MAX_LABEL_COUNT 10
#define MAX_LABEL_KEY_LEN 64
#define MAX_LABEL_VALUE_LEN 128

typedef struct {
    char key[MAX_LABEL_KEY_LEN];
    char value[MAX_LABEL_VALUE_LEN];
} Label;

typedef struct {
    Label labels[MAX_LABEL_COUNT];
    int count;
} LabelList;

typedef struct {
    int type;                  // 메시지 타입 (1이면 alm, 다른 값이면 hst 등)
    ALM_Q_MSG* msg;            // ALM_Q_MSG 포인터
} AlarmMessage;

class CExporter {
private:
    std::deque<std::pair<std::chrono::system_clock::time_point, std::string>> labelQueue;
    std::mutex labelQueueMutex;
    std::vector<std::string> labelListOnly;
    std::mutex labelListMutex;
    std::queue<AlarmMessage*> alarmQueue;
    std::mutex alarmQueueMutex;

public:
    CExporter();
    ~CExporter();

    bool startPrometheusServer(const char* ip, const char* port);
    bool SetMetricValue(const char* name, const LabelList* labels, double value, const char* ip, const char* port);
    bool RemoveMetric(const char* name, const char* labelStr, const char* ip, const char* port);
    bool RemoveMetricName(const char* name, const char* ip, const char* port);
    void RemoveMetricByPartialLabel(const char* name, const char* key, const char* value, const char* ip, const char* port);
    void monitorServerHealth(const char* ip, const char* port);
    void startAndMonitorServer(const char* ip, const char* port);

    void AddLabel(const std::string& labelStr);
    void AddLabelOnly(const std::string& labelStr);
    void DelLabel(const std::string& labelStr);
    void DelLabelOnly(const std::string& labelStr);

    void PushAlarmMessage(int type, ALM_Q_MSG* msg);
    void PopAndProcessAlarmMessage(const char* ip, const char* port);
    void PopAndProcessHistoryMessage(const char* ip, const char* port);

    void GetConfigAsInfo(const char* ip, const char* port);
    void GetConfigAsListInfo(const char* ip, const char* port);
    void GetOverloadInfo(const char* ip, const char* port);
    void GetStateServerInfo(const char* ip, const char* port);
    void GetHaServerInfo(const char* ip, const char* port);
    void GetStateModuleInfo(const char* ip, const char* port);
    void GetAsStatInfo(const char* ip, const char* port);
    void GetHadwareInfo(const char* ip, const char* port);
    void GetChannelUsage(const char* ip, const char* port);
    void GetConfigThresholdInfo(const char* ip, const char* port);
    void GetExternServerListInfo(const char* ip, const char* port);
    void GetServiceGraphInfo(const char* ip, const char* port);
    void GetExternServerInfo(const char* ip, const char* port);
    void GetAlarmInfo(const char* ip, const char* port, const AlarmMessage& alarmMessage);

    void SetFixMetricData(const char* ip, const char* port);
    void SetAlarmMetricData(const char* ip, const char* port);
    void CheckDeleteLabels(const char* ip, const char* port);
    void startFixMetrics(const char* ip, const char* port);
    void startAlarmMetrics(const char* ip, const char* port);
};

#endif // CEXPORTERLIB_H

