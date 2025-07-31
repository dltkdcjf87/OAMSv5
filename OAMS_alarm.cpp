#include "OAMS_alarm.h"

AlarmManager::AlarmManager()
{
}

AlarmManager::~AlarmManager()
{
}


bool AlarmManager::Init()
{
	AlarmDataAllDelete();

	thread t(&AlarmManager::alarmRcvDeQueueThread, this);
    t.detach();

	return true;

}

void AlarmManager::alarmRcvDeQueueThread()
{
	AlarmInfo*		alarmData;
	int ret;

	while(1)
	{
		if(alarmDataRcvQueue.IsEmpty() == true)
        {
            sleep(1);
            continue;
        }

		alarmData = alarmDataRcvQueue.Pop();
		if (alarmData == nullptr)
			continue;

		Log.printf(LOG_LV3, "alarmRcvDeQueueThread svrid[%d], alarmID[%d], INDEX[%d], ONOFF[%d], commentCNT[%d] ", alarmData->svrId, alarmData->alarmId, alarmData->indexId, alarmData->alarmFlag, alarmData->commentCnt );
		try {
        	int ret = alarmRcvDataProcess(alarmData);
            if (ret != 0) {
				Log.printf(LOG_ERR, "alarmRcvDeQueueThread FAIL !! =[%d]", ret);
            }
        } catch (const std::exception& e) {
			Log.printf(LOG_ERR, "alarmRcvDeQueueThread EXCEPTION !! =[%d]", ret);
        }
		delete(alarmData);
	}

}

int AlarmManager::AlarmOccured(AlarmInfo alarmMsg)
{
    AlarmInfo* alarmData = new AlarmInfo;
    if (alarmData == NULL) {
        return -1;
    }

	*alarmData = alarmMsg;

    alarmDataRcvQueue.Push(alarmData);

    return 0;
}

void AlarmManager::SendAlarm(int svrId, int moduleId, int alarmId, int indexId, int alarmFlag, int commentCnt, const char comments[][64])
{
    Log.printf(LOG_LV2, "SendAlarm svrId[%d] moduleID[%d]  alarmId[%d] indexId[%d] alarmFlag[%d] commentCnt[%d] comment0[%s] comment1[%s] ",
              svrId, moduleId, alarmId, indexId, alarmFlag, commentCnt, comments[0], comments[1]);	
    AlarmInfo alarmMsg;
    alarmMsg.svrId = svrId;
	alarmMsg.modId = moduleId;
    alarmMsg.alarmId = alarmId;
    alarmMsg.indexId = indexId; 
    alarmMsg.alarmFlag = alarmFlag;
    alarmMsg.commentCnt = commentCnt;

    for (int i = 0; i < commentCnt && i < 8; ++i) {
        strncpy(alarmMsg.comment[i], comments[i], sizeof(alarmMsg.comment[i]) - 1);
        alarmMsg.comment[i][sizeof(alarmMsg.comment[i]) - 1] = '\0'; // 안전하게 널 종료
    }

    AlarmOccured(alarmMsg);
}

int AlarmManager::alarmRcvDataProcess(AlarmInfo* alarmMsg)
{
	bool ret;
	char comment[128];
	memset(comment, 0x00, sizeof(comment));

	char KEY[KEY_MAXSIZE];
	memset(KEY,0x00, sizeof(KEY));

	bool exist;

	SERVER_INFO svrData;

	ALARM_CONFIG	alarmCfgData;
	alarmconfigManager.getAlarmConfig(alarmMsg->alarmId, alarmCfgData);
	if(alarmCfgData.AlarmCode < 0) {
		Log.printf(LOG_LV2, "ALARM DB NOT DEFINE[%d] ", alarmMsg->alarmId);	
		return -2;
	}

	serverInfoManager.getServerInfo(alarmMsg->svrId, svrData);

	ret = beforeLogicProcess(alarmMsg);
	    if(!ret){
        return -1;
    }

	ret = alarmCreateKey(alarmCfgData.AlarmType, svrData.asId, alarmMsg->svrId, alarmMsg->indexId, alarmMsg->alarmId, KEY);
	if(!ret){
		return -1;	
	}
/*
	if(alarmMsg->alarmId == ALM_CODE_CSCF_DOWN)
	{
		SSW_CONFIG_INFO SSWDATA;
		int INDEX = externServerManager.CscfGetIdToIndex(alarmMsg->indexId);
		sswconfigInfoManager.getSswConfigInfo(INDEX, SSWDATA);
        strncpy(alarmMsg->comment[0], SSWDATA.systemName, sizeof(alarmMsg->comment[0]) - 1);
        alarmMsg->comment[0][sizeof(alarmMsg->comment[0]) - 1] = '\0';
	}
*/
	CommentProcess(svrData.asName, svrData.serverName, alarmCfgData.AlarmDesc, alarmMsg->commentCnt, alarmMsg->comment, comment);

	ALM_Q_MSG* msg = alarmExistSearch(KEY);
	if (msg) {	// EXIST ALARM SETTING (UPDATE / DELETE)
		exist = 1;

		int preAlarmLevel = msg->alarmLevel;

		SetSvrModstate(false, msg);
		if(alarmCfgData.AlarmType == ALM_TYPE_LIMIT) {
			msg->alarmLevel = alarmMsg->alarmFlag;
		}else if(alarmCfgData.AlarmCode == ALM_CMS_OVERLOAD) {
			msg->alarmLevel = alarmMsg->alarmFlag;
		} else {
			msg->alarmLevel = alarmMsg->alarmFlag ? alarmCfgData.AlarmLevel : 0;
		}
		strcpy(msg->comment, comment);

		SetSvrModstate(true, msg);

		if(preAlarmLevel == msg->alarmLevel)
		{
			Log.printf(LOG_LV2, "[alarmRcvDataProcess] RECV ALARM LEVEL EQUAL - EXIST/NEW ALARM LEVEL[%d] ", preAlarmLevel);
			return -1;
		}

		Log.printf(LOG_LV2, "UPDATE OR DELETE - EXIST ALARM LEVEL[%d], KEY[%s], COMMENT[%s] ", msg->alarmLevel, msg->alarmKey, msg->comment);
	}
	else // NEW ALARM SETTING
	{
	 	if(alarmMsg->alarmFlag == 0) {  //ALARM NOT OCCUR
            Log.printf(LOG_LV2, "ALARM NOT OCCUR CLEAR // ALARMID[%d], SVRID[%d], INDEXID[%d] ",alarmMsg->alarmId, alarmMsg->svrId, alarmMsg->indexId);
            return -1;
        }

		exist = 0;

		msg = new ALM_Q_MSG;
    	memset(msg, 0x00, sizeof(ALM_Q_MSG));

		struct timeval tv;
		time_t now = time(NULL);
		struct tm *t = localtime(&now);
	    char    buffer[20];
	    memset(buffer, 0x00, sizeof(buffer));
            
	    gettimeofday(&tv, NULL);

//	    strftime(msg->alarmTime, sizeof(msg->alarmTime), "%Y%m%d%H%M%S", t);
		strftime(buffer, sizeof(buffer), "%Y%m%d%H%M%S", t);

    	sprintf(msg->alarmTime, "%s%03ld", buffer, tv.tv_usec / 1000);

		Log.printf(LOG_LV2, "MYUNG_TEST LOGTIME[%s]", msg->alarmTime);

	    strcpy(msg->alarmKey, KEY);
	    msg->alarmType 	= alarmCfgData.AlarmType;
	    msg->alarmId   	= alarmMsg->alarmId;
	    msg->asId      	= svrData.asId;
	    msg->serverId  	= alarmMsg->svrId;
		msg->moduleId	= alarmMsg->modId;
	    msg->indexId   	= alarmMsg->indexId;
	    msg->mask      	= 0;
	    if(alarmCfgData.AlarmType == ALM_TYPE_LIMIT) {
	    	msg->alarmLevel = alarmMsg->alarmFlag;
        }else if(alarmCfgData.AlarmCode == ALM_CMS_OVERLOAD) {
            msg->alarmLevel = alarmMsg->alarmFlag;
	    } else {
	    	msg->alarmLevel	= alarmMsg->alarmFlag ? alarmCfgData.AlarmLevel : 0;
		}
	    strcpy(msg->comment, comment);

		Log.printf(LOG_LV2, "INSERT - NEW ALARM LEVEL[%d], KEY[%s], COMMENT[%s] ", msg->alarmLevel, msg->alarmKey, msg->comment);
		SetSvrModstate(true, msg);
	}

	AfterLogicProcess(msg);
    UpdateAlarm(exist, KEY, msg);

    return 0;
}

bool AlarmManager::alarmCreateKey(int type, int asId, int svrId, int index, int alarmCode, char* alarmKey)
{
	char KEY[KEY_MAXSIZE];
	memset(KEY, 0x00, sizeof(KEY));

	if (type > 99 || asId > 99 || svrId > 99 || index > 999 || alarmCode > 999) {
		Log.printf(LOG_ERR,"[OAMS_ALARM][alarmCreateKey] Variable Size Over ! ");
		Log.printf(LOG_ERR,"[OAMS_ALARM][alarmCreateKey] type =%d asId=%d svrId=%d index=%d alarmCode=%d ", type, asId, svrId, index, alarmCode);
    	return false;
	}
    if(alarmCode == ALM_SPY_AS_BLOCK) { //A, B SIDE 구분이 힘들기 때문에 하나로 ALARM_KEY관리- 화면 상 AS상태로 표시됨.	
	    sprintf(KEY, "%02d%02d%03d", type, asId, alarmCode);
    }else if(alarmCode == ALM_CMS_OVERLOAD) { //A, B SIDE 구분이 힘들기 때문에 하나로 ALARM_KEY관리- 화면 상 AS상태로 표시됨.	
	    sprintf(KEY, "%02d%02d%03d", type, asId, alarmCode);
    }else if(alarmCode == ALM_CODE_AS_DISCONNET) { //A, B SIDE 구분이 힘들기 때문에 하나로 ALARM_KEY관리- 화면 상 AS상태로 표시됨.	
	    sprintf(KEY, "%02d%02d%03d", type, asId, alarmCode);
    } else {
    	sprintf(KEY, "%02d%02d%02d%03d%03d", type, asId, svrId, index, alarmCode);
    }
	strcpy(alarmKey, KEY);	

	Log.printf(LOG_LV1,"[OAMS_ALARM][alarmCreateKey] KEY Create[%s],type[%d] asid[%d], svrid[%d], index[%d], alarmcd[%d] ",alarmKey, type, asId,svrId, index, alarmCode );

	return true;

}

ALM_Q_MSG* AlarmManager::alarmExistSearch(const char* KEY)
{
	std::lock_guard<std::mutex> lock(m_mapMutex);
	std::string key = KEY;

	Log.printf(LOG_LV1,"[OAMS_ALARM][alarmExistSearch] KEY[%s] ", KEY);
	auto it = m_AlarmMap.find(key);
    if (it != m_AlarmMap.end()) {
        return it->second;
    }

    return nullptr; 
}

int AlarmManager::UpdateAlarm(int exist, const char* KEY, ALM_Q_MSG* alarmData)
{

    std::string key = KEY;

	ALM_Q_MSG msg;

	strcpy(msg.alarmKey, alarmData->alarmKey);
	msg.alarmType = alarmData->alarmType;
	msg.alarmId = alarmData->alarmId;
	msg.asId = alarmData->asId;
	msg.serverId = alarmData->serverId;
    msg.moduleId = alarmData->moduleId;
	msg.indexId = alarmData->indexId;
	msg.mask = alarmData->mask;
	strcpy(msg.alarmTime, alarmData->alarmTime);
	msg.alarmLevel = alarmData->alarmLevel;
	strcpy(msg.comment, alarmData->comment);

//	alarmMetric.PushAlarmMessage(1, &msg);

	Log.printf(LOG_LV2, "EXIST ALARM??[%d], KEY[%s], type[%d], ID[%d], as[%d], svr[%d], moduleId[%d] idx[%d], mask[%d], almTime[%s], level[%d], comment[%s] ", 
				exist, msg.alarmKey, msg.alarmType, msg.alarmId, msg.asId, msg.serverId, msg.moduleId, msg.indexId, msg.mask, msg.alarmTime, msg.alarmLevel, msg.comment);

	if(alarmData->alarmLevel != 0){ // ALARM ON
		if(!exist){ //NEW ALARM REGIST
			insertAlarmMap(msg.alarmKey, alarmData);		//ALARM MAP REGI

			AlarmDataInsert(msg);				// ALARM DB INSERT
			Log.printf(LOG_LV2, "NEW ALARM OCCUR !![%s] ",msg.comment);
		}
		else { //EXIST ALARM UPDATE
			insertAlarmMap(msg.alarmKey, alarmData);		//ALARM MAP REGI

			AlarmDataDelete(KEY);				//ALARM DB DELETE
			AlarmDataInsert(msg);				//ALARM DB INSERT
			Log.printf(LOG_LV2, "EXIST ALARM UPDATE !![%s] ",msg.comment);
		}
	}
	else //ALARM OFF
	{
		deleteAlarmMap(msg.alarmKey);	//ALARM MAP DEREGI

		AlarmDataDelete(KEY);	//ALARM DB DELETE
		AlarmLogInsert(alarmData); //ALARM LOG INSERT
        delete alarmData;
	}

	struct timeval tv;
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
	char	buffer[20];
	memset(buffer, 0x00, sizeof(buffer));

	gettimeofday(&tv, NULL);


//    strftime(msg.alarmTime, sizeof(msg.alarmTime), "%Y%m%d%H%M%S", t);
    strftime(buffer, sizeof(buffer), "%Y%m%d%H%M%S", t);

	sprintf(msg.alarmTime, "%s%03ld", buffer, tv.tv_usec / 1000);
	Log.printf(LOG_LV2, "MYUNG_TEST LOGTIME[%s]", msg.alarmTime);

	alarmMetric.PushAlarmMessage(1, &msg);

	return 0;
}

int AlarmManager::insertAlarmMap(char* key, ALM_Q_MSG* alarmData)
{
	std::string sKEY = key;

    Log.printf(LOG_LV2, "insertAlarmMap key[%s] ", key);
	std::lock_guard<std::mutex> lock(m_mapMutex);
	m_AlarmMap[sKEY] = alarmData;

	return 0;
}

int AlarmManager::deleteAlarmMap(char* key)
{
	std::string sKEY = key;

    Log.printf(LOG_LV2, "deleteAlarmMap key[%s] ", key);
	std::lock_guard<std::mutex> lock(m_mapMutex);
	m_AlarmMap.erase(sKEY);

	return 0;
}

int AlarmManager::CommentProcess(char* asName, char* svrName, char* origComment, int Count, char variable[][64], char* CommentResult)
{
	const char* p = origComment;
    char* out = CommentResult;
    int varIndex = 0;

	char firstText[64];
	memset(firstText, 0x00, sizeof(firstText));

	sprintf(firstText, "[%s][%s] ", asName, svrName);
	strcpy(out, firstText);
	out += strlen(firstText);

    while (*p != '\0') {
        if (*p == '%' && *(p + 1) != '\0') {
            p++; 

            if (varIndex >= Count) {
                *out = '\0';
                return 0 ;
            }

            if (*p == 's') {
                strcpy(out, variable[varIndex]);
                out += strlen(variable[varIndex]);
                varIndex++;
            } else if (*p == 'd') {
                sprintf(out, "%d", atoi(variable[varIndex]));
                out += strlen(out);
                varIndex++;
            } else {
                *out++ = '%';
                *out++ = *p;
            }

            p++;
        } else {
            *out++ = *p++;
        }

    }

    *out = '\0';

	return 0;
}


int AlarmManager::FaultOccured(int asId, int svrId, const char* faultMsg)
{
	ALM_Q_MSG msg;
	struct timeval tv;
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
	char    buffer[20];
    memset(buffer, 0x00, sizeof(buffer));

	gettimeofday(&tv, NULL);

//    strftime(msg.alarmTime, sizeof(msg.alarmTime), "%Y%m%d%H%M%S", t);
    strftime(buffer, sizeof(buffer), "%Y%m%d%H%M%S", t);

	sprintf(msg.alarmTime, "%s%03ld", buffer, tv.tv_usec / 1000);

	Log.printf(LOG_LV2, "MYUNG_TEST LOGTIME[%s]", msg.alarmTime);

    strcpy(msg.alarmKey, "FAULT");
    msg.alarmType = 99;
    msg.alarmId = 10000;
    msg.asId = asId;
    msg.moduleId = -1;
    msg.serverId = svrId;
    msg.indexId = 0;
    msg.mask = 0;
    msg.alarmLevel = 0;
    strcpy(msg.comment, faultMsg);

	FaultDataToDB(msg.asId, msg.serverId, msg.indexId, msg.alarmId, msg.alarmTime, msg.alarmLevel, msg.comment);
	alarmMetric.PushAlarmMessage(2, &msg);

	return 0;
}

int AlarmManager::RelayOccured(int asId, int svrId, int moId, const char* relayMsg)
{
    ALM_Q_MSG msg; 
    struct timeval tv;
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char    buffer[20];
    memset(buffer, 0x00, sizeof(buffer));
    
    gettimeofday(&tv, NULL);

//    strftime(msg.alarmTime, sizeof(msg.alarmTime), "%Y%m%d%H%M%S", t);
    strftime(buffer, sizeof(buffer), "%Y%m%d%H%M%S", t);
    
    sprintf(msg.alarmTime, "%s%03ld", buffer, tv.tv_usec / 1000);

	Log.printf(LOG_LV2, "MYUNG_TEST LOGTIME[%s]", msg.alarmTime);

    strcpy(msg.alarmKey, "RELAY");
    msg.alarmType = 98;
    msg.alarmId = 10001;
    msg.asId = asId;
    msg.serverId = svrId;
    msg.moduleId = moId;
    msg.indexId = 0;
    msg.mask = 0;
    msg.alarmLevel = 0;
    strcpy(msg.comment, relayMsg);

    alarmMetric.PushAlarmMessage(3, &msg);


	return 0;
}


int AlarmManager::AfterLogicProcess(ALM_Q_MSG* alarmData)
{
	switch(alarmData->alarmId)
	{
		case ALM_CODE_CSCF_DOWN:
		{
			int runFlag;

			if(alarmData->alarmLevel == 0) 	runFlag = STATE_UP; //NORMAL
			else 							runFlag = STATE_DOWN; //DOWN

			externServerManager.UpdateCSCFStatus(alarmData->asId, alarmData->indexId, runFlag, alarmData->alarmTime);
			break;
		}
        case ALM_CMS_OVERLOAD:
        {
            asInfoManager.setOverLoadASInfo(alarmData->asId, alarmData->alarmLevel); 
            break;
        }
        case ALM_SPY_AS_BLOCK :
        {
            asInfoManager.setBlockASInfo(alarmData->asId, alarmData->alarmLevel); 
            break;
        }
		default:
			break;

	}
    return 0;

}

bool AlarmManager::beforeLogicProcess(AlarmInfo* alarmMsg)
{
	switch(alarmMsg->alarmId)
	{
		case ALM_CODE_MRF_DOWN:
		{
			alarmMsg->indexId = externServerManager.MrfGetIdToIndex(string(alarmMsg->comment[0]));
			if(alarmMsg->indexId < 0)	return false;
			Log.printf(LOG_LV2,"MrfGetIdToIndex MSNAME[%s], MSINDEX[%d] ", alarmMsg->comment[0], alarmMsg->indexId);
			break;
		}
		case ALM_CODE_CSCF_DOWN:
		{
	        SSW_CONFIG_INFO SSWDATA;
        	int INDEX = externServerManager.CscfGetIdToIndex(alarmMsg->indexId);
			if(INDEX < 0)	return false;
        	sswconfigInfoManager.getSswConfigInfo(INDEX, SSWDATA);
        	strncpy(alarmMsg->comment[0], SSWDATA.systemName, sizeof(alarmMsg->comment[0]) - 1);
        	alarmMsg->comment[0][sizeof(alarmMsg->comment[0]) - 1] = '\0';
			break;
		}
		default:
			return true;
			break;
	}

	return true;

}

void AlarmManager::SetSvrModstate(bool set, ALM_Q_MSG* alarmMsg)
{
	if(alarmMsg->alarmType == ALM_TYPE_AS)	return; // AS Type인경우 표시하지 않는다.
	if(alarmMsg->alarmLevel == ALM_OFF)		return; 
    //20250717 sangchul hw 장애 표시안돼서 주석처리 
	//if(alarmMsg->moduleId == -1)			return; //MODULE NOT DISPLAY  

	if(set == true)
	{
		serverInfoManager.plusAlarmCountServerInfo(alarmMsg->serverId, alarmMsg->alarmLevel);	
		moduleInfoManager.plusAlarmCountModuleInfo(alarmMsg->asId, alarmMsg->moduleId, alarmMsg->alarmLevel);
	}
	else
	{
		serverInfoManager.minusAlarmCountServerInfo(alarmMsg->serverId, alarmMsg->alarmLevel);
        moduleInfoManager.minusAlarmCountModuleInfo(alarmMsg->asId, alarmMsg->moduleId, alarmMsg->alarmLevel);
	}
}
