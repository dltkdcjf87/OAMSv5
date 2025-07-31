//
//  LIB_nsmlog.h
//  LIB
//
//  Created by SMCHO on 2014. 4. 16..
//  Copyright (c) 2014  SMCHO. All rights reserved.
//

#ifndef __LIB_NSMLOG_H__
#define __LIB_NSMLOG_H__

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>
#include <ctype.h>
#include "OAMS_define.h"

// Log.printf
#define	LOG_INF         5
#define LOG_ERR         4
#define LOG_OFF         LOG_ERR
#define	LOG_LV3         3
#define	LOG_LV2         2
#define	LOG_LV1         1

// ANSI COLOR CODE
/*
#define COLOR_GRAY      "\033[0;47;30m"
#define COLOR_RED       "\033[0;40;31m"
#define COLOR_GREEN     "\033[0;40;32m"
#define COLOR_YELLOW    "\033[0;40;33m"
#define COLOR_BLUE      "\033[0;40;34m"
#define COLOR_MAGENTA   "\033[0;40;35m"
#define COLOR_CYAN      "\033[0;40;36m"
#define COLOR_WHITE     "\033[0;40;37m"

// R: Reverse
#define COLOR_R_GRAY    "\033[0;40;37m"
#define COLOR_R_RED     "\033[0;41;37m"
#define COLOR_R_GREEN   "\033[0;42;37m"
#define COLOR_R_YELLOW  "\033[0;43;37m"
#define COLOR_R_BLUE    "\033[0;44;37m"
#define COLOR_R_MAGENTA "\033[0;45;37m"
#define COLOR_R_CYAN    "\033[0;46;37m"
#define COLOR_R_WHITE   "\033[0;47;30m"

#define COLOR_RESET     "\033[0m"
*/


#ifndef MAX_LOG_NAME_LEN
#   define MAX_LOG_NAME_LEN     128
#endif

/* Class Header
 **pdh********************************************************
 * CLASS-NAME     : NSM_LOG
 * HISTORY        : 2014/04 - First written 
 * DESCRIPTION    : Log Class
 *                : 1. Once a file is opened, it continues to write  
 *                :    without closing it and performs fflush() each time. 
 *                : 2. Close and open a new file when it exceeds a certain size or the date changes. 
 * REMARKS        :
 **end*******************************************************/
class NSM_LOG
{
private:
    mutable std::mutex lock_mutex;
    
    int         m_open_day;         // Date of the currently open log file name - to change the file when the day changes 
    uint32_t    m_max_limit;        // LOG file size (BYTE) 
    int8_t      m_level;            // Log Level
    
    char        m_log_file_format[MAX_LOG_NAME_LEN+1];     // LOG FILE Format 
    char        m_backup_file_format[MAX_LOG_NAME_LEN+1];  // LOG BACKUPFILE Format 
    char        m_filename[MAX_LOG_NAME_LEN+1];            // open Log Filename 
    FILE        *m_fp;              // Log file pointer
    FILE        *fileopen(void);    // Log file Open 
    void        check_file_size_limit(struct tm *p_now);   // Function to check if the log file size is larger than the limit 
    
public:
    NSM_LOG(void);
	~NSM_LOG(void);
    
    bool    init(const char *path, const char *name);         // The first part of the PATH and name of the log file (the last part is fixed as YYYYMMDD.txt) 
    void    set_level(int8_t nLevel);       // Log Level Set 
    int8_t  get_level(void);                // Log Level Get 
    bool    set_max_limit(uint32_t nLimit); // Log File Size Set 
    const char *get_filename(void);         // Log Filename Get 
    void    close(void);                    // Open Log FP close 
    
    void    printf(char nLogLevel, const char *fmt, ...);           // Log printf 
    void    cprintf(char nLogLevel, const char *fmt, ...);          // A function that simply outputs [HH:MM:SS][LV] without formatting 
    void    head(int nLogLevel, const char *fmt, char *strBuf, int nLen);   // Function that prints only the beginning of a string/
    
    void    warning(const char *fmt, ...);                                      // add 160823 by SMCHO
    void    set_debug();                                                        // add 160823 by SMCHO - log level DEBUG level(LV1) Set
};

#endif /* defined(__LIB_NSMLOG_H__) */
