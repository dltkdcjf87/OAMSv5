#include "OAMS_common.h"

CConfig   g_cfg;
NSM_LOG    Log;

AsInfoManager      asInfoManager;
ChassisInfoManager chassisInfoManager;
ServerInfoManager  serverInfoManager;
ModuleInfoManager  moduleInfoManager;
AsStatsInfoManager asstatsInfoManager;
SystemResourceStatManager  systemresourcestatManager;
PerformanceManager performanceManager;
ThresholdInfoManager thresholdInfoManager;
SswConfigInfoManager sswconfigInfoManager;
MsConfigInfoManager  msconfigInfoManager;
ExternalIfInfoManager externalIfInfoManager;
CdbStatusInfoManager  cdbstatusInfoManager;
AlarmConfigManager    alarmconfigManager;
StatisticManager	statisticManager;

CExporter fixMetric;
CExporter alarmMetric;
ClimitProcess limitProc;
AlarmManager alarmMNG;
ExternServerManager	externServerManager;

std::map<stAsModId, stChassisServerId*>    xbusid_Map;

CHWMonitor	hwMonitorManager;

uint8_t  MY_SIDE;
uint8_t  OTHER_SIDE;

char    as_ip_a[MAX_AS][IPADDR_LEN+1] = {0};
char    as_ip_b[MAX_AS][IPADDR_LEN+1] = {0};
int     r_buf = 0;
int     gMgmFd[MAX_AS_EMS] = {0};
LibQ    g_mgmQue[MAX_AS];
bool    g_mgmTimeOut[MAX_AS] = {false};

char    gFixPort[6];
char    gAlarmPort[6];
bool    g_bDEBUG_LOGFILE;
int     g_nDebug_level;
char    gMccsUse[6];
int     gSvrEmsA;
int     gSvrEmsB;
char    gMccsEmsA[16];
char    gMccsEmsB[16];
int     g_curCntClients = 0;
int   	g_Alarm_Flag = 0;
int     g_NMS_Alarm_Sendflag;    // 0 : off, 1 : on

bool	dbConnect = false;

std::vector<std::pair<std::string, std::string>>  gMccsProcess;
std::vector<std::pair<std::string, std::string>>  gEmsProcess;

int ParsingDelimeter(char *bp, char delimeter, char **valp, int size)
{
    int     i, n;

    if (size == 0)
        return(0);

    n = 0;
    valp[n] = bp;
    for ( i=0 ; i<size ; i++ )
    {
        if (bp[i] == delimeter || bp[i] == '\n' || bp[i] == '\r' || bp[i] == '\0')
        {
            bp[i] = '\0';
            valp[++n] = &bp[i+1];
        }
    }

//    valp[n] = '\0';

    return(n+1);
}
int Trim(char *pszBuf, int nLen)
{
    int     i, j;
    char    szTemp[2048];

    memset(szTemp, 0, sizeof(szTemp));

    j = 0;
    for ( i=0 ; i<nLen ; i++ )
    {
        if (pszBuf[i] != ' ' && pszBuf[i] != '\t' && pszBuf[i] != '\r' && pszBuf[i] != '\n' && pszBuf[i] != '\0')
            szTemp[j++] = pszBuf[i];
    }

    sprintf(pszBuf, "%s", szTemp);

    return(j);
}
int atoiN(const char *pszData, int nSize)
{
    char    szTemp[1024];

    memset(szTemp, 0x00, sizeof(szTemp));
    memcpy(szTemp, pszData, sizeof(char)*nSize);

    return(atoi(szTemp));
}
int GetServerIdFromXbus(int asId, int xbusId, int& outServerId) {
    if(asId < 0 ||asId > MAX_AS) 			return false;
	if(xbusId < 0 || xbusId > MAX_MODULE) 	return false;
    stAsModId key = { asId, xbusId };
    if (xbusid_Map.empty()) {
        Log.printf(LOG_ERR, "[GetServerIdFromXbus] xbusid_Map is empty!");
		return false;
    }
    auto it = xbusid_Map.find(key);

    if (it != xbusid_Map.end()) {
        Log.printf(LOG_LV2, "[GetServerIdFromXbus] asId : %d xbusId: %d Found -> ChassisId: %d, ServerId: %d",
                   asId, xbusId, it->second->chassisId, it->second->serverId);
        outServerId = it->second->serverId;
        return true;
    } else {
        Log.printf(LOG_ERR, "[GetServerIdFromXbus] Not Found -> asId: %d, xbusId: %d", asId, xbusId);
        return false;
    }
}
bool convertLimitName(int alarmField, const char*& LimitName)
{
    switch(alarmField)
    {
        case 0: LimitName = "cpu";      break;
        case 1: LimitName = "memory";       break;
        case 2: LimitName = "network";  break;
        case 3: LimitName = "disk";     break;
        case 4: LimitName = "channel";  break;

        case 10: LimitName = "temperature"; break;
        case 11: LimitName = "tablespace";  break;

        case 13: LimitName = "ntp";     break;
        case 14: LimitName = "session"; break;
        case 15: LimitName = "CDB API"; break;
        default:    return false;
    }

    return true;
}
void formatDateTime(const char* input, char* output, size_t output_size) {
    int year, month, day, hour, minute, second, millisecond;

    // 입력 문자열 전체를 sscanf로 한 번에 파싱 시도
    // %3d는 3자리 정수를 의미합니다.
    if (sscanf(input, "%4d%2d%2d%2d%2d%2d%3d",
               &year, &month, &day, &hour, &minute, &second, &millisecond) == 7) {
        // 모든 필드가 성공적으로 파싱된 경우
        snprintf(output, output_size, "%04d-%02d-%02d %02d:%02d:%02d.%03d",
                 year, month, day, hour, minute, second, millisecond);
    } else {
        // 파싱 실패 시 (예: 입력 문자열 길이가 17이 아니거나 형식이 맞지 않는 경우)
        snprintf(output, output_size, "Invalid format");
    }

    // 최종 결과 로그 출력
    Log.printf(LOG_LV1, "[formatDateTime] input[%s] output[%s] millisecond[%d] \n", input, output, millisecond);
}
void make_strTime(char strTime[15], long t)
{
    struct  tm  tm;

    localtime_r((const time_t *)&t, &tm);
    strftime(strTime, 15, "%Y%m%d%H%M%S", &tm);
    strTime[14] = '\0';
}
void shut_down(int reason)
{
    LIB_unset_run_pid(MY_PS_NAME);

    Log.printf(LOG_INF, "-----------------------------------------------------------------------------------\n");
    Log.printf(LOG_INF, "                      OAMS Service Shutdown... reason = %d \n", reason);
    Log.printf(LOG_INF, "-----------------------------------------------------------------------------------\n");
    exit(-1);
}
void trim(char* str) {
    if (!str) return;

    // 1. 좌측 공백 제거
    char* start = str;
    while (*start == ' ') ++start;

    // 2. 우측 공백 및 '|' 제거
    char* end = start + strlen(start) - 1;
    while (end > start && (*end == ' ' || *end == '|')) {
        *end-- = '\0';
    }

    // 3. 앞쪽 공백을 제거한 내용을 원래 str로 복사 (start가 변경된 경우만)
    if (start != str) {
        memmove(str, start, strlen(start) + 1);  // +1 for null terminator
    }
}

void ToUpperCase(char* str) {
    if (str == nullptr) return;

    for (size_t i = 0; i < strlen(str); ++i) {
        str[i] = std::toupper(static_cast<unsigned char>(str[i]));
    }
}
int ConverAlarmCode(int code) {
    if(code == 269)  return 200;
    else if(code == 270)  return 400;
    else if(code == 408)  return 303;
    else if(code == 312)  return 300;
    else if(code == 418)  return 313;
    else if(code == 419)  return 313;
    else if(code == 421)  return 313;
    else if(code == 426)  return 313;
    else if(code == 427)  return 313;
    else if(code == 299)  return 401;
    else if(code == 441)  return 307;
//    else if(code == 2004)  return 313;
    else return code;

}
void clean_str(char* str) {
    if (!str) return;

    // 1. 앞쪽 공백 제거
    char* start = str;
    while (*start == ' ') ++start;

    // 2. '|' 있으면 자르고, 없으면 문자열 끝까지
    char* pipe = strchr(start, '|');
    if (pipe) *pipe = '\0';  // '|' 위치를 널 종료

    // 3. 뒤쪽 공백 제거
    char* end = start + strlen(start) - 1;
    while (end > start && *end == ' ') {
        *end-- = '\0';
    }

    // 4. 앞쪽 공백 제거 후, 원위치로 이동
    if (start != str) {
        memmove(str, start, strlen(start) + 1);  // +1은 널 문자 포함
    }
}




