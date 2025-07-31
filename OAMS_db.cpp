#include "OAMS_db.h"

AltiCinf *pDB;
AltiCinf *pExecDB;

pthread_t   pthreadhd;

void initDBThread()
{
   	pthread_create(&pthreadhd, NULL, &initDB, NULL);
    pthread_detach(pthreadhd);
	
}

void* initDB(void* arg)
{
	pthread_t   pthreadhd;
	bool 		ret;

	pDB = new AltiCinf;
	pExecDB = new AltiCinf;

    char connectString[128];

	char DBIP[60];
	char PORT[10];
	char USERID[20];
	char PASSWD[20];
	char Temp[50];
	int  myServerID;

	memset(DBIP, 0x00, sizeof(DBIP));
	memset(PORT, 0x00, sizeof(PORT));
	memset(USERID, 0x00, sizeof(USERID));
	memset(PASSWD, 0x00, sizeof(PASSWD));
	memset(Temp, 0x00, sizeof(Temp));
	
	g_cfg.GetConfigString("DB.IP", DBIP);
	g_cfg.GetConfigString("DB.PORT", PORT);
	g_cfg.GetConfigString("DB.USER", USERID);
	g_cfg.GetConfigString("DB.PASSWD", PASSWD);
	g_cfg.GetConfigString("DB.MYID", Temp);
	myServerID = atoi(Temp);

    sprintf(connectString, CONNSTR, DBIP, PORT, USERID, PASSWD);

	ret = pDB->initialize(connectString);
	if(!ret)
	{
		Log.printf(LOG_ERR,"[OAMS_DB] SELECT DB INIT FAIL \n");
		delete pDB;
		return nullptr;
	}

	ret = pExecDB->initialize(connectString);
    if(!ret)
    {
		Log.printf(LOG_ERR,"[OAMS_DB] EXECUTE DB INIT FAIL \n");
        delete pDB;
        return nullptr;
    }

	sleep(1);
    DataSetAsInfo();
    DataSetChassisInfo();
    DataSetServerInfo();
    DataSetModuleInfo();
    DataSetServerIdMap();
    DataSetThreshold();
	DataSetServiceKey();
	AlarmDataSelect();
//	SvcLimitSelect();

	dbConnect = true;

	while(1)
	{	
		DataSetNTP();
		DataSetTableSpace(myServerID);
		DataSetDBSession(myServerID);
		sleep(10);
	}

	delete pDB;
}


bool DataSetAsInfo()
{
	vector<map<string, string>> resultData;
	int asId;
	const char* asName;
	const char* description;

	Log.printf(LOG_LV2,"[DATA][OAMS_DB] DataSetAsInfo START \n");
    resultData = pDB->sqlSelect("SELECT as_idx, as_name, description FROM tbl_as_config");

    for (vector<map<string, string>>::iterator it = resultData.begin(); it != resultData.end(); ++it) {
        for (std::map<std::string, std::string>::iterator mit = it->begin(); mit != it->end(); ++mit) {
			if(mit->first == "AS_IDX")		asId = stoi(mit->second);
			if(mit->first == "AS_NAME")		asName = mit->second.c_str();
			if(mit->first == "DESCRIPTION")	description = mit->second.c_str();
        }
  		asInfoManager.setASInfo(asId, asName, description);
		Log.printf(LOG_LV3,"[DATA][OAMS_DB] asId[%d], asName[%s], desc[%s]\n", asId, asName, description);
    }
	Log.printf(LOG_LV2,"[DATA][OAMS_DB] DataSetAsInfo END \n");

	return true;
}


bool DataSetChassisInfo()
{
    vector<map<string, string>> resultData;
	int chassisId;
    int asId;
	int rackId;
	int hdType;
    const char* chassisName;
    int runFlag;

	Log.printf(LOG_LV2,"[DATA][OAMS_DB] DataSetChassisInfo START \n");
    resultData = pDB->sqlSelect("SELECT ch_idx, as_idx, rk_idx, hd_type, ch_name, run_flag FROM tbl_chassis_config;");

    for (vector<map<string, string>>::iterator it = resultData.begin(); it != resultData.end(); ++it) {
        for (std::map<std::string, std::string>::iterator mit = it->begin(); mit != it->end(); ++mit) {
            if(mit->first == "CH_IDX")      chassisId = stoi(mit->second);
            if(mit->first == "AS_IDX")      asId = stoi(mit->second);
            if(mit->first == "RK_IDX")      rackId = stoi(mit->second);
            if(mit->first == "HD_TYPE")     hdType = stoi(mit->second);
            if(mit->first == "CH_NAME")     chassisName = mit->second.c_str();
            if(mit->first == "RUN_FLAG")    runFlag = stoi(mit->second);
        }
        chassisInfoManager.setChassisInfo(chassisId, asId, rackId, hdType, chassisName, runFlag); //switch status
		Log.printf(LOG_LV3,"[DATA][OAMS_DB] DataSetChassisInfo chassisId[%d], asId[%d], rackId[%d], hdType[%d], chassisName[%s], runFlag[%d]\n", 
							chassisId, asId, rackId, hdType, chassisName, runFlag);
    }
	Log.printf(LOG_LV2,"[DATA][OAMS_DB] DataSetChassisInfo END \n");

	return true;
}

bool DataSetServerInfo()
{
    vector<map<string, string>> resultData;
    int svrId;
	int chassisId;
	const char* svrName;
	int svrType;
	int svrCode;
	const char* ipAddr;
	int runFlag;
    int asId;
    const char* asName;

	Log.printf(LOG_LV2,"[DATA][OAMS_DB] DataSetServerInfo START \n");
    resultData = pDB->sqlSelect("SELECT b.bl_idx, b.ch_idx, b.bl_name, b.bl_type, b.svr_code, b.ip_addr, b.run_flag, c.as_idx, a.as_name \
								FROM tbl_blade_config b \
								JOIN tbl_chassis_config c ON b.ch_idx = c.ch_idx \
								JOIN tbl_as_config a ON c.as_idx = a.as_idx;");

    for (vector<map<string, string>>::iterator it = resultData.begin(); it != resultData.end(); ++it) {
        for (std::map<std::string, std::string>::iterator mit = it->begin(); mit != it->end(); ++mit) {
            if(mit->first == "BL_IDX")      svrId = stoi(mit->second);
            if(mit->first == "CH_IDX")     	chassisId = stoi(mit->second);
            if(mit->first == "BL_NAME") 	svrName = mit->second.c_str();
			if(mit->first == "BL_TYPE") 	svrType = stoi(mit->second);
			if(mit->first == "SVR_CODE") 	svrCode = stoi(mit->second);
			if(mit->first == "IP_ADDR") 	ipAddr = mit->second.c_str();
			if(mit->first == "RUN_FLAG") 	runFlag = stoi(mit->second); //run flag 미존재OC
			if(mit->first == "AS_IDX") 		asId = stoi(mit->second);
			if(mit->first == "AS_NAME") 	asName = mit->second.c_str();
        }
		serverInfoManager.setServerInfo(svrId, svrId, asId, asName, chassisId, svrName, svrType, svrCode, ipAddr);
		Log.printf(LOG_LV3,"[DATA][OAMS_DB] DataSetServerInfo svrId[%d], asId[%d], asName[%s], chassisId[%d], svrName[%s], svrType[%d], svrCd[%d], IP[%s]\n", 
							svrId, asId, asName, chassisId, svrName, svrType, svrCode, ipAddr);
    }
	Log.printf(LOG_LV2,"[DATA][OAMS_DB] DataSetServerInfo END \n");

    return true;
}

bool DataSetModuleInfo()
{
    vector<map<string, string>> resultData;
    int moduleId;
	int svrCode;
	int svrId;
    const char* moduleName;
    const char* svrName;
	int asId;
	const char* asName;

	Log.printf(LOG_LV2,"[DATA][OAMS_DB] DataSetModuleInfo START \n");
    resultData = pDB->sqlSelect("SELECT m.module_code, m.svr_code, m.module_name, b.BL_IDX, b.bl_name, c.as_idx, a.as_name \
								FROM tbl_module_config m \
								JOIN tbl_blade_config b ON m.svr_code = b.svr_code \
								JOIN tbl_chassis_config c ON b.ch_idx = c.ch_idx	\
								JOIN tbl_as_config a ON c.as_idx = a.as_idx;");

    for (vector<map<string, string>>::iterator it = resultData.begin(); it != resultData.end(); ++it) {
        for (std::map<std::string, std::string>::iterator mit = it->begin(); mit != it->end(); ++mit) {
            if(mit->first == "MODULE_CODE")     moduleId = stoi(mit->second);
			if(mit->first == "SVR_CODE")      	svrCode = stoi(mit->second);
            if(mit->first == "MODULE_NAME")     moduleName = mit->second.c_str();
			if(mit->first == "BL_IDX")			svrId = stoi(mit->second);
            if(mit->first == "BL_NAME")			svrName = mit->second.c_str();
			if(mit->first == "AS_IDX")      	asId = stoi(mit->second);
			if(mit->first == "AS_NAME") 		asName = mit->second.c_str();
        }
        moduleInfoManager.setModuleInfo(asId, moduleId, moduleId, svrCode, svrId, asName, moduleName, svrName);
		Log.printf(LOG_LV3,"[DATA][OAMS_DB] DataSetModuleInfo ASID[%d], MdId[%d], svrCd[%d], svrId[%d], asName[%s], mdName[%s], svrName[%s]\n", 
							asId, moduleId, svrCode, svrId, asName, moduleName, svrName);
    }
	Log.printf(LOG_LV2,"[DATA][OAMS_DB] DataSetModuleInfo END \n");

    return true;
}


bool DataSetServerIdMap()
{
	vector<map<string, string>> resultData;
	stAsModId   key;

    int asid;
    int serverid;
    int chassisid;
    int moduleid;

	Log.printf(LOG_LV2,"[DATA][OAMS_DB] DataSetServerIdMap START \n");
    resultData = pDB->sqlSelect("SELECT c.as_idx, b.bl_idx, b.ch_idx, m.module_code FROM tbl_module_config m JOIN tbl_blade_config b ON m.svr_code = b.svr_code JOIN tbl_chassis_config c ON b.ch_idx = c.ch_idx;");
    for (vector<map<string, string>>::iterator it = resultData.begin(); it != resultData.end(); ++it) {
        for (std::map<std::string, std::string>::iterator mit = it->begin(); mit != it->end(); ++mit) {
            if(mit->first == "AS_IDX")      asid=stoi(mit->second);
            if(mit->first == "BL_IDX")      serverid=stoi(mit->second);
            if(mit->first == "CH_IDX")      chassisid=stoi(mit->second);
            if(mit->first == "MODULE_CODE") moduleid=stoi(mit->second);
        }
        key.asId = asid;
        key.moduleId = moduleid;

		stChassisServerId *value = new stChassisServerId;

        value->chassisId = chassisid;
        value->serverId = serverid;
        xbusid_Map.insert(map<stAsModId, stChassisServerId*>::value_type(key, value));
		Log.printf(LOG_LV3,"[DATA][OAMS_DB] XBUS MAP INSERT KEY:ASID[%d], MODULEID[%d] VALUE:CHASSISID[%d], SERVERID[%d] \n", asid, moduleid, chassisid, serverid);
	}
	Log.printf(LOG_LV2,"[DATA][OAMS_DB] DataSetServerIdMap END \n");

	return true;
}

bool DataRemoveServerIdMap()
{
	Log.printf(LOG_LV2,"[DATA][OAMS_DB] DataRemoveServerIdMap START \n");
	std::map<stAsModId, stChassisServerId*>::iterator it;
	for(it =xbusid_Map.begin(); it != xbusid_Map.end(); ++it){
//	for (auto& pair : xbusid_Map) {
//    	delete pair.second;
    	delete it->second;
	}

	xbusid_Map.clear();
	Log.printf(LOG_LV2,"[DATA][OAMS_DB] DataRemoveServerIdMap END \n");
	
	return true;
}

bool DataSetThreshold()
{
    vector<map<string, string>> resultData;
    int alarmField;
    int critical;
    int major;
    int minor;
	int warning;
	const char* type;	
	int ret;

	Log.printf(LOG_LV2,"[DATA][OAMS_DB] DataSetThreshold START \n");
    resultData = pDB->sqlSelect("SELECT ALARM_FIELD, CRITICAL_LIMIT, MAJOR_LIMIT, MINOR_LIMIT, WARNING_LIMIT FROM TBL_ALARM_LIMIT;");

    for (vector<map<string, string>>::iterator it = resultData.begin(); it != resultData.end(); ++it) {
        for (std::map<std::string, std::string>::iterator mit = it->begin(); mit != it->end(); ++mit) {
            if(mit->first == "ALARM_FIELD")     alarmField 	= stoi(mit->second);
            if(mit->first == "CRITICAL_LIMIT")  critical 	= stoi(mit->second);
            if(mit->first == "MAJOR_LIMIT")     major 		= stoi(mit->second);
            if(mit->first == "MINOR_LIMIT")     minor	 	= stoi(mit->second);
            if(mit->first == "WARNING_LIMIT")   warning 	= stoi(mit->second);

			ret = convertLimitName(alarmField, type);
			if(!ret)	continue;
        }
        thresholdInfoManager.setThresHoldInfo(type, critical, major, minor, warning);
		Log.printf(LOG_LV3,"[DATA][OAMS_DB] DataSetThreshold TYPE[%s], CRITICAL[%d], MAJOR[%d], MINOR[%d], WARNNING[%d]\n", type, critical, major, minor, warning);
    }
	Log.printf(LOG_LV2,"[DATA][OAMS_DB] DataSetThreshold END \n");

    return true;

}

bool DataSetNTP()
{
    vector<map<string, string>> resultData;
    int asId;
    int svrId;
	double offset = 0;
    double offset1; 
    double offset2;
        
    Log.printf(LOG_LV2,"[DATA][OAMS_DB] DataSetNTP START \n");
    resultData = pDB->sqlSelect("SELECT N.AS_IDX, B.bl_idx, ABS(N.OFFSET1) AS OFFSET1, ABS(N.OFFSET2) AS OFFSET2 FROM tbl_ntp_time N \
								JOIN TBL_BLADE_CONFIG B ON N.SVR_CODE = B.SVR_CODE;");
            
    for (vector<map<string, string>>::iterator it = resultData.begin(); it != resultData.end(); ++it) {
        for (std::map<std::string, std::string>::iterator mit = it->begin(); mit != it->end(); ++mit) {
            if(mit->first == "AS_IDX")     	asId  	= stoi(mit->second);
            if(mit->first == "BL_IDX")  	svrId	= stoi(mit->second);
            if(mit->first == "OFFSET1")     offset1	= stod(mit->second);
            if(mit->first == "OFFSET2")     offset2	= stod(mit->second);
        }

		if(offset1 < offset2) 	offset = offset2;
		else					offset = offset1;		

		offset = offset * 1000; // change (ms)

        performanceManager.setNtpPerformance(svrId, (int)offset);
        Log.printf(LOG_LV3,"[DATA][OAMS_DB] DataSetNTP ASID[%d], SERVERID[%d], OFFSET[%d]\n", asId, svrId, (int)offset);
    }
    Log.printf(LOG_LV2,"[DATA][OAMS_DB] DataSelectNTP END \n");

	return true;
}


bool DataSetTableSpace(int svrId)
{
    vector<map<string, string>> resultData;
	int tsIdx = -1;
    const char* tsName;
    int useSpace;
    const char* totalSpace;
    int usePercent;

    Log.printf(LOG_LV2,"[DATA][OAMS_DB] DataSetTableSpace START \n");

    resultData = pDB->sqlSelect("SELECT * from( \
SELECT distinct	\
t.name TABLESPACE_NAME \
,decode(type, 7, round((SELECT(sum(total_extent_count*page_count_in_extent)*page_size)/1024/1024 FROM v$udsegs)+(SELECT(sum(total_extent_count*page_count_in_extent)*page_size)/1024/1024 FROM v$tssegs),2),round(allocated_page_count * page_size / 1024 / 1024, 2)) 'USED_SPACE'	\
, to_char(round(d.max * page_size / 1024 /1024, 2)) 'TOTAL_SPACE' \
, decode(type, 7, round(((SELECT sum(total_extent_count*page_count_in_extent) FROM v$udsegs)+(SELECT sum(total_extent_count*page_count_in_extent) FROM v$tssegs)) / d.max* 100, 2), 3 \
, round((nvl(ds.used, 0))/(d.max*page_size)* 100, 2),4, round((nvl(ds.used, 0))/(d.max*page_size)* 100, 2) \
,round(allocated_page_count / d.max * 100, 2)) 'PERCENTAGE' \
FROM v$datafiles a left outer join v$tablespaces t on a.spaceid=t.id left outer join \
(SELECT space_id, sum(total_used_size) USED FROM x$segment GROUP by space_id) ds on ds.space_id = t.id \
,(SELECT spaceid, sum(decode(maxsize, 0, currsize, maxsize)) as MAX \
FROM v$datafiles GROUP by spaceid) d WHERE t.id = d.spaceid) WHERE TABLESPACE_NAME LIKE 'TS_%' ORDER BY TABLESPACE_NAME ASC;");

    for (vector<map<string, string>>::iterator it = resultData.begin(); it != resultData.end(); ++it) {
        for (std::map<std::string, std::string>::iterator mit = it->begin(); mit != it->end(); ++mit) {
            if(mit->first == "TABLESPACE_NAME")		tsName		= mit->second.c_str();
            if(mit->first == "USED_SPACE")      	useSpace	= stoi(mit->second);
            if(mit->first == "TOTAL_SPACE")     	totalSpace	= mit->second.c_str();
            if(mit->first == "PERCENTAGE")     		usePercent 	= stoi(mit->second);
        }

        performanceManager.setTableSpacePerformance(svrId, ++tsIdx, tsName, usePercent);
        Log.printf(LOG_LV3,"[DATA][OAMS_DB] DataSetTableSpace TABLESPACE[%s], USE_PERCENT[%d], USE_SPACE[%d], TOTAL_SPACE[%s]\n", tsName, usePercent, useSpace, totalSpace);
    }
    Log.printf(LOG_LV2,"[DATA][OAMS_DB] DataSetTableSpace END \n");

    return true;
}


bool DataSetDBSession(int svrId)
{
    vector<map<string, string>> resultData;
    int dbSession;
    const char* maxSession;
	int percentSession;

    Log.printf(LOG_LV2,"[DATA][OAMS_DB] DataSetDBSession START \n");

    resultData = pDB->sqlSelect("SELECT \
    (SELECT COUNT(ID) FROM V$SESSION) AS SessionCnt, \
    (SELECT VALUE1 FROM v$property WHERE NAME = 'MAX_CLIENT') AS MaxClient FROM DUAL;");

    for (vector<map<string, string>>::iterator it = resultData.begin(); it != resultData.end(); ++it) {
        for (std::map<std::string, std::string>::iterator mit = it->begin(); mit != it->end(); ++mit) {
            if(mit->first == "MAXCLIENT")     maxSession  = mit->second.c_str();
            if(mit->first == "SESSIONCNT")    dbSession   = stoi(mit->second);
        }
	
		if(atoi(maxSession) <= 0)
		{
			percentSession = 0;
		}
		else
		{
			percentSession = (int)((double)dbSession/atoi(maxSession) * 100);
		}

        performanceManager.setDBSessionPerformance( svrId, percentSession);
        Log.printf(LOG_LV3,"[DATA][OAMS_DB] DataSetDBSession svrId[%d], dbSession[%d], maxsess[%d], percentSession[%d]\n", svrId, dbSession, atoi(maxSession), percentSession);
    }
    Log.printf(LOG_LV2,"[DATA][OAMS_DB] DataSetDBSession END \n");

    return true;
}

bool DataSetServiceKey()
{   
    vector<map<string, string>> resultData;
    int asSvcKey;
    const char* asSvcCode;
    const char* asSvcName;
    const char* asSvcEng;
    
    Log.printf(LOG_LV2,"[DATA][OAMS_DB] DataSetServiceKey START \n");
    
    resultData = pDB->sqlSelect("SELECT AS_SVC_KEY, AS_SVC_CODE, AS_SVC_NAME, AS_SVC_ENG FROM TBL_AS_SERVICE ORDER BY AS_SVC_CODE;");

    for (vector<map<string, string>>::iterator it = resultData.begin(); it != resultData.end(); ++it) {
        for (std::map<std::string, std::string>::iterator mit = it->begin(); mit != it->end(); ++mit) {
            if(mit->first == "AS_SVC_KEY")		asSvcKey	= stoi(mit->second);
            if(mit->first == "AS_SVC_CODE")     asSvcCode   = mit->second.c_str();
            if(mit->first == "AS_SVC_NAME")     asSvcName 	= mit->second.c_str();
            if(mit->first == "AS_SVC_ENG")      asSvcEng	= mit->second.c_str();
        }   
        
		for(int i = 0; i<MAX_AS ; i++)
		{
        	asstatsInfoManager.setAsStatsInfo(i, asSvcCode);
        	Log.printf(LOG_LV3,"[DATA][OAMS_DB] DataSetServiceKey asSvcKey[%d], asSvcCode[%s], asSvcName[%s], asSvcEng[%s]\n", asSvcKey, asSvcCode, asSvcName, asSvcEng);
		}
    }   
    Log.printf(LOG_LV2,"[DATA][OAMS_DB] DataSetServiceKey END \n");
    
    return true;
}  


int AlarmDataSelect()
{
    vector<map<string, string>> resultData;

	ALARM_CONFIG alarmTempData;

    Log.printf(LOG_LV2,"[DATA][OAMS_DB] AlarmDataSelect START \n");

    resultData = pDB->sqlSelect("SELECT ALARM_FIELD, ALARM_CODE, ALARM_PHASE, ALARM_DESC, ALARM_REPAIR, EXT_SEND_YN FROM TBL_ALARM_CONFIGV2;");

    for (vector<map<string, string>>::iterator it = resultData.begin(); it != resultData.end(); ++it) {
        for (std::map<std::string, std::string>::iterator mit = it->begin(); mit != it->end(); ++mit) {
            if(mit->first == "ALARM_FIELD")   	alarmTempData.AlarmType	= stoi(mit->second);
			if(mit->first == "ALARM_CODE")		alarmTempData.AlarmCode	= stoi(mit->second);
            if(mit->first == "ALARM_PHASE")     alarmTempData.AlarmLevel = stoi(mit->second);
            if(mit->first == "ALARM_DESC")     	strcpy(alarmTempData.AlarmDesc, mit->second.c_str());
			if(mit->first == "ALARM_REPAIR")	strcpy(alarmTempData.AlarmRepair, mit->second.c_str());
            if(mit->first == "EXT_SEND_YN")     alarmTempData.ext_send = stoi(mit->second);
        }



            alarmconfigManager.setAlarmConfig(alarmTempData.AlarmCode, alarmTempData);
            Log.printf(LOG_LV3,"[DATA][OAMS_DB] AlarmDataSelect Code[%d], Type[%d], Level[%d], Desc[%s], ExtSend[%d]\n", 
						alarmTempData.AlarmCode, alarmTempData.AlarmType, alarmTempData.AlarmLevel, alarmTempData.AlarmDesc, alarmTempData.ext_send);
    }

    Log.printf(LOG_LV2,"[DATA][OAMS_DB] AlarmDataSelect END \n");

    return true;
}

int AlarmDataAllDelete()
{
	Log.printf(LOG_LV2,"[DATA][OAMS_DB] AlarmDataDelete START \n");

	pExecDB->sqlSelect("DELETE FROM TBL_ALARM_STATEV2;");

	Log.printf(LOG_LV2,"[DATA][OAMS_DB] AlarmDataDelete END \n");

	return 0;
}

int AlarmDataInsert(ALM_Q_MSG msg)
{
    SqlRequest Request;
    Request.query = "INSERT INTO TBL_ALARM_STATEV2 (ALARM_KEY, ALARM_CODE, AS_IDX, SERVER_IDX, MODULE_IDX, IDX, ALARMED_TIME, ALARM_PHASE, ALARM_DETAIL) VALUES \
                       (?, ?, ?, ?, ?, ?, ?, ?)";
    Request.bindParams = {
							{1, msg.alarmKey},
							{2, msg.alarmId},
							{3, msg.asId},
							{4, msg.serverId},
							{5, msg.moduleId},
							{6, msg.indexId},
							{7, msg.alarmTime},
							{8, msg.alarmLevel},
							{9, msg.comment}
						};

	pExecDB->sqlBindExcute(Request);

	return 0;
}

int AlarmDataDelete(const char* key)
{
	
	SqlRequest Request;
	Request.query = "DELETE FROM TBL_ALARM_STATEV2 WHERE ALARM_KEY = ?";
	
	Request.bindParams = {{1,key}};

	pExecDB->sqlBindExcute(Request);

	return 0;
}

int AlarmLogInsert(ALM_Q_MSG* msg)
{
	char ClearTime[20];
    time_t now = time(NULL);
    struct tm t;
    localtime_r(&now, &t);
    ALM_Q_MSG copy_msg;
    memcpy(&copy_msg, msg, sizeof(ALM_Q_MSG));


    strftime(ClearTime, sizeof(ClearTime), "%Y%m%d%H%M%S", &t);

    SqlRequest Request;
    Request.query = "INSERT INTO TBL_ALARM_LOGV2 (ALARM_KEY, ALARM_CODE, AS_IDX, SERVER_IDX, IDX, ALARMED_TIME,ALARM_PHASE, CLEARED_TIME, ALARM_DETAIL) VALUES \
                       (?, ?, ?, ?, ?, ?, ?, ?, ?)";
    Request.bindParams = {  
                            {1, copy_msg.alarmKey},
                            {2, copy_msg.alarmId},
                            {3, copy_msg.asId},
                            {4, copy_msg.serverId},
                            {5, copy_msg.indexId},
                            {6, copy_msg.alarmTime},
							{7, copy_msg.alarmLevel},
                            {8, ClearTime},
                            {9, copy_msg.comment}
                        };
    
    pExecDB->sqlBindExcute(Request);
    
    return 0;
}

int FaultDataToDB(int asId, int serverId, int indexId, int alarmId, char* alarmedTime, int phase, char* detail)
{   
	char sql[1024];
	
	memset(sql, 0x00, sizeof(sql));
	
    sprintf(sql, "begin sp_InsertFaultLog(%d, %d, %d, %d, 0, '%s', %d, '%s'); end;",
                    alarmId,
                    asId,
                    serverId,
                    indexId,
                    alarmedTime,
                    phase,
                    detail);
 
	pExecDB->sqlSelect(sql);

    return(true);
}


bool CscfDataSelect()
{
	vector<map<string, string>> resultData;
	int asIdx;
	int sswId;
	const char* sswName;
	const char* sswIp;
	const char* sswPort;
	int isBlock;
	int isNormal;
	int isMonitor;
	int status[12];

	int i = 0;
	SSW_CONFIG_INFO sswData;

	resultData = pDB->sqlSelect("SELECT AS_IDX, SSW_ID, SSW_NAME, SSW_IP, SSW_PORT, USE_FLAG, STATUS_FLAG, RUN_FLAG_1, RUN_FLAG_2, RUN_FLAG_3, RUN_FLAG_4, RUN_FLAG_5, RUN_FLAG_6, RUN_FLAG_7, RUN_FLAG_8, RUN_FLAG_9, RUN_FLAG_10, RUN_FLAG_11, RUN_FLAG_12 FROM TBL_SSW_CONFIG WHERE SSW_TYPE = 1 ORDER BY SSW_ID;");

    for (vector<map<string, string>>::iterator it = resultData.begin(); it != resultData.end(); ++it) {
        for (std::map<std::string, std::string>::iterator mit = it->begin(); mit != it->end(); ++mit) {
            if(mit->first == "AS_IDX") 		sswData.asId	= stoi(mit->second);
            if(mit->first == "SSW_ID")  	sswData.index   = stoi(mit->second);
            if(mit->first == "SSW_NAME")    strcpy(sswData.systemName, mit->second.c_str());
            if(mit->first == "SSW_IP")     	strcpy(sswData.ip, mit->second.c_str());
            if(mit->first == "SSW_PORT")   	strcpy(sswData.port, mit->second.c_str());
			if(mit->first == "USE_FLAG")   	isMonitor	= stoi(mit->second);
			if(mit->first == "STATUS_FLAG") isBlock 	= stoi(mit->second);
			if(mit->first == "RUN_FLAG_1")  status[0]   = stoi(mit->second);
			if(mit->first == "RUN_FLAG_2")  status[1]   = stoi(mit->second);
			if(mit->first == "RUN_FLAG_3")  status[2]   = stoi(mit->second);
			if(mit->first == "RUN_FLAG_4")  status[3]   = stoi(mit->second);
			if(mit->first == "RUN_FLAG_5")  status[4]   = stoi(mit->second);
			if(mit->first == "RUN_FLAG_6")  status[5]   = stoi(mit->second);
			if(mit->first == "RUN_FLAG_7")  status[6]   = stoi(mit->second);
			if(mit->first == "RUN_FLAG_8")  status[7]   = stoi(mit->second);
			if(mit->first == "RUN_FLAG_9")  status[8]   = stoi(mit->second);
			if(mit->first == "RUN_FLAG_10") status[9]   = stoi(mit->second);
			if(mit->first == "RUN_FLAG_11") status[10]  = stoi(mit->second);
			if(mit->first == "RUN_FLAG_12") status[11]  = stoi(mit->second);
        }

		sswData.status = status[sswData.asId];
		sswconfigInfoManager.setSswConfigInfo(i++, sswData);
	}

	return true;
}


int CscfDataUpdate(int nASID, int nCSCFID, int runFlag, const char* logTime)
{   
	string QUERY;    

    SqlRequest Request;

    Log.printf(LOG_LV2,"cscfDataUpdate nASID[%d] nCSCFID[%d] runFlag[%d] logTime[%s] \n", nASID, nCSCFID, runFlag, logTime);
	
	QUERY = "UPDATE TBL_SSW_CONFIG SET RUN_FLAG_" + std::to_string(nASID+1) + "=?, UPDATE_DATE=? WHERE SSW_ID=?";
	Log.printf(LOG_LV1, "cscfDataUpdate QUERY[%s] ", QUERY.c_str());

    Request.query = QUERY;
    Request.bindParams = {
						{1,runFlag},
						{2,logTime},
						{3,nCSCFID}
						};
    
    pExecDB->sqlBindExcute(Request);

    return 0;
}

bool MrfDataSelect()
{
	vector<map<string, string>> resultData;
	MS_CONFIG_INFO msData;

	msData.type = EXT_MRF_TYPE; //MRF TYPE 5
	int i = 0;
	
	msconfigInfoManager.Init();

	resultData = pDB->sqlSelect("SELECT AS_IDX, MS_NAME, MS_IP, MS_PORT, BLOCK_FLAG, ALIVE_FLAG, STATUS_FLAG FROM TBL_MS_CONFIG;");

    for (vector<map<string, string>>::iterator it = resultData.begin(); it != resultData.end(); ++it) {
        for (std::map<std::string, std::string>::iterator mit = it->begin(); mit != it->end(); ++mit) {
            if(mit->first == "AS_IDX")		msData.asId    = stoi(mit->second);
            if(mit->first == "MS_NAME")    	strcpy(msData.systemName, mit->second.c_str());
            if(mit->first == "MS_IP")      	strcpy(msData.ip, mit->second.c_str());
            if(mit->first == "MS_PORT")    	strcpy(msData.port, mit->second.c_str());
            if(mit->first == "BLOCK_FLAG")  msData.block_flag   = stoi(mit->second);
            if(mit->first == "ALIVE_FLAG") 	msData.alive_flag  	= stoi(mit->second);
            if(mit->first == "STATUS_FLAG") msData.state_flag   = stoi(mit->second);
        }
        
		msData.index = i;
        msconfigInfoManager.setMsConfigInfo(i++, msData);
    }


	return true;
}

bool CdbDataSelect()
{
    vector<map<string, string>> resultData;
    CDB_STATUS_INFO cdbData;
    int reqCount, respCount;
    int i = 0;

	int ServerID_TEMP;

	//temp
	int group_id, group_seq, block;

    resultData = pDB->sqlSelect("SELECT C.AS_ID, C.GROUP_ID, C.GROUP_SEQ, C.IP, C.PORT, C.STATUS, C.GROUP_NAME, C.ROUTING_BLOCK, C.REQCOUNT, C.RESPCOUNT, (SELECT MAX(E.SVR_ID) \
							    FROM EMSUSER.TBL_EXTERNAL_TYPE_MAPPING E	\
						        WHERE E.EXTERNAL_TYPE = CASE C.SERVER_ID	\
							    WHEN 0 THEN 100	\
							    WHEN 1 THEN 200	\
        END) AS EXTERNAL_SVR_ID FROM EMSUSER.TBL_CDB_STATUS C WHERE C.USE_FLAG = 1 ORDER BY C.AS_ID, C.SERVER_ID, C.GROUP_ID, C.GROUP_SEQ;");

    for (vector<map<string, string>>::iterator it = resultData.begin(); it != resultData.end(); ++it) {
        for (std::map<std::string, std::string>::iterator mit = it->begin(); mit != it->end(); ++mit) {
            if(mit->first == "AS_ID")		cdbData.asId    = stoi(mit->second);
			//if(mit->first == "SERVER_ID")   ServerID_TEMP	= stoi(mit->second);
            if(mit->first == "GROUP_ID")    cdbData.groupId = stoi(mit->second);
            if(mit->first == "GROUP_SEQ")   cdbData.groupSeq = stoi(mit->second);
            if(mit->first == "IP")  		strcpy(cdbData.ip, mit->second.c_str());
            if(mit->first == "PORT")  		strcpy(cdbData.port, mit->second.c_str());
            if(mit->first == "STATUS") 		cdbData.status  = stoi(mit->second);
			if(mit->first == "GROUP_NAME")  strcpy(cdbData.systemName, mit->second.c_str());
			if(mit->first == "ROUTING_BLOCK")	block   = stoi(mit->second);
			if(mit->first == "REQCOUNT")   	reqCount   = stoi(mit->second);
			if(mit->first == "RESPCOUNT")   respCount   = stoi(mit->second);
			if(mit->first == "EXTERNAL_SVR_ID") cdbData.serverID = stoi(mit->second);
        }

        if(reqCount == 0 || reqCount < respCount || reqCount - respCount < 10 )	cdbData.completeRate = 0;
        else{
            cdbData.completeRate = 100 - ((int)((double)respCount/reqCount) * 100);
        }

        cdbstatusInfoManager.setCdbStatusInfo(i++, cdbData);
	}   
	return true;
}

bool ExtDataSelect()
{   
    vector<map<string, string>> resultData;
    EXTERNAL_IF_INFO externData;
    
    int i = 0;
    
    resultData = pDB->sqlSelect("SELECT b.svr_id, b.SYSTEM_NAME, a.AS_IDX, a.EXTERNAL_TYPE, a.EXTERNAL_NAME, a.EXTERNAL_IP, a.EXTERNAL_PORT, a.EXTERNAL_STATE FROM TBL_EXTERNAL_IF_INFO a, tbl_external_type_mapping b WHERE a.EXTERNAL_TYPE = b.EXTERNAL_TYPE;");
    
    for (vector<map<string, string>>::iterator it = resultData.begin(); it != resultData.end(); ++it) {
        for (std::map<std::string, std::string>::iterator mit = it->begin(); mit != it->end(); ++mit) {
            if(mit->first == "AS_IDX")       	externData.asId    = stoi(mit->second);
            if(mit->first == "EXTERNAL_TYPE")   externData.system_type = stoi(mit->second);
            if(mit->first == "EXTERNAL_NAME")   strcpy(externData.systemName, mit->second.c_str());
            if(mit->first == "EXTERNAL_IP")     strcpy(externData.ip, mit->second.c_str());
            if(mit->first == "EXTERNAL_PORT")   strcpy(externData.port, mit->second.c_str());
            if(mit->first == "EXTERNAL_STATE")  externData.status  = stoi(mit->second);
			if(mit->first == "SVR_ID")  		externData.ConnSvrID  = stoi(mit->second);
			if(mit->first == "SYSTEM_NAME")   strcpy(externData.system, mit->second.c_str()); 
        }
        
        externalIfInfoManager.setExternalIfInfo(i++, externData);
    }
    
    return true;
}

int InsertOperatorLog(ORA_Operator_Log *oplog)
{
    SqlRequest Request;
    vector<map<string, string>> resultData;

    int nSeq;

    Log.printf(LOG_LV2,"[DATA][OAMS_DB] InsertOperatorLog START \n");

    resultData = pDB->sqlSelect("SELECT NVL(MAX(LOG_SEQ), 0) + 1 MAX_SEQ FROM TBL_OPERATOR_LOG;");
    for (vector<map<string, string>>::iterator it = resultData.begin(); it != resultData.end(); ++it) {
        for (std::map<std::string, std::string>::iterator mit = it->begin(); mit != it->end(); ++mit) {
            if(mit->first == "MAX_SEQ")      nSeq = stoi(mit->second);
            Log.printf(LOG_LV2,"[DATA][OAMS_DB] MAX_SEQ=[%d] \n", nSeq);
        }
    }

    Request.query = "INSERT INTO TBL_OPERATOR_LOG (LOG_SEQ, LOG_DATE, G_ID, OP_ID, COMMAND, COMMENTS, RESULT, KEYWORD, OP_IP) \
                     VALUES  \
                     (?, ?, ?, ?, ?, ?, ?, ?, ?)";

    Request.bindParams = {
        {1, nSeq},
        {2, oplog->log_time},
        {3, "ADMIN"},
        {4, oplog->op_id},
        {5, oplog->command},
        {6, oplog->comments},
        {7, oplog->result},
        {8, oplog->keyword},
        {9, oplog->op_ip}
    };


    pExecDB->sqlBindExcute(Request);
    Log.printf(LOG_LV2,"[DATA][OAMS_DB] InsertOperatorLog END \n");

    return 0;
}


int RscStatInsert(int asId, int svrCode, char* DBTime, SYSTEM_RESOURCE_STAT RscStat)
{
	if(RscStat.cpu_summery.count <= 0)   
	{
		RscStat.cpu_summery.max = 0;
		RscStat.cpu_summery.min = 0;
		RscStat.cpu_summery.avg = 0;
		strcpy(RscStat.cpu_summery.peaktime, "-");
	}
	else	RscStat.cpu_summery.avg = (int)((double)RscStat.cpu_summery.total / RscStat.cpu_summery.count + 0.5);

	if(RscStat.mem_summery.count <= 0)
	{
		RscStat.mem_summery.max = 0;
		RscStat.mem_summery.min = 0;
		RscStat.mem_summery.avg = 0;
		strcpy(RscStat.mem_summery.peaktime, "-");
	}
	else 	RscStat.mem_summery.avg = (int)((double)RscStat.mem_summery.total / RscStat.mem_summery.count + 0.5);

	if(RscStat.net_summery.count <= 0)
	{
		RscStat.net_summery.max = 0;
		RscStat.net_summery.min = 0;
		RscStat.net_summery.avg = 0;
		strcpy(RscStat.net_summery.peaktime, "-");
	}
	else	RscStat.net_summery.avg = (int)((double)RscStat.net_summery.total / RscStat.net_summery.count + 0.5);

	if(RscStat.disk_summery.count <= 0)
	{
		RscStat.disk_summery.max = 0;
		RscStat.disk_summery.min = 0;
		RscStat.disk_summery.avg = 0;
		strcpy(RscStat.disk_summery.peaktime, "-");
	}
	else	RscStat.disk_summery.avg = (int)((double)RscStat.disk_summery.total / RscStat.disk_summery.count + 0.5);

    SqlRequest Request;
    Request.query = "INSERT INTO  TBL_SERVER_LOG(AS_IDX, SVR_CODE, LOG_TIME, CPU_MAX, CPU_MIN, CPU_AVG, CPU_PEAK, MEM_MAX, MEM_MIN, MEM_AVG, MEM_PEAK, \
					NET_MAX, NET_MIN, NET_AVG, NET_PEAK, DISK_MAX, DISK_MIN, DISK_AVG, DISK_PEAK) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
    Request.bindParams = {
                            {1, asId},
                            {2, svrCode},
                            {3, DBTime},
                            {4, RscStat.cpu_summery.max},
                            {5, RscStat.cpu_summery.min},
                            {6, RscStat.cpu_summery.avg},
                            {7, RscStat.cpu_summery.peaktime},
                            {8, RscStat.mem_summery.max},
							{9, RscStat.mem_summery.min},
							{10, RscStat.mem_summery.avg},
							{11, RscStat.mem_summery.peaktime},
							{12, RscStat.net_summery.max},
							{13, RscStat.net_summery.min},
							{14, RscStat.net_summery.avg},
							{15, RscStat.net_summery.peaktime},
							{16, RscStat.disk_summery.max},
							{17, RscStat.disk_summery.min},
							{18, RscStat.disk_summery.avg},
							{19, RscStat.disk_summery.peaktime}
                        };

    pExecDB->sqlBindExcute(Request);

    return 0;
}

int TemperStatInsert(int asId, int svrCode, char* DBTime, STAT_SUMMERY tempStat)
{
	if(tempStat.count <= 0)
	{
		tempStat.min = 0;
		tempStat.avg = 0;
		tempStat.max = 0;
	}
	tempStat.avg = (int)((double)tempStat.total / tempStat.count + 0.5);

    SqlRequest Request;
    Request.query = "INSERT INTO TBL_DELLTEMPERATURE_LOG (AS_IDX, SVR_CODE, LOG_TIME, MIN_TEMPERATURE, MAX_TEMPERATURE, AVG_TEMPERATURE) VALUES(?, ?, ?, ?, ?, ?)";
    Request.bindParams = {  
                            {1, asId},
                            {2, svrCode},
                            {3, DBTime},
                            {4, tempStat.min},
                            {5, tempStat.max},
                            {6, tempStat.avg}
                        };
    
    pExecDB->sqlBindExcute(Request);
    
    return 0;
}

int CpsStatInsert(int asId, char* DBTime, AS_STATS asStat)
{
	if(asStat.CpsSummery.count == 0)
	{
		asStat.CpsSummery.min = 0;
		asStat.CpsSummery.avg = 0;
		asStat.CpsSummery.max = 0;
		strcpy(asStat.CpsSummery.peaktime, "-");
	}
	else
	{
		asStat.CpsSummery.avg = (int)((double)asStat.CpsSummery.total / asStat.CpsSummery.count + 0.5);
	}

    SqlRequest Request;
    Request.query = "INSERT INTO TBL_CPS_LOG (AS_IDX, LOG_TIME, CPS_MIN, CPS_AVG, CPS_MAX, CPS_PEAK) VALUES (?, ?, ?, ?, ?, ?)";
    Request.bindParams = {  
                            {1, asId},
                            {2, DBTime},
                            {3, asStat.CpsSummery.min},
                            {4, asStat.CpsSummery.avg},
                            {5, asStat.CpsSummery.max},
                            {6, asStat.CpsSummery.peaktime}
                        };
    
    pExecDB->sqlBindExcute(Request);
    
    return 0;
}

/*
int SelectTrafficStat(ORA_Traffic_Param *param, int dbAsId)
{
    SqlRequest Request;
    Request.query = "INSERT INTO TBL_ALARM_STATEV2 (ALARM_KEY, ALARM_CODE, AS_IDX, SERVER_IDX, MODULE_IDX, IDX, ALARMED_TIME, ALARM_PHASE, ALARM_DETAIL) VALUES \
                       (?, ?, ?, ?, ?, ?, ?, ?)";
    Request.bindParams = {
                            {1, msg.alarmKey},
                            {2, msg.alarmId},
                            {3, msg.asId},
                            {4, msg.serverId},
                            {5, msg.moduleId},
                            {6, msg.indexId},
                            {7, msg.alarmTime},
                            {8, msg.alarmLevel},
                            {9, msg.comment}
                        };

    pExecDB->sqlBindExcute(Request);

    return 0;
}
*/

/*
bool SvcLimitSelect()
{
    vector<map<string, string>> resultData;
    SERVICE_LIMIT svcLimitData;

    int i = 0;

    resultData = pDB->sqlSelect("SELECT SERVICE_KEY, MIN_CALL_COUNT, SUCCESS_RATE FROM TBL_SERVICE_ALARM_LIMIT;");

    for (vector<map<string, string>>::iterator it = resultData.begin(); it != resultData.end(); ++it) {
        for (std::map<std::string, std::string>::iterator mit = it->begin(); mit != it->end(); ++mit) {
            if(mit->first == "SERVICE_KEY")			strcpy(svcLimitData.ServiceName, mit->second.c_str());
            if(mit->first == "MIN_CALL_COUNT")    	svcLimitData.ThreshholdCallCount = stoi(mit->second);
            if(mit->first == "SUCCESS_RATE")       	svcLimitData.SuccessRate = stoi(mit->second);
        }
		thresholdInfoManager.setServiceHoldInfo(i, svcLimitData);
    }

    return true;
}
*/
