#include "OAMS_mgm.h"

int readExact(int fd, char* buffer, int length) {
    int total = 0;
    while (total < length) {
        int n = read(fd, buffer + total, length - total);
        if (n > 0) {
            total += n;
        } else if (n == 0) {
            return 0; // EOF
        } else {
            if (errno == EINTR) {
                continue;
            } else if (errno == EAGAIN) {
                Log.printf(LOG_ERR, "[readExact] EAGAIN: retrying...");
                continue;
            } else {
                Log.printf(LOG_ERR, "[readExact] read failed: errno=%d", errno);
                return -1;
            }
        }
    }
    return total;
}

int my_ReadDataWORD(int fd, char *buf, int rmax) {
    Log.printf(LOG_LV2, "[my_ReadDataWORD] fd=%d START", fd);

    unsigned char header[2];
    if (readExact(fd, (char*)header, 2) != 2) {
        Log.printf(LOG_ERR, "[my_ReadDataWORD] Failed to read length header ");
        return -1;
    }

    unsigned short length = (header[1] << 8) | header[0];  // little endian

    if (length >= rmax) {
        Log.printf(LOG_INF, "[my_ReadDataWORD] Payload too large (%d >= %d), discarding", length, rmax);
        std::vector<char> dummy(length - 2);
        readExact(fd, dummy.data(), length - 2);
        return 1;
    }

    // header already in buf[0,1]
    buf[0] = header[0];
    buf[1] = header[1];

    if (readExact(fd, buf + 2, length - 2) != length - 2) {
        Log.printf(LOG_ERR, "[my_ReadDataWORD] Failed to read payload");
        return -2;
    }

    Log.printf(LOG_LV2, "[my_ReadDataWORD] fd=%d END (len=%d)", fd, length);
    return (int)length;
}
void SetMsgHeaderToMGM(int len, WORD cmdId, MMCHD *head)
{
    Log.printf(LOG_LV2, "[SetMsgHeaderToMGM] cmdId : %d START ", cmdId);
    head->Len    = len;
    head->MsgID  = MSG_ID_MGM;
    head->CmdNo  = cmdId;               // CLIENT TYPE
    head->From   = 0x00;                // don't care
    head->To     = 0x00;                // don't care
    head->Type   = 0;                   // Don't care here
    head->JobNo  = 0;                   // don't care
    head->Time   = (LONG)time(NULL);
    head->SockFD = 0;                   // Don't care
    Log.printf(LOG_LV1, "[SetMsgHeaderToMGM] Len : %d MsgId : %d CmdNo: %d Time : %d ", head->Len, head->MsgID, head->CmdNo, head->Time);
    Log.printf(LOG_LV2, "[SetMsgHeaderToMGM] cmdId : %d END ", cmdId);
}

int SendMmcMsgToMGM(int idx, BYTE asId, int len, MMCPDU *send)
{
    Log.printf(LOG_LV2, "[MGM:%d][SendMmcMsgToMGM] idx : %d asId: %d START ", asId,  idx, asId);
    send->Head.Len   = len;
    send->Head.MsgID = MSG_ID_MMC;
    send->Head.Time  = (LONG)time(NULL);
    send->Head.arg1  = (BYTE)idx;

    Log.printf(LOG_LV1, "[SendMmcMsgToMGM] Len : %d  MsgId: %d Time : %d arg1 : %d  ", send->Head.Len, send->Head.MsgID, send->Head.Time, send->Head.arg1);


    if(TCP_WriteData(gMgmFd[asId], (char *)send, len) <= 0)
    {
        Log.printf(LOG_ERR, "[MGM:%d][SendMmcMsgToMGM] asId:%d SendMmcMsgToMGM write fail reason = %s ", asId, asId, sERROR);
        return false;
    }
    Log.printf(LOG_LV2, "[MGM:%d][SendMmcMsgToMGM] idx : %d asId: %d STOP ", asId,  idx, asId);

    return true;
}
int Send_LoopBack(int asId)
{
    LoopBackRQ_S send;

    Log.printf(LOG_LV2, "[MGM:%d][Send_LoopBack] asId: %d START ", asId, asId);
    memset(&send,0x00, sizeof(LoopBackRQ_S));

    send.head.CmdNo = CNM_LoopBack;
    send.asno = (asId); //ALL
    send.sequence = time(NULL); //ALL

    Log.printf(LOG_LV2, "[MGM:%d][Send_LoopBack] asId: %d END ", asId, asId);
    return(SendMmcMsgToMGM(0, asId, sizeof(LoopBackRQ_S), (MMCPDU *)&send));
}
int Send_HA_Query(int asId)
{
    HARQ_S send;

    memset(&send,0, sizeof(HARQ_S));

    Log.printf(LOG_LV2, "[MGM:%d][Send_HA_Query] asId:%d  START ", asId, asId);
    send.head.CmdNo = CNM_DisHaCMS;
    send.side = 2; //ALL
    SendMmcMsgToMGM(0, asId, sizeof(HARQ_S), (MMCPDU *)&send);

    send.head.CmdNo = CNM_DisHaOMS;
    send.side = 2; //ALL
    SendMmcMsgToMGM(0, asId, sizeof(HARQ_S), (MMCPDU *)&send);

#ifdef _DBS_MODE
    send.head.CmdNo = CNM_DisHaDBS;
    send.side = 2; //ALL
    SendMmcMsgToMGM(0, asId, sizeof(HARQ_S), (MMCPDU *)&send);
#endif

#ifdef ATS_MODE
    send.head.CmdNo = CNM_DisHaATS;
    send.side = 2; //ALL
    SendMmcMsgToMGM(0, asId, sizeof(HARQ_S), (MMCPDU *)&send);
#endif
    Log.printf(LOG_LV2, "[MGM:%d][Send_HA_Query] asId:%d  STOP ", asId, asId);
    return true;
}

void wait_for_event_from_mgm(int asId, int iMgmFd) {
    BYTE rcv[MAX_MGM_DATA_LEN + 1] = {};
    struct timeval TO;
    fd_set rmask;
    int ret;
    time_t tLastLoopBack = time(0x00) + 1;

    Log.printf(LOG_LV2, "[MGM:%d][wait_for_event_from_mgm] asId: %d START sockfd : %d ", asId, asId, iMgmFd);

    g_mgmTimeOut[asId] = 0;

    while (true) {
        FD_ZERO(&rmask);
        FD_SET(iMgmFd, &rmask);

        TO.tv_sec = 3;
        TO.tv_usec = 0;
        ret = select(iMgmFd + 1, &rmask, nullptr, nullptr, &TO);
        
        if (ret < 0) {
            Log.printf(LOG_ERR, "[MGM:%d][wait_for_event_from_mgm] asId:%d-ERR select fail err = %d ", asId, asId, errno);
            close(iMgmFd);
            return;
        }

        if (time(0x00) >= tLastLoopBack) {
            Send_LoopBack(asId);
            tLastLoopBack = time(NULL) + 5;
        }

        if (ret == 0) continue;

        if (FD_ISSET(iMgmFd, &rmask)) {
            memset(rcv, 0x00, sizeof(BYTE) * (MAX_MGM_DATA_LEN + 1));
            ret = my_ReadDataWORD(iMgmFd, reinterpret_cast<char *>(rcv), MAX_MGM_DATA_LEN);
            switch(ret)
            {
                case -2:
                case -1: 
                    Log.printf(LOG_ERR, "[MGM:%d][wait_for_event_from_mgm] asId:%d-ERR Disconnect Ling(ret:%d) sockfd : %d ", asId, asId, ret, iMgmFd);
                    close(iMgmFd);
                    return;
                case  0: 
                    Log.printf(LOG_INF, "[MGM:%d][wait_for_event_from_mgm] asId: %d my_ReadDataWORD() interrupt, fd : %d ", asId, asId, iMgmFd);
                    usleep(100000);
                    continue;
                case  1:
                    Log.printf(LOG_INF, "[MGM:%d][wait_for_event_from_mgm] asId: %d Read Data too BIG > %d ", asId, asId, MAX_MGM_DATA_LEN);
                    continue;
                default: 
                    MGM_MSG mgm_msg;
                    memset(&mgm_msg, 0, sizeof(MGM_MSG));
                    mgm_msg.nASID = asId;
                    mgm_msg.rcvSize = ret;
                    memcpy(mgm_msg.rcv, rcv, sizeof(BYTE)*MAX_MGM_DATA_LEN);
                
                    // insert-Queue
                    g_mgmQue[asId].enqueue(sizeof(MGM_MSG), (char *)&mgm_msg);
                    break;
            }
        }
    }
    close(iMgmFd);
    Log.printf(LOG_LV2, "[MGM:%d][wait_for_event_from_mgm] asId: %d Stop sockfd : %d ", asId, asId, iMgmFd);
}

int send_client_type(int sockfd, BYTE my_type)
{
    MMCPDU  send;
    int     mmc_len;

    mmc_len = sizeof(MMCHD) + 1;

    SetMsgHeaderToMGM(mmc_len, CNM_ClientTypeRP, (MMCHD *)&send);

    send.Body[0] = my_type;

    TCP_WriteData(sockfd, (char *)&send, mmc_len);

    return true;
}
int send_initial_alarm_request(int asId, int sockfd)
{
    MMCPDU  send;
    int     len;

    Log.printf(LOG_LV2, "[MGM:%d][send_initial_alarm_request] asId: %d CNM_InitAlarmRQ SEND sockfd : %d ", asId, asId, sockfd);
    len = sizeof(MMCHD);

    SetMsgHeaderToMGM(len, CNM_InitAlarmRQ, (MMCHD *)&send);

    TCP_WriteData(sockfd, (char *)&send, len);
    return true;
}

void* ConnectAS_MGM(void* arg) 
{
    int asId = *reinterpret_cast<int *>(arg);
    int side = 0;
    BYTE reSts = NORMAL;
    BYTE xbusId;
    BYTE aLevel;
    int aCode = ALARM_AS_DISCONNECT;
    char strCmt[4] = {};
    int check_fail = 0;
    char szIP1[128], szIP2[128];
    char szMgmPort[5 + 1];
    char szRbuf[50 + 1];
    int  rbuf, mgmport;
    int  svrId, nReturn;
    char    comment[8][64];

    Log.printf(LOG_INF, "[MGM:%d][ConnectAS_MGM] START ConnectAS_MGM TRY asId : %d ", asId, asId);
    memset(szMgmPort, 0x00, sizeof(szMgmPort));
    memset(szRbuf, 0x00, sizeof(szRbuf));
    memset(&comment, 0x00, sizeof(comment));
    memset(szIP1, 0x00, sizeof(szIP1));
    memset(szIP2, 0x00, sizeof(szIP2));

    g_cfg.GetConfigString("MGM.MGM_PORT", szMgmPort);
    g_cfg.GetConfigString("MGM.R_BUF", szRbuf);
    rbuf = atoi(szRbuf);
    mgmport = atoi(szMgmPort);
    if (g_cfg.GetMgmInfo(asId, szIP1, szIP2))
    {
        Log.printf(LOG_INF, "[MGM:%d][ConnectAS_MGM] MGM_INFO AsId:%d, IP A: %s, IP B : %s ", asId,  asId, szIP1, szIP2);
    }

    while (true) {
        sleep(1);
        side = !side;
        char *ip = side ? szIP1 : szIP2;
        
        if((gMgmFd[asId]=TCP_MakeClientEx(ip, mgmport, 3))<0)
        {

            Log.printf(LOG_ERR, "[MGM:%d][ConnectAS_MGM] asId: %d-ERROR  Fail(%d) to Connect MGM of [%s:%d] ", asId, asId, gMgmFd[asId], ip, mgmport);
            if (++check_fail > 5) {
                reSts = ABNORMAL;
               
                if(side == 0) xbusId = MGM_A; //MGM_A 
                else xbusId = MGM_B;          //MGM_B 

                if(GetServerIdFromXbus(asId, xbusId, svrId)){
                    alarmMNG.SendAlarm(svrId, xbusId, ALM_CODE_AS_DISCONNET, xbusId, reSts, 0, comment);
                    asInfoManager.setConnectedASInfo(asId, 1);
                }
            }
            continue;
        }
        
        setsockopt(gMgmFd[asId], SOL_SOCKET, SO_RCVBUF, &rbuf, sizeof(rbuf));
        setsockopt(gMgmFd[asId], SOL_SOCKET, SO_SNDBUF, &rbuf, sizeof(rbuf));
        Log.printf(LOG_LV3, "[MGM:%d][ConnectAS_MGM] asId: %d  Connected[%d:%s] fd : %d ", asId, asId, side, ip, gMgmFd[asId]);
        
        send_client_type(gMgmFd[asId], CLIENT_AFSM);
        //send_initial_alarm_request(asId, gMgmFd[asId]);
        check_fail = 0;
        
        reSts = NORMAL;
        for (int i=0;i<2;++i) {
            if(i == 0) xbusId = MGM_A; //MGM_A 
            else xbusId = MGM_B;       //MGM_B 

            //if(GetServerIdFromXbus(asId, xbusId, svrId)){
            //    alarmMNG.SendAlarm(svrId, xbusId, ALM_CODE_AS_DISCONNET, xbusId, reSts, 0, comment);
            //}

        }
        wait_for_event_from_mgm(asId, gMgmFd[asId]);
    }
}
void SetPerfData(BYTE asId, BYTE xbusId, HW_USE *usage, int xoam_state)
{
    int svrId;
    SERVER_INFO serverData; 

    if(asId == -1) return;

    if (!GetServerIdFromXbus(asId, xbusId, svrId)) {
        return;
    }

    serverInfoManager.getServerInfo(svrId, serverData);
    Log.printf(LOG_LV2, "[MGM:%d][SetPerfData] asId:%d START xbusId:%d svrId=[%d] ", asId, asId, xbusId, serverData.serverId);
    Log.printf(LOG_LV1, "[MGM:%d][SetPerfData] asId:%d svrId:%d cpu:%d mem:%d net:%d disk:%d  ",
               asId, asId, svrId, usage->cpu, usage->mem, usage->net, usage->disk);

    performanceManager.setResourcePerformance(svrId, usage->cpu, usage->mem, usage->net, usage->disk); 
    serverInfoManager.setBlockServerInfo(svrId, usage->blocked); 
    systemresourcestatManager.setSystemResourceStat(svrId, "CPU", usage->cpu);
    systemresourcestatManager.setSystemResourceStat(svrId, "MEM", usage->mem);
    systemresourcestatManager.setSystemResourceStat(svrId, "NET", usage->net);
    systemresourcestatManager.setSystemResourceStat(svrId, "DISK", usage->disk);

    Log.printf(LOG_LV2, "[MGM:%d][SetPerfData] asEmsId:%d END xbusId:%d ", asId, asId, xbusId);
}
void SetSvcUseData(BYTE asId, int len, SVC_USE *rcv)
{
    Log.printf(LOG_LV2, "[MGM:%d][SetSvcUseData] asId:%d START count:%d len:%d ", asId, asId, rcv->count, len);

    for(int i=0;i<rcv->count;i++) 
    { 
        Log.printf(LOG_LV1, "[MGM:%d][SetSvcUseData] i:%d SvcKey:%s TotalCh:%ld BusyCh:%ld TotalUsage:%ld TotalSuccess:%ld ", asId, i, rcv->item[i].SvcKey,
                   rcv->item[i].TotalCh, rcv->item[i].BusyCh, rcv->item[i].TotalUsage, rcv->item[i].TotalSuccess);
        asstatsInfoManager.setSvcAsStatsInfo(asId, rcv->item[i].SvcKey, rcv->item[i].TotalCh, rcv->item[i].BusyCh, rcv->item[i].TotalUsage, rcv->item[i].TotalSuccess );
    }
    Log.printf(LOG_LV2, "[MGM:%d][SetSvcUseData] asId:%d END count:%d len:%d ", asId, asId, rcv->count, len);
}

void SetHaData(BYTE asId, BYTE xbusId, int len, HA_USE *rcv)
{
    MODULE_INFO module_info;

    Log.printf(LOG_LV2, "[MGM:%d][SetHaData] asId:%d START xbusId:%d ha:%d ", asId, asId, xbusId, rcv->newState);
    if(asId == -1) return;

    moduleInfoManager.getModuleInfo(asId, xbusId, module_info);
    moduleInfoManager.setHaServerModuleInfo(asId, module_info.serverId, rcv->newState); 
    serverInfoManager.setHaServerInfo(module_info.serverId, rcv->newState);

    Log.printf(LOG_LV1, "[MGM:%d][SetHaData] asId:%d uTopId:%d haValue:%d ", asId, asId, xbusId, rcv->newState);
    Log.printf(LOG_LV2, "[MGM:%d][SetHaData] asId:%d END xbusId:%d ha:%d ", asId, asId, xbusId, rcv->newState);
}
void SetModStsData(BYTE asId, AllMoStsRP *rcv)
{
    BYTE    state;
    int     alarmFlag;
    int     xbusId, svrId, ha;
    char    comment[8][64];
    MODULE_INFO module_info;
    SERVER_INFO serverData;

    if(asId == -1) return;

    for(xbusId = 0 ; xbusId < MAX_MODULE ; xbusId++)
    {

        state = rcv->state[xbusId];
        if(!((state == 0)||(state == 1))) continue ;

        moduleInfoManager.getModuleInfo(asId, xbusId, module_info);
        if (!GetServerIdFromXbus(asId, xbusId, svrId)) {
            continue;
        }
        memset(&comment, 0x00, sizeof(comment));
  
        serverInfoManager.getServerInfo(svrId, serverData);

        if(state==0)  // down 
        {
            alarmFlag = ALARM_ON;
            strcpy(comment[1], "DOWN");
        }
        else        // alive
        {
            alarmFlag = ALARM_OFF;
            strcpy(comment[1], "UP");
        }
        strcpy(comment[0], module_info.moduleName);
        if(module_info.status != state) {
            alarmMNG.SendAlarm(svrId, xbusId, ALM_TYPE_PROCESS, xbusId, alarmFlag, 2, comment);
        }

        if(serverData.serverStatus.ha == 2) ha = 3;
        else ha = 2;
        Log.printf(LOG_LV2, "[MGM:%d][SetModStsData] asId:%d xbusId:%d state:%d ha:%d ", asId, asId, xbusId, state, ha);
        moduleInfoManager.setHaModuleInfo(asId, xbusId, ha);
        moduleInfoManager.setStatusModuleInfo(asId, xbusId, state);
    }
}



void SetBoardUseData(BYTE asId, BYTE xbusId, int len, int cmdNo, void *arg) 
{
    int svrId;
	SERVER_INFO serverData;
    BYTE      uTopId ;
    BD_USE    *rcv_v1;
    BD_USE_V2 *rcv_v2;
    BD_USE_V3 *rcv_v3;

    if(asId == -1) return;

    Log.printf(LOG_LV2, "[MGM:%d][SetBoardUseData] asId:%d START xbusId:%d cmdNo:%d ", asId, asId, xbusId, cmdNo);

    if(cmdNo == CNS_BoardUseRP){
        rcv_v1 = (BD_USE *)arg;
        asstatsInfoManager.setChAsStatsInfo(asId, cmdNo, rcv_v1);
		performanceManager.setChPerformance(asId, cmdNo, rcv_v1);
    } else if(cmdNo == CNS_BoardUseRPL){
        rcv_v2 = (BD_USE_V2 *)arg;
        asstatsInfoManager.setChAsStatsInfo(asId, cmdNo, rcv_v2);
		performanceManager.setChPerformance(asId, cmdNo, rcv_v2);
    } else if(cmdNo == CNS_BoardUseRPB){
        rcv_v3 = (BD_USE_V3 *)arg;
        asstatsInfoManager.setChAsStatsInfo(asId, cmdNo, rcv_v3);
		performanceManager.setChPerformance(asId, cmdNo, rcv_v3);
    }
    Log.printf(LOG_LV2, "[MGM:%d][SetBoardUseData] asId:%d END xbusId:%d cmdNo:%d ", asId, asId, xbusId, cmdNo);
}
int RcvMmcMsgFromMGM(BYTE asId, WORD len, MMCPDU *rcv)
{   
    int cmdNo, ha=0, svrId=0, server_len = 0;
    SERVER_INFO serverData;

    Log.printf(LOG_LV2, "[MGM:%d][RcvMmcMsgFromMGM] asId:%d START cmdNo:%d ", asId, asId, rcv->Head.CmdNo);
    
    cmdNo = rcv->Head.CmdNo;
    switch(cmdNo)
    {
    case CNM_RunPrcCMS:
    case CNM_RunPrcOMS:
    case CNM_RunPrcDBS:
    case CNM_RunPrcSES: 
    case CNM_RunPrcATS:{
        RunPsRP* rp = (RunPsRP*)rcv;
        Log.printf(LOG_LV3, "[MGM:%d][RcvMmcMsgFromMGM] asId:%d Start Process Result From %d Index = %d, Result = %d ", asId, asId, rp->head.From, rp->ps_ptr, rp->result);
        break;
    }
    case CNM_KillPrcCMS:
    case CNM_KillPrcOMS:
    case CNM_KillPrcDBS:
    case CNM_KillPrcSES:
    case CNM_KillPrcATS:{
        KillPsRP* rp = (KillPsRP*)rcv;
        Log.printf(LOG_LV3, "[MGM:%d][RcvMmcMsgFromMGM] asId:%d Stop Process Result From %d Index = %d, Result = %d ", asId, asId, rp->head.From, rp->ps_ptr, rp->result);
        break;        
    }
    case CNM_DisHaCMS:
    case CNM_DisHaOMS:
    case CNM_DisHaDBS:
    case CNM_DisHaATS:{
        HaPsRP* rp = (HaPsRP*)rcv;

        if (!GetServerIdFromXbus(asId, rp->head.From, svrId)) {
            break;
        }

        serverInfoManager.getServerInfo(svrId, serverData);

        server_len = strlen(serverData.serverName);
        if (server_len > 0) {
            char lastChar = serverData.serverName[server_len - 1];

            if (lastChar == 'A') {
                ha = rp->haRoleA;
            } else if (lastChar == 'B') {
                ha = rp->haRoleB;
            } else {
                Log.printf(LOG_ERR, "[MGM:%d][RcvMmcMsgFromMGM] asId:%d DisHA ServerName Error ", asId, asId);
                break;
            }
        } else {
            Log.printf(LOG_ERR, "[MGM:%d][RcvMmcMsgFromMGM] asId:%d DisHA ServerName NULL Error ", asId, asId);
            break;
        }

        Log.printf(LOG_LV3, "[MGM:%d][RcvMmcMsgFromMGM] asId:%d DisHA Process Result From %d Result = %d haA = %d haB = %d ",
                   asId, asId, rp->head.From, rp->result, rp->haRoleA, rp->haRoleB);

        serverInfoManager.setHaServerInfo(svrId, ha); 
        break;         
    }
    case CNM_BlockSCM: 
        Log.printf(LOG_LV3, "[MGM:%d][RcvMmcMsgFromMGM] asId:%d CNM_BlockSCM-ignore ", asId, asId);
        break;                                       
    case CNM_TraceOnSPY:                             
        Log.printf(LOG_LV3, "[MGM:%d][RcvMmcMsgFromMGM] asId:%d CNM_TraceOnSpy ", asId, asId);
        break;            
    case CNM_TraceOffSPY:                          
        Log.printf(LOG_LV3, "[MGM:%d][RcvMmcMsgFromMGM] asId:%d CNM_TraceOffSPY ", asId, asId);
        break;                                                                                    
    case CNM_TraceListSPY:                         
        Log.printf(LOG_LV3, "[MGM:%d][RcvMmcMsgFromMGM] asId:%d CNM_TraceListSPY ", asId, asId);
        break;                                                                                    
    default: 
        Log.printf(LOG_ERR, "[MGM:%d][RcvMmcMsgFromMGM] asId:%d Invalid CmdNo = 0x%x ", asId, asId, rcv->Head.CmdNo);
        break;
    }   
        
    Log.printf(LOG_LV2, "[MGM:%d][RcvMmcMsgFromMGM] asId:%d END cmdNo:%d ", asId, asId, rcv->Head.CmdNo);
    return true;
}
int RcvStsMsgFromMGM(BYTE asId, int len, MMCPDU *rcv)
{
    BYTE    from;

    from = rcv->Head.From;

    Log.printf(LOG_LV2, "[MGM:%d][RcvStsMsgFromMGM] asId:%d cmdNo:%d START ", asId, asId, rcv->Head.CmdNo);
    switch(rcv->Head.CmdNo)
    {
        case CNS_PerfDataRP:
            SetPerfData(asId, from, (HW_USE *)rcv->Body, XOAM_STATE_UP); 
            break;
        case CNS_SvcUseRP:
            SetSvcUseData(asId, len, (SVC_USE *)rcv);        
            break;
        case CNS_HaStsChgRP:
            SetHaData(asId, from, len, (HA_USE *)rcv);   
            break;
        case CNS_BoardUseRP:
            SetBoardUseData(asId, from, len, CNS_BoardUseRP, (BD_USE *)rcv);
            break;
        case CNS_BoardUseRPL:
            SetBoardUseData(asId, from, len, CNS_BoardUseRPL, (BD_USE_V2 *)rcv);    // MGM -> OAMS BD Data 
            break;
        case CNS_BoardUseRPB:
            SetBoardUseData(asId, from, len, CNS_BoardUseRPB, (BD_USE_V3 *)rcv);    // MGM -> OAMS BD Data
            break;
        default:
            Log.printf(LOG_ERR, "[MGM:%d][RcvStsMsgFromMGM] asId:%d Invalid cmdNo:%d ", asId, asId, rcv->Head.CmdNo);
            break;
    }
    Log.printf(LOG_LV2, "[MGM:%d][RcvStsMsgFromMGM] asId:%d cmdNo:%d END ", asId, asId, rcv->Head.CmdNo);

    return true;
}
int RcvMgmMsgFromMGM(BYTE asId, int len, MMCPDU *rcv)
{
    BYTE    from;
    BYTE    reSts  ;
    BYTE    xbusId ;
    BYTE    aLevel ;
    int     aCode ;
    char    strCmt[4] ;
    int     svrId, state;
    char    comment[8][64];

    memset(strCmt,0x00,sizeof(strCmt));
    memset(&comment, 0x00, sizeof(comment));

    Log.printf(LOG_LV2, "[MGM:%d][RcvMgmMsgFromMGM] asId:%d CmdNo : %d START ", asId, asId, rcv->Head.CmdNo);

    from = rcv->Head.From;

    switch(rcv->Head.CmdNo)
    {
        case CNM_ClientTypeRP:
            Log.printf(LOG_LV2, "[MGM:%d][RcvMgmMsgFromMGM] CNM_ClientTypeRP asId:%d CmdNo : 0x%x ", asId, asId, rcv->Head.CmdNo);

            if (!GetServerIdFromXbus(asId, from, svrId)) {
                return 0;
            }
            if (rcv->Body[1] == MMC_TYPE_END) {
                reSts = ABNORMAL;
                state = 0;
            } else {
                reSts = NORMAL;
                state = 1;
            }

            send_initial_alarm_request(asId, gMgmFd[asId]);
            Send_HA_Query(asId);

            asInfoManager.setConnectedASInfo(asId, reSts);
            moduleInfoManager.setStatusModuleInfo(asId, from, state);

            alarmMNG.SendAlarm(svrId, from,  ALM_CODE_AS_DISCONNET, from, reSts, 0, comment);

            break;
        case CNM_AllModStsRP:
            Log.printf(LOG_LV2, "[MGM:%d][RcvMgmMsgFromMGM] CNM_AllModStsRP asId:%d CmdNo : 0x%x ", asId, asId, rcv->Head.CmdNo);
            SetModStsData(asId, (AllMoStsRP *)rcv);
            Send_HA_Query(asId);
            break;
        case CNM_LoopBack:
            Log.printf(LOG_LV2, "[MGM:%d][RcvMgmMsgFromMGM] CNM_LoopBack asId:%d CmdNo : 0x%x ", asId, asId, rcv->Head.CmdNo);
            break;
        default:
            Log.printf(LOG_ERR, "[MGM:%d][RcvMgmMsgFromMGM] Invalid asId:%d CmdNo : 0x%x ", asId, asId, rcv->Head.CmdNo);
            break;
    }
    Log.printf(LOG_LV2, "[MGM:%d][RcvMgmMsgFromMGM] asId:%d CmdNo : %d END ", asId, asId, rcv->Head.CmdNo);

    return true;
}
int RcvAlmMsgFromMGM(BYTE asId, int len, ALM_MSG_V2 *rcv)
{
    int svrId = 0;
    char    comment[8][64]={0};

    if (!GetServerIdFromXbus(asId, rcv->header.From, svrId)) {
        return 0;
    }
    Log.printf(LOG_LV2, "[MGM:%d][RcvAlmMsgFromMGM] asId:%d START  svrId[%d] alarmId[%d] From:%d  indexId:%d alarmFlag[%d] commentCnt[%d] ",
               asId, asId, svrId, rcv->alarmId, rcv->header.From, rcv->indexId, rcv->alarmFlag, rcv->commentCnt);
    if(rcv->alarmId >=1000) {
        strcpy(comment[0], rcv->comment[0]);
        alarmMNG.RelayOccured(asId, svrId, rcv->header.From, comment[0]);
    } else {
        alarmMNG.SendAlarm(svrId, rcv->header.From,  rcv->alarmId, rcv->indexId, rcv->alarmFlag, rcv->commentCnt, rcv->comment);
    }
    Log.printf(LOG_LV2, "[MGM:%d][RcvAlmMsgFromMGM] asId:%d END  svrId[%d] alarmId[%d] From:%d  indexId:%d alarmFlag[%d] commentCnt[%d] ",
               asId, asId, svrId, rcv->alarmId, rcv->header.From, rcv->indexId, rcv->alarmFlag, rcv->commentCnt);
    return 0;

}
int RcvFltMsgFromMGM(BYTE asId, int len, ALM_MSG_V2 *rcv)
{
    int svrId = 0;
    char    comment[8][64]={0};

    Log.printf(LOG_LV2, "[MGM:%d][RcvFltMsgFromMGM] asId:%d START", asId, asId);
    if (!GetServerIdFromXbus(asId, rcv->header.From, svrId)) {
        return 0;
    }
    strcpy(comment[0], rcv->comment[0]);
    alarmMNG.FaultOccured(asId, svrId, comment[0]); 
    Log.printf(LOG_LV2, "[MGM:%d][RcvFltMsgFromMGM] asId:%d END ", asId, asId);

    return 0;
   
}

void RcvDataFromMGM(BYTE asId, int len, MMCPDU *msg)
{
    Log.printf(LOG_LV2, "[MGM:%d][RcvDataFromMGM] asId:%d START msg->Head.MsgId:%d ", asId, asId, msg->Head.MsgID);
    switch(msg->Head.MsgID)
    {
    case MSG_ID_MMC:
        RcvMmcMsgFromMGM(asId, len, msg);
        break;
    case MSG_ID_STS:
        RcvStsMsgFromMGM(asId, len, msg);
        break;
    case MSG_ID_ALM:
        RcvAlmMsgFromMGM(asId, len, (ALM_MSG_V2 *)msg);   
        break;
    case MSG_ID_MGM:
        RcvMgmMsgFromMGM(asId, len, msg);              
        break;
    case MSG_ID_FLT:
        RcvFltMsgFromMGM(asId, len, (ALM_MSG_V2 *)msg); 
        break;
    default:
        Log.printf(LOG_ERR, "[MGM:%d][RcvDataFromMGM] asId:%d ERROR dRcvDataFromMGM Invalid MsgID=0x%x ", asId, asId, msg->Head.MsgID);
        break;
    }
    Log.printf(LOG_LV2, "[MGM:%d][RcvDataFromMGM] asId:%d END msg->Head.MsgId:%d ", asId, asId, msg->Head.MsgID);
}
void *process_MGM_msg(void *arg)
{
    int     *ID;
    int     len, asId, msg_len;
    MGM_MSG mgm_msg;

    ID = (int *)arg;
    asId = *ID;
    memset(&mgm_msg, 0x00, sizeof(mgm_msg));
    Log.printf(LOG_INF, "[MGM:%d][process_MGM_msg] asId:%d START ", asId, asId);
    while(1)
    {
        if (!g_mgmQue[asId].is_empty())
        {
            memset(&mgm_msg, 0, sizeof(MGM_MSG));
            g_mgmQue[asId].dequeue(&len, (char *)&mgm_msg);

            msg_len = mgm_msg.rcvSize;
            RcvDataFromMGM(asId, msg_len, (MMCPDU *)mgm_msg.rcv);
        }
        else
            usleep(3000);
    }
}
void startMGM_IF()
{
    pthread_t   connect_tid[MAX_AS], process_tid[MAX_AS];
    char    szIP1[128], szIP2[128];

    for(int i=0 ; i<MAX_AS ; i++ )
    {
        memset(szIP1, 0x00, sizeof(szIP1));
        memset(szIP2, 0x00, sizeof(szIP2));

        tempASID[i] = i;
        if (g_cfg.GetMgmInfo(i, szIP1, szIP2))
        {
            Log.printf(LOG_INF, "[MGM:%d][startMGM_connect] GetMgmInfo AsId:%d, IP A: %s, IP B : %s ", i, i, szIP1, szIP2);
        }
        if(strlen(szIP1) || strlen(szIP2))
        {
            if (pthread_create(&process_tid[i], 0, process_MGM_msg, (void *)&tempASID[i]))
            {
                shut_down(20);
            }
	        pthread_detach(process_tid[i]);
            if(pthread_create(&connect_tid[i], 0, ConnectAS_MGM, (void *)&tempASID[i]))
            {
                shut_down(21);
            }
            pthread_detach(connect_tid[i]);
        }
    }

}
