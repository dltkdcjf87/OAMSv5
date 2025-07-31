#ifndef __OAMS_HWMON_H__
#define __OAMS_HWMON_H__

#include <oams_shm_lib.h>

class CHWMonitor
{
private:
	bool				m_bShmFlag;

public:
	CHWMonitor();
	~CHWMonitor();

	int Init_HWMonitor(void);

	static void* monitorHWThread(void* arg);

};

#endif	//__OAMS_HWMON_H__
