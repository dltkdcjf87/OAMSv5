#include "OAMS_stat.h"


StatisticManager::StatisticManager()
{
}

StatisticManager::~StatisticManager()
{
}

bool StatisticManager::StatMngInit()
{
	pthread_create(&tidStat5M, NULL, StatTimerThread_5M, this);
	pthread_detach(tidStat5M);

	return true;
}

void* StatisticManager::StatTimerThread_5M(void* arg)
{
	StatisticManager* pThis = static_cast<StatisticManager*>(arg);
    pThis->StatTimer5M();
    return nullptr;

}


void StatisticManager::StatTimer5M()
{
    while (true) {
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        struct tm* tm_now = std::localtime(&now_c);

        int next_minute = ((tm_now->tm_min / 5) + 1) * 5;
        if (next_minute >= 60) {
            tm_now->tm_hour += 1;
            next_minute = 0;
        }
        tm_now->tm_min = next_minute;
        tm_now->tm_sec = 0;

        std::time_t next_time_c = std::mktime(tm_now);
        auto next_time = std::chrono::system_clock::from_time_t(next_time_c);

        std::this_thread::sleep_until(next_time);

		Log.printf(LOG_LV2, "[StatTimerThread_5M] 5Minit Count! ");
        DoTask_5M();
    }
}

int StatisticManager::DoTask_5M()
{
	systemRscStatProcess();
	AsCpsStatProcess();


	return 0;
}


void StatisticManager::systemRscStatProcess()
{
	SYSTEM_RESOURCE_STAT	statData;
	int asId, svrCode;
	char DBTime[15];

	for(int i = 0; i < MAX_SERVER; i++)
	{
		systemresourcestatManager.getSystemResourceStat(i, statData);
//		if(statData.cpu_summery.max <= 0) continue;

		asId = serverInfoManager.getServerASID(i);
		if(asId < 0)
		{
			//Log.printf(LOG_ERR, "[systemRscStatProcess] ASID Search Fail!! ");
			continue;
		}

		svrCode = serverInfoManager.getServerCode(i);
		if(svrCode < 0)
		{
			//Log.printf(LOG_ERR, "[systemRscStatProcess] ServerCode Search Fail!! ");
			continue;
		}

		time_t now = time(NULL);
	    struct tm t;
		localtime_r(&now, &t);

	    strftime(DBTime, sizeof(DBTime), "%Y%m%d%H%M%S", &t);

		RscStatInsert(asId, svrCode, DBTime, statData);
		TemperStatInsert(asId, svrCode, DBTime, statData.temperature_summery);
		Log.printf(LOG_LV3, "[systemRscStatProcess] Resource DB DATA INSERT !! ");
	}

	systemresourcestatManager.Init();

}


void StatisticManager::AsCpsStatProcess()
{
	AS_INFO  asInfo;
	AS_STATS asData;
	char DBTime[15];

	for(int i = 0; i < MAX_AS_EMS; i++)
	{
		asInfoManager.getASInfo(i, asInfo);
		if(asInfo.asId == -1)	continue;
		asstatsInfoManager.getAsStats(i, asData);

        time_t now = time(NULL);
        struct tm t;
        localtime_r(&now, &t);
        
        strftime(DBTime, sizeof(DBTime), "%Y%m%d%H%M%S", &t);

		CpsStatInsert(i, DBTime, asData);
		Log.printf(LOG_LV3, "[AsCpsStatProcess] CPS STAT DATA INSERT !! ");
	}

	asstatsInfoManager.InitCpsStat();
}
