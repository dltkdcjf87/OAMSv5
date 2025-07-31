#ifndef __MGMMANNAGER_DEFINE_H__
#define __MGMMANNAGER_DEFINE_H__

#include "OAMS_define.h"
#include "OAMS_common.h"

static int tempASID[MAX_AS];

typedef struct {
    int nASID;
    int rcvSize;
    unsigned char rcv[1024 + 1];
} MGM_MSG;

int readExact(int fd, char* buffer, int length);
int my_ReadDataWORD(int fd, char *buf, int rmax);
void wait_for_event_from_mgm(int asId, int iMgmFd);
void SetMsgHeaderToMGM(int len, WORD cmdId, MMCHD *head);
int SendMmcMsgToMGM(int idx, BYTE asId, int len, MMCPDU *send);
int Send_LoopBack(int asId);
int send_client_type(int sockfd, BYTE my_type);
int send_initial_alarm_request(int asId, int sockfd);

void SetPerfData(BYTE asEmsId, BYTE xbusId, HW_USE *usage, int xoam_state);
void SetSvcUseData(BYTE asId, int len, SVC_USE *rcv);
void SetHaData(BYTE asId, BYTE xbusId, int len, HA_USE *rcv);
void SetModStsData(BYTE asId, AllMoStsRP *rcv);
void SetBoardUseData(BYTE asId, BYTE xbusId, int len, int cmdNo, void *arg);

int RcvMmcMsgFromMGM(BYTE asId, WORD len, MMCPDU *rcv);
int RcvStsMsgFromMGM(BYTE asId, int len, MMCPDU *rcv);
int RcvMgmMsgFromMGM(BYTE asId, int len, MMCPDU *rcv);
int RcvAlmMsgFromMGM(BYTE asId, int len, ALM_MSG_V2 *rcv);
int RcvFltMsgFromMGM(BYTE asId, int len, ALM_MSG_V2 *rcv);

void RcvDataFromMGM(BYTE asId, int len, MMCPDU *msg);

void *process_MGM_msg(void *arg);
void *ConnectAS_MGM(void *arg);  
void startMGM_IF();

#endif //__MGMMANNAGER_DEFINE_H__

