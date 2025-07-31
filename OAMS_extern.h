#ifndef OAMS_EXTERN_H
#define OAMS_EXTERN_H
#include "OAMS_common.h"
#include <unordered_map>

#define CSCF_MAP	0
#define	MRF_MAP		1
#define CDB_MAP		2
#define EXTERN_MAP	3


#define EXT_MRF_TYPE 5

typedef struct {
	int totalCnt;
	int errCnt;
} ExternCnt;

class ExternServerManager {
private:
	std::mutex m_CscfMutex;
	std::mutex m_MrfMutex;

//	std::vector<std::thread> threads;
	ExternCnt externCnt[MAX_AS_EMS];

	pthread_t cscfTid, mrfTid, cdbTid, externTid;
	pthread_attr_t attr;
    static void* CscfThread(void* arg);
    static void* MrfThread(void* arg);
    static void* CdbThread(void* arg);
    static void* ExternThread(void* arg);

    void CscfFunc(); 
    void MrfFunc();
    void CdbFunc();
    void ExternFunc();

	std::unordered_map<int, int> CscfIdToIndex;
	std::unordered_map<std::string, int> MrfIdToIndex;

	bool ExternMapClear(int type);

	int ReadCSCFConfigToDB();
	bool CscfSetIdToIndex(int sswId, int Index);

	int ReadMRFConfigToDB();
	bool MrfSetIdToIndex(string msName, int Index);

	int ReadCDBConfigToDB();

	int ReadExternConfigToDB();

public:
	ExternServerManager();
	~ExternServerManager();

	bool Init();
	int InitCSCFConfig(void);
	int InitMRFConfig(void);
	int InitCDBConfig(void);
	int InitExternConfig(void);

	int  CscfGetIdToIndex(int sswId);
	int UpdateCSCFStatus(int nASId, int nCSCFId, int runFlag, char *logTime);
	int  MrfGetIdToIndex(string msName);

};

#endif
