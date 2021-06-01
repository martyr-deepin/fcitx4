/***************************************************************************
 *   Copyright (C) 2010~2010 by CSSlayer                                   *
 *   wengxt@gmail.com                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.              *
 ***************************************************************************/

#include <iconv.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <wchar.h>
#include <libgen.h>
#include <time.h>

#include "config.h"
#include "fcitx/fcitx.h"
#include "utf8.h"
#include "log.h"
#include "utils.h"

static iconv_t iconvW = NULL;
static int init = 0;
static int is_utf8 = 0;

#ifndef _DEBUG
static FcitxLogLevel errorLevel = FCITX_WARNING;
#else
static FcitxLogLevel errorLevel = FCITX_DEBUG;
#endif

static const int RealLevelIndex[] = {0, 2, 3, 4, 1, 6};

#define LOGFILE "/tmp/fcitx-log.log"
static FILE* gFp = NULL;

/**
 * @brief gettime
 * @return 当前时间字符串
 * 获取当前时间
 */
char *gettime()
{
    static char timestr[40];
    time_t t;
    struct tm *nowtime;
    time(&t);
    nowtime = localtime(&t);
    strftime(timestr,sizeof(timestr),"%Y-%m-%d %H:%M:%S",nowtime);
    return timestr;
}

FCITX_EXPORT_API
void FcitxLogSetLevel(FcitxLogLevel e) {
    if ((int) e < 0)
        e = 0;
    else if (e > FCITX_NONE)
        e = FCITX_NONE;
    errorLevel = e;
}

FcitxLogLevel FcitxLogGetLevel()
{
    return errorLevel;
}


FCITX_EXPORT_API void
FcitxLogFuncV(FcitxLogLevel e, const char* filename, const int line,
              const char* fmt, va_list ap)
{
    if (!init) {
        init = 1;
        is_utf8 = fcitx_utils_current_locale_is_utf8();
    }

    if ((int) e < 0) {
        e = 0;
    } else if (e >= FCITX_NONE) {
        e = FCITX_INFO;
    }

    int realLevel = RealLevelIndex[e];
    int globalRealLevel = RealLevelIndex[errorLevel];

    if (realLevel < globalRealLevel)
        return;

    switch (e) {
    case FCITX_INFO:
        fprintf(stderr, "%s (INFO-", gettime());
        fprintf(gFp, "%s (INFO-", gettime());
        break;
    case FCITX_ERROR:
        fprintf(stderr, "%s (ERROR-", gettime());
        fprintf(gFp, "%s (ERROR-", gettime());
        break;
    case FCITX_DEBUG:
        fprintf(stderr, "%s (DEBUG-", gettime());
        fprintf(gFp, "%s (DEBUG-", gettime());
        break;
    case FCITX_WARNING:
        fprintf(stderr, "%s (WARN-", gettime());
        fprintf(gFp, "%s (WARN-", gettime());
        break;
    case FCITX_FATAL:
        fprintf(stderr, "%s (FATAL-", gettime());
        fprintf(gFp, "%s (FATAL-", gettime());
        break;
    default:
        break;
    }

    char *buffer = NULL;
    fprintf(stderr, "%d %s:%u) ", getpid(), filename, line);
    fprintf(gFp, "%d %s:%u)", getpid(), filename, line);
    vasprintf(&buffer, fmt, ap);

    if (is_utf8) {
        fprintf(stderr, "%s\n", buffer);
        fprintf(gFp, "%s\n", buffer);
        free(buffer);
        return;
    }

    if (iconvW == NULL)
        iconvW = iconv_open("WCHAR_T", "utf-8");

    if (iconvW == (iconv_t) - 1) {
        fprintf(stderr, "%s\n", buffer);
        fprintf(gFp, "%s\n", buffer);
    } else {
        size_t len = strlen(buffer);
        wchar_t *wmessage = NULL;
        size_t wlen = (len) * sizeof(wchar_t);
        wmessage = (wchar_t *)fcitx_utils_malloc0((len + 10) * sizeof(wchar_t));

        IconvStr inp = buffer;
        char *outp = (char*) wmessage;

        iconv(iconvW, &inp, &len, &outp, &wlen);

        fprintf(stderr, "%ls\n", wmessage);
        fprintf(gFp, "%ls\n", wmessage);
        free(wmessage);
    }
    free(buffer);
}

FCITX_EXPORT_API void
FcitxLogFunc(FcitxLogLevel e, const char* filename, const int line,
             const char* fmt, ...)
{
    char *username = getlogin();
    char default_name[32] = {0};

    int log_path_len = 0;
    if (username) {
        log_path_len = strlen(username)+strlen(LOGFILE)+2;
    } else {
        //if getlogin() failed, call getuid(). getuid() is always successful
        uid_t uid = getuid();
        sprintf(default_name, "user_id_%u", uid);
        username = default_name;

        log_path_len = strlen(username)+strlen(LOGFILE)+2;
    }

    char *log_path = fcitx_utils_malloc0(log_path_len);
    if (NULL == log_path) {
        return;
    }

    memset(log_path, 0, log_path_len);
    sprintf(log_path, "%s_%s", LOGFILE, username);
    gFp=fopen(log_path,"a");
    if (NULL == gFp) {
        free(log_path);
        log_path = NULL;
        return;
    }

    va_list ap;
    char *file = strdup(filename);
    if (!file) {
        fclose(gFp);
        gFp = NULL;
        free(log_path);
        log_path = NULL;
        return;
    }
    va_start(ap, fmt);
    FcitxLogFuncV(e, basename(file), line, fmt, ap);
    free(file);
    va_end(ap);
    fclose(gFp);
    gFp = NULL;
    free(log_path);
    log_path = NULL;
}

// kate: indent-mode cstyle; space-indent on; indent-width 0;
