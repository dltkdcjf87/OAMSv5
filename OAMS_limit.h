#ifndef __CLIMIT_H__
#define __CLIMIT_H__

#include <stdio.h>
#include <pthread.h>
#include "OAMS_define.h"

enum class LimitAlarmLevel {
    NONE = 0,
    WARNING,
    MINOR,
    MAJOR,
    CRITICAL
};

class ClimitProcess {
private :
	static void* LimitCheckThread(void* arg);
	static LimitAlarmLevel LimitCheckFunc(int use, int limitLevel[]);


public : 
    ClimitProcess();
    ~ClimitProcess();

	static bool initialize();

};

#endif // __CLIMIT_H__
