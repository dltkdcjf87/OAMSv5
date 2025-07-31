#ifndef ALTIBASE_DB_H
#define ALTIBASE_DB_H

#include <alticapi.h>
#include "OAMS_common.h"
/*
#include <stdio.h>
#include <iostream>
#include <string>
#include <alticapi.h>
#include <string.h>
#include <thread>
#include <unistd.h>
#include <vector>
#include <queue>
#include <map>
#include <vector>
#include "CQueueBase.h"

#include    "libsmqueue.h"
*/
using namespace std;
typedef pthread_t THREAD_HANDLE;


#define CONNSTR "DSN=%s;PORT_NO=%s;UID=%s;PWD=%s"
#define PARAM_COUNT 32

#define SEND_MAX_SIZE 1024

enum class BindType { INT, STRING, MAXBIND };
enum class QueryType { DIRECT, BIND };


// 바인드 변수 구조체
class BindParam {
private:

public:
    int index;        // 바인드 위치
    BindType type;    // 데이터 타입
    union {
        int intValue;
        char strValue[256];  // 문자열 (고정 크기)
    };

    BindParam(int idx, int value) : index(idx), type(BindType::INT), intValue(value) {}
//    BindParam(int idx, const string &value) : index(idx), type(BindType::STRING) {
    BindParam(int idx, const char* value) : index(idx), type(BindType::STRING) {
		strncpy(strValue, value, sizeof(strValue) - 1);
//        strncpy(strValue, value.c_str(), sizeof(strValue) - 1);
        strValue[sizeof(strValue) - 1] = '\0';
    }
};



// SQL 요청을 저장하는 구조체
struct SqlRequest {
    string query;
    vector<BindParam> bindParams;
};

typedef vector<BindType> SqlResponse;


typedef struct {
int a;
int b;
int c;
} test_select;


class AltiCinf
{
private:
	bool m_bConnected;
	ALTIBASE 		altibase;
	ALTIBASE_BIND 	bind[PARAM_COUNT];
	ALTIBASE_LONG length[static_cast<int>(BindType::MAXBIND)];

	pthread_t thread1, thread2;

//    void connectCheckThread(const char* connstr);
//    void dbQueueProcessThread();


	static void* connectCheckThread(void* arg);
	static void* dbQueueProcessThread(void* arg);

	bool connect(const char* connectString/* ="" */);
	void disconnect(void);

	CQueueBase<SqlRequest *> sqlQueue;
	mutex altiMutex;

	char dbConnAddr[256];

public:
	AltiCinf();
	virtual ~AltiCinf();

	inline bool isConnect(void) { return m_bConnected; };

	bool initialize(const char* szConnStr);
	void uninitialize();

	vector<map<string, string>> sqlSelect(const char* sql);
	int sqlBindExcute(SqlRequest SQLRecv);
	bool DirectQuery(const char* sql);
	bool closeResultSet(ALTIBASE_RES* rs);
	bool sqlSelect(const char* sql, std::string& sResultData);
	bool bindQueryExcute(SqlRequest* request);
	bool bindQueryExcute(ALTIBASE_STMT* altibaseBindStmt, SqlRequest request);

	test_select f_test_select(int a);
	bool test(int a, int b, int c);
//	bool bindTest(SqlRequest request);

	bool commit();
	bool rollback();
};

#endif // ALTIBASE_DB_H


