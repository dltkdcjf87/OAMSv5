#include "OAMS_monitor.h"

int getMccsState()
{                                                                                                                 
    FILE* pOutput;                                                                                                
    char strBuffer[256];                                                                                          
    int nRead = 0, nResult = 0;                                                                                   
                                                                                                                  
    pOutput = popen("ps aux | grep '/bin ft' | awk -F' ' '{printf $12}'", "r");                                   
    memset(strBuffer, 0x00, sizeof(strBuffer));                                                                   
    if (pOutput)                                                                                                  
    {                                                                                                             
        nRead = fread(strBuffer, sizeof(strBuffer), 1, pOutput);                                                  
        pclose(pOutput);                                                                                          
    } else {
        // popen 실패 시 로깅
        Log.printf(LOG_ERR, "[MonitorMCCS][getMccsState]  failed to open process");
        return CHECKFAIL_TIMEOUT;  // 오류 처리
    }
    Log.printf(LOG_LV1, "[MonitorMCCS][getMccsState] strBuffer=[%s] ", strBuffer);

    if (strBuffer[0]!='\0')                                                                                       
    {                                                                                                             
        //ftAgent       : 1                                                                                           
        //ftStateMon    : 2                                                                                       
        //ftProcMon     : 4                                                                                           
        //ftRuleManager : 8                                                                                       
        if (strstr(strBuffer, "ftAgent"))                                                                         
            nResult += 1;                                                                                         
        if (strstr(strBuffer, "ftStateMon"))                                                                      
            nResult += 2;                                                                                         
        if (strstr(strBuffer, "ftProcMon"))                                                                       
            nResult += 4;                                                                                         
        if (strstr(strBuffer, "ftRuleManager"))                                                                   
            nResult += 8;                                                                                         
    }                                                                                                             

	if(nResult == 8) return ABNORMAL ;
	else return NORMAL;
}    

BYTE getRemoteMccsState()
{
    FILE* pOutput;
    char strBuffer[256];
    int nRead = 0, nResult = 0;

    std::string sshCmd;

    memset(strBuffer, 0x00, sizeof(strBuffer));                                                                   

    if (MY_SIDE == 0) {
        sshCmd = "ssh root@" + std::string(gMccsEmsB) +
                 " \"ps aux | grep '/bin ft'\" | awk -F' ' '{printf $12}'";
    } else {
        sshCmd = "ssh root@" + std::string(gMccsEmsA) +
                 " \"ps aux | grep '/bin ft'\" | awk -F' ' '{printf $12}'";
    }

    pOutput = popen(sshCmd.c_str(), "r");

    if (!pOutput) {
        Log.printf(LOG_ERR, "[MonitorMCCS][getRemoteMccsState] failed to open process");
        return CHECKFAIL_TIMEOUT;
    }

    nRead = fread(strBuffer, sizeof(strBuffer), 1, pOutput);
    pclose(pOutput);

    Log.printf(LOG_LV3, "[MonitorMCCS][getRemoteMccsState] strResult=[%s] ", strBuffer);
    if (strBuffer[0]!='\0')
    {
        //ftAgent       : 1
        //ftStateMon    : 2
        //ftProcMon     : 4
        //ftRuleManager : 8
        if (strstr(strBuffer, "ftAgent"))
            nResult += 1;
        if (strstr(strBuffer, "ftStateMon"))
            nResult += 2;
        if (strstr(strBuffer, "ftProcMon"))
            nResult += 4;
        if (strstr(strBuffer, "ftRuleManager"))
            nResult += 8;                                                                                         
    }                                                                                                             

	//if(nResult == 8) return ABNORMAL ;
    if (!(nResult & 1) || !(nResult & 2) || !(nResult & 4) || !(nResult & 8)) return ABNORMAL;
	else return NORMAL;
}

BYTE GetFdMccsProc(FILE *fp, const char*  moduleName) 
{
	char buf[1024];
	char *find;

    Log.printf(LOG_LV3, "[GetFdMccsProc] ################moduleName=[%s] ", moduleName);
	if(fp == NULL) return CHECKFAIL_TIMEOUT;
	if(moduleName == NULL) return CHECKFAIL_TIMEOUT;
    memset(buf, 0x00, sizeof(buf));

	fseek(fp,0,SEEK_SET);
	while(fgets(buf, sizeof(buf), fp))
    {
        if ((find = strstr(buf, moduleName)) && strstr(find, "Online"))
            return NORMAL;
        else if (find)
            return ABNORMAL;
    }
    return CHECKFAIL_TIMEOUT;
}

BYTE CheckProcRunning(const char* command, const char* moduleName)
{
    FILE* pOutput = popen(command, "r");
    char strBuffer[256];

    memset(strBuffer, 0x00, sizeof(strBuffer));                                                                   

    Log.printf(LOG_LV2, "[MonitorMCCS][CheckProcRunning] module=%s command=%s", moduleName, command);
    if (pOutput)
    {
        fread(strBuffer, sizeof(strBuffer), 1, pOutput);
        pclose(pOutput);
    } else {
        // popen 실패 시 로깅
        Log.printf(LOG_ERR, "[MonitorMCCS][CheckProcRunning] module=%s failed to open process", moduleName);
        return CHECKFAIL_TIMEOUT;  // 오류 처리
    }

    Log.printf(LOG_LV2, "[MonitorMCCS][CheckProcRunning] module=%s strBuffer=[%s]", moduleName, strBuffer);

    if (strBuffer[0] != '\0') {
        return strstr(strBuffer, "Running") ? NORMAL : ABNORMAL;
    }
    return CHECKFAIL_TIMEOUT;
}

BYTE GetMccsProc(const char* moduleName)
{
    const char* command = nullptr;
    char cmdBuffer[256] = {0};

    if (moduleName == nullptr || strlen(moduleName) == 0) {
        return NORMAL;  // 정의되지 않은 모듈 이름은 정상 처리로 간주
    }
    memset(cmdBuffer, 0x00, sizeof(cmdBuffer));

#ifndef _SINGLE_MODE
    snprintf(cmdBuffer, sizeof(cmdBuffer),
             "/opt/EMCas543/bin/ftcli -c \"getProc %s\" | grep \"Runtime information\"",
             moduleName);
    command = cmdBuffer;
#else
    if (strcmp(moduleName, "ALTIBASEDB") == 0) {
        command = "/home/mcpas/bin/shell/altibase_status.sh";
    } else if (strcmp(moduleName, "DEFAULT") == 0) {
        command = "/home/mcpas/bin/shell/was_status.sh";
    } else {
        snprintf(cmdBuffer, sizeof(cmdBuffer),
                 "/opt/EMCas543/bin/ftcli -c \"getProc %s\" | grep \"Runtime information\"",
                 moduleName);
        command = cmdBuffer;
    }
#endif

    if (command != nullptr) {
        return CheckProcRunning(command, moduleName);
    }

    return CHECKFAIL_TIMEOUT;
}


void updateEmsModule(const char* moduleName)
{
	int result, alarmFlag, svrId=0, xbusId=0;
    char    comment[8][64];
    MODULE_INFO module_info;


    memset(&comment, 0x00, sizeof(comment));
    memset(&module_info, 0x00, sizeof(MODULE_INFO));

    xbusId = moduleInfoManager.FindModuleIdByName(moduleName);

    moduleInfoManager.getModuleInfo(EMSGWS_ID, xbusId, module_info);

    if(xbusId == 0) return;

    if (!GetServerIdFromXbus(MAX_AS, xbusId, module_info.serverId)) {
        return;
    }

	result = get_module_state(xbusId);
    if(result == 1) { 
        alarmFlag = 0 ;                                                                                        
        strcpy(comment[1], "UP");
    }else{
        alarmFlag = 1 ;                                                                                        
        strcpy(comment[1], "DOWN");
    }
    strcpy(comment[0], module_info.moduleName);
    if(module_info.status != result) {
        alarmMNG.SendAlarm(svrId, module_info.moduleId, 0, module_info.moduleId, alarmFlag, 2, comment);
    }

    moduleInfoManager.setHaEmsModuleInfo(MY_SIDE, result);
    moduleInfoManager.setStatusModuleInfo(EMSGWS_ID, xbusId, result);

    Log.printf(LOG_LV2, "[MonitorMCCS][updateEmsModule] svrId : %d moduleId : %d moduleName : %s status : %s alarmFlag : %d ",
               svrId, module_info.moduleId, comment[0], comment[1], alarmFlag);
}
void chkEmsModuleState()
{
	int result ; // 0 :ABNORMAL , 1: NORMAL
    char moduleName[MODULE_NAME_LEN];

    Log.printf(LOG_INF, "chkEmsModuleState called, gEmsProcess size: %zu", gEmsProcess.size());
    for (const auto& val : gEmsProcess)
    {
        memset(moduleName, 0x00, sizeof(moduleName));

        if (val.first.empty() || strlen(val.first.c_str()) == 0 || val.first.size() == 0) {
            Log.printf(LOG_INF, "[chkEmsModuleState] gEmsProcess empty");
            continue;
        }

        std::string label;
        if (MY_SIDE == 0) {
            label = val.first + "_A";
        } else {
            label = val.first + "_B";
        }

        strncpy(moduleName, label.c_str(), MODULE_NAME_LEN - 1);
        moduleName[MODULE_NAME_LEN - 1] = '\0'; 
        Log.printf(LOG_LV2, "[MonitorMCCS][chkEmsModuleState] moduleName : %s", moduleName);
        updateEmsModule(moduleName); 
    }  
}

void *MonitorMccs(void *arg)
{
    int svrId;
    int othersvrId;
    int alarmId = 0;
    int xbusId, status, emsaID, emsbID;
    char comment[8][64] = {{0}};
    char strCmd[256];
    char myHost[16];
    char ProcessName[MODULE_NAME_LEN] ={0};
    MODULE_INFO module_info;

    memset(myHost, 0x00, sizeof(myHost));
    memset(strCmd, 0x00, sizeof(strCmd));
    Log.printf(LOG_INF, "[MonitorMccs] START MCCS_USE =%s ", gMccsUse);

    emsaID = serverInfoManager.FindServerIdByName("EMSA");
    emsbID = serverInfoManager.FindServerIdByName("EMSB");

    if(MY_SIDE==0) {
        svrId = emsaID;     
        othersvrId = emsbID;     
    } else { 
        svrId = emsbID;     
        othersvrId = emsaID;     
    }

    if(svrId != -1) serverInfoManager.setHaServerInfo(svrId, 3);
    if(othersvrId != -1) serverInfoManager.setHaServerInfo(othersvrId, 2);
 
    while (1)
    {
        sleep(3);

        chkEmsModuleState();

        if (strcmp(gMccsUse, "TRUE") == 0)
        {
            Log.printf(LOG_LV2, "[MonitorMCCS] MCCS_USE TRUE : %s", strCmd);
            //system(strCmd);
            FILE* fp = fopen("/home/mcpas/log/oams/.MCCS_info", "r");
            if (fp != nullptr)
            {
                BYTE result;
                for (const auto& val : gMccsProcess)
                {
                    if (val.first.empty()) {
                        Log.printf(LOG_ERR, "[MonitorMCCS] gMccsProcess key is empty! Skip...");
                        continue;
                    }
                    Log.printf(LOG_LV1, "[MonitorMCCS] gMccsProcess key=%s, value=%s", val.first.c_str(), val.second.c_str());
                    memset(&comment, 0x00, sizeof(comment));
                    strcpy(ProcessName, val.first.c_str()); 
                    result = GetFdMccsProc(fp, ProcessName);
                    Log.printf(LOG_LV3, "[MonitorMCCS] GetFdMccsProc Process =%s svrId = %d alarmId = %d result = %d", val.first.c_str(), svrId, std::stoi(val.second.c_str()), result);
                    if(result == NORMAL) {
                        strcpy(comment[0], "UP");
                        status = 1;
                    }else {
                        strcpy(comment[0], "DOWN");
                        status = 0;
                    }
                   try {
                       alarmId = std::stoi(val.second.c_str());
                   } catch (const std::exception& e) {
                       Log.printf(LOG_ERR, "[MonitorMCCS] stoi failed: %s", e.what());
                       continue;
                    }
                    xbusId = moduleInfoManager.FindModuleIdByName(ProcessName);
                    moduleInfoManager.getModuleInfo(EMSGWS_ID, xbusId, module_info);
                    Log.printf(LOG_LV1, "[MonitorMCCS] GetFdMccsProc Process =%s befstatus=%d aftstats=%d", ProcessName, module_info.status, status);
                    if(module_info.status != status) {
                        alarmMNG.SendAlarm(svrId, xbusId, alarmId, xbusId, result, 1, comment);
                    }
                    moduleInfoManager.setStatusModuleInfo(EMSGWS_ID, xbusId, status); 
                    moduleInfoManager.setHaModuleInfo(EMSGWS_ID, xbusId, 3); 
                }
                fclose(fp);
            }
            else
            {
                Log.printf(LOG_ERR, "[MonitorMCCS] Failed to open MCCS info file");
                // fallback 알람 전송
                for (const auto& val : gMccsProcess)
                {
                    memset(&comment, 0x00, sizeof(comment));
                    strcpy(comment[0], "Down");
                    strcpy(ProcessName, val.first.c_str()); 
                    xbusId = moduleInfoManager.FindModuleIdByName(ProcessName);
                    alarmMNG.SendAlarm(svrId, xbusId, std::stoi(val.second.c_str()), xbusId, 0, 1, comment);
                }
            }
        }
        else  // MCCS 미사용: Mccs로 직접 확인
        {
            Log.printf(LOG_LV2, "[MonitorMCCS] MCCS_USE FALSE");
            BYTE result;

            for (const auto& val : gMccsProcess)
            {   
                memset(&comment, 0x00, sizeof(comment));
                result = GetMccsProc(val.first.c_str());
                Log.printf(LOG_LV3, "[MonitorMCCS] GetMccsProc Process =%s alarmId = %d result = %d", val.first.c_str(), std::stoi(val.second.c_str()), result);
                if(result == NORMAL) {
                    strcpy(comment[0], "UP");
                }else {
                    strcpy(comment[0], "DOWN");
                }

                xbusId = moduleInfoManager.FindModuleIdByName(ProcessName);
                moduleInfoManager.getModuleInfo(EMSGWS_ID, xbusId, module_info);
                if(module_info.status != status) {
                    alarmMNG.SendAlarm(svrId, xbusId, alarmId, xbusId, result, 1, comment);
                }
                moduleInfoManager.setStatusModuleInfo(EMSGWS_ID, xbusId, status);
                moduleInfoManager.setHaModuleInfo(EMSGWS_ID, xbusId, 3);

            }
        }

        // MCCS 미사용 시 추가로 상태 알림
        if (strcmp(gMccsUse, "FALSE") == 0)
        {
            memset(&comment, 0x00, sizeof(comment));

            BYTE localStatus  = getMccsState();
            BYTE remoteStatus = getRemoteMccsState();

            Log.printf(LOG_LV3, "[MonitorMCCS] getMccsState status = %d", localStatus);
            Log.printf(LOG_LV3, "[MonitorMCCS] getRemoteMccsState status = %d", remoteStatus);

            if(remoteStatus == NORMAL) strcpy(comment[0], "UP");
            else strcpy(comment[0], "DOWN");
            alarmMNG.SendAlarm(othersvrId, NO_PROCESS_ID, ALM_CODE_PROCESS_MCCS, 0, remoteStatus, 1, comment);

            if(localStatus == NORMAL) strcpy(comment[0], "UP");
            else strcpy(comment[0], "DOWN");
            alarmMNG.SendAlarm(svrId, NO_PROCESS_ID, ALM_CODE_PROCESS_MCCS, 0, localStatus, 1, comment);
        }
    }
    Log.printf(LOG_INF, "[MonitorMccs] END ");
}

void startEmsCheckMonitor()
{
    pthread_t   thid;

    Log.printf(LOG_INF, "[startEmsCheckMonitor] START ");
    sleep(3);

    if(pthread_create(&thid, NULL, MonitorMccs, 0) != 0)
    {
        Log.printf(LOG_ERR, "[startEmsCheckMonitor]  Can't create thread ");
        shut_down(30);
    }
    pthread_detach(thid);
}
