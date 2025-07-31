#include "OAMS_define.h" 
#include "OAMS_common.h" 

bool OAMS_init_data(int argc, char* argv[])
{
    int  pid;
    char szPath[128];
    char szLogMaxSize[10];
    char szLogLevel[2];
    char szSvrEmsA[5];
    char szSvrEmsB[5];

    memset(szPath, 0x00, sizeof(szPath));
    memset(szLogMaxSize, 0x00, sizeof(szLogMaxSize));
    memset(szLogLevel, 0x00, sizeof(szLogLevel));
    memset(szSvrEmsA, 0x00, sizeof(szSvrEmsA));
    memset(szSvrEmsB, 0x00, sizeof(szSvrEmsB));

    //LOG CFG READ
    g_cfg.ReadConfig();
    g_cfg.GetConfigString("DEBUG.LOG_MAX_SIZE", szLogMaxSize);
    g_cfg.GetConfigString("DEBUG.LOG_LEVEL", szLogLevel);
    g_cfg.GetConfigString("DEBUG.LOG_PATH", szPath);

    g_cfg.GetConfigString("EXPORTER.FIX_PORT", gFixPort);
    g_cfg.GetConfigString("EXPORTER.ALARM_PORT", gAlarmPort);

    g_cfg.GetConfigString("MCCS.CFG_MCCS_USE",  gMccsUse);
    g_cfg.GetConfigString("MCCS.EMSA", gMccsEmsA); //ems hostname
    g_cfg.GetConfigString("MCCS.EMSB", gMccsEmsB); //ems hostname

    MY_SIDE = g_cfg.GetMY_SIDE();
    OTHER_SIDE = (MY_SIDE == 0) ? 1 : 0;
    gSvrEmsA = atoi(szSvrEmsA);
    gSvrEmsB = atoi(szSvrEmsB);
    g_bDEBUG_LOGFILE = true;
    g_nDebug_level = atoi(szLogLevel);

    //LOG CLASS SET
    if (!Log.init(szPath, "oams")) {
        exit(-1);
    }
    Log.set_max_limit(atoi(szLogMaxSize) * 1024 * 1024);
    Log.set_level(g_nDebug_level);

    Log.printf(LOG_LV2, "[OAMS_init_data] START ");

    signal(SIGINT,  shut_down);
    signal(SIGTERM, shut_down);
    //signal(SIGQUIT, shut_down);
    signal(SIGHUP,  SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGALRM, SIG_IGN);

    if(LIB_is_running_process(MY_PS_NAME, &pid) == true)
    {
        printf("Already running process check PID %d ", pid);
        Log.printf(LOG_ERR, "Already running process check PID %d ", pid);
        exit(0);
    }

    if(LIB_set_run_pid(MY_PS_NAME) == false)                //  프로세스 정보 등록 - 중복 실행 방지를 위해
    {
        Log.printf(LOG_ERR, "set /var/run/%s.pid error ", MY_PS_NAME);
        return(false);
    }

    g_cfg.PrintConfig();

    // MCCS PROCESS NAME, ALARM_CODE
    if (g_cfg.GetConfigValuesBySection("MCCS_PROC", gMccsProcess) > 0) {
        for (const auto& val : gMccsProcess)
        {
            Log.printf(LOG_INF, "MCCS PROCESS KEY: %s VALUE: %s", val.first.c_str(), val.second.c_str());
        }
    } else {
        Log.printf(LOG_ERR, "MCCS SECTION NOT FOUND");
    }
    // EMS PROCESS NAME, ALARM_CODE
    if (g_cfg.GetConfigValuesBySection("EMS_PROC", gEmsProcess) > 0) {
        for (const auto& val : gEmsProcess)
        {   
            Log.printf(LOG_INF, "EMS PROCESS KEY: %s VALUE: %s", val.first.c_str(), val.second.c_str());
        }
    } else {
        Log.printf(LOG_ERR, "EMS SECTION NOT FOUND");
    }

    asInfoManager.Init();
    chassisInfoManager.Init();
    serverInfoManager.Init();
    moduleInfoManager.Init();
    asstatsInfoManager.Init();
    systemresourcestatManager.Init();
    performanceManager.Init();
    thresholdInfoManager.Init();
    sswconfigInfoManager.Init();
    msconfigInfoManager.Init();
    externalIfInfoManager.Init();
    cdbstatusInfoManager.Init();
    alarmconfigManager.Init();

    if(MY_SIDE == 0)
        init_btxbus(OAMS_A, sbus_callback, mbus_callback);
    else
        init_btxbus(OAMS_B, sbus_callback, mbus_callback);

    reg_mod_state_change(moduleEmsGwsStateChange);

    Log.printf(LOG_LV2, "[OAMS_init_data] END ");

    return(true);

}
int main(int argc, char* argv[])
{
    int     ret;
    struct rlimit rlim;
	int svrid;

    if(OAMS_init_data(argc, argv) == false) { exit(-1); }

    getrlimit( RLIMIT_STACK, &rlim );
    printf( "Current Stack Size : [%d] Max Current Stack Size : [%d] \n", rlim.rlim_cur, rlim.rlim_max );
    rlim.rlim_cur = (1024 * 1024 * 10);
    rlim.rlim_max = (1024 * 1024 * 10);
    setrlimit( RLIMIT_STACK, &rlim );
    printf( "Current Stack Size : [%d] Max Current Stack Size : [%d] \n", rlim.rlim_cur, rlim.rlim_max );


    // DB INterface START
/*
    pthread_t   pthreadhd;

    pthread_create(&pthreadhd, NULL, &initDB, NULL);
    pthread_detach(pthreadhd);
	sleep(1);
*/
	initDBThread();

	while(!dbConnect)
	{
		sleep(1);
		Log.printf(LOG_LV1, "[OAMS_init_DB] DB Connect Wait!! ");
		continue;
	}

	limitProc.initialize();
	statisticManager.StatMngInit();

    // XBUS Interface START
    startXbusMsg(); 

	alarmMNG.Init();
	externServerManager.Init();

    // MGM Interface START
    startMGM_IF();

    // EMS Monitoring START   
    startEmsCheckMonitor();

    fixMetric.startFixMetrics("", gFixPort);
    alarmMetric.startAlarmMetrics("", gAlarmPort);

	hwMonitorManager.Init_HWMonitor();

    while(true)
    {
        sleep(5);
    }

    return 0;
}

