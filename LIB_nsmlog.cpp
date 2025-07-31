/* File Header
 *fsh**************************************************************
 ******************************************************************
 **
 **  FILE : LIB_nsmlog.cpp
 **
 ******************************************************************
 ******************************************************************
 BLOCK          : LIB
 SUBSYSTEM      : LIB
 SOR-NAME       :
 VERSION        : V1.X
 DATE           : 2014/07/
 AUTHOR         : SEUNG-MO, CHO
 HISTORY        :
 PROCESS(TASK)  :
 PROCEDURES     :
 DESCRIPTION    : Log file create/write/backup manage class 
 *end*************************************************************/


#include "LIB_nsmlog.h"

const char LOG_LEVEL_STR[6][6] = { "", "LV1  ", "LV2  ", "LV3  ", "ERROR", "INFO " };

/* Procedure Header
 **pdh********************************************************
 * PROCEDURE-NAME : NSM_LOG
 * CLASS-NAME     : NSM_LOG
 * PARAMETER      : -
 * RET. VALUE     : -
 * DESCRIPTION    : NSM_LOG constructor
 * REMARKS        :
 **end*******************************************************/
NSM_LOG::NSM_LOG(void)
{
    m_fp        = NULL;
    m_level     = LOG_LV1;                  // default log level
    m_max_limit = 1024*1024*300;            // default = 300M Byte
    m_open_day  = -1;
    snprintf(m_log_file_format, sizeof(m_log_file_format), "pid_%d_%%04d%%02d%%02d.txt", getpid());
}

/* Procedure Header
 **pdh********************************************************
 * PROCEDURE-NAME : ~NSM_LOG
 * CLASS-NAME     : NSM_LOG
 * PARAMETER      : -
 * RET. VALUE     : -
 * DESCRIPTION    : NSM_LOG destructor 
 * REMARKS        :
 **end*******************************************************/
NSM_LOG::~NSM_LOG(void)
{
    if(m_fp != NULL) { fclose(m_fp); m_fp = 0; }
}


/* Procedure Header
 **pdh********************************************************
 * PROCEDURE-NAME : init
 * CLASS-NAME     : NSM_LOG
 * PARAMETER    IN: path - Log file PATH 
 * RET. VALUE     : BOOL
 * DESCRIPTION    : NSM_LOG init (LOG FILE NAME CREATE/OPEN)
 * REMARKS        :
 **end*******************************************************/
bool NSM_LOG::init(const char *path, const char *name)
{
    mkdir(path, 0777); 
    if(path[strlen(name)-1] == '_')
    {
        snprintf(m_log_file_format,    sizeof(m_log_file_format),    "%s/%s%%04d%%02d%%02d.txt",    path, name);
        snprintf(m_backup_file_format, sizeof(m_backup_file_format), "%s/%s%%04d%%02d%%02d.b%%03d", path, name);
    }
    else
    {
        snprintf(m_log_file_format,    sizeof(m_log_file_format),    "%s/%s_%%04d%%02d%%02d.txt",    path, name);
        snprintf(m_backup_file_format, sizeof(m_backup_file_format), "%s/%s_%%04d%%02d%%02d.b%%03d", path, name);
    }
    if((m_fp = fileopen()) == NULL) { return(false); }
    
    return(true);
}

/* Procedure Header
 **pdh********************************************************
 * PROCEDURE-NAME : get_filename
 * CLASS-NAME     : NSM_LOG
 * PARAMETER      : -
 * RET. VALUE     : filename
 * DESCRIPTION    : Log filename get 
 * REMARKS        :
 **end*******************************************************/
const char *NSM_LOG::get_filename(void)
{
    return(m_filename);
}

/* Procedure Header
 **pdh********************************************************
 * PROCEDURE-NAME : fileopen
 * CLASS-NAME     : NSM_LOG
 * PARAMETER      : -
 * RET. VALUE     : 0(ERR)/FILE *
 * DESCRIPTION    : LOG file open
 * REMARKS        :
 **end*******************************************************/
FILE *NSM_LOG::fileopen(void)
{
    struct  tm      p_now;
    struct  timeval t_now;
    
    if(gettimeofday(&t_now, NULL) != 0)            { return(0); }
    if(localtime_r(&t_now.tv_sec, &p_now) == NULL) { return(0); }
    
	snprintf(m_filename, sizeof(m_filename), m_log_file_format, p_now.tm_year+1900, p_now.tm_mon+1, p_now.tm_mday);
    m_open_day = p_now.tm_mday;    // open date
    
    return(fopen(m_filename, "a+"));
}

/* Procedure Header
 **pdh********************************************************
 * PROCEDURE-NAME : close
 * CLASS-NAME     : NSM_LOG
 * PARAMETER      : -
 * RET. VALUE     : -
 * DESCRIPTION    : Log Handle close
 * REMARKS        :
 **end*******************************************************/
void NSM_LOG::close(void)
{
    if(m_fp != 0) { fclose(m_fp); m_fp = 0; }
}


/* Procedure Header
 **pdh********************************************************
 * PROCEDURE-NAME : check_file_size_limit
 * CLASS-NAME     : NSM_LOG
 * PARAMETER      : -
 * RET. VALUE     : -
 * DESCRIPTION    : Function to backup if the LOG file size is larger than m_max_limit 
 * REMARKS        : Move existing file and re-open with existing log name
 **end*******************************************************/
void NSM_LOG::check_file_size_limit(struct tm *p_now)
{
    struct stat statbuf;
    int ret, nIndex;
    char backup_filename[128];

    if ((ret = stat(m_filename, &statbuf)) == 0 && statbuf.st_size > m_max_limit)
    {
        fclose(m_fp);
        m_fp = NULL;

        for (nIndex = 0; nIndex < 1000; nIndex++)
        {
            snprintf(backup_filename, sizeof(backup_filename), m_backup_file_format, p_now->tm_year+1900, p_now->tm_mon+1, p_now->tm_mday, nIndex);
            if (stat(backup_filename, &statbuf) == -1 && errno == ENOENT)
            {
                rename(m_filename, backup_filename);
                m_fp = fileopen();
                return;
            }
        }

        snprintf(backup_filename, sizeof(backup_filename), m_backup_file_format, p_now->tm_year+1900, p_now->tm_mon+1, p_now->tm_mday, 999);
        rename(m_filename, backup_filename);
        m_fp = fileopen();
    }
}
/* Procedure Header
 **pdh********************************************************
 * PROCEDURE-NAME : printf
 * CLASS-NAME     : -
 * PARAMETER    IN: nLogLevel - LOG LEVEL
 *              IN: fmt       - LOG format
 * RET. VALUE     : -
 * DESCRIPTION    : Function to save the contents corresponding to fmt to the load file 
 * REMARKS        : All in black and white - fflush() without close 
 **end*******************************************************/
void NSM_LOG::printf(char nLogLevel, const char *fmt, ...)
{
    struct tm p_now;
    struct timeval t_now;
    va_list arg;
    char buf[4096];      // 로그 메시지 전체
    char msg[4096];      // 포맷된 메시지만
    size_t len;

    if (m_fp == NULL || nLogLevel < m_level) return;

    std::lock_guard<std::mutex> lock(lock_mutex);
    {
        gettimeofday(&t_now, NULL);
        localtime_r(&t_now.tv_sec, &p_now);

        if (m_open_day != p_now.tm_mday)
        {
            fclose(m_fp);
            if ((m_fp = fileopen()) == NULL) {
                return;
            }
        }

        va_start(arg, fmt);
        vsnprintf(msg, sizeof(msg), fmt, arg);
        va_end(arg);

        // \n 자동 추가
        len = strlen(msg);
        if (len == 0 || msg[len - 1] != '\n') {
            strncat(msg, "\n", sizeof(msg) - len - 1);
        }

        // 최종 로그 메시지 생성
        snprintf(buf, sizeof(buf),
                 "[%02d:%02d:%02d.%06ld][%s] %s",
                 p_now.tm_hour, p_now.tm_min, p_now.tm_sec,
                 t_now.tv_usec, LOG_LEVEL_STR[(int)nLogLevel], msg);

        fputs(buf, m_fp);
        fflush(m_fp);
        check_file_size_limit(&p_now);
    }
}
/* Procedure Header
 **pdh********************************************************
 * PROCEDURE-NAME : cprintf
 * CLASS-NAME     : NSM_LOG
 * PARAMETER    IN: nLogLevel - LOG LEVEL
 *              IN: fmt       - LOG format
 * RET. VALUE     : -
 * DESCRIPTION    : Function to save the contents corresponding to fmt to the load file 
 * REMARKS        : All in black and white - fflush() without close 
 **end*******************************************************/
void NSM_LOG::cprintf(char nLogLevel, const char *fmt, ...)
{
    struct  tm      p_now;
    struct  timeval t_now;
    va_list arg;
    
    if(m_fp == NULL)        { return; }   // file is not open 
	if(nLogLevel < m_level) { return; }
    
    std::lock_guard<std::mutex> lock(lock_mutex);
    {
        gettimeofday(&t_now, NULL);
        localtime_r(&t_now.tv_sec, &p_now);
        
        if(m_open_day != p_now.tm_mday)   // Open date and current date are different - Change date 
        {
            fclose(m_fp);
            if((m_fp = fileopen()) == NULL) { return; }
        }
        
        va_start(arg, fmt);
        vfprintf(m_fp, fmt, arg);
        va_end(arg);
        
        
        fflush(m_fp);
        
        check_file_size_limit(&p_now);
    }
}
/* Procedure Header
 **pdh********************************************************
 * PROCEDURE-NAME : head
 * CLASS-NAME     : NSM_LOG
 * PARAMETER    IN: nLogLevel - LOG LEVEL
 *              IN: fmt       - LOG format
 *              IN: strBuf    - LOG message 
 *              IN: nLen      - LOG message len 
 * RET. VALUE     : -
 * DESCRIPTION    : A function that prints only nLen from the beginning of a message 
 * REMARKS        : Used when printing only part of a long message. 
 **end*******************************************************/
void NSM_LOG::head(int nLogLevel, const char *fmt, char *strBuf, int nLen)
{
    char        temp;
    
    temp = strBuf[nLen];   // backup
    strBuf[nLen] = '\0';   // set NULL for print log
    printf(nLogLevel, "%s %s\n", fmt, strBuf);
    strBuf[nLen] = temp;   // restore
}

/* Procedure Header
 **pdh********************************************************
 * PROCEDURE-NAME : warning
 * CLASS-NAME     : NSM_LOG
 * PARAMETER    IN: fmt - LOG format
 * RET. VALUE     : -
 * DESCRIPTION    : Function to save the contents corresponding to fmt to the load file 
 * REMARKS        : Operates with LOG_LV3 - add 20160823
 **end*******************************************************/
void NSM_LOG::warning(const char *fmt, ...)
{
    struct  tm      p_now;
    struct  timeval t_now;
    va_list arg;
    
    if(m_fp == NULL)       { return; }      // file is not open 
    if(m_level >= LOG_OFF) { return; }      // No warning output when LOG_OFF (actually considered the same level as Level 3) 
    
    std::lock_guard<std::mutex> lock(lock_mutex);
    {
        gettimeofday(&t_now, NULL);
        localtime_r(&t_now.tv_sec, &p_now);
        
        if(m_open_day != p_now.tm_mday)     // Open date and current date are different - Change date 
        {
            fclose(m_fp);
            if((m_fp = fileopen()) == NULL) { return; }
        }

        fprintf(m_fp, "[%02d:%02d:%02d.%06ld][%s] ", p_now.tm_hour, p_now.tm_min, p_now.tm_sec, t_now.tv_usec, "WARN ");
        
        va_start(arg, fmt);
        vfprintf(m_fp, fmt, arg);
        va_end(arg);

        fflush(m_fp);
        
        check_file_size_limit(&p_now);
        
    }
}
/* Procedure Header
 **pdh********************************************************
 * PROCEDURE-NAME : set_level
 * CLASS-NAME     : NSM_LOG
 * PARAMETER    IN: nLevel - LOG LEVEL
 * RET. VALUE     : -
 * DESCRIPTION    : set loglevel 
 * REMARKS        :
 **end*******************************************************/
void NSM_LOG::set_level(int8_t nLevel)
{
    switch(nLevel)
    {
        case LOG_LV1:
        case LOG_LV2:
        case LOG_LV3: m_level = nLevel;  return;
        default:      m_level = LOG_ERR; return;
    }
}
/* Procedure Header
 **pdh********************************************************
 * PROCEDURE-NAME : set_debug
 * CLASS-NAME     : NSM_LOG
 * PARAMETER      : -
 * RET. VALUE     : -
 * DESCRIPTION    : set LOG_LV1
 * REMARKS        : add 20160823
 **end*******************************************************/
void NSM_LOG::set_debug()
{
    m_level = LOG_LV1;
}
/* Procedure Header
 **pdh********************************************************
 * PROCEDURE-NAME : get_level
 * CLASS-NAME     : NSM_LOG
 * PARAMETER      : -
 * RET. VALUE     : - 
 * DESCRIPTION    : get loglevel 
 * REMARKS        :
 **end*******************************************************/
int8_t NSM_LOG::get_level(void)
{
    return(m_level);
}
/* Procedure Header
 **pdh********************************************************
 * PROCEDURE-NAME : set_max_limit
 * CLASS-NAME     : NSM_LOG
 * PARAMETER    IN: nLimit - logfile max size 
 * RET. VALUE     : BOOL
 * DESCRIPTION    : set logfile max size 
 * REMARKS        : 100M <= nLimit < 2G
 **end*******************************************************/
bool NSM_LOG::set_max_limit(uint32_t nLimit)
{
    if((100000000 <= nLimit) && (nLimit < (uint32_t)(2000000000)))
    {
        m_max_limit = nLimit;
        return(true);
    }
    return(false);
}


