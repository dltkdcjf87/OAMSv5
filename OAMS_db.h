#include "OAMS_altiCInf.h"
#include "CQueueBase.h"
#include "OAMS_common.h"

class AltiCinf;

extern AltiCinf *pDB;
extern AltiCinf *pExecDB;
//extern std::map<stAsModId, stChassisServerId*>    m_Map;

//extern CQueueBase<SqlRequest *>		sqlQueue;
void initDBThread();
void* initDB(void* arg);
void SelectASInfo();
bool DataSetAsInfo();
bool DataSetChassisInfo();
bool DataSetServerInfo();
bool DataSetModuleInfo();

bool DataSetServerIdMap();
bool DataRemoveServerIdMap();
bool DataSetThreshold();

bool DataSetNTP();
bool DataSetTableSpace(int svrId);
bool DataSetDBSession(int svrId);
bool DataSetServiceKey();

int AlarmDataAllDelete();
int AlarmDataSelect();
int AlarmDataInsert(ALM_Q_MSG msg);
int AlarmDataInsert_TEST(ALM_Q_MSG msg);
int AlarmDataDelete(const char* key);
int AlarmLogInsert(ALM_Q_MSG* msg);

int FaultDataToDB(int asId, int serverId, int indexId, int alarmId, char* alarmedTime, int phase, char* detail);

bool CscfDataSelect();
int CscfDataUpdate(int nASID, int nCSCFID, int runFlag, const char* logTime);

bool MrfDataSelect();
bool CdbDataSelect();
bool ExtDataSelect();

int testDB();
int InsertOperatorLog(ORA_Operator_Log *oplog);

int RscStatInsert(int asId, int svrCode, char* DBTime, SYSTEM_RESOURCE_STAT RscStat);
int TemperStatInsert(int asId, int svrCode, char* DBTime, STAT_SUMMERY TempStat);
int CpsStatInsert(int asId, char* DBTime, AS_STATS asStat);

