#ifndef __CCONFIG_H__
#define __CCONFIG_H__

#include <pthread.h>
#include <string>
#include <map>
using namespace std;


#define ROLE_DEF_FILE   "/home/mcpas/cfg/role.def"
#define OAMS_CFG_FILE   "/home/mcpas/cfg/EMS%d/oams.cfg"

typedef struct {
	bool	bUseflag;
	char	OMSA_IP[128];	// OMS-A IP
	char	OMSB_IP[128];	// OMS-B IP
} MGM_INFO;

class CConfig
{
public:
	CConfig();
	~CConfig();

	int	ReadConfig();
	int	PrintConfig();

	int GetConfigString(const char *pszkey, char *pszValue);
	int	GetMgmInfo(int nIndex, char *pszIP1, char *pszIP2)
	{
		if (m_MgmInfo[nIndex].bUseflag == false)
			return(0);
		sprintf(pszIP1, "%s", m_MgmInfo[nIndex].OMSA_IP);
		sprintf(pszIP2, "%s", m_MgmInfo[nIndex].OMSB_IP);

		return(1);
	}
    int GetMY_SIDE(void) { return m_nEMSID; }
    int GetConfigValuesBySection(const char* pszSection, std::vector<std::pair<std::string, std::string>>& vecKeyValues);

	int GetNetDeviceName(int nIndex, char *pszName)
	{
		if ( strlen(m_szNetDeviceName[nIndex]) <= 0 )
			return(0);

		sprintf(pszName, "%s", m_szNetDeviceName[nIndex]);

		return(1);
	}

protected:
	int GetEMS_ID(void);
	int ReadCfgValue(char *pszBuf, char *pszKey, char *pszValue);


private:
    mutable std::mutex m_pMutex;

	map<string, string> m_CfgMap;		// 일반 config Map
	// 특수 config
	MGM_INFO			m_MgmInfo[MAX_AS];	// AS 별 OMS A/B IP
	//
    //MCCS VALUE 	
    vector<string> MccsProcess;

	int	m_nEMSID;

	char			m_szNetDeviceName[MAX_LAN_PORT][20];
};

#endif //__CCONFIG_H__
