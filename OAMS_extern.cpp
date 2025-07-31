#include "OAMS_extern.h"

ExternServerManager::ExternServerManager()
{
}

ExternServerManager::~ExternServerManager()
{
}


bool ExternServerManager::Init()
{
    pthread_create(&cscfTid, NULL, CscfThread, this);
    pthread_create(&mrfTid, NULL, MrfThread, this);
    pthread_create(&cdbTid, NULL, CdbThread, this);
    pthread_create(&externTid, NULL, ExternThread, this);

    pthread_detach(cscfTid);
    pthread_detach(mrfTid);
	pthread_detach(cdbTid);
	pthread_detach(externTid);

	return true;
}


void ExternServerManager::CscfFunc()
{
	int ret;
	InitCSCFConfig();

	sleep(10);

	while(1)
	{
		ret = ReadCSCFConfigToDB();

		if(ret < 0)	{
			sleep(1);
			continue;
		}

		sleep(30);
	}
}

void ExternServerManager::MrfFunc()
{
	int ret;
	InitMRFConfig();

    while(1)
    {
		ret = ReadMRFConfigToDB();
        
        if(ret < 0) {
            sleep(1);
            continue;
        }

        sleep(30);
    }
}

void ExternServerManager::CdbFunc()
{
	int ret;
	InitCDBConfig();

    while(1)
    {
		ret = ReadCDBConfigToDB();

		if(ret < 0) {
			sleep(1);
			continue;
		}

        sleep(30);
    }
}

void ExternServerManager::ExternFunc()
{
	bool ret;
	InitExternConfig();

    while(1)
    {
        ret = ReadExternConfigToDB();

        if(ret < 0) {
            sleep(1);
            continue;
        }

        sleep(30);

    }
}


int ExternServerManager::ReadCSCFConfigToDB()
{
	bool ret;
	ret = CscfDataSelect();
	if(!ret) return -1;

    return 0;
}


int ExternServerManager::ReadMRFConfigToDB()
{
	bool ret;
	ret = MrfDataSelect();
	if(!ret)	return -1;

	return 0;
}

int ExternServerManager::ReadCDBConfigToDB()
{
    bool ret;
    ret = CdbDataSelect();
    if(!ret)    return -1;

    return 0;
}

int ExternServerManager::ReadExternConfigToDB()
{
    bool ret;
    ret = ExtDataSelect();
    if(!ret)    return -1;

    return 0;
}



/*
int GetCSCFConfig(REC_SSW_Config *pCSCF)
{   
    int nCnt;
    
    CSCFLock();
    
    nCnt = g_nCSCFCnt;
    memcpy(pCSCF, g_CSCFList, sizeof(REC_SSW_Config)*MAX_SOFT_SW_CNT);
    
    CSCFUnlock();
    
    return(nCnt);
}
*/


int ExternServerManager::UpdateCSCFStatus(int nASId, int nCSCFId, int runFlag, char *logTime)
{
	int		INDEX;

	INDEX = CscfGetIdToIndex(nCSCFId);
	sswconfigInfoManager.setSswStatusInfo(INDEX, runFlag);

	CscfDataUpdate(nASId, nCSCFId, runFlag, (const char*)logTime);

    return 0;
}


int ExternServerManager::InitCSCFConfig(void)
{
    int         ret;
	SSW_CONFIG_INFO sswData;

	ExternMapClear(CSCF_MAP);
	sswconfigInfoManager.Init();

    ret = ReadCSCFConfigToDB();

	memset(externCnt, 0x00, sizeof(externCnt));

    for(int i = 0; i < MAX_SSW; i++)
    {   
    	sswconfigInfoManager.getSswConfigInfo(i, sswData);
       	if(sswData.asId < 0)   break;        

		CscfSetIdToIndex(sswData.index, i);
		Log.printf(LOG_LV3,"[CSCF INFO] i[%d], asid[%d], index[%d], name[%s], status[%d]", i, sswData.asId, sswData.index, sswData.systemName, sswData.status);
    }

    return(ret);
}

int ExternServerManager::InitMRFConfig(void)
{
	int	ret;
	MS_CONFIG_INFO	msData;
	AS_INFO asData;

	ExternMapClear(MRF_MAP);
	ret = ReadMRFConfigToDB();	

    for(int i = 0; i < MAX_MS; i++)
    {   
        msconfigInfoManager.getMsConfigInfo(i, msData);
        if(msData.asId < 0)   break;
        
        MrfSetIdToIndex(string(msData.systemName), i);
		Log.printf(LOG_LV3,"[MS INFO] i[%d], asid[%d], index[%d], name[%s], status[%d]", i, msData.asId, msData.index, msData.systemName, msData.state_flag);
    }


    return(ret);

}

int ExternServerManager::InitCDBConfig(void)
{
	int ret;
	CDB_STATUS_INFO	cdbData;

	ret = ReadCDBConfigToDB();
    for(int i = 0; i < MAX_CDB; i++)
    {
        cdbstatusInfoManager.getCdbStatusInfo(i, cdbData);
        if(cdbData.asId < 0)   break;
        
		Log.printf(LOG_LV3, "[CDB INFO] i[%d], svrid[%d], asid[%d], groupID[%d], groupSEQ[%d], name[%s], status[%d]", i, cdbData.serverID, cdbData.asId, cdbData.groupId, cdbData.groupSeq, cdbData.systemName, cdbData.status);
    }

    return(ret);
}

int ExternServerManager::InitExternConfig()
{
	int ret;
    EXTERNAL_IF_INFO externData;
	AS_INFO asData;

    ret = ReadExternConfigToDB();

    for(int i = 0; i < MAX_EXTERNAL; i++)
    {
		externalIfInfoManager.getExternalIfInfo(i, externData);
        if(externData.asId < 0)   break;

		Log.printf(LOG_LV3, "[EXTERN INFO] i[%d], asid[%d], TYPE[%d], name[%s], status[%d] svrID[%d], system[%s]\n", i, externData.asId, externData.system_type, externData.systemName, externData.status, externData.ConnSvrID, externData.system);
    }

	return(ret);
}

int ExternServerManager::CscfGetIdToIndex(int sswId)
{
	int index = -1; 

	std::lock_guard<std::mutex> lock(m_CscfMutex);
    auto it = CscfIdToIndex.find(sswId);
    if (it != CscfIdToIndex.end()) {
       	index = it->second;
    } else {
		Log.printf(LOG_ERR, "CscfGetIdToIndex FIND FAIL SSW[%d] \n",sswId);
		return -1;
    }


	return index;
}

bool  ExternServerManager::CscfSetIdToIndex(int sswId, int Index)
{
    if (sswId < 0 || Index < 0) {
        return false; 
    }

	std::lock_guard<std::mutex> lock(m_CscfMutex);
	CscfIdToIndex[sswId] = Index;

	return true;
}

int ExternServerManager::MrfGetIdToIndex(string msName)
{
    int index = -1;
	std::lock_guard<std::mutex> lock(m_MrfMutex);

    auto it = MrfIdToIndex.find(msName);
    if (it != MrfIdToIndex.end()) {
        index = it->second;
    } else {
        Log.printf(LOG_ERR, "MrfGetIdToIndex FIND FAIL MSNAME[%s] \n",msName.c_str());
        return -1;
    }


    return index;
}

bool  ExternServerManager::MrfSetIdToIndex(string msName, int Index)
{
    if (Index < 0 || msName.empty()) {
        return false;
    }

	std::lock_guard<std::mutex> lock(m_MrfMutex);
    MrfIdToIndex[msName] = Index;

    return true;
}


bool ExternServerManager::ExternMapClear(int type)
{
	switch(type)
	{
		case CSCF_MAP :
		{
			std::lock_guard<std::mutex> lock(m_CscfMutex);
			CscfIdToIndex.clear();
			break;
		}
        case MRF_MAP :
        {
			std::lock_guard<std::mutex> lock(m_MrfMutex);
			MrfIdToIndex.clear();
            break;
        }
        case CDB_MAP :
        {
            break;
        }
        case EXTERN_MAP :
        {
            break;
        }
		default :
			Log.printf(LOG_ERR, "ExternServerMapClear Fail (TYPE NOT DEFINE!) \n");
			break;
	}
    return TRUE;
}

void* ExternServerManager::CscfThread(void* arg) {
    ExternServerManager* pThis = static_cast<ExternServerManager*>(arg);
    pThis->CscfFunc();
    return nullptr;
}

void* ExternServerManager::MrfThread(void* arg) {
    ExternServerManager* pThis = static_cast<ExternServerManager*>(arg);
   	pThis->MrfFunc();
    return nullptr;
}

void* ExternServerManager::CdbThread(void* arg) {
    ExternServerManager* pThis = static_cast<ExternServerManager*>(arg);
    pThis->CdbFunc();
    return nullptr;
}

void* ExternServerManager::ExternThread(void* arg) {
    ExternServerManager* pThis = static_cast<ExternServerManager*>(arg);
    pThis->ExternFunc();
    return nullptr;
}
