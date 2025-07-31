#include "OAMS_limit.h"
#include "OAMS_common.h"

ClimitProcess::ClimitProcess() {}
ClimitProcess::~ClimitProcess() {}

bool ClimitProcess::initialize()
{
	pthread_t tid;
	pthread_create(&tid, NULL, ClimitProcess::LimitCheckThread, NULL);
	pthread_detach(tid);

	return 0;

}

void* ClimitProcess::LimitCheckThread(void* arg)
{
	THRESHOLD_INFO 	threshold;
	PERFORMANCE		perf;
	LimitAlarmLevel AlarmLevel;

	LimitAlarmLevel cpuAlarm[MAX_SERVER] = { LimitAlarmLevel::NONE };
	LimitAlarmLevel memAlarm[MAX_SERVER] = { LimitAlarmLevel::NONE };
	LimitAlarmLevel netAlarm[MAX_SERVER] = { LimitAlarmLevel::NONE };
	LimitAlarmLevel diskAlarm[MAX_SERVER] = { LimitAlarmLevel::NONE };
	LimitAlarmLevel channelAlarm[MAX_SERVER] = { LimitAlarmLevel::NONE };
	LimitAlarmLevel ntpAlarm[MAX_SERVER] = { LimitAlarmLevel::NONE };
	LimitAlarmLevel temperAlarm[MAX_SERVER] = { LimitAlarmLevel::NONE };
	LimitAlarmLevel dbsessionAlarm[MAX_SERVER] = { LimitAlarmLevel::NONE };
	LimitAlarmLevel cdbErrAlarm[MAX_SERVER] = { LimitAlarmLevel::NONE };
	CDB_STATUS_INFO cdbInfo;

	LimitAlarmLevel tsAlarm[MAX_SERVER][MAX_TS];

	char	comment[8][64];

	for (int i = 0; i < MAX_SERVER; i++) {
    	for (int j = 0; j < MAX_TS; j++) {
        	tsAlarm[i][j] = LimitAlarmLevel::NONE;
    	}
	}

	AlarmInfo alarmMsg;
	memset(&alarmMsg, 0x00, sizeof(alarmMsg));

	while(1)
	{
		memset(&threshold, 0x00, sizeof(threshold));
		thresholdInfoManager.getThresHoldInfo(threshold);

		for(int i=0; i<MAX_SERVER; i++)
		{
			memset(&perf, 0x00, sizeof(perf));
			performanceManager.getPerformance(i, perf);

			if(perf.cpu < 0) continue;	// NOT USE SERVER

			AlarmLevel = ClimitProcess::LimitCheckFunc(perf.cpu, 	threshold.cpu);
			if(AlarmLevel != cpuAlarm[i]) {
				
				memset(comment, 0x00, sizeof(comment));
				if (static_cast<int>(AlarmLevel) != 0)	// 알람 발생
				{
					sprintf(comment[0], "%d%%", perf.cpu);
					sprintf(comment[1], "OCCUR");
				}
				else					// 알람 해제
				{	
					sprintf(comment[0], "%d%%", perf.cpu);
					sprintf(comment[1], "Clear");
				}

				alarmMNG.SendAlarm(i, NO_PROCESS_ID, ALM_CODE_LIMIT_CPU, 0, static_cast<int>(AlarmLevel), 2, comment);
				Log.printf(LOG_LV3, "[LimitCheckThread] limit cpu alarm![%d], LEVEL[%d]\n",perf.cpu,    static_cast<int>(AlarmLevel));
				cpuAlarm[i] = AlarmLevel;
			}

			AlarmLevel = ClimitProcess::LimitCheckFunc(perf.memory, threshold.memory);
			if(AlarmLevel != memAlarm[i]) {

                memset(comment, 0x00, sizeof(comment));
				if (static_cast<int>(AlarmLevel) != 0)    // 알람 발생
				{
					sprintf(comment[0], "%d%%", perf.memory);
					sprintf(comment[1], "OCCUR");
				}
				else                    // 알람 해제
				{
					sprintf(comment[0], "%d%%", perf.memory);
					sprintf(comment[1], "Clear");
				}

				alarmMNG.SendAlarm(i, NO_PROCESS_ID, ALM_CODE_LIMIT_MEM, 0, static_cast<int>(AlarmLevel), 2, comment);
				Log.printf(LOG_LV3, "[LimitCheckThread] limit memory alarm![%d], LEVEL[%d]\n",perf.memory, static_cast<int>(AlarmLevel));
				memAlarm[i] = AlarmLevel;
			}
	
			AlarmLevel = ClimitProcess::LimitCheckFunc(perf.network, threshold.network);
			if(AlarmLevel != netAlarm[i]) {

                memset(comment, 0x00, sizeof(comment));
				if (static_cast<int>(AlarmLevel) != 0)    // 알람 발생
				{
					sprintf(comment[0], "%dKB", perf.network);
					sprintf(comment[1], "OCCUR");
				}
				else                    // 알람 해제
				{
					sprintf(comment[0], "%dKB", perf.network);
					sprintf(comment[1], "Clear");
				}

				alarmMNG.SendAlarm(i, NO_PROCESS_ID,  ALM_CODE_LIMIT_NET, 0, static_cast<int>(AlarmLevel), 2, comment);
				Log.printf(LOG_LV3, "[LimitCheckThread] limit network alarm![%d], LEVEL[%d]\n",perf.network, static_cast<int>(AlarmLevel));
				netAlarm[i] = AlarmLevel;
			}

			AlarmLevel = ClimitProcess::LimitCheckFunc(perf.disk, 	threshold.disk);
			if(AlarmLevel != diskAlarm[i]) {

                memset(comment, 0x00, sizeof(comment));
				if (static_cast<int>(AlarmLevel) != 0)    // 알람 발생
				{
					sprintf(comment[0], "%d%%", perf.disk);
					sprintf(comment[1], "OCCUR");
				}
				else                    // 알람 해제
				{
					sprintf(comment[0], "%d%%", perf.disk);
					sprintf(comment[1], "Clear");
				}

				alarmMNG.SendAlarm(i, NO_PROCESS_ID, ALM_CODE_LIMIT_DISK, 0, static_cast<int>(AlarmLevel), 2, comment);
				Log.printf(LOG_LV3, "[LimitCheckThread] limit disk alarm![%d], LEVEL[%d]\n",perf.disk,   static_cast<int>(AlarmLevel));
				diskAlarm[i] = AlarmLevel;
			}
	
			AlarmLevel = ClimitProcess::LimitCheckFunc(perf.channel_rate, threshold.channel);
			if(AlarmLevel != channelAlarm[i]) {

                memset(comment, 0x00, sizeof(comment));
				if (static_cast<int>(AlarmLevel) != 0)    // 알람 발생
				{
					sprintf(comment[0], "%d%%", perf.channel_rate);
					sprintf(comment[1], "OCCUR");
				}
				else                    // 알람 해제
				{
					sprintf(comment[0], "%d%%", perf.channel_rate);
					sprintf(comment[1], "Clear");
				}

				alarmMNG.SendAlarm(i, NO_PROCESS_ID, ALM_CODE_LIMIT_CHANNEL, 0, static_cast<int>(AlarmLevel), 2, comment);
				Log.printf(LOG_LV3, "[LimitCheckThread] limit channel alarm![%d], LEVEL[%d]\n",perf.channel_rate, static_cast<int>(AlarmLevel));
				channelAlarm[i] = AlarmLevel;
			}

			AlarmLevel = ClimitProcess::LimitCheckFunc(perf.ntp, 	threshold.ntp);
			if(AlarmLevel != ntpAlarm[i]) {

                memset(comment, 0x00, sizeof(comment));
				if (static_cast<int>(AlarmLevel) != 0)    // 알람 발생
				{
					sprintf(comment[0], "%dms", perf.ntp);
					sprintf(comment[1], "OCCUR");
				}
				else                    // 알람 해제
				{
					sprintf(comment[0], "%dms", perf.ntp);
					sprintf(comment[1], "Clear");
				}

				alarmMNG.SendAlarm(i, NO_PROCESS_ID,  ALM_CODE_LIMIT_NTP, 0, static_cast<int>(AlarmLevel), 2, comment);
				Log.printf(LOG_LV3, "[LimitCheckThread] limit ntp alarm![%d], LEVEL[%d]\n",perf.ntp, static_cast<int>(AlarmLevel));
				ntpAlarm[i] = AlarmLevel;
			}

            for(int j=0 ; j<MAX_TS; j++)
            {		
				if(perf.tablespace[j].use < 0) break;
				AlarmLevel = ClimitProcess::LimitCheckFunc(perf.tablespace[j].use,	threshold.tablespace);
				if(AlarmLevel != tsAlarm[i][j]) {

					memset(comment, 0x00, sizeof(comment));
					if (static_cast<int>(AlarmLevel) != 0)    // 알람 발생
					{
						strcpy(comment[0], perf.tablespace[j].name);
						sprintf(comment[1], "%d%%", perf.tablespace[j].use);
						sprintf(comment[2], "OCCUR");
					}
					else                    // 알람 해제
					{
                        strcpy(comment[0], perf.tablespace[j].name);
                        sprintf(comment[1], "%d%%", perf.tablespace[j].use);
                        sprintf(comment[2], "Clear");
					}

					alarmMNG.SendAlarm(i, NO_PROCESS_ID, ALM_CODE_LIMIT_TABLESPACE, j, static_cast<int>(AlarmLevel), 3, comment);
					Log.printf(LOG_LV3, "[LimitCheckThread] limit tablespace[%s] alarm![%d], LEVEL[%d]\n", perf.tablespace[j].name, perf.tablespace[j].use, static_cast<int>(AlarmLevel));
					tsAlarm[i][j] = AlarmLevel;
				}
			}

			AlarmLevel = ClimitProcess::LimitCheckFunc(perf.temperature, 	threshold.temperature);
			if(AlarmLevel != temperAlarm[i]) {

                memset(comment, 0x00, sizeof(comment));
				if (static_cast<int>(AlarmLevel) != 0)    // 알람 발생
				{
                	sprintf(comment[0], "%d", perf.temperature);
					sprintf(comment[1], "OCCUR");
				}
				else                    // 알람 해제
				{
					sprintf(comment[0], "%d", perf.temperature);
					sprintf(comment[1], "Clear");
				}

				alarmMNG.SendAlarm(i, NO_PROCESS_ID, ALM_CODE_LIMIT_TEMP, 0, static_cast<int>(AlarmLevel), 2, comment);
				Log.printf(LOG_LV3, "[LimitCheckThread] limit temp alarm![%d], LEVEL[%d]\n", perf.temperature, static_cast<int>(AlarmLevel));
				temperAlarm[i] = AlarmLevel;
			}

			AlarmLevel = ClimitProcess::LimitCheckFunc(perf.dbSession, 	threshold.session);
			if(AlarmLevel != dbsessionAlarm[i]) {

                memset(comment, 0x00, sizeof(comment));
				if (static_cast<int>(AlarmLevel) != 0)    // 알람 발생
				{
                	sprintf(comment[0], "%d%%", perf.dbSession);
					sprintf(comment[1], "OCCUR");
				}
				else                    // 알람 해제
				{
					sprintf(comment[0], "%d%%", perf.dbSession);
					sprintf(comment[1], "Clear");
				}

				alarmMNG.SendAlarm(i, NO_PROCESS_ID, ALM_CODE_LIMIT_DBSESSION, 0, static_cast<int>(AlarmLevel), 2, comment);
				Log.printf(LOG_LV3, "[LimitCheckThread] limit db session alarm![%d], LEVEL[%d]\n", perf.dbSession, static_cast<int>(AlarmLevel));
				dbsessionAlarm[i] = AlarmLevel;
			}
			
		}

		for(int i = 0; i < MAX_CDB; i++)
		{
			cdbstatusInfoManager.getCdbStatusInfo(i, cdbInfo);

            AlarmLevel = ClimitProcess::LimitCheckFunc(cdbInfo.completeRate,  threshold.session);
            if(AlarmLevel != cdbErrAlarm[i]) {

                memset(comment, 0x00, sizeof(comment));
				if (static_cast<int>(AlarmLevel) != 0)    // 알람 발생
				{
					sprintf(comment[0], "%d%%", cdbInfo.completeRate);
					sprintf(comment[1], "OCCUR");
				}
				else                    // 알람 해제
				{
					sprintf(comment[0], "%d%%", cdbInfo.completeRate);
					sprintf(comment[1], "Clear");
				}
        
                alarmMNG.SendAlarm(cdbInfo.serverID, NO_PROCESS_ID, ALM_CODE_LIMIT_CDBRATE, i, static_cast<int>(AlarmLevel), 2, comment);
                Log.printf(LOG_LV3, "[LimitCheckThread] CDB Complete Rate alarm![%d], LEVEL[%d]\n", cdbInfo.completeRate, static_cast<int>(AlarmLevel));
                cdbErrAlarm[i] = AlarmLevel;
            }
		}
	
		sleep(5);
	}
}

LimitAlarmLevel ClimitProcess::LimitCheckFunc(int use, int limitLevel[])
{
    if (use >= limitLevel[0])
        return LimitAlarmLevel::CRITICAL;
    else if (use >= limitLevel[1])
        return LimitAlarmLevel::MAJOR;
    else if (use >= limitLevel[2])
        return LimitAlarmLevel::MINOR;
    else if (use >= limitLevel[3])
        return LimitAlarmLevel::WARNING;
    else
        return LimitAlarmLevel::NONE;
}
