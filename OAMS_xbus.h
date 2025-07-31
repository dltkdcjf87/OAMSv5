#ifndef OAMS_XBUS_H
#define OAMS_XBUS_H

#include "OAMS_define.h"
#include "OAMS_common.h"

// 함수 프로토타입
void WaitForQueue(void);
void set_mmchead(MMCHD* response, MMCHD* request, int len, bool bContinue);
void set_mmc_response(MMCPDU* msg);
void send_config_mmc(MMCPDU *msg_MMC);
void send_as_version(MMCPDU *msg_MMC);
void send_stat_mmc(MMCPDU *msg_MMC);
int getSideFromServerName(const char* server_name);
int GetSystemCommandCode(const char* serverName, int msgId, int& NumOfProcess);
void extractModuleBaseName(const char* moduleName, char* result, size_t resultSize);
int AllProcessStartStop(const char* server_name, int msgId, int asId, int sendAsId, int haSide, char *out_comment);
int ServerStartStop(int msgId, CLIENT_INFO *rcv);
int ProcessStartStop(int msgId, CLIENT_INFO *rcv);
int ServerChangeHa(int msgId, CLIENT_INFO *rcv);
int ServerShutdownOrReboot(int msgId, CLIENT_INFO *rcv);
int AsSESBlock(int msgId, CLIENT_INFO *rcv);
void xbus_process_message(int len, void *rmsg_p);
void *proc_received_xbus_message(void *arg);
void Pthread_send_signal();
int sbus_callback(int len, BYTE *packet);
int mbus_callback(int len, BYTE *packet);
void moduleEmsGwsStateChange(BYTE xbusId, int state);
void startXbusMsg();

#endif // OAMS_XBUS_H
