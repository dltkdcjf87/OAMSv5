#include "OAMS_altiCInf.h"

AltiCinf::AltiCinf()  
{
	m_bConnected = false;
    length[static_cast<int>(BindType::INT)] = sizeof(int);
    length[static_cast<int>(BindType::STRING)] = ALTIBASE_NTS;
}

AltiCinf::~AltiCinf() 
{
	altibase_close(altibase);
}

//DB OPTION/CONNECT/QUEUE PROCESS
bool AltiCinf::initialize(const char* szConnStr)
{
	int rc;

	strcpy(dbConnAddr, szConnStr);

	altibase     = altibase_init();
    if (altibase == NULL)
    {
		Log.printf(LOG_ERR, "ALTIBASE INIT FAIL !! \n");   
	    return false;
    }

	rc = altibase_set_autocommit(altibase, ALTIBASE_AUTOCOMMIT_ON);
	
	rc = altibase_set_option(altibase, ALTIBASE_CONNECTION_TIMEOUT, "5");

	pthread_create(&thread1, NULL, connectCheckThread, this);
	pthread_detach(thread1);

    pthread_create(&thread2, NULL, dbQueueProcessThread, this);
    pthread_detach(thread2);

/*
	thread t(&AltiCinf::connectCheckThread, this, szConnStr);
	t.detach();

	thread t2(&AltiCinf::dbQueueProcessThread, this);
	t2.detach();
*/
	Log.printf(LOG_LV3, "ALTIBASE INIT SUCC !! \n");

	return true;
}

/*
void AltiCinf::connectCheckThread(const char* connstr)
{
    int nSleep[] = { 0, 100, 500, 1000, 2000, 4000, 8000, 16000, 32000 };
    int nDBWaitCount = 0;

    while(1)
    {
        if(this->isConnect())
        {
            nDBWaitCount = 0;
            sleep(1);
            continue;
        }

        usleep(nSleep[nDBWaitCount] * 1000);  //milli sec
        if(sizeof(nSleep)/sizeof(*nSleep) > nDBWaitCount+1) nDBWaitCount++;

        Log.printf(LOG_LV3, "ALTIBASE CONNECT TRY[%d] !! \n",nDBWaitCount);
        this->connect(connstr);

    }
}

void AltiCinf::dbQueueProcessThread()
{
    SqlRequest *pSqlRequest;

    while(1)
    {
        if(sqlQueue.IsEmpty() == true)
        {
            sleep(1);
            continue;
        }
        pSqlRequest = sqlQueue.Pop();
        if(pSqlRequest->bindParams.size()==0){
            DirectQuery(pSqlRequest->query.c_str());
        } else {
            bindQueryExcute(pSqlRequest);
        }

        delete(pSqlRequest);
    }
}
*/



//DB DISCONNECT시 CONNECT RETRY

void* AltiCinf::connectCheckThread(void* arg)
{
	AltiCinf *pThis = static_cast<AltiCinf *>(arg);

    int nSleep[] = { 0, 100, 500, 1000, 2000, 4000, 8000, 16000, 32000 };
    int nDBWaitCount = 0;

	while(1)
	{
		if(pThis->isConnect())
		{
			nDBWaitCount = 0;
			sleep(1);
			continue;
		}

		usleep(nSleep[nDBWaitCount] * 1000);  //milli sec
        if(sizeof(nSleep)/sizeof(*nSleep) > nDBWaitCount+1) nDBWaitCount++;

		Log.printf(LOG_LV3, "ALTIBASE CONNECT TRY[%d] !! \n",nDBWaitCount);
		pThis->connect(pThis->dbConnAddr);

	}
}

//QUEUE PROCESS THREAD
void* AltiCinf::dbQueueProcessThread(void* arg)
{
	AltiCinf *pThis = static_cast<AltiCinf *>(arg); 
	SqlRequest *pSqlRequest;

	while(1)
	{
		if(pThis->sqlQueue.IsEmpty() == true)
		{
			sleep(1);
			continue;
		}
		pSqlRequest = pThis->sqlQueue.Pop();
		if(pSqlRequest->bindParams.size()==0){
			pThis->DirectQuery(pSqlRequest->query.c_str());
		} else {
			pThis->bindQueryExcute(pSqlRequest);
		}

		delete(pSqlRequest);
	}
}

// CONNECT
bool AltiCinf::connect(const char* connectString = "") 
{

	int rc;

	rc = altibase_connect(altibase, connectString);
	if (ALTIBASE_NOT_SUCCEEDED(rc))
	{
		Log.printf(LOG_ERR, "ALTIBASE CONNECT FAIL\n");
		return false;
	}
	
	m_bConnected = true;
	Log.printf(LOG_LV3, "ALTIBASE CONNECT !\n");

    return true;
}

// DISCONNECT
void AltiCinf::disconnect() {

	int rc;

	rc = altibase_close(altibase);

	m_bConnected = false;

}

// Direct Query
bool AltiCinf::DirectQuery(const char* sql)
{

	std::lock_guard<std::mutex> lock(altiMutex);
	int rc = altibase_query(altibase, sql);

    Log.printf(LOG_LV2, "[DirectQuery] START");
    if (ALTIBASE_NOT_SUCCEEDED(rc))
    {
		Log.printf(LOG_ERR, "DirectQuery ErrNo[%05X], ErrMsg[%s], SqlState[%s] ", altibase_errno(altibase)), altibase_error(altibase), altibase_sqlstate(altibase);
        return false;
    }
    Log.printf(LOG_LV2, "[DirectQuery] END");

	return true;
}

int AltiCinf::sqlBindExcute(SqlRequest SQLRecv)
{
	if(!this->isConnect())
    {
		Log.printf(LOG_ERR, "sqlBindExcute Connect Fail !! \n");
        return -1;
    }


	if((strstr(SQLRecv.query.c_str(), "SELECT") == NULL) && (strstr(SQLRecv.query.c_str(), "select") == NULL))
    {
        SqlRequest *request = new SqlRequest;
        *request = SQLRecv;
//		Log.printf(LOG_LV2, "sqlBindExcute SQL[%s][%s]\n ", request->query.c_str(),request->bindParams[0]);
        sqlQueue.Push(request);
        return 0;
    }

	return 0;
}

// SELECT Query
vector<map<string, string>> AltiCinf::sqlSelect(const char* sql)
{

	if(!this->isConnect())
    {
        cout<<"Not Connect!"<<endl;
        return {};
    }

	Log.printf(LOG_LV2, "QUERY REQUEST[%s] ", sql);

    if((strstr(sql, "SELECT") == NULL) && (strstr(sql, "select") == NULL))
    {   
        SqlRequest *request = new SqlRequest;
		*request = {sql,{}};
        sqlQueue.Push(request);
        return {};
    }


	ALTIBASE_FIELD *field;
	ALTIBASE_RES   result;
	ALTIBASE_ROW   row;
	ALTIBASE_LONG *lengths;
	int            num_fields;
	int            rc;
	int            i;

	string field_name;
	string field_data;

	vector<map<string, string>> results = {};
	map<string, string> rowDataMap;

	std::lock_guard<std::mutex> lock(altiMutex);
	rc = altibase_query(altibase, sql);
    if (ALTIBASE_NOT_SUCCEEDED(rc))
    {   
		Log.printf(LOG_ERR, "SQL SELECT ErrNo[%05X], ErrMsg[%s], SqlState[%s] ", altibase_errno(altibase)), altibase_error(altibase), altibase_sqlstate(altibase);
        
        return {};
    }
	result = altibase_use_result(altibase);

	num_fields = altibase_num_fields(result);
	while ((row = altibase_fetch_row(result)) != NULL)
	{
	   	lengths = altibase_fetch_lengths(result);
		rowDataMap.clear();
	   	for (i = 0; i < num_fields; i++)
	   	{
			field = altibase_field(result, i);
			rowDataMap.insert(make_pair(string(field->name), string(row[i] ? row[i] : "NULL")));
	   	}
		results.push_back(rowDataMap);
	}

	rc = altibase_free_result(result);

	return results;
}

// COMMIT
bool AltiCinf::commit() 
{
	std::lock_guard<std::mutex> lock(altiMutex);
	altibase_commit(altibase);

    return true;
}

// ROLLBACK
bool AltiCinf::rollback() 
{
	std::lock_guard<std::mutex> lock(altiMutex);
	altibase_rollback(altibase);

    return true;
}


// BIND QUERY
bool AltiCinf::bindQueryExcute(SqlRequest *request)
{
/*
	ALTIBASE_LONG length[static_cast<int>(BindType::MAXBIND)];
	length[static_cast<int>(BindType::INT)] = sizeof(int);
	length[static_cast<int>(BindType::STRING)] = ALTIBASE_NTS; 
*/

	ALTIBASE_STMT   altibaseBindStmt;
	
	std::lock_guard<std::mutex> lock(altiMutex);
    altibaseBindStmt = altibase_stmt_init(altibase);
    if(altibaseBindStmt == NULL)
    {
		Log.printf(LOG_ERR, "STMT NULL ErrNo[%05X], ErrMsg[%s], SqlState[%s] ",             
								altibase_stmt_errno(altibaseBindStmt),
					            altibase_stmt_error(altibaseBindStmt),
					            altibase_stmt_sqlstate(altibaseBindStmt)
		);
        return false;
    }

	int rc;
	rc  = altibase_stmt_prepare(altibaseBindStmt, request->query.c_str());
    if (ALTIBASE_NOT_SUCCEEDED(rc))
    {
		Log.printf(LOG_ERR, "BIND PREPARE FAIL ErrNo[%05X], ErrMsg[%s], SqlState[%s] ",             
								altibase_stmt_errno(altibaseBindStmt),
					            altibase_stmt_error(altibaseBindStmt),
					            altibase_stmt_sqlstate(altibaseBindStmt)	
		);
    }

	memset(bind, 0x00, sizeof(bind));

	for (int i =0; i<request->bindParams.size();i++) {
		switch(request->bindParams[i].type)
		{
		case BindType::INT:
	    	bind[i].buffer_type   = ALTIBASE_BIND_INTEGER;
			bind[i].buffer	=	&request->bindParams[i].intValue;
			bind[i].length = &length[static_cast<int>(BindType::INT)];
			break;
		case BindType::STRING:
			bind[i].buffer_type   = ALTIBASE_BIND_STRING;
            bind[i].buffer  =   request->bindParams[i].strValue;
			bind[i].buffer_length = sizeof(request->bindParams[i].strValue);
            bind[i].length = &length[static_cast<int>(BindType::STRING)];
			break;
		default:
			Log.printf(LOG_ERR, "BIND TYPE NOT DEFINE !\n");
			return false;
		
		}
    }

	rc=altibase_stmt_bind_param(altibaseBindStmt, bind);
    if (ALTIBASE_NOT_SUCCEEDED(rc))
    {
		Log.printf(LOG_ERR, "BIND FAIL ErrNo[%05X], ErrMsg[%s], SqlState[%s] \n", 
			altibase_stmt_errno(altibaseBindStmt), 
			altibase_stmt_error(altibaseBindStmt), 
			altibase_stmt_sqlstate(altibaseBindStmt));
		return false;
    }

    rc = altibase_stmt_execute (altibaseBindStmt);
	    if (ALTIBASE_NOT_SUCCEEDED(rc))
	    {
			Log.printf(LOG_ERR, "EXECUTE FAIL ErrNo[%05X], ErrMsg[%s], SqlState[%s] \n",
		            altibase_stmt_errno(altibaseBindStmt),
            altibase_stmt_error(altibaseBindStmt),
            altibase_stmt_sqlstate(altibaseBindStmt));
			return false;
	    }

    altibase_stmt_close(altibaseBindStmt);


	return true;
}



test_select AltiCinf::f_test_select(int a)
{
	test_select result;
	ALTIBASE_BOOL is_null[40];

    SqlRequest request = {
       "SELECT MIN_NUM, MAX_NUM, AVG_NUM FROM TBL_TEST WHERE MIN_NUM = ?",
        {
            {1, a}
        }
    };	

	std::lock_guard<std::mutex> lock(altiMutex);
	ALTIBASE_STMT   altibaseBindStmt;
    altibaseBindStmt = altibase_stmt_init(altibase);
    if(altibaseBindStmt == NULL)
    {
		Log.printf(LOG_ERR, "FAIL ErrNo[%05X], ErrMsg[%s], SqlState[%s] ", altibase_errno(altibase)), altibase_error(altibase), altibase_sqlstate(altibase);
        return {};
    }

	if(!bindQueryExcute(&altibaseBindStmt, request))
	{
		Log.printf(LOG_ERR, "TEST BIND QUERY FAIL ErrNo[%05X], ErrMsg[%s], SqlState[%s] ", altibase_errno(altibase)), altibase_error(altibase), altibase_sqlstate(altibase);
	}

	bind[0].buffer_type   = ALTIBASE_BIND_INTEGER;
    bind[0].buffer  =   &result.a;
    bind[0].length = &length[static_cast<int>(BindType::INT)];
	bind[0].is_null       = &is_null[0];

    bind[1].buffer_type   = ALTIBASE_BIND_INTEGER;
    bind[1].buffer  =   &result.b;
    bind[1].length = &length[static_cast<int>(BindType::INT)];
	bind[1].is_null       = &is_null[1];

    bind[2].buffer_type   = ALTIBASE_BIND_INTEGER;
    bind[2].buffer  =   &result.c;
    bind[2].length = &length[static_cast<int>(BindType::INT)];
	bind[2].is_null       = &is_null[2];


int rc = altibase_stmt_bind_result(altibaseBindStmt, bind);
if (ALTIBASE_NOT_SUCCEEDED(rc))
{
    /* ... error handling ... */
}

/* altibase_stmt_store_result() is optional */
rc = altibase_stmt_store_result(altibaseBindStmt);
/* ... check return value ... */

for (int row = 0; (rc = altibase_stmt_fetch(altibaseBindStmt)) != ALTIBASE_NO_DATA; row++)
{
    if (ALTIBASE_NOT_SUCCEEDED(rc))
    {
        /* ... error handling ... */
        break;
    }

    printf("row %d : ", row);
	printf("[%d][%d][%d]!!!!!!!!!!!!!!!!!\n", result.a, result.b, result.c);
}

	altibase_stmt_close(altibaseBindStmt);

return result;
}



bool AltiCinf::test(int a, int b, int c)
{
	SqlRequest *request = new SqlRequest;
	
    *request = {
       "INSERT INTO TBL_TEST (MIN_NUM, MAX_NUM, AVG_NUM) VALUES (?, ?, ?)", 
        {
            {1, a},
            {2, b},
            {3, c}
        }
    };

	sqlQueue.Push(request);
	return true;


/*
    ALTIBASE_STMT altibaseBindStmt = altibase_stmt_init(altibase);
    if(altibaseBindStmt == NULL)
    {
        cout<<"not stmt init"<<endl;
        return false;
    }

	int rc = bindQueryExcute(altibaseBindStmt, request);
	int int_data[3];

	memset(bind, 0x00, sizeof(bind));
	for(int i = 0; i < para_count; i++)
	{
            bind[i].buffer_type   = ALTIBASE_BIND_INTEGER;
            bind[i].buffer  =   &int_data[i];
            bind[i].length = &length[static_cast<int>(BindType::INT)];
	}


rc = altibase_stmt_bind_result(altibaseBindStmt, bind);
if (ALTIBASE_NOT_SUCCEEDED(rc))
{
	printf("bind fialfsd\n");
}

rc = altibase_stmt_store_result(altibaseBindStmt);

for (int i = 0; (rc = altibase_stmt_fetch(altibaseBindStmt)) != ALTIBASE_NO_DATA; i++)
{
    if (ALTIBASE_NOT_SUCCEEDED(rc))
    {
        break;
    }

    printf("row %d : ", i);
    printf("[%d][%d][%d]\n",int_data[0], int_data[1], int_data[2]);
}



	altibase_stmt_close(altibaseBindStmt);

	return true;
*/
}


//vector<string> AltiCinf::bindQueryExcute(SqlRequest request, SqlResponse response)

bool AltiCinf::bindQueryExcute(ALTIBASE_STMT* altibaseBindStmt, SqlRequest request)
{

    int rc;

    Log.printf(LOG_ERR, "[bindQueryExcute] START ");

    rc  = altibase_stmt_prepare(*altibaseBindStmt, request.query.c_str());
    if (ALTIBASE_NOT_SUCCEEDED(rc))
    {
        Log.printf(LOG_ERR, "STMT PREPARE FAIL ErrNo[%05X], ErrMsg[%s], SqlState[%s]",
            altibase_stmt_errno(altibaseBindStmt),
            altibase_stmt_error(altibaseBindStmt),
            altibase_stmt_sqlstate(altibaseBindStmt));
		return false;
    }

    memset(bind, 0x00, sizeof(bind));

    for (int i =0; i<request.bindParams.size();i++) {
        switch(request.bindParams[i].type)
        {
        case BindType::INT:
            bind[i].buffer_type   = ALTIBASE_BIND_INTEGER;
            bind[i].buffer  =   &request.bindParams[i].intValue;
            bind[i].length = &length[static_cast<int>(BindType::INT)];
            break;
        case BindType::STRING:
            bind[i].buffer_type   = ALTIBASE_BIND_STRING;
            bind[i].buffer  =   request.bindParams[i].strValue;
            bind[i].buffer_length = sizeof(request.bindParams[i].strValue);
            bind[i].length = &length[static_cast<int>(BindType::STRING)];
            break;
        default:
            Log.printf(LOG_ERR, "[bindQueryExcute] BindType Not Type ");
            return false;

        }
    }

    rc=altibase_stmt_bind_param(*altibaseBindStmt, bind);
    if (ALTIBASE_NOT_SUCCEEDED(rc))
    {
        Log.printf(LOG_ERR, "Bind error no : %05X\n",  altibase_stmt_errno(altibaseBindStmt));
        Log.printf(LOG_ERR, "bind1 error msg : %s\n", altibase_stmt_error(altibaseBindStmt));
        Log.printf(LOG_ERR, "bind1 sqlstate  : %s\n", altibase_stmt_sqlstate(altibaseBindStmt));

		return false;
    }

    rc = altibase_stmt_execute (*altibaseBindStmt);
        if (ALTIBASE_NOT_SUCCEEDED(rc))
    {
        Log.printf(LOG_ERR, "STMT EXECUTE error no : %05X\n",  altibase_stmt_errno(altibaseBindStmt));
        Log.printf(LOG_ERR, "bind1 error msg : %s\n", altibase_stmt_error(altibaseBindStmt));
        Log.printf(LOG_ERR, "bind1 sqlstate  : %s\n", altibase_stmt_sqlstate(altibaseBindStmt));

		return false;
    }

    Log.printf(LOG_ERR, "[bindQueryExcute] END ");
	return true;
}

