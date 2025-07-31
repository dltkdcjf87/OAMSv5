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

extern int  ParsingDelimeter(char *bp, char delimeter, char **valp, int size);
extern int  Trim(char *pszBuf, int nLen);


CConfig::CConfig()
{
	m_nEMSID = -1;

	memset(m_szNetDeviceName, 0x00, sizeof(char)*MAX_LAN_PORT*20);
}


CConfig::~CConfig()
{
}


int CConfig::GetEMS_ID(void)
{
    FILE    *pFile = NULL;
    char    szBuf[128], *pszParam[50];
    int     nParamCnt;

    pFile = fopen(ROLE_DEF_FILE, "r");
    if (pFile)
    {
        while(!feof(pFile))
        {
            if (fgets(szBuf, sizeof(szBuf), pFile) != NULL)
            {
                if(!strncmp(szBuf, "EMS", strlen("EMS")))
                {
                    nParamCnt = ParsingDelimeter(szBuf, '=', pszParam, strlen(szBuf));
					m_nEMSID = atoi(pszParam[1]);
                }
            }
        }
        fclose(pFile);
    }
    else
        return(0);

    return(1);
}


int CConfig::ReadCfgValue(char *pszBuf, char *pszKey, char *pszValue)
{   
    int     nParamCnt;
    char    szTemp[256], *pszParam[50];
    
    //printf("[CConfig] ReadCfgValue START \n ");
    nParamCnt = ParsingDelimeter(pszBuf, '=', pszParam, strlen(pszBuf));
    if (nParamCnt != 2)
    {   
        printf("[CConfig] ReadCfgValue Invalid config format - <%s> \n ", pszParam[0]);
        return(0);
    }
	sprintf(pszKey, "%s", pszParam[0]);
    sprintf(pszValue, "%s", pszParam[1]);
    
    //printf("[CConfig] ReadCfgValue END \n ");
    return(1);
}


int CConfig::ReadConfig(void)
{
    int     nParamCnt;
    FILE    *pFile = NULL;
    char    szFileName[128], szBuf[256], szTemp[256], *pszParam[50], *ptr1;
	char	szSection[128], szKey[128], szMapKey[256];
	bool	bInvalidSec = false;

    if (!GetEMS_ID())
    {
        printf("[CConfig] ReadConfig GetEMS_ID() failed. %s file error \n ", ROLE_DEF_FILE);
        return(-1);
    }

    sprintf(szFileName, OAMS_CFG_FILE, m_nEMSID);
	printf("Read <%s> \n", szFileName);

	// 초기화
	m_CfgMap.clear();
	memset(m_MgmInfo, 0x00, sizeof(MGM_INFO)*MAX_AS);
	//

    pFile = fopen(szFileName, "r");
    if (pFile)
    {
        while(!feof(pFile))
        {
            if (fgets(szBuf, sizeof(szBuf), pFile) != NULL)
            {
				// Skip comment
				if (szBuf[0] == '#' || !strncmp(szBuf, "//", 2))
					continue;

				// Delete comment
				ptr1 = strstr(szBuf, "#");
				if (ptr1 != NULL)
					*ptr1 = '\0';

                Trim(szBuf, strlen(szBuf));
				if (strlen(szBuf) == 0)
					continue;

				// Parsing Section
				if (szBuf[0] == '[')
				{
					ptr1 = strstr(szBuf, "]");
					if (ptr1 != NULL)
					{
						*ptr1 = '\0';
						sprintf(szSection, "%s", &szBuf[1]);
						bInvalidSec = false;
					}
					else
					{
						bInvalidSec = true;
                        printf("[CConfig] ReadConfig Invalid Section %s \n ", szBuf);
					}
				}
				//
				else if (!strncmp(szBuf, "MGM_INFO", strlen("MGM_INFO")))
				{
					char	szforLog[512];
					sprintf(szforLog, "%s", szBuf);

					if (!ReadCfgValue(szBuf, szKey, szTemp))   return(-3);

					nParamCnt = ParsingDelimeter(szTemp, ':', pszParam, strlen(szTemp));
					if (nParamCnt != 3)
					{
                        printf("[CConfig] ReadConfig Invalid config format %s \n ", szforLog);
						return(-4);
					}

					int	nId;

					nId = atoi(pszParam[0]);
					m_MgmInfo[nId].bUseflag = true;
					sprintf(m_MgmInfo[nId].OMSA_IP, "%s", pszParam[1]);
					sprintf(m_MgmInfo[nId].OMSB_IP, "%s", pszParam[2]);
				}
				else if (!strncmp(szBuf, "NET_DEVICE_NAME", strlen("NET_DEVICE_NAME")))
				{
					char    szforLog[512];
					sprintf(szforLog, "%s", szBuf);

					if (!ReadCfgValue(szBuf, szKey, szTemp))   return(-3);
				
					nParamCnt = ParsingDelimeter(szTemp, ':', pszParam, strlen(szTemp));
					if (nParamCnt != 2)
					{
						printf("[CConfig] ReadConfig Invalid config format %s \n ", szforLog);
						return(-4);					
					}

					int nId;

					nId = atoi(pszParam[0]);
					sprintf(m_szNetDeviceName[nId], "%s", pszParam[1]);
				}
				// Parsing Normal-Key
				else
				{
					if (bInvalidSec)
					{
                        printf("[CConfig] ReadConfig Invalid Section <%s> ignored \n ", szBuf);
						continue;
					}
					if (!ReadCfgValue(szBuf, szKey, szTemp))   return(-3);

					sprintf(szMapKey, "%s.%s", szSection, szKey);
					// Insert config-map
					m_CfgMap.insert(map<string, string>::value_type(szMapKey, szTemp));
				}
			}
		}
		fclose(pFile);
	}
    else
    {
        printf("[CConfig] ReadConfig failed, %s file error \n ", szFileName);
        return(-2);
    }

	return(1);
}


int CConfig::PrintConfig(void)
{
	int		i;
	map<string, string>::iterator 	iter;
	string		strKey, strValue;

    Log.printf(LOG_INF, "oams.cfg ###################################################");
    std::lock_guard<std::mutex> lock(m_pMutex);
	for ( iter = m_CfgMap.begin() ; iter != m_CfgMap.end() ; iter++ )
	{
		strKey = (*iter).first;
		strValue = (*iter).second;

        Log.printf(LOG_INF, "%s = %s ", strKey.c_str(), strValue.c_str());

	}

	// MGM_INFO
    for ( i=0 ; i<MAX_AS ; i++ )
    {
		if (m_MgmInfo[i].bUseflag)
		{
            Log.printf(LOG_INF, "MGM_INFO = %d:%s:%s", i, m_MgmInfo[i].OMSA_IP, m_MgmInfo[i].OMSB_IP); 
        }
    }

	// NET_DEVICE_NAME
	for ( i=0 ; i<MAX_LAN_PORT ; i++ )
	{
		if (strlen(m_szNetDeviceName[i]) > 0)
			Log.printf(LOG_INF, "NET_DEVICE_NAME = %d:%s", i, m_szNetDeviceName[i]);
	}

	return(1);
}


int CConfig::GetConfigString(const char *pszkey, char *pszValue)
{
	map<string, string>::iterator   iter;
	string strValue;

    std::lock_guard<std::mutex> lock(m_pMutex);
	iter = m_CfgMap.find(pszkey);
	if (iter == m_CfgMap.end())	// Not found
	{
		return(-1);
	}
	strValue = (*iter).second;
	sprintf(pszValue, "%s", strValue.c_str());

	return(1);
}
int CConfig::GetConfigValuesBySection(const char* pszSection, std::vector<std::pair<std::string, std::string>>& vecKeyValues)
{
    if (pszSection == nullptr || strlen(pszSection) == 0)
        return -1;

    std::lock_guard<std::mutex> lock(m_pMutex);
    vecKeyValues.clear();
    string sectionPrefix = string(pszSection) + ".";

    for (const auto& item : m_CfgMap)
    {
        const string& fullKey = item.first;

        Log.printf(LOG_INF, "[GetConfigValuesBySection] fullKey=[%s] value=[%s]", fullKey.c_str(), item.second.c_str());

        if (fullKey.compare(0, sectionPrefix.size(), sectionPrefix) == 0)
        {
            // Key: 섹션 이후의 키 이름만 추출
            string keyOnly = fullKey.substr(sectionPrefix.size());
            Log.printf(LOG_INF, "[GetConfigValuesBySection] --> keyOnly=[%s]", keyOnly.c_str());

            if (keyOnly.empty() || item.second.empty()) {
                Log.printf(LOG_ERR, "[GetConfigValuesBySection] Invalid config entry fullKey=[%s] value=[%s]", fullKey.c_str(), item.second.c_str());
                continue;
            }
#ifdef ALTIBASE_MODE
			if(strcmp(keyOnly.c_str(), "LISTNER")==0) continue;
#endif
#ifndef OFCS_IN_EMS
			if(strcmp(keyOnly.c_str(), "CDRM")==0) continue;
#endif
#ifndef _USE_NMI
			if(strcmp(keyOnly.c_str(), "NMSIF")==0) continue;
#endif

            vecKeyValues.emplace_back(keyOnly, std::string(item.second));
        }
    }

    return static_cast<int>(vecKeyValues.size());
}
