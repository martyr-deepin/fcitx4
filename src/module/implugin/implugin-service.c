/*
 * Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
 *
 * Author:     chenshijie <chenshijie@uniontech.com>
 *
 * Maintainer: chenshijie <chenshijie@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "fcitx-utils/utils.h"
#include "minIni.h"

#include <dirent.h>
#include <fcntl.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include <dbus/dbus-glib.h>
#include <dbus/dbus.h>

#define EVENT_NUM 12
#define IS_FILE 8
#define IS_DIR 4
#define DATA_W 200

static struct dir_path {
    int id;
    char **path;
} dir;

char *event_str[EVENT_NUM] = {"IN_ACCESS",
                              "IN_MODIFY", //文件修改
                              "IN_ATTRIB",
                              "IN_CLOSE_WRITE",
                              "IN_CLOSE_NOWRITE",
                              "IN_OPEN",
                              "IN_MOVED_FROM", //文件移动from
                              "IN_MOVED_TO",   //文件移动to
                              "IN_CREATE",     //文件创建
                              "IN_DELETE",     //文件删除
                              "IN_DELETE_SELF",
                              "IN_MOVE_SELF"};

char *gDimConfigPath = NULL;
char *gImPluginConfigPath = NULL;
FILE *gFp = NULL;

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

/**
 * @brief id_add
 * @param 添加目录名
 * 添加目录
 */
void id_add(char *path_id) {
    dir.path[dir.id] = (char *)malloc(DATA_W);
    memset(dir.path[dir.id], 0, DATA_W);
    strncpy(dir.path[dir.id], path_id, DATA_W);
    dir.id = dir.id + 1;
}

/**
 * @brief inotify_watch_dir
 * @param 监控目录树，inotify文件描述符
 * 添加监控目录树
 */
int inotify_watch_dir(char *dirPath, int fd) {
    int wd = 0;
    DIR *dp = NULL;
    char pdirHome[DATA_W];
    char pdir[DATA_W];
    strcpy(pdirHome, dirPath);
    struct dirent *dirp;
    if (fd < 0) {
        fprintf(gFp, "%s: inotify_init failed\n", gettime());
        return -1;
    }
    wd = inotify_add_watch(fd, dirPath, IN_CREATE | IN_ATTRIB | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO);
    if ((dp = opendir(dirPath)) == NULL) {
        return -1;
    }
    while ((dirp = readdir(dp)) != NULL) {
        if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0 || dirp->d_type == IS_FILE) {
            continue;
        }
        if (dirp->d_type == IS_DIR) {
            strncpy(pdir, pdirHome, DATA_W);
            strncat(pdir, "/", 1);
            strncat(pdir, dirp->d_name, BUFSIZ);
            id_add(pdir);
            inotify_watch_dir(pdir, fd);
        }
    }
    closedir(dp);
    return 0;
}

/**
 * @brief str_find_target
 * @param 字符后缀，匹配字符，获取字符
 * 获取目标字符串
 */
int str_find_target(const char *strBack, const char *strFilter, char **strTarget) {
    if (strFilter == NULL) {
        return 0;
    }

    char localName[BUFSIZ];
    memset(localName, 0, BUFSIZ);

    //[2] 第一次与 "strback" 匹配的字段位置
    const char *ePointPosition = strstr(strFilter, strBack);
    if (ePointPosition == NULL) {
        return 0;
    }
    size_t endPosition = ePointPosition - strFilter;

    //确保是 后缀是 "strback"
    if (endPosition > 0) {
        for (int i = 0; i < (int)endPosition; i++) {
            localName[i] = *(strFilter + i);
        }

        *strTarget = malloc(strlen(localName) + 1);
        memset(*strTarget, 0, (strlen(localName) + 1));
        strncpy(*strTarget, localName, strlen(localName));
        return 1;
    }
    return 0;
}

/**
 * @brief str_find_target
 * @param 字符后缀，匹配字符，获取字符
 * 获取字符目标
 */
void delete_comment(char str[]) {
    int i, j;
    for (i = j = 0; str[i] != '\0'; i++) {
        if (str[i] != '\#' || str[i + 1] == ' ') {
            str[j++] = str[i];
        }
    }
    str[j] = '\0';
}

/**
 * @brief file_comment_modify
 * @param 目录名
 * 文件注释检查并修改
 */
void file_comment_modify(const char *filename) {
    int fd = 0;
    int len = 0;
    char str[BUFSIZ];

    fd = open(filename, O_RDWR, S_IRUSR | S_IWUSR);
    if (fd) {
        len = read(fd, str, BUFSIZ);
        str[len] = '\0';
        delete_comment(str);
        ftruncate(fd, 0);
        lseek(fd, 0, SEEK_SET);
        write(fd, str, strlen(str));
        close(fd);
    }
}

/**
 * @brief get_curindex_inputmethod
 * @param 输入法位置索引 输入法名称
 * 获取输入法名称
 */
int get_curindex_inputmethod(int32_t sigvalue, char **imname) {
    DBusError err;
    DBusConnection *connection;
    DBusMessage *msg;
    DBusMessageIter arg;
    DBusPendingCall *pending;

    dbus_error_init(&err);

    connection = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err)) {
        fprintf(gFp, "%s: ConnectionErr: %s!\n", err.message, gettime());
        dbus_error_free(&err);
    }
    if (connection == NULL) {
        return -1;
    }

    if ((msg = dbus_message_new_method_call("org.fcitx.Fcitx-0", "/inputmethod",
                                            "org.fcitx.Fcitx.InputMethod",
                                            "GetIMByIndex")) == NULL) {
        fprintf(gFp, "%s: Method is NULL!\n", gettime());
        return -1;
    }

    dbus_message_iter_init_append(msg, &arg);
    if (!dbus_message_iter_append_basic(&arg, DBUS_TYPE_INT32, &sigvalue)) {
        fprintf(gFp,  "%s: Send is error!\n", gettime());
        return -1;
    }
    if (!dbus_connection_send_with_reply(connection, msg, &pending, -1)) {
        fprintf(gFp, "%s: Reply is error!\n", gettime());
        return -1;
    }
    if (NULL == pending) {
        fprintf(gFp, "%s: RPending Call Null!\n", gettime());
        return -1;
    }

    dbus_connection_flush(connection);
    dbus_message_unref(msg);

    dbus_pending_call_block(pending);
    msg = dbus_pending_call_steal_reply(pending);
    if (msg == NULL) {
        fprintf(gFp, "%s: Reply Null!\n", gettime());
        return -1;
    }
    if (!dbus_message_iter_init(msg, &arg)){
        fprintf(gFp, "%s: Message has no arguments!\n", gettime());
        return -1;
    }
    else if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&arg)){
        fprintf(gFp, "%s: Argument is not string!\n", gettime());
        return -1;
    }
    else{
        dbus_message_iter_get_basic(&arg, imname);
    }
    dbus_connection_flush(connection);

    dbus_message_unref(msg);

    return 0;
}

/**
 * @brief get_config_path
 * 获取输入法配置文件目录
 */
void get_config_path() {
    FILE *fp = NULL;
    if (!gDimConfigPath) {
        //获取默认输入法配置文件路径
        fp = FcitxXDGGetFileUserWithPrefix("conf", "fcitx-defaultim.config",
                                           "r", &gDimConfigPath);
        if (fp) {
            fclose(fp);
            file_comment_modify(gDimConfigPath);
        }
    }

    if (!gImPluginConfigPath) {
        //获取输入法插件配置文件路径
        fp = FcitxXDGGetFileUserWithPrefix("conf", "fcitx-implugin.config", "r",
                                           &gImPluginConfigPath);
        if (fp) {
            fclose(fp);
            file_comment_modify(gImPluginConfigPath);
        }
    }
}

/**
 * @brief display_inotify_event
 * @param 监控描述符
 * 根据监控描述符进行事件处理
 */
void display_inotify_event(struct inotify_event *i) {
    get_config_path();
    if (i->mask & IN_MOVED_TO) {
        char *imName = NULL;
        if ((strcmp(dir.path[i->wd], "/usr/share/fcitx/inputmethod") == 0 ||
             strcmp(dir.path[i->wd], "/usr/share/fcitx/table") == 0) &&
            str_find_target(".conf", i->name, &imName) == 1) {

            if (gDimConfigPath) {
                ini_puts("DefaultIM", "IMNAME", imName, gDimConfigPath);
            }

            sleep(3);
            fcitx_utils_launch_restart();

            char settingWizard[BUFSIZ];
            memset(settingWizard, 0, FCITX_ARRAY_SIZE(settingWizard));
            if (gImPluginConfigPath) {
                ini_gets(imName, "SettingWizard", "none", settingWizard,
                         FCITX_ARRAY_SIZE(settingWizard), gImPluginConfigPath);
            }
            char *pSettingWizard = malloc((strlen(settingWizard) + 1));
            memset(pSettingWizard, 0, (strlen(settingWizard) + 1));
            strncpy(pSettingWizard, settingWizard, (strlen(settingWizard) + 1));

            char parameter[BUFSIZ];
            memset(parameter, 0, FCITX_ARRAY_SIZE(parameter));
            if (gImPluginConfigPath) {
                ini_gets(imName, "Parameter", NULL, parameter,
                         FCITX_ARRAY_SIZE(parameter), gImPluginConfigPath);
            }
            char *pParameter = malloc((strlen(parameter) + 1));
            memset(pParameter, 0, (strlen(parameter) + 1));
            strncpy(pParameter, parameter, (strlen(parameter) + 1));

            if (strcmp(pSettingWizard, "none") != 0 &&
                strcmp(imName, "baidupinyin") != 0) {
                if (pParameter) {
                    char *commod[] = {pSettingWizard,
                                      (char *)(intptr_t)pParameter, NULL};
                    fcitx_utils_start_process(commod);
                } else {
                    char *commod[] = {pSettingWizard, NULL};
                    fcitx_utils_start_process(commod);
                }
            } else if (strcmp(imName, "none") != 0 &&
                       strcmp(dir.path[i->wd],"/usr/share/fcitx/table") != 0) {
                fprintf(gFp, "%s: commod is %s; \n", gettime(), "start");
                sleep(20);
                char *result = malloc(DATA_W);
                memset(result, 0, DATA_W);
                strcat(result, "fcitx-config-gtk3 ");
                strcat(result, "fcitx-");
                strcat(result, imName);
                char *commod[] = {result, NULL};
                fprintf(gFp, "%s: commod is end %s; \n", gettime(), result);
                free(result);
                result = NULL;
                fcitx_utils_start_process(commod);
            } else if (strcmp(imName, "none") != 0 &&
                       strcmp(dir.path[i->wd],"/usr/share/fcitx/table") == 0) {
                sleep(5);
                char *commod[] = {"fcitx-config-gtk3", "fcitx-table"};
                fcitx_utils_start_process(commod);
            }
            free(pSettingWizard);
            pSettingWizard = NULL;
            free(pParameter);
            pParameter = NULL;
            fprintf(gFp, "%s: add imname = %s; \n", gettime(), imName);
            free(imName);
            imName = NULL;
        }
    }
    if (i->mask & IN_DELETE) {
        char *imName = NULL;
        if ((strcmp(dir.path[i->wd], "/usr/share/fcitx/inputmethod") == 0 ||
             strcmp(dir.path[i->wd], "/usr/share/fcitx/table") == 0) &&
            str_find_target(".conf", i->name, &imName) == 1) {

            sleep(3);
            fcitx_utils_launch_restart();

            if (gDimConfigPath) {
                char curDeimName[BUFSIZ];
                memset(curDeimName, 0, FCITX_ARRAY_SIZE(curDeimName));
                ini_gets("DefaultIM", "IMNAME", "fcitx-keyboard-us",
                         curDeimName, FCITX_ARRAY_SIZE(curDeimName),
                         gDimConfigPath);

                char *pCurDeimName = malloc((strlen(curDeimName) + 1));
                memset(pCurDeimName, 0, (strlen(curDeimName) + 1));
                strncpy(pCurDeimName, curDeimName, (strlen(curDeimName) + 1));

                if (strcmp(pCurDeimName, imName) == 0) {
                    char *secName;
                    get_curindex_inputmethod(2, &secName);
                    if (strcmp(secName, "") == 0) {
                        secName = "fcitx-keyboard-us";
                    }
                    ini_puts("DefaultIM", "IMNAME", secName, gDimConfigPath);
                    free(secName);
                    secName = NULL;
                }
                free(pCurDeimName);
                pCurDeimName = NULL;
            }
            fprintf(gFp, "%s: remove imname = %s; \n", gettime(), imName);
            free(imName);
            imName = NULL;
        }
    }
}

/**
 * @brief display_inotify_event
 * @param 监控描述符
 * 根据监控描述符进行事件处理
 */
int main(int argc, char *argv[]) {
    dir.id = 1;
    dir.path = (char **)malloc(65534);
    char *watchpath = fcitx_utils_get_fcitx_path("pkgdatadir");
    id_add(watchpath);
    int fd = inotify_init();

    inotify_watch_dir(watchpath, fd);

    int i = 0;
    int len = 0;
    int nread = 0;
    struct stat res;
    char path[BUFSIZ];
    char buf[BUFSIZ];
    struct inotify_event *event;
    char logDir[DATA_W]={};
    strncat(logDir,"/tmp/fcitx-inotify.log",23);
    gFp=fopen(logDir,"a");
    buf[sizeof(buf) - 1] = 0;
    while ((len = read(fd, buf, sizeof(buf) - 1)) > 0) {
        nread = 0;
        while (len > 0) {
            event = (struct inotify_event *)&buf[nread];
            for (i = 0; i < EVENT_NUM; i++) {
                if ((event->mask >> i) & 1) {
                        if (event->len > 0 && strncmp(event->name, ".", 1)) {
                            fprintf(gFp, "%s: %s/%s --- %s\n",
                                    gettime(),dir.path[event->wd], event->name,
                                    event_str[i]);
                            display_inotify_event(event);

                            if ((strcmp(event_str[i], "IN_CREATE") == 0) ||
                                (strcmp(event_str[i], "IN_MOVED_TO") == 0)) {
                                memset(path, 0, sizeof path);
                                strncat(path, dir.path[event->wd], BUFSIZ);
                                strncat(path, "/", 1);
                                strncat(path, event->name, BUFSIZ);
                                stat(path, &res);
                                if (S_ISDIR(res.st_mode)) {
                                    id_add(path);
                                    inotify_add_watch(
                                        fd, path,
                                        IN_CREATE | IN_ATTRIB | IN_DELETE |
                                            IN_MOVED_FROM | IN_MOVED_TO);
                                }
                            }
                            if ((strcmp(event_str[i], "IN_DELETE") == 0) ||
                                (strcmp(event_str[i], "IN_MOVED_FROM") == 0)) {
                                memset(path, 0, sizeof path);
                                strncat(path, dir.path[event->wd], BUFSIZ);
                                strncat(path, "/", 1);
                                strncat(path, event->name, BUFSIZ);
                                stat(path, &res);
                                if (S_ISDIR(res.st_mode)) {
                                    free(dir.path[event->wd]);
                                    dir.path[event->wd] = NULL;
                                }
                            }
                        }
                        fflush(gFp);
                }
            }
            nread = nread + sizeof(struct inotify_event) + event->len;
            len = len - sizeof(struct inotify_event) - event->len;
        }
    }

    for (i=0;i<dir.id;i++) {
        free(dir.path[i]);
        dir.path[i] = NULL;

    }
    fclose(gFp);
    gFp = NULL;
    free(gDimConfigPath);
    gDimConfigPath = NULL;
    free(gImPluginConfigPath);
    gImPluginConfigPath = NULL;
    return 0;
}
