#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <sys/time.h>

#include "OAMS_common.h"
#include "OAMS_config.h"


CHWMonitor::CHWMonitor()
{
	m_bShmFlag = false;
}


CHWMonitor::~CHWMonitor()
{
}


int CHWMonitor::Init_HWMonitor(void)
{
	int	nRet;
	pthread_t hth;

	nRet = oams_shm_init(OAMS_I);
	if (nRet < 0)
		Log.printf(LOG_ERR, "[CHWMonitor][Init_HWMonitor] H/W Shared memory attach fail !!!");
	else
		m_bShmFlag = true;

	// H/W 감시 monitoring start
	pthread_create(&hth, NULL, monitorHWThread, this);
	pthread_detach(hth);

	return(1);	
}


void* CHWMonitor::monitorHWThread(void* arg)
{
    CHWMonitor *pThis = static_cast<CHWMonitor *>(arg);

	time_t				SvrRebuildTimer[SHM_MAX_SERVER];
	SHM_SERVER_INFO     shmSvr[SHM_MAX_SERVER];
	SERVER_INFO			dtlSvr;
	int 				nRet, i, j;
	char    			comment[8][64];

	memset(SvrRebuildTimer, 0, sizeof(time_t)*SHM_MAX_SERVER);

	while(1)
	{
		// H/W Shared memory 미접속 시 로그 출력, 추후 주기적으로 attach 시도 추가 ???
		if (!pThis->m_bShmFlag)
		{
			Log.printf(LOG_ERR, "[CHWMonitor][monitorHWThread] H/W Shared memory attach fail !!!");
			sleep(5);
			continue;
		}
		sleep(1);

		// Copy Shared memory Server Info
		nRet = shm_getShmServerInfo(shmSvr);
		if (nRet < 0)
		{
			Log.printf(LOG_ERR, "[CHWMonitor][monitorHWThread] H/W Shared memory get fail (ret=%d)", nRet);
			sleep(4);
			continue;
		}

		// Copy DataLayer Server Info
		for ( i=0 ; i<MAX_SERVER ; i++ )
		{
			memset(&dtlSvr, -1, sizeof(SERVER_INFO));
			serverInfoManager.getServerInfo(i, dtlSvr);

			if (dtlSvr.serverId == -1)
				continue;	// 미사용 서버

			// Check FAN
			for ( j=0 ; j<MAX_HW ; j++ )
			{
				if ( shmSvr[i].HwData.HwFan[j] == -1)
					continue;

				Log.printf(LOG_LV1, "[CHWMonitor][monitorHWThread] ServerID = %d(%s), FAN[%d], Shm[%s](%d) - DTL[%s](%d)", i, dtlSvr.serverName, j, shmSvr[i].HwData.HwFan[j]==0?"DOWN":"UP", shmSvr[i].HwData.HwFan[j], dtlSvr.hwData.fanStatus[j]==0?"DOWN":"UP", dtlSvr.hwData.fanStatus[j]);
				if ( shmSvr[i].HwData.HwFan[j] == ST_DOWN and dtlSvr.hwData.fanStatus[j] == ST_UP )
				{
					// FAN 알람 발생
					memset(comment, 0x00, sizeof(comment));
					sprintf(comment[0], "%d", j);
					sprintf(comment[1], "DOWN");
					alarmMNG.SendAlarm(i, -1, ALM_CODE_HW_FAN, j, ALM_ON, 2, comment);

					Log.printf(LOG_ERR, "[CHWMonitor][monitorHWThread] ServerID = %d(%s), FAN[%d] DOWN alarm-detect", i, dtlSvr.serverName, j);
					dtlSvr.hwData.fanStatus[j] = ST_DOWN;
				}
				else if ( shmSvr[i].HwData.HwFan[j] == ST_UP and dtlSvr.hwData.fanStatus[j] == ST_DOWN)
				{
					// FAN 알람 해제
					memset(comment, 0x00, sizeof(comment));
                    sprintf(comment[0], "%d", j);
                    sprintf(comment[1], "UP");
					alarmMNG.SendAlarm(i, -1, ALM_CODE_HW_FAN, j, ALM_OFF, 2, comment);

					Log.printf(LOG_INF, "[CHWMonitor][monitorHWThread] ServerID = %d(%s), FAN[%d] UP alarm-clear", i, dtlSvr.serverName, j);
					dtlSvr.hwData.fanStatus[j] = ST_UP;
				}
			}

			// Check Power
			for ( j=0 ; j<MAX_HW ; j++ )
			{
				if ( shmSvr[i].HwData.HwPower[j] == -1)
					continue;

				Log.printf(LOG_LV1, "[CHWMonitor][monitorHWThread] ServerID = %d(%s), POWER[%d], Shm[%s](%d) - DTL[%s](%d)", i, dtlSvr.serverName, j, shmSvr[i].HwData.HwPower[j]==0?"DOWN":"UP", shmSvr[i].HwData.HwPower[j], dtlSvr.hwData.powerStatus[j]==0?"DOWN":"UP", dtlSvr.hwData.powerStatus[j]);
				if ( shmSvr[i].HwData.HwPower[j] == ST_DOWN and dtlSvr.hwData.powerStatus[j] == ST_UP )
				{
					// POWER 알람 발생
                    memset(comment, 0x00, sizeof(comment));
                    sprintf(comment[0], "%d", j);
                    sprintf(comment[1], "DOWN");
                    alarmMNG.SendAlarm(i, -1, ALM_CODE_HW_POWER, j, ALM_ON, 2, comment);

					Log.printf(LOG_ERR, "[CHWMonitor][monitorHWThread] ServerID = %d(%s), POWER[%d] DOWN alarm-detect", i, dtlSvr.serverName, j);
					dtlSvr.hwData.powerStatus[j] = ST_DOWN;
				}
				else if (shmSvr[i].HwData.HwPower[j] == ST_UP and dtlSvr.hwData.powerStatus[j] == ST_DOWN)
				{
					// POWER 알람 해제
                    memset(comment, 0x00, sizeof(comment));
                    sprintf(comment[0], "%d", j);
                    sprintf(comment[1], "UP");
                    alarmMNG.SendAlarm(i, -1, ALM_CODE_HW_POWER, j, ALM_OFF, 2, comment);

					Log.printf(LOG_INF, "[CHWMonitor][monitorHWThread] ServerID = %d(%s), POWER[%d] UP alarm-clear", i, dtlSvr.serverName, j);
					dtlSvr.hwData.powerStatus[j] = ST_UP;
				}
			}

			// Check Disk
			for ( j=0 ; j<MAX_HW ; j++ )
			{
				if ( shmSvr[i].HwData.HwDisk[j] == -1)
					continue;

				Log.printf(LOG_LV1, "[CHWMonitor][monitorHWThread] ServerID = %d(%s), DISK[%d], Shm[%s](%d) - DTL[%s](%d)", i, dtlSvr.serverName, j, shmSvr[i].HwData.HwDisk[j]==0?"DOWN":"UP", shmSvr[i].HwData.HwDisk[j], dtlSvr.hwData.diskStatus[j]==0?"DOWN":"UP", dtlSvr.hwData.diskStatus[j]);
				if ( shmSvr[i].HwData.HwDisk[j] == ST_DOWN and dtlSvr.hwData.diskStatus[j] == ST_UP )
				{
					// Disk 알람 발생
                    memset(comment, 0x00, sizeof(comment));
                    sprintf(comment[0], "%d", j);
                    sprintf(comment[1], "DOWN");
                    alarmMNG.SendAlarm(i, -1, ALM_CODE_HW_DISK, j, ALM_ON, 2, comment);

					Log.printf(LOG_ERR, "[CHWMonitor][monitorHWThread] ServerID = %d(%s), DISK[%d] DOWN alarm-detect", i, dtlSvr.serverName, j);
					dtlSvr.hwData.diskStatus[j] = ST_DOWN;
				}
				else if (shmSvr[i].HwData.HwDisk[j] == ST_UP and dtlSvr.hwData.diskStatus[j] == ST_DOWN)
				{
					// Disk 알람 해제
                    memset(comment, 0x00, sizeof(comment));
                    sprintf(comment[0], "%d", j);
                    sprintf(comment[1], "UP");
                    alarmMNG.SendAlarm(i, -1, ALM_CODE_HW_DISK, j, ALM_OFF, 2, comment);

					Log.printf(LOG_INF, "[CHWMonitor][monitorHWThread] ServerID = %d(%s), DISK[%d] UP alarm-clear", i, dtlSvr.serverName, j);
					dtlSvr.hwData.diskStatus[j] = ST_UP;
				}
			}

			// Check Network
			for ( j=0 ; j<MAX_LAN_PORT ; j++ )
			{
				if ( shmSvr[i].HwData.HwNetwork[j] == -1)
					continue;

				char	szNetDevice[128];
				sprintf(szNetDevice, "%d Unknown", j);

				Log.printf(LOG_LV1, "[CHWMonitor][monitorHWThread] ServerID = %d(%s), NETWORK[%d], Shm[%s](%d) - DTL[%s](%d)", i, dtlSvr.serverName, j, shmSvr[i].HwData.HwNetwork[j]==0?"DOWN":"UP", shmSvr[i].HwData.HwNetwork[j], dtlSvr.hwData.netStatus[j].status==0?"DOWN":"UP", dtlSvr.hwData.netStatus[j].status);
				if ( shmSvr[i].HwData.HwNetwork[j] == ST_DOWN and dtlSvr.hwData.netStatus[j].status == ST_UP )
				{
					g_cfg.GetNetDeviceName(j, szNetDevice);
	
					// Network 알람 발생
                    memset(comment, 0x00, sizeof(comment));
                    sprintf(comment[0], "%s", szNetDevice);
                    sprintf(comment[1], "DOWN");
                    alarmMNG.SendAlarm(i, -1, ALM_CODE_HW_NETWORK, j, ALM_ON, 2, comment);

					Log.printf(LOG_ERR, "[CHWMonitor][monitorHWThread] ServerID = %d(%s), NETWORK[%s] DOWN alarm-detect", i, dtlSvr.serverName, szNetDevice);
					dtlSvr.hwData.netStatus[j].status = ST_DOWN;
				}
				else if ( shmSvr[i].HwData.HwNetwork[j] == ST_UP and dtlSvr.hwData.netStatus[j].status == ST_DOWN)
				{
					g_cfg.GetNetDeviceName(j, szNetDevice);

					// Network 알람 해제
                    memset(comment, 0x00, sizeof(comment));
                    sprintf(comment[0], "%s", szNetDevice);
                    sprintf(comment[1], "UP");
                    alarmMNG.SendAlarm(i, -1, ALM_CODE_HW_NETWORK, j, ALM_OFF, 2, comment);

					Log.printf(LOG_INF, "[CHWMonitor][monitorHWThread] ServerID = %d(%s), NETWORK[%s] UP alarm-clear", i, dtlSvr.serverName, szNetDevice);
					dtlSvr.hwData.netStatus[j].status = ST_UP;
				}
			}

			// Check Rebuild
			if (shmSvr[i].nRebuildFlag == RD_DEFAULT)
				shmSvr[i].nRebuildFlag = RD_NORMAL;
			Log.printf(LOG_LV1, "[CHWMonitor][monitorHWThread] ServerID = %d(%s), Rebuild flag Shm[%s](%d) - DTL[%s](%d)", i, dtlSvr.serverName, shmSvr[i].nRebuildFlag==1?"REBUILD":"NORMAL", shmSvr[i].nRebuildFlag, dtlSvr.rebuild==1?"REBUILD":"NORMAL", dtlSvr.rebuild);
			if ( shmSvr[i].nRebuildFlag != dtlSvr.rebuild )
			{
				if ( dtlSvr.rebuild == RD_NORMAL )
				{
					// Rebuild 상태 알람
					memset(comment, 0x00, sizeof(comment));
					sprintf(comment[0], "OCCUR");

                    alarmMNG.SendAlarm(i, -1, ALM_CODE_HW_DISKREBUILD, j, ALM_ON, 1, comment);
					SvrRebuildTimer[i] = time(NULL);
				}
				else
				{
					// Rebuild 상태 알람 해제
					memset(comment, 0x00, sizeof(comment));
					sprintf(comment[0], "Clear");

					alarmMNG.SendAlarm(i, -1, ALM_CODE_HW_DISKREBUILD, j, ALM_OFF, 1, comment);

					// Rebuilding 진행률 전송.. 100%
					char    szMsg[256];
					sprintf(szMsg, "[%s][%s] DISK REBUILDING 100 %%", dtlSvr.asName, dtlSvr.serverName);
					alarmMNG.RelayOccured(dtlSvr.asId, dtlSvr.serverId, -1, szMsg);
					SvrRebuildTimer[i] = 0;
				}
				dtlSvr.rebuild = shmSvr[i].nRebuildFlag;
			}
			if ( shmSvr[i].nRebuildFlag == RD_REBUILD )
			{
				if ( SvrRebuildTimer[i] != 0 && ((time(NULL) - SvrRebuildTimer[i])>60))
				{
					char	szMsg[256];
					sprintf(szMsg, "[%s][%s] DISK REBUILDING %d %%", dtlSvr.asName, dtlSvr.serverName, shmSvr[i].nRebuildRate);
					alarmMNG.RelayOccured(dtlSvr.asId, dtlSvr.serverId, -1, szMsg); // Rebuilding 진행률 전송....
					Log.printf(LOG_LV2, "[CHWMonitor][monitorHWThread] ServerID = %d(%s), Rebuilding %d %%", i, dtlSvr.serverName, shmSvr[i].nRebuildRate);
					SvrRebuildTimer[i] = time(NULL);
				}
			}

			// H/W 상태 update
			serverInfoManager.setRebuildAndHWServerInfo(i, dtlSvr.rebuild, dtlSvr.hwData);

			Log.printf(LOG_LV2, "[CHWMonitor][monitorHWThread] ServerID = %d(%s), Temperature : %d", i, dtlSvr.serverName, shmSvr[i].nCpuTemp);
			if ( shmSvr[i].nCpuTemp >= 0)
			{
				// 온도 데이터 update
				performanceManager.setTemperaturePerformance(i, shmSvr[i].nCpuTemp);

				// 온도 통계 (min, ave, max) 함수 호출
    			systemresourcestatManager.setSystemResourceStat(i, "TEMPERATURE", shmSvr[i].nCpuTemp);
			}
		}
	}

	return(NULL);
}

