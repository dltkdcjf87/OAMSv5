#ifndef OAMS_STAT_H
#define OAMS_STAT_H
#include "OAMS_common.h"

typedef struct {
	int aa;
}	StatDataCPS[MAX_AS];


class StatisticManager {
private:
	pthread_t tidStat5M;

	void systemRscStatProcess();
	void AsCpsStatProcess();
	static void* StatTimerThread_5M(void* arg);
	void StatTimer5M();
	//void StatTimerThread_30s();
	int DoTask_5M();

    static void* DoTask5MThread(void* arg);
    static void* MrfThread(void* arg);

public:
	StatisticManager();
	~StatisticManager();
	bool StatMngInit();

};

#endif
