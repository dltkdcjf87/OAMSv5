#include "OAMS_xbus.h"

LibQ        MbusQ;      
int         xwait_flag;
pthread_mutex_t xmut = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  xcon = PTHREAD_COND_INITIALIZER;

void WaitForQueue(void)
{
    pthread_mutex_lock(&xmut);
    pthread_cond_wait(&xcon, &xmut);
    pthread_mutex_unlock(&xmut);
}
void set_mmchead(MMCHD* response, MMCHD* request, int len, bool bContinue)
{
    response->Len = len;
    response->MsgID = request->MsgID;
    response->CmdNo = request->CmdNo;
    response->From  = OAMS_A;   //OAMS
    response->To    = request->From;
    response->Type  = bContinue ? MMC_TYPE_CONTINUE : MMC_TYPE_END;
    response->JobNo = request->JobNo;
    response->arg1 = request->arg1;
    response->arg2 = request->arg2;
    response->Time  = (uint32_t)time(NULL);
    response->SockFD= request->SockFD;
}
void set_mmc_response(MMCPDU* msg) {
    msg->Head.Len  = sizeof(MMCHD);
    msg->Head.From = 0xD1;
    msg->Head.Type = MMC_TYPE_END;
    // 응답 전송
    msendsig(sizeof(MMCHD), MMCM, MSG_ID_MMC, (BYTE*)msg);
}

void send_config_mmc(MMCPDU *msg_MMC)
{
    DIS_CONFIG_OAMS  msg_CONFIG_OAMS;
    memset(&msg_CONFIG_OAMS, 0x00, sizeof(msg_CONFIG_OAMS));
    set_mmchead(&msg_CONFIG_OAMS.mmc_head, &msg_MMC->Head, sizeof(msg_CONFIG_OAMS), false);
    msg_CONFIG_OAMS.LogFile    = g_bDEBUG_LOGFILE ;//g_bDEBUG_LOGFILE;
    msg_CONFIG_OAMS.DebugLevel = g_nDebug_level;
    msendsig(sizeof(msg_CONFIG_OAMS), MMCM, MSG_ID_MMC, (BYTE *)&msg_CONFIG_OAMS);
}
#ifdef _USE_NMI
void send_as_version(MMCPDU *msg_MMC)
{
    int i;
    int moId;
    MO_VER mo_ver;
    memset(&mo_ver, 0x00, sizeof(MO_VER));
    moId = msg_MMC->Body[0];
    //SelectModuleVersion(moId, &mo_ver.mover);
    for(i=0; i<mo_ver.mover.count; i++)
    {
        Log.printf(LOG_LV2, "[XBUS] MMC AS_VERSION Get Module Info[%d] %s : %s",
                  i, mo_ver.mover.data[i].module_name, mo_ver.mover.data[i].module_version);
    }
    set_mmchead(&mo_ver.Head, &msg_MMC->Head, (sizeof(MO_VER) - ((30 - mo_ver.mover.count) * sizeof(ORA_MOVER))), false);
    msendsig(mo_ver.Head.Len, MMCM, MSG_ID_MMC, (BYTE *)&mo_ver);
    Log.printf(LOG_LV2, "[XBUS] MMC AS_VERSION Get Module[%d] Count =>%d send Len =>%d", moId, mo_ver.mover.count, mo_ver.Head.Len);
}
#endif
void send_stat_mmc(MMCPDU *msg_MMC)
{
    DIS_STAT_OAMS    msg_STAT_OAMS;
    memset(&msg_STAT_OAMS, 0x00, sizeof(msg_STAT_OAMS));
    set_mmchead(&msg_STAT_OAMS.mmc_head, &msg_MMC->Head, sizeof(msg_STAT_OAMS), false);
    msg_STAT_OAMS.nCMC_Counts = g_curCntClients;
    msendsig(sizeof(msg_STAT_OAMS), MMCM, MSG_ID_MMC, (BYTE *)&msg_STAT_OAMS);
}
int getSideFromServerName(const char* server_name) {
    int side;
    if (strstr(server_name, "SES") != nullptr) {
        side = atoi(&server_name[3]);
    } else {
        if(server_name[3] == 'A') side = 0; 
        else if(server_name[3] == 'B') side = 1;
        else return -1;
       
    }
    Log.printf(LOG_LV2, "[XBUS] getSideFromServerName side[%d] ", side);
    return side;
}
int GetServerCommandCode(const char* serverName, int msgId)
{
    struct CmdMapping {
        const char* prefix;
        int runCmd;
        int killCmd;
        int changeHaCmd;
        int rebootCmd;
        int shutdownCmd;
    };

    const CmdMapping cmdMap[] = {
        {"CMS", CNM_RunPrcCMS, CNM_KillPrcCMS, CNM_ChgHaCMS, CNM_RebootCMS, CNM_ShutdownCMS},
        {"OMS", CNM_RunPrcOMS, CNM_KillPrcOMS, CNM_ChgHaOMS, CNM_RebootOMS, CNM_ShutdownOMS},
        {"DBS", CNM_RunPrcDBS, CNM_KillPrcDBS, CNM_ChgHaDBS, CNM_RebootDBS, CNM_ShutdownDBS},
        {"ATS", CNM_RunPrcATS, CNM_KillPrcATS, CNM_ChgHaATS, CNM_RebootATS, CNM_ShutdownATS},
        {"SES", CNM_RunPrcSES, CNM_KillPrcSES, -1, CNM_RebootSES, CNM_ShutdownSES},
        {"EMS", CNM_RunPrcEMS, CNM_KillPrcEMS, -1, CNM_RebootEMS, CNM_ShutdownEMS},
    };

    for (const auto& entry : cmdMap) {
        if (strncmp(serverName, entry.prefix, 3) == 0) {

            switch (msgId) {
                case CMD_CLIENT_SERVER_START:
                case CMD_CLIENT_PROCESS_START:
                    return entry.runCmd;
                case CMD_CLIENT_SERVER_STOP:
                case CMD_CLIENT_PROCESS_STOP:
                    return entry.killCmd;
                case CMD_CLIENT_SYSTEM_REBOOT:
                    return entry.rebootCmd;
                case CMD_CLIENT_SYSTEM_SHUTDOWN:
                    return entry.shutdownCmd;
                case CMD_CLIENT_SYSTEM_CHANGEHA:
                    return entry.changeHaCmd;
                default:
                    Log.printf(LOG_ERR, "[GetSystemCommandCode] Unknown message ID: %d", msgId);
                    return -1;
            }
        }
    }
    Log.printf(LOG_ERR, "[GetSystemCommandCode] Unknown server type: %s", serverName);
    return -1;
}

void extractModuleBaseName(const char* moduleName, char* result, size_t resultSize) {
    const char* underscorePos = strchr(moduleName, '_'); // '_' 위치 찾기

    if (underscorePos != nullptr) {
        size_t length = underscorePos - moduleName;
        if (length >= resultSize) {
            length = resultSize - 1; // 버퍼 크기 초과 방지
        }
        strncpy(result, moduleName, length);
        result[length] = '\0'; // null-terminate
    } else {
        // '_'가 없으면 전체 복사
        strncpy(result, moduleName, resultSize - 1);
        result[resultSize - 1] = '\0';
    }
}
int AllProcessStartStop(const char* server_name, int msgId, int asId, int sendAsId, int haSide, char *out_comment)
{
    int CmdNo;
    int NumOfProcess = 0;
    int MgmIdx = 0;
    int nMsgId = 0;
    char cmdType[10] = {0};
    RunPsRQ_S send;
    char sectionKey[30] = {0};
    char sectionMgmKey[30] = {0};
    char hostName[20] = {0};
    char strCMD[256] = {0};
    char comments[256] = {0};
    char szNum[3] = {0};
    char szMgmIdx[3] = {0};
    ORA_Operator_Log opLog = {0};  // 필요시 외부에서 넘길 수도 있음

    Log.printf(LOG_LV2, "[AllProcessStartStop] server_name:%s msgId:%d asId:%d sendAsId:%d haSide:%d", server_name, msgId, asId, sendAsId, haSide);

    if(msgId == CMD_CLIENT_SERVER_START || msgId == CMD_CLIENT_PROCESS_START)
    {
        nMsgId = CMD_CLIENT_PROCESS_START;
        strcpy(cmdType, "START");
    } else {  
        nMsgId = CMD_CLIENT_PROCESS_STOP;
        strcpy(cmdType, "STOP");
    }
#ifdef ONESERVER_MODE
    CmdNo = GetServerCommandCode("CMS",nMsgId);
    sprintf(sectionKey, "%s.%s", "CMS", "NUM");
    sprintf(sectionMgmKey, "%s.%s", "CMS", "MGM");
    g_cfg.GetConfigString(sectionKey, szNum);
    g_cfg.GetConfigString(sectionMgmKey, szMgmIdx);
    MgmIdx = atoi(szMgmIdx);
#else
    strncpy(serverName, (const char*)rcv->server_name, 3);
    serverName[3] = '\0';
    CmdNo = GetServerCommandCode(server_name, nMsgId);
    sprintf(sectionKey, "%s.%s", serverName, "NUM");
    sprintf(sectionMgmKey, "%s.%s", serverName, "MGM" );
    g_cfg.GetConfigString(sectionKey, szNum);
    g_cfg.GetConfigString(sectionMgmKey, szMgmIdx);
    MgmIdx = atoi(szMgmIdx);
#endif
    if(CmdNo == -1 ) return FALSE;
    MgmIdx += 1;
    NumOfProcess = atoi(szNum);

    send.head.CmdNo = CmdNo;
    send.side = haSide;

    sprintf(comments, "as%d %s %s", asId, server_name, cmdType);
    strcpy(out_comment, comments);

#ifndef ONESERVER_MODE
    if (strcmp(server_name, "DBS") == 0)
#endif
    {
        send.ps = 99;
        SendMmcMsgToMGM(0, sendAsId, sizeof(RunPsRQ_S), (MMCPDU *)&send);
    }

    Log.printf(LOG_LV2, "[AllProcessStartStop] NumOfProcess[%d]", NumOfProcess);
    for (int i = 1; i <= NumOfProcess; i++) {
        if(i == MgmIdx) {
            Log.printf(LOG_LV2, "[AllProcessStartStop] MsgIdx[%d] MGM SKIP", MgmIdx);
            continue;
        }
        Log.printf(LOG_LV2, "[AllProcessStartStop] ps[%d] Send Mgm", i);
        send.ps = i;
        SendMmcMsgToMGM(0, sendAsId, sizeof(RunPsRQ_S), (MMCPDU *)&send);
        sleep(1);
    }

    return TRUE;
}

int ServerStartStop(int msgId, CLIENT_INFO *rcv)
{
    BYTE asId = asInfoManager.FindAsIdByName((const char*)rcv->as_name);
    BYTE sendAsId = asId;
    BYTE haSide = getSideFromServerName((const char*)rcv->server_name);
    int  nReturn;
    char serverName[4] = {0};
    char cmdType[10] = {0};
    char strCMD[128] = {0};
    char hostName[20] = {0};
    char sectionKey[30] = {0};
    char szComments[128] = {0};
  
    RunPsRQ_S send;
    ORA_Operator_Log opLog;

    memset(&send, 0x00, sizeof(send));
    memset(&opLog, 0x00, sizeof(opLog));

    if(asId == -1 || haSide == -1) return -1; 

    Log.printf(LOG_LV2, "START ServerStartStop() asName=%s serverName=%s----- ", (const char*)rcv->as_name, (const char*)rcv->server_name);

    strncpy(serverName, (const char*)rcv->server_name, 3);
    serverName[3] = '\0';
    if(msgId == CMD_CLIENT_SERVER_START) strcpy(cmdType, "START");
    else if (msgId == CMD_CLIENT_SERVER_STOP) strcpy(cmdType, "STOP");

    nReturn =  AllProcessStartStop((const char*)rcv->server_name, msgId, asId, sendAsId, haSide, szComments);
    if(nReturn == FALSE) return FALSE;

    make_strTime(opLog.log_time, time(NULL));
    strcpy(opLog.op_id, "grafana");
    strcpy(opLog.op_ip, "localhost");
    sprintf(opLog.command, "SERVER Start/Stop");
    strcpy(opLog.comments, szComments);
    sprintf(opLog.result, "OK");
    sprintf(opLog.keyword, "C_S_KILL");
    Log.printf(LOG_LV2, "opLog ServerStartStop() user=%s ip=%s command=%s comments=%s result=%s keword=%s----- ",
              opLog.op_id, opLog.op_ip, opLog.command, opLog.comments, opLog.result, opLog.keyword);
    InsertOperatorLog(&opLog);

    Log.printf(LOG_LV2, "END ServerStartStop() asName=%s serverName=%s----- ", (const char*)rcv->as_name, (const char*)rcv->server_name);

    return TRUE;
}
int ProcessStartStop(int msgId, CLIENT_INFO *rcv)
{
    BYTE asId = asInfoManager.FindAsIdByName((const char*)rcv->as_name);
    BYTE sendAsId = asId;
    BYTE haSide = getSideFromServerName((const char*)rcv->server_name);
    int psId = 0;
    int CmdNo;
    char serverName[4] = {0};
    char baseName[10] = {0};
    char cmdType[10] = {0};
    char strCMD[128] = {0};
    char sectionKey[30] = {0};
    char szPsId[3] = {0};
    char hostName[20] = {0};
    RunPsRQ_S send;
    ORA_Operator_Log opLog;

    memset(&send, 0x00, sizeof(send));
    memset(&opLog, 0x00, sizeof(opLog));

    if(asId == -1 || haSide == -1) return FALSE; 

    strncpy(serverName, (const char*)rcv->server_name, 3);
    serverName[3] = '\0';
    extractModuleBaseName((const char*)rcv->module_name, baseName, sizeof(baseName));

    if(msgId == CMD_CLIENT_PROCESS_START) strcpy(cmdType, "START");
    else if (msgId == CMD_CLIENT_PROCESS_STOP) strcpy(cmdType, "STOP");
 
#ifdef ONESERVER_MODE 
    CmdNo= GetServerCommandCode("CMS", msgId) ;
#else
    CmdNo= GetServerCommandCode((const char*)rcv->server_name, msgId) ;
#endif
    sprintf(sectionKey, "%s.%s", serverName, baseName); 
    g_cfg.GetConfigString(sectionKey, szPsId);
    psId = atoi(szPsId) + 1;
   
    Log.printf(LOG_LV2, "START ProcessStartStop() CmdNo=%d  asName=%s serverName=%s moduleName=%s----- ", CmdNo, (const char*)rcv->as_name, (const char*)rcv->server_name, (const char*)rcv->module_name);

    send.head.CmdNo = CmdNo;
    send.ps = psId;                                                                                               
    send.side = haSide;
                                                                                                                  
    if (strcmp(serverName, "DBS")==0 && strcmp(baseName, "MMDB")==0)  
    {                                                                                                             
        send.ps = 99;
        SendMmcMsgToMGM(0, sendAsId, sizeof(RunPsRQ_S), (MMCPDU *)&send);
        Log.printf(LOG_LV2, "SEND ProcessStartStop() send.head.CmdNo=%d ps=%d side=%d moduleName=%s-----", send.head.CmdNo, send.ps, send.side, (const char*)rcv->module_name);
    } else {                                                                                                             
        if(strcmp(baseName, "MGM")==0) Log.printf(LOG_LV2, "ProcessStartStop MGM SKIP-----");
        else {
            Log.printf(LOG_LV2, "SEND ProcessStartStop() send.head.CmdNo=%d ps=%d side=%d moduleName=%s-----", send.head.CmdNo, send.ps, send.side, (const char*)rcv->module_name);
            SendMmcMsgToMGM(0, sendAsId, sizeof(RunPsRQ_S), (MMCPDU *)&send);                                           
        }
    }

    sprintf(opLog.comments, "AS%d  %s %s - %s", asId, (const char*)rcv->server_name, baseName, cmdType);
	make_strTime(opLog.log_time, time(NULL));                                                                     
    strcpy(opLog.op_id, "grafana");
    strcpy(opLog.op_ip, "localhost");
    sprintf(opLog.command, "PROCESS Start/Stop");                                                                
    sprintf(opLog.result, "OK");                                                                                
    sprintf(opLog.keyword, "C_P_KILL");                                                                           
    Log.printf(LOG_LV2, "opLog ProcessStartStop() user=%s ip=%s command=%s comments=%s result=%s keword=%s-----",
              opLog.op_id, opLog.op_ip, opLog.command, opLog.comments, opLog.result, opLog.keyword);
    InsertOperatorLog(&opLog);                                                                                    

    Log.printf(LOG_LV2, "END ProcessStartStop() asName=%s serverName=%s moduleName=%s-----", (const char*)rcv->as_name, (const char*)rcv->server_name, (const char*)rcv->module_name);
    return(TRUE);                                                                                                 
} 
int ServerChangeHa(int msgId, CLIENT_INFO *rcv)
{
    BYTE asId = asInfoManager.FindAsIdByName((const char*)rcv->as_name);
    BYTE sendAsId = asId;
    BYTE haSide = getSideFromServerName((const char*)rcv->server_name);
    int serverHa = serverInfoManager.FindServerHaByName((const char*)rcv->server_name);
    int CmdNo;
    char serverName[4] = {0};
    ChangeHaRQ_S send;
    ORA_Operator_Log opLog;

    Log.printf(LOG_LV2, "START ServerChangeHa() asName=%s serverName=%s-----", (const char*)rcv->as_name, (const char*)rcv->server_name);

    memset(&send, 0x00, sizeof(send));
    memset(&opLog, 0x00, sizeof(opLog));

    strncpy(serverName, (const char*)rcv->server_name, 3);
    serverName[3] = '\0';

#ifdef ONESERVER_MODE 
    CmdNo= GetServerCommandCode("CMS", msgId) ;
#else
    CmdNo= GetServerCommandCode((const char*)rcv->server_name, msgId) ;
#endif
    send.head.CmdNo = CmdNo;

    SendMmcMsgToMGM(0, sendAsId, sizeof(ChangeHaRQ_S), (MMCPDU *)&send);

    if (serverHa == -1) {
        sprintf(opLog.command, "SERVER Change HA");
    } else if (haSide == 0 && serverHa == 3) {
        sprintf(opLog.command, "SB[%d] SERVER Change HA %s Active A->B", asId, serverName);
    } else {
        sprintf(opLog.command, "SB[%d] SERVER Change HA %s Active B->A", asId, serverName);
    }

    make_strTime(opLog.log_time, time(NULL));
    strcpy(opLog.op_id, "grafana");
    strcpy(opLog.op_ip, "localhost");
    sprintf(opLog.comments, "as%d %s", asId, serverName);
    sprintf(opLog.result, "OK");
    sprintf(opLog.keyword, "C_A_RELOCATE");
    Log.printf(LOG_LV2, "opLog ServerChangeHa() user=%s ip=%s command=%s comments=%s result=%s keword=%s-----",
              opLog.op_id, opLog.op_ip, opLog.command, opLog.comments, opLog.result, opLog.keyword);
    InsertOperatorLog(&opLog);
    Log.printf(LOG_LV2, "END ServerChangeHa() asName=%s serverName=%s-----", (const char*)rcv->as_name, (const char*)rcv->server_name);

    return TRUE;
}
int ServerShutdownOrReboot(int msgId, CLIENT_INFO *rcv)
{
    BYTE asId = asInfoManager.FindAsIdByName((const char*)rcv->as_name);
    BYTE sendAsId = asId;
    BYTE haSide = getSideFromServerName((const char*)rcv->server_name);
    int  CmdNo;
    char serverName[4] = {0};
    char serverSideName[10] = {0};
    //char hostName[20] = {0};
    char sectionKey[30] = {0};
    char strCMD[128] = {0};
    char cmdType[10] = {0};
    char szComments[128] = {0};

    ORA_Operator_Log opLog;
    ShutdownRebootRQ_S send;

    Log.printf(LOG_LV2, "START ServerShutdownOrReboot() asName=%s serverName=%s-----", (const char*)rcv->as_name, (const char*)rcv->server_name);

    memset(&opLog, 0x00, sizeof(opLog));

    strncpy(serverName, (const char*)rcv->server_name, 3);
    serverName[3] = '\0';

    //sprintf(sectionKey, "ETC.%s", rcv->server_name);
    //g_cfg.GetConfigString(sectionKey, hostName);
    
    //Log.printf(LOG_LV2, "ServerShutdownOrReboot() sectionKey=%s hostName=%s-----", sectionKey, hostName);
    //if(strcmp(hostName, "")==0) return FALSE;

    AllProcessStartStop((const char*)rcv->server_name, msgId, asId, sendAsId, haSide, szComments);

    //if (msgId == CMD_CLIENT_SYSTEM_SHUTDOWN) strcpy(cmdType, "poweroff");
    //if (msgId == CMD_CLIENT_SYSTEM_REBOOT) strcpy(cmdType, "reboot");
    //sprintf(strCMD, "ssh %s 'sync;%s'", hostName, cmdType);
    //Log.printf(LOG_LV2, "ServerShutdownOrReboot() strCMD[%s]-----", strCMD);
    
#ifdef ONESERVER_MODE 
    CmdNo= GetServerCommandCode("CMS", msgId) ;
#else
    CmdNo= GetServerCommandCode((const char*)rcv->server_name, msgId) ;
#endif
    send.head.CmdNo = CmdNo;
    send.side = haSide;
    SendMmcMsgToMGM(0, sendAsId, sizeof(ShutdownRebootRQ_S), (MMCPDU *)&send);                                           
    //system(strCMD);

    sprintf(opLog.comments, "%s", strCMD);
    make_strTime(opLog.log_time, time(NULL));
    strcpy(opLog.op_id, "grafana");
    strcpy(opLog.op_ip, "localhost");
    sprintf(opLog.command, "SERVER %s", (msgId == CMD_CLIENT_SYSTEM_SHUTDOWN) ? "Shutdown" : "Reboot");
    sprintf(opLog.result, "OK");
    sprintf(opLog.keyword, "C_S_KILL");
    InsertOperatorLog(&opLog);
    Log.printf(LOG_LV2, "opLog ServerShutdownOrReboot() user=%s ip=%s command=%s comments=%s result=%s keword=%s-----",
              opLog.op_id, opLog.op_ip, opLog.command, opLog.comments, opLog.result, opLog.keyword);

    Log.printf(LOG_LV2, "END ServerShutdownOrReboot() asName=%s serverName=%s-----", (const char*)rcv->as_name, (const char*)rcv->server_name);
    return TRUE;
}
int AsSESBlock(int msgId, CLIENT_INFO *rcv)
{
    const char* boardNo = (const char*)rcv->server_name + 4;  
    char cmdType[10] = {0};
    BYTE asId = asInfoManager.FindAsIdByName((const char*)rcv->as_name);
    BlockRQ_S send;
    ORA_Operator_Log opLog;

    Log.printf(LOG_LV2, "START AsSESBlockRQ() asName=%s serverName=%s-----", (const char*)rcv->as_name, (const char*)rcv->server_name);

    memset(&opLog,0x00, sizeof(opLog));
    memset(&send,0x00, sizeof(BlockRQ_S));

    if (msgId==CMD_CLIENT_SES_BLOCK) {
        send.head.CmdNo = CNM_BlockSCM;
        strcpy(cmdType, "BLOCK");
    }else if (msgId==CMD_CLIENT_SES_UNBLOCK) {
        send.head.CmdNo = CNM_UnblockSCM;  
        strcpy(cmdType, "UNBLOCK");
    }
    send.side = 2;      //ALL
    send.board = atoi(boardNo) ;    //SES0~SES15

    SendMmcMsgToMGM(0, asId, sizeof(BlockRQ_S), (MMCPDU *)&send);
    make_strTime(opLog.log_time, time(NULL));
    strcpy(opLog.op_id, "grafana");
    strcpy(opLog.op_ip, "localhost");
    sprintf(opLog.command, "SES BLOCK/UNBLOCK", send.board);
    sprintf(opLog.comments, "%s - %s", (const char*)rcv->server_name, cmdType);
    sprintf(opLog.result, "OK");
    sprintf(opLog.keyword, "C_SES_BLOCK");
    InsertOperatorLog(&opLog);
    Log.printf(LOG_LV2, "opLog AsSESBlockRQ() user=%s ip=%s command=%s comments=%s result=%s keword=%s-----",
              opLog.op_id, opLog.op_ip, opLog.command, opLog.comments, opLog.result, opLog.keyword);

    Log.printf(LOG_LV2, "END AsSESBlockRQ() asName=%s serverName=%s-----", (const char*)rcv->as_name, (const char*)rcv->server_name);
    return TRUE;
}
void xbus_process_message(int len, void *rmsg_p)
{
    XBUS_MSG        *x_p = (XBUS_MSG*)rmsg_p;
    PACK_WAPM_HEAD *wapm_msg = (PACK_WAPM_HEAD*)x_p->Data;
    PACK_RESPONSE wapm_response;
    //ALM_MSG *rcv = (ALM_MSG*)x_p->Data;
    ALM_MSG_V2 *rcv = (ALM_MSG_V2*)x_p->Data;
    MMCPDU  *msg_MMC = (MMCPDU*)x_p->Data;
    SET_CONFIG_OAMS *msg_CONFIG_OAMS = (SET_CONFIG_OAMS*)x_p->Data;
    PACK_ALARM_CONFIG *alarm_config_msg = (PACK_ALARM_CONFIG*)((char*)wapm_msg+sizeof(PACK_WAPM_HEAD));
#ifdef _USE_NMI
    PACK_ALARM_CONFIG2 *alarm_config_msg2 = (PACK_ALARM_CONFIG2*)((char*)wapm_msg+sizeof(PACK_WAPM_HEAD));
#endif
    CLIENT_INFO client_msg;
    SERVER_INFO serverData;
    MMCMsgRP_S  send;

    char *client_info_start = (char*)x_p->Data + sizeof(PACK_WAPM_HEAD);
    char service_key[101] = {0}; 
    char    comment[8][64]={0};
    int svrId = moduleInfoManager.FindSvrIdByModuleId(rcv->header.From);
    int nReturn;

    Log.printf(LOG_LV2, "[XBUS] xbus_process_message MsgId :%d ", x_p->hdr.MsgId);

    serverInfoManager.getServerInfo(svrId, serverData);

    switch(x_p->hdr.MsgId)
    {
        case MSG_ID_MMC:
            Log.printf(LOG_LV2, "[XBUS] MSG_ID_MMC cmdNo :%d ", msg_MMC->Head.CmdNo);
            switch(msg_MMC->Head.CmdNo)
            {
                case MSG_DIS_CONFIG:
                    Log.printf(LOG_LV2, "[XBUS] MMC Dispaly Config Comaand Request");
                    send_config_mmc(msg_MMC);
                    break;
                case MSG_DIS_STAT:
                    Log.printf(LOG_LV2, "[XBUS] MMC Dispaly Statistics Comaand Request");
                    send_stat_mmc(msg_MMC);
                    break;
                case MSG_SET_CONFIG:
                    g_bDEBUG_LOGFILE = msg_CONFIG_OAMS->LogFile ? TRUE : FALSE;
                    g_nDebug_level = msg_CONFIG_OAMS->DebugLevel;
                    Log.printf(LOG_LV2, "[XBUS] MMC Set Config Command Request... LogFile=%s, Level=%d ", g_bDEBUG_LOGFILE ? "TRUE" : "FALSE", g_nDebug_level);
                    //UpdateLogLevel(MY_SIDE, g_bDEBUG_LOGFILE ? g_nDebug_level : 0); -- TBL_MODULE_SETTING UPDATE 
                    Log.set_level(g_nDebug_level); 
                    send_config_mmc(msg_MMC);
                    break;
                case MSG_ALARM_SWITCH :
                    g_Alarm_Flag = msg_MMC->Body[0];
                    Log.printf(LOG_LV2, "[XBUS] MMC MSG_ALARM_SWITCH[%d][%d]", msg_MMC->Head.Len, msg_MMC->Body[0]);
                    set_mmc_response(msg_MMC);
                    break;
                case MSG_READ_SMSCFG:
                    Log.printf(LOG_LV2, "[XBUS] MMC MSG_READ_SMSCFG receive ");
#ifdef DF_SMS_SEND
                    set_mmc_response(msg_MMC);
                    //ReadSMSConfig();
#endif
                    break;
                case MSG_READ_CSCFCFG:
                    Log.printf(LOG_LV2, "[XBUS] MMC MSG_READ_CSCFCFG receive ");
                    set_mmc_response(msg_MMC);
                    externServerManager.InitCSCFConfig();
                    fixMetric.RemoveMetricName("get_extern_server_list_info", "", gFixPort);
                    break;
                case MSG_READ_SERVICE:
                    Log.printf(LOG_LV2, "[XBUS] MMC MSG_READ_SERVICE receive ");
                    set_mmc_response(msg_MMC);
                    memcpy(service_key, (char*)x_p->Data + sizeof(PACK_WAPM_HEAD) + 6, 100); 
                    clean_str(service_key);
                    DataSetServiceKey();
                    fixMetric.RemoveMetricByPartialLabel("get_service_graph_info", "SERVICE", service_key, "", gFixPort);
                    msendsig(sizeof(MMCHD), MMCM, MSG_ID_MMC, (BYTE *)msg_MMC);
                    break;
#ifdef _USE_NMI
                case MSG_GET_ASVER: /*Get AS Version */
                    Log.printf(LOG_LV2, "[XBUS] MMC AS_VERSION  =%d ", msg_HEALTH_OAMS->asId);
                    send_as_version(msg_MMC);
                    break;
#endif
            } //switch end(inner)
            break;
        case MSG_ID_STS:  // MMCM EMS GWS Performance .
            //if (!is_db_connect) break;
            if (MMCM_A+MY_SIDE*16==x_p->hdr.From) //MMCM xbusid a or b
            {
                Log.printf(LOG_LV2, "[XBUS] MSG_ID_STS EMS Performance data received. XBus-From=%x, MMC-From=%x ", x_p->hdr.From, ((MMCPDU*)x_p->Data)->Head.From);
                RcvStsMsgFromMGM(MAX_AS, len-sizeof(XB_HEAD), (MMCPDU*)x_p->Data);
            }
            else
            {
                Log.printf(LOG_LV2, "[XBUS] XBUS_MSG_ID_STS_NOTMATCH_ID!!!%02x||%02x ", MMCM_A+MY_SIDE*16, x_p->hdr.From);
            }
            break;
        case MMCM_MSG_ID:
            Log.printf(LOG_LV2, "[XBUS] MMCM_MSG_ID ");
            //SendMMCMsgToCMC((char*)x_p->Data, len-sizeof(XB_HEAD));
            memset(&send,0x00, sizeof(MMCMsgRP_S));
            memcpy(send.data, (char*)x_p->Data, len-sizeof(XB_HEAD));
            send.data[len] = '\0'; 
            alarmMNG.RelayOccured(serverData.asId, svrId, x_p->hdr.From, send.data); 
            break;
        case MSG_ID_ALM:
            strcpy(comment[0], rcv->comment[0]);
            Log.printf(LOG_LV2, "[XBUS] MSG_ID_ALM svrId:%d, alarmId:%d rcv->MoId:%d, alarmFlag:%d ",
                       svrId, rcv->alarmId, rcv->header.From, rcv->alarmFlag);
            if(rcv->alarmId >= 1000) {
                alarmMNG.RelayOccured(serverData.asId, svrId, rcv->header.From, comment[0]);  
            } else {
                alarmMNG.SendAlarm(svrId, rcv->header.From, rcv->alarmId, rcv->indexId, rcv->alarmFlag, rcv->commentCnt, rcv->comment);
            }
            break;
        case MSG_ID_FLT:
            strcpy(comment[0], rcv->comment[0]);
            Log.printf(LOG_LV2, "[XBUS] MSG_ID_FLT svrId:%d, alarmId:%d rcv->MoId:%d, alarmFlag:%d, comment:%s ",
                       svrId, rcv->alarmId, rcv->header.From, rcv->alarmFlag, comment);
            alarmMNG.FaultOccured(serverData.asId, svrId, comment[0]);
            break;
        case MSG_ID_STA:
            Log.printf(LOG_LV2, "[XBUS] MSG_ID_STA ");
            break;
        case WAPM_MSG_ID:
            Log.printf(LOG_LV2, "[XBUS] msg income WAPM_MSG_ID subMsgId : 0x%x(%d)----", wapm_msg->id, wapm_msg->id);

            memcpy((char*)&wapm_response.head, (char*)wapm_msg, sizeof(PACK_WAPM_HEAD));
            wapm_response.response = '1';
            msendsig(sizeof(PACK_WAPM_HEAD)+1, WAPM, WAPM_MSG_ID, (BYTE *)&wapm_response);
            if(nReturn == FALSE) Log.printf(LOG_ERR, "[XBUS] #WAPM from web id=%04x failure[%d]", wapm_msg->id, wapm_msg->id);
            else Log.printf(LOG_LV2, "[XBUS] #WAPM from web id=%04x done[%d]", wapm_msg->id, wapm_msg->id);

            if(wapm_msg->id < CMD_NOMS_GET_SERVER_INFO) //WEB
            {
                switch(wapm_msg->id)
                {
                    case CMD_ALARM_LIMIT:
                        DataSetThreshold();
                        fixMetric.RemoveMetricName("get_config_threshold_info", "", gFixPort);
                        break;
                    case CMD_ALARM_CONFIG:
                        AlarmDataSelect();
                        break;
#ifdef _USE_NMI
                    case CMD_ALARM_CONFIG2:
                        //ProcessAlarmConfig2(alarm_config_msg2);
                        break;
                    case CMD_NMS_SEND_FLAG_UPDATE:
                        char *pData;
                        pData = (char *)((char*)wapm_msg+sizeof(PACK_WAPM_HEAD));
                        g_NMS_Alarm_Sendflag = *pData - '0';
                        Log.printf(LOG_LV2, "[XBUS] CMD_NMS_SEND_FLAG_UPDATE : FLAG [%s]", g_NMS_Alarm_Sendflag?"ON":"OFF");
                        //Write_NMS_Sendflag(); //[JIRA AS-216]
                        break;
#endif
                    case CMD_EXTERNAL_SYSTEM_CONFIG :
                         externServerManager.InitExternConfig();
                         fixMetric.RemoveMetricName("get_extern_server_list_info", "", gFixPort);
                        break;
                    case CMD_SWITCH_CONFIG:
                         externServerManager.InitCSCFConfig();
                         fixMetric.RemoveMetricName("get_extern_server_list_info", "", gFixPort);
                        break;
                    case CMD_REGI_CPS_OVERLOAD:
                        Log.printf(LOG_LV2, "[XBUS] CMD_REGI_CPS_OVERLOAD !!!!!!!!!");
                        //RegiCPSLimit();
                        break;
                    case CMD_AS_SERVICE:
                        memcpy(service_key, (char*)x_p->Data + sizeof(PACK_WAPM_HEAD) + 6, 100); 
                        clean_str(service_key);
                        Log.printf(LOG_LV2, "[XBUS] CMD_AS_SERVICE MSG [%s] !!!!!!!!!", service_key);
                        DataSetServiceKey();
                        fixMetric.RemoveMetricByPartialLabel("get_service_graph_info", "SERVICE", service_key, "", gFixPort);
                        break;

                }
            } else if (wapm_msg->id > CMD_CLIENT_MSG_INFO){
                memcpy(client_msg.as_name, client_info_start, 32);
                client_msg.as_name[4] = '\0';
                trim((char*)client_msg.as_name);
                memcpy(client_msg.server_name, client_info_start + 32, 32);
                client_msg.server_name[4] = '\0';
                trim((char*)client_msg.server_name);
                memcpy(client_msg.module_name, client_info_start + 64, 32);
                client_msg.module_name[4] = '\0';
                trim((char*)client_msg.module_name);

                Log.printf(LOG_LV2, "[XBUS] #WAPM client_msg as_name[%s] server_name[%s] module_name[%s]", client_msg.as_name, client_msg.server_name, client_msg.module_name);
                switch(wapm_msg->id)
                {
                    case CMD_CLIENT_SERVER_STOP:
                    case CMD_CLIENT_SERVER_START:
                        nReturn = ServerStartStop(wapm_msg->id, &client_msg);
                        break;
                    case CMD_CLIENT_PROCESS_STOP:
                    case CMD_CLIENT_PROCESS_START:
                        nReturn = ProcessStartStop(wapm_msg->id, &client_msg);
                        break;
                    case CMD_CLIENT_SYSTEM_REBOOT:
                    case CMD_CLIENT_SYSTEM_SHUTDOWN:
                        nReturn = ServerShutdownOrReboot(wapm_msg->id, &client_msg);
                        break;
                    case CMD_CLIENT_SYSTEM_CHANGEHA:
                        nReturn = ServerChangeHa(wapm_msg->id, &client_msg);
                        break;
                    case CMD_CLIENT_SES_BLOCK:
                    case CMD_CLIENT_SES_UNBLOCK:
                        nReturn = AsSESBlock(wapm_msg->id, &client_msg);
                        break;
                }
                if(nReturn == FALSE) Log.printf(LOG_ERR, "[XBUS] #WAPM from web id=%04x failure[%d]", wapm_msg->id, wapm_msg->id);
                else Log.printf(LOG_LV2, "[XBUS] #WAPM from web id=%04x done[%d]", wapm_msg->id, wapm_msg->id);
            }  

            break;

        case OAMS_MSG_ID:
        case SNCM_MSG_ID:
        case CCIM_MSG_ID:
            break;
        default:
            Log.printf(LOG_LV2, "[XBUS] XBus read msg : Undefined MsgID [0x%x]", x_p->hdr.MsgId);
            break;
    }
}
void *proc_received_xbus_message(void *arg)
{
    int     len;
    XBUS_MSG        xbusMsg;
    for(;;)
    {
        if(MbusQ.is_empty())
        {
            xwait_flag = TRUE;
            WaitForQueue();
            xwait_flag = FALSE;
        }
        else
        {
            MbusQ.dequeue(&len, (char *)&xbusMsg);
            xbus_process_message(len, &xbusMsg);
        }
    }
    return(0);
}
void Pthread_send_signal()
{
    if(xwait_flag == TRUE)
    {
        pthread_cond_signal(&xcon);
    }
}
int sbus_callback(int len, BYTE *packet)
{
    return(0);
}
int mbus_callback(int len, BYTE *packet)
{
    MbusQ.enqueue(len, (char *)packet);
    Pthread_send_signal();
    return(0);
}
void moduleEmsGwsStateChange(BYTE xbusId, int state)
{
    int asId = EMSGWS_ID;
    int svrId, ha;
    SERVER_INFO serverData;

    if (!GetServerIdFromXbus(asId, xbusId, svrId)) return;
    serverInfoManager.getServerInfo(svrId, serverData);

    if(serverData.serverStatus.ha == 2) ha = 3;
    else ha = 2;

    Log.printf(LOG_LV2, "[XBUS] Process State Changed - xbusId:%d, ha:%d\n", xbusId, ha);
    moduleInfoManager.setHaModuleInfo(asId, xbusId, ha);
    return;  
}
void startXbusMsg()
{
    Log.printf(LOG_LV2, "[XBUS] startXbusMsg START ");
    pthread_t tid;
    if (pthread_create(&tid, NULL, proc_received_xbus_message, nullptr) != 0)
    {
        Log.printf(LOG_ERR, "[XBUS] Can't create thread for message processor");
        shut_down(40);
    }
    pthread_detach(tid);
}
