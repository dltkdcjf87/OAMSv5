#ifndef __OAMS_DEFINE_H__
#define __OAMS_DEFINE_H__

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <mutex>
#include <thread>
#include <map>
#include <vector>
#include <queue>
#include <cstdlib>
#include <chrono>
#include <libnet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <btxbus3e.h>
#include <MMC_head.h>
#include <libsmqueue.h>
#include <sys/resource.h>
#include <deque>
#include <chrono>
#include <utility>
#include <algorithm>
#include <signal.h>
#include <arpa/inet.h>
#include <cctype>

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define CONNSTR "DSN=%s;PORT_NO=%s;UID=%s;PWD=%s"

#define MY_PS_NAME "oams"

#define MAX_AS   	12
#define MAX_AS_EMS  (MAX_AS +1)
#define EMSGWS_ID   MAX_AS
#define	MAX_SERVER	100
#define MAX_CHASSIS 100
#define	MAX_BOARD	30
#define	MAX_MODULE	256
#define MAX_OAMS_CLIENTS 		30
#define	OAMS_A	    0xE1
#define	OAMS_B	    0xF1
#define	MAX_SYS_INT 12 

#define SVC_KEY_LEN     16
#define SVC_NAME_LEN    32
#define	MAX_SVC_USE  	50

#define MAX_HW		8
#define MAX_TS		16
#define MAX_LAN_PORT 32
#define MAX_ALARM   1000
#define MAX_MGM_DATA_LEN         2000
#define MGM_A       0x73
#define MGM_B       0x83
#define MMCM_A      0xE6
#define MMCM_B      0xF6

#define NORMAL       0   
#define ABNORMAL     1    // 장애 발생
#define CHECKFAIL_TIMEOUT  2   // time out 또는 check fail

#define ALARM_AS_DISCONNECT     0x0313
#define IPADDR_LEN 15

#define CFG_AS_IP_A as_ip_a
#define CFG_AS_IP_B as_ip_b
#define CFG_MGM_PORT 5678
#define CFG_R_BUF r_buf
#define CFG_MCCS_USE	        mccs_use

#define	sERROR   strerror(errno)

#define CNM_ASBlockSPY 4004                                                                                    
#define CNM_BlockSCM   2015
#define CNM_UnblockSCM 2016  

#define CNM_TraceOnSPY   4200
#define CNM_TraceOffSPY  4201
#define CNM_TraceListSPY 4202

#define XOAM_STATE_DOWN -1
#define XOAM_STATE_UP   1

#define AS_NAME_LEN     15
#define SERVER_NAME_LEN 15
#define MODULE_NAME_LEN 15
#define SYSTEM_NAME_LEN 40
#define IP_LEN          64
#define PORT_LEN        10

#define MAX_SSW         50
#define MAX_MS          50
#define MAX_EXTERNAL    50
#define MAX_CDB         100
#define MAX_GROUP_SESSION 5

#define EXT_CSCF_TYPE	1
#define EXT_OFCS_TYPE	2
#define EXT_MRF_TYPE	5
#define EXT_CDB_TYPE	6

#define	STATE_NONE	-1
#define STATE_DOWN	0
#define STATE_UP	1

#define HA_UNASSIGN	-1
#define	HA_ACTIVE	2
#define HA_STANDBY	3

#define BLOCK_NORMAL	0
#define	BLOCK_PBLOCK	1
#define	BLOCK_MBLOCK	2
#define	BLOCK_DBLOCK	3

#define REBUILD_NONE	-1
#define	REBUILD_NORMAL	0
#define	REBUILD_REBUILD	1

//////////// ALARM DEFINE

#define ALM_OFF			0
#define ALM_ON			1

#define ALM_LEVEL_WARN	1
#define ALM_LEVEL_MINOR	2
#define ALM_LEVEL_MAJOR	3
#define ALM_LEVEL_CRITICAL 4

#define ALM_CODE_PROCESS_DB	        96
#define ALM_CODE_PROCESS_LISTENER	97
#define ALM_CODE_PROCESS_WAS    	98
#define ALM_CODE_PROCESS_MCCS    	99

#define ALM_CODE_HW_FAN				100
#define ALM_CODE_HW_POWER			101
#define ALM_CODE_HW_DISK			102
#define ALM_CODE_HW_DISKREBUILD		103
#define ALM_CODE_HW_NETWORK			104

#define ALM_CODE_LIMIT_ASOVERLOAD    200
#define ALM_CODE_LIMIT_CPU	201
#define ALM_CODE_LIMIT_MEM	202
#define ALM_CODE_LIMIT_NET	203
#define ALM_CODE_LIMIT_DISK	204
#define ALM_CODE_LIMIT_CHANNEL	205
#define ALM_CODE_LIMIT_TABLESPACE	206
#define ALM_CODE_LIMIT_TEMP	207
#define ALM_CODE_LIMIT_DBSESSION	208
#define ALM_CODE_LIMIT_NTP	209
#define ALM_CODE_LIMIT_CDBRATE	210

#define ALM_CODE_CSCF_DOWN	300
#define ALM_CODE_ISCPM_DOWN	301
#define ALM_CODE_ISCPS_DOWN	302

#define ALM_CODE_MRF_DOWN	307

#define ALM_CMS_OVERLOAD	600
#define ALM_SPY_AS_BLOCK	601
#define ALM_CODE_AS_DISCONNET	602

#define ALM_TYPE_PROCESS	0
#define ALM_TYPE_HW 		1
#define ALM_TYPE_LIMIT 		2
#define ALM_TYPE_SYSTEM 	3
#define ALM_TYPE_SERVICE	4
#define ALM_TYPE_AS 		6

#define EXPORTER_ALARM_ERR    1
#define EXPORTER_ALARM_FAULT  2 
#define EXPORTER_ALARM_RELAY  3 

#define MMCM_MSG_ID         0xD101
#define MSG_DIS_STAT        31000
#define MSG_DIS_CONFIG      31001
#define MSG_SET_CONFIG      31002 
#define MSG_ALARM_SWITCH    31005 
#define MSG_READ_SMSCFG     31010
#define MSG_READ_CSCFCFG    31011
#define MSG_READ_SERVICE    31012
#define MSG_GET_ASVER       31020

#define OAMS_MSG_ID   0xE101
#define SNCM_MSG_ID   0xE103
#define CCIM_MSG_ID   0xE104
#define WAPM_MSG_ID   0xD301

#define NO_PROCESS_ID -1

enum {  CMD_CHASSIS_CONFIG,
        CMD_BLADE_CONFIG,
        CMD_AS_SERVICE,
        CMD_SERVER_TO_AS,
        CMD_ALARM_NOTICE,
        CMD_ALARM_CONFIG,
        CMD_ALARM_LIMIT,
        CMD_ALARM_METHOD,
        CMD_EXTERNAL_SYSTEM_CONFIG,
        CMD_OVERLOAD_CONFIG,
        CMD_SWITCH_CONFIG,
        CMD_CHECKTIME_CONFIG=0x11,
        CMD_REGI_CPS_OVERLOAD=0x12,
        CMD_OAMS_SMS_SEND=0x20,
        CMD_OAMS_SMS_SEND2=0x21,
        CMD_ALARM_CONFIG2=0x30,
        CMD_NMS_SEND_FLAG_UPDATE=0x31,
        CMD_NOMS_GET_SERVER_INFO = 0x1000,
        CMD_NOMS_GET_MEMORY_INFO,
        CMD_NOMS_GET_DISK_INFO, CMD_NOMS_GET_CPU_INFO,
        CMD_NOMS_GET_PROCESS_INFO,
        CMD_NOMS_GET_NIC_INFO,
        CMD_NOMS_GET_RESOURCE_INFO,
        CMD_CLIENT_MSG_INFO = 0x2000,
        CMD_CLIENT_SERVER_STOP,
        CMD_CLIENT_SERVER_START,
        CMD_CLIENT_PROCESS_STOP,
        CMD_CLIENT_PROCESS_START,
        CMD_CLIENT_SYSTEM_REBOOT,
        CMD_CLIENT_SYSTEM_SHUTDOWN,
        CMD_CLIENT_SYSTEM_CHANGEHA,
        CMD_CLIENT_SES_BLOCK,
        CMD_CLIENT_SES_UNBLOCK,
};

typedef unsigned char   BYTE;

#ifdef VOIP 
const char cmdStartMMDB[128] = {"ssh %s /etc/rc3.d/S99altibase start"};
const char cmdStopMMDB[128] = {"ssh %s /etc/rc3.d/S99altibase stop"};
#elif ATS_MODE
const char cmdStartMMDB[128] = {"ssh as%d%s /etc/init.d/origoasdb start"};
const char cmdStopMMDB[128] = {"ssh as%d%s /etc/init.d/origoasdb stop"};
#else   
const char cmdStartMMDB[128] = {"ssh as%d%s /etc/rc3.d/S99altibase start"};
const char cmdStopMMDB[128] = {"ssh as%d%s /etc/rc3.d/S99altibase stop"};
#endif

#pragma pack(push,1)
//MGM IF SSS
typedef struct
{
    MMCHD   head;           // head                                                                            
    int             asno;
    int             sequence;
} LoopBackRQ_S;
typedef struct
{
    MMCHD   head;           // head                                                                            
    BYTE    side;
} HARQ_S;
typedef struct                                        
{
    char    name[31];
    char    run_state;
    int     pid;                                                                     
} PSINFO;
typedef struct   
{    
    MMCHD   head;           // head                                                                            
    BYTE    result;    
    BYTE    reason;    
    BYTE    ps_ptr;    
    PSINFO  ps;        
} RunPsRP;
typedef struct
{                           
    MMCHD   head;           // head                                                                            
    BYTE    result;    
    BYTE    reason;    
    BYTE    ps_ptr;    
    PSINFO  ps;                                               
} KillPsRP;  
typedef struct
{
    MMCHD   head;
    BYTE    result;
    BYTE    reason;
    BYTE    moduleIdA;
    BYTE    haRoleA;        //null, unsign, unsign, active, unsign, standby                                    
    BYTE    moduleIdB;
    BYTE    haRoleB;
} HaPsRP;
typedef struct
{
    char    SvcKey[SVC_KEY_LEN];
    unsigned long TotalCh;
    unsigned long BusyCh;
    unsigned long TotalUsage;     //
    unsigned long TotalSuccess;
    char    Flag;
} SVC_ITEM;

typedef struct
{
    MMCHD       Head;
    BYTE        count;
    SVC_ITEM    item[MAX_SVC_USE];
} SVC_USE;

typedef struct
{
    int cpu;            // cpu use(%)
    int mem;            // memory use(%)
    int net;            // packet numbers
    int disk;           // disk use(%)
    unsigned int ch_use;// SES channel use (count) 
    unsigned short ch_percent;      // SES channel use (%)
    int time;           // time()
    BYTE blocked;
} HW_USE;
typedef struct
{       
    MMCHD   Head;
    BYTE dummy;
    BYTE moId;    
    BYTE oldState;           
    BYTE newState;                                                                                     
    BYTE reserve[8];
    BYTE string[64];         
} HA_USE;  

typedef struct
{
    LONG    TotalCh;
    LONG    BusyCh;
    BYTE    Flag;
} BD_ITEM;

typedef struct
{
    MMCHD   Head;
    BYTE    cReady;
    BYTE    cDummy;
    short   cCPS;
    BYTE    count;
    BD_ITEM item[30];
} BD_USE;

typedef struct
{
    MMCHD   Head;
    BYTE    cReady;
    BYTE    cDummy;
    short   cCPS;
    unsigned int nOverloadCount;
    BYTE    count;
    BD_ITEM item[30];
} BD_USE_V2;

typedef struct
{
    LONG    TotalCh;
    LONG    BusyCh;
    LONG    nSyncCount;
    BYTE    Flag;
} BD_ITEM_V3;

typedef struct
{
    MMCHD   Head;
    BYTE    cReady;
    BYTE    cDummy;
    short   cCPS;
    unsigned int nOverloadCount;
    BYTE    count;
    BD_ITEM_V3 item[30];
} BD_USE_V3;

//ALARM->EXPORTER
typedef struct
{
    char alarmKey[16]; 
    int alarmType;
    int alarmId;             //알람 코드
    int asId;
    int serverId;
	int moduleId;
    int indexId;             //INDEX  //xbus id or extemd index
    int mask;
    char    alarmTime[20];
    int alarmLevel;
    char    comment[128];    // 참고 메세지
} ALM_Q_MSG;

// OAMS_xbus
typedef struct dis_config_oams
{
    MMCHD   mmc_head;
    BYTE    LogFile;
    BYTE    DebugLevel;
} DIS_CONFIG_OAMS;

typedef struct set_config_oams
{   
    MMCHD   mmc_head;
    BYTE    LogFile;
    BYTE    DebugLevel;
//  int     nServerTimer;
} SET_CONFIG_OAMS;
    
typedef struct dis_stat_oams
{   
    MMCHD   mmc_head;
    int     nCMC_Counts;                
} DIS_STAT_OAMS;
                                                                                                          
typedef struct
{
    int id;
    long session;
    char resv[16];
} PACK_WAPM_HEAD;

typedef struct
{
    PACK_WAPM_HEAD head;
    char    response;
} PACK_RESPONSE;

typedef struct
{
    char    action[6];
    char    alarm_idx[4];
    char    alarm_code[10];
    char    alarm_phase;
    char    alarm_flag;
    char    alarm_desc[256];
    char    alarm_notice[4];
} PACK_ALARM_CONFIG;
#ifdef _USE_NMI
typedef struct
{
    char    action[6];
    char    alarm_idx[4];
    char    alarm_code[10];
    char    alarm_phase;
    char    alarm_flag;
    char    alarm_desc[256];
    char    alarm_notice[4];
    char    alarm_nms_yn[1];
} PACK_ALARM_CONFIG2;
#endif
typedef struct
{
    int svr_code;
    char svr_name[10];
    int module_code;
    char module_name[10];
    char module_version[10];
} ORA_MOVER;
typedef struct
{
    int count;
    ORA_MOVER data[30];
} MOVER;

typedef struct
{
   MMCHD Head;
   MOVER mover;
} MO_VER;

typedef struct
{
    MMCHD   head;           // head                                                                            
    BYTE    side;
    BYTE    ps;
} RunPsRQ_S;
typedef struct
{
    MMCHD   head;           // head
    BYTE    side;
} ChangeHaRQ_S;
typedef struct
{
    MMCHD   head;           // head                                                                            
    BYTE    board;  //SES0~SES15 (0~15), ALL(255)                                                              
    BYTE    side;   //A(0), B(1), ALL(2)                                                                       
} BlockRQ_S;
typedef struct
{   
    MMCHD   head;           // head
    BYTE    side;
} ShutdownRebootRQ_S;
typedef struct
{
    WORD    len;        // head + body 
    WORD    sockfd;
    WORD    msgId;      // MSG_ID for OAM client
    BYTE    asEmsId;
    BYTE    svrId;      // system Id 
    BYTE    moduleId;
    BYTE    modeType;   // 0 :Server, 1:Module, 2: Hardware, 3 : external      
    BYTE    indxNum ;
    BYTE    reserved;
} st_CmcHd;
typedef struct
{
    st_CmcHd    head;
    char        data[256];
} MMCMsgRP_S; //MID_AS_MMC_MSG_RP

typedef struct
{
    char    log_time[15];
    char    op_id[15];
    char    op_ip[18];
    char    command[128];
    char    comments[128];
    char    result[6];
    char    keyword[13];
} ORA_Operator_Log;
typedef struct
{
    char len[4];
    char asidx[2];
    char reserve[4];
    char dest_svr[2];
    char dest_module[2];
    char msgId[4];
} CLIENT_HEAD;
typedef struct
{
//    PACK_WAPM_HEAD head;
    BYTE        as_name[32];
    BYTE        server_name[32];
    BYTE        module_name[32];
} CLIENT_INFO;

#pragma pack(pop)

#endif //__OAMS_DEFINE_H__
