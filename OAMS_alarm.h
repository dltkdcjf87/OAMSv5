#ifndef OAMS_ALARM_H
#define OAMS_ALARM_H
#include "OAMS_common.h"
#include "CQueueBase.h"
#define KEY_MAXSIZE	16

struct AlarmInfo {
	int 	svrId;
	int		modId;
	int     alarmId;        //알람 코드
    int     indexId;        //INDEX  //xbus id or extemd index
    uint8_t alarmFlag;      //알람 On/Off
    int     commentCnt;     // 참고 메세지 개수
    char    comment[8][64]; // 참고 메세지
};


class AlarmManager {
private:
	std::mutex	m_mapMutex;
	int insertAlarmMap(char* key, ALM_Q_MSG* alarmData);
	int deleteAlarmMap(char* key);

	map<string, ALM_Q_MSG*> m_AlarmMap; 
	ALM_Q_MSG* alarmExistSearch(const char* KEY);

	CQueueBase<AlarmInfo *> alarmDataRcvQueue;

	void alarmRcvDeQueueThread();
	int alarmRcvDataProcess(AlarmInfo* alarmData);
	bool alarmCreateKey(int type, int asId, int svrId, int index, int alarmCode, char* alarmKey);
	int CommentProcess(char* asName, char* svrName, char* origComment, int Count, char variable[][64], char* CommentResult);
	int UpdateAlarm(int exist, const char* KEY, ALM_Q_MSG* alarmData);
	int AfterLogicProcess(ALM_Q_MSG* alarmData);
	bool beforeLogicProcess(AlarmInfo* alarmMsg);
	void SetSvrModstate(bool set, ALM_Q_MSG* alarmMsg);


public:
	AlarmManager();
	~AlarmManager();
	bool Init();
	int AlarmOccured(AlarmInfo alarmMsg);
	int FaultOccured(int asId, int svrId, const char* faultMsg);
	int RelayOccured(int asId, int svrId, int moId,  const char* relayMsg);
    void SendAlarm(int svrId, int moduleId, int alarmId, int indexId, int alarmFlag, int commentCnt, const char comments[][64]);

};


#endif
