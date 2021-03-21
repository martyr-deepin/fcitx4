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

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <malloc.h>
#include <fcntl.h>

#include "fcitx-utils/utils.h"

#include <dbus/dbus-glib.h>
#include <dbus/dbus.h>

#include "minIni.h"
#define EVENT_NUM 12
#define IS_FILE 8
#define IS_DIR 4
#define DATA_W 200

static struct dir_path {
    int id;
    char **path;
} dir;

char *event_str[EVENT_NUM] = {
    "IN_ACCESS",
    "IN_MODIFY",        //文件修改
    "IN_ATTRIB",
    "IN_CLOSE_WRITE",
    "IN_CLOSE_NOWRITE",
    "IN_OPEN",
    "IN_MOVED_FROM",    //文件移动from
    "IN_MOVED_TO",      //文件移动to
    "IN_CREATE",        //文件创建
    "IN_DELETE",        //文件删除
    "IN_DELETE_SELF",
    "IN_MOVE_SELF"
};

//=======================================================================
int id_add(char *path_id)
{
    //free(dir.path[dir.id]);
    dir.path[dir.id] = (char *)malloc(DATA_W);
    strncpy(dir.path[dir.id], path_id, DATA_W);
    dir.id = dir.id + 1;
    return 0;
}
//=======================================================================
int inotify_watch_dir(char *dir_path, int fd)
{
    int wd;
    DIR *dp;
    char pdir_home[DATA_W];
    char pdir[DATA_W];
    strcpy(pdir_home, dir_path);
    struct dirent *dirp;
    struct inotify_event *event;
    if (fd < 0) {
        fprintf(stderr, "inotify_init failed\n");
        return -1;
    }
    wd = inotify_add_watch(fd, dir_path, IN_CREATE | IN_ATTRIB | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO);
    if ((dp = opendir(dir_path)) == NULL) {
        return -1;
    }
    while ((dirp = readdir(dp)) != NULL) {
        if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0) {
            continue;
        }

        if (dirp->d_type == IS_FILE) {
            continue;
        }
        if (dirp->d_type == IS_DIR) {
            strncpy(pdir, pdir_home, DATA_W);
            strncat(pdir, "/", 1);
            strncat(pdir, dirp->d_name, BUFSIZ);
            id_add(pdir);
            inotify_watch_dir(pdir, fd);
        }
    }
    closedir(dp);
}
// \brief 检索目标内容 fcitx-vvv.soxxxx中间内容
// \param input: 传入等待检测的字段
// \return 返回 传出中间字段的内容， NULL
// \note
// 用法
// const char *strexam = "fcitx-s1.sodpkgcccc";
// char* result = NULL;
// if(find_target("fcitx-",".so",strexam,result) == 1)
//      printf("%s", result);

int str_find_target(const char *strback,const char *strfilter,char **strTarget) {
    if (strfilter == NULL) {
        return 0;
    }

    char localName[BUFSIZ];
    memset(localName, 0, BUFSIZ);

    //[2] 第一次与 "strback" 匹配的字段位置
    const char *ePointPosition = strstr(strfilter, strback);
    if (ePointPosition == NULL) {
        return 0;
    }
    size_t endPosition = ePointPosition - strfilter;

    //确保是 后缀是 "strback"
    if (endPosition > 0) {
        for (int i = 0; i < (int)endPosition; i++) {
            localName[i] = *(strfilter + i);
        }

        *strTarget = malloc(strlen(localName) + 1);
        memset(*strTarget, 0, (strlen(localName) + 1));
        strncpy(*strTarget, localName, strlen(localName));

        return 1;
    }
    return 0;
}
// \brief 删除注释符号
// \param input: 传入需要修改的字符串
void delete_comment(char str[]) {
    int i, j;
    for (i = j = 0; str[i] != '\0'; i++) {
        if (str[i] != '\#' || str[i + 1] == ' ') {
            str[j++] = str[i];
        }
    }
    str[j] = '\0';
}

// \brief 文件注释检查并修改
void file_comment_modify(const char *filename) {
    int fd, len;
    char str[BUFSIZ];

    fd = open(filename, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd) {
        len = read(fd, str, BUFSIZ);
        str[len] = '\0';
        delete_comment(str);
        ftruncate(fd, 0);
        lseek(fd, 0, SEEK_SET);
        write(fd, str, strlen(str));
    }
    close(fd);
}

int get_curindex_inputmethod(int32_t sigvalue, char **imname) {
    DBusError err;
    DBusConnection *connection;
    DBusMessage *msg;
    DBusMessageIter arg;
    DBusPendingCall *pending;

    dbus_error_init(&err);

    connection = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "ConnectionErr: %s\n", err.message);
        dbus_error_free(&err);
    }

    if (connection == NULL) {
        return -1;
    }

    if ((msg = dbus_message_new_method_call("org.fcitx.Fcitx-0", "/inputmethod",
                                            "org.fcitx.Fcitx.InputMethod",
                                            "GetIMByIndex")) == NULL) {
        fprintf(stderr, "MethodNULL\n");
        return -1;
    }

    dbus_message_iter_init_append(msg, &arg);
    if (!dbus_message_iter_append_basic(&arg, DBUS_TYPE_INT32, &sigvalue)) {
        fprintf(stderr, "Out of Memory!\n");
        return -1;
    }

    if (!dbus_connection_send_with_reply(connection, msg, &pending, -1)) {
        fprintf(stderr, "Out of Memory!\n");
        return -1;
    }

    if (NULL == pending) {
        fprintf(stderr, "Pending Call Null\n");
        exit(1);
    }

    dbus_connection_flush(connection);
    dbus_message_unref(msg);

    dbus_pending_call_block(pending);
    msg = dbus_pending_call_steal_reply(pending);
    if (msg == NULL) {
        fprintf(stderr, "Reply Null/n");
        exit(1);
    }

    if (!dbus_message_iter_init(msg, &arg))
        fprintf(stderr, "Message has no arguments!n");
    else if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&arg))
        fprintf(stderr, "Argument is not string!n");
    else
        dbus_message_iter_get_basic(&arg, imname);

    dbus_connection_flush(connection);

    // free message
    dbus_message_unref(msg);

    return 0;
}

void get_config_path(char** dimConfigPath,char** imPluginConfigPath)
{
    FILE *fp = NULL;
    if (!*dimConfigPath) {
        //获取默认输入法配置文件路径
        fp = FcitxXDGGetFileUserWithPrefix("conf", "fcitx-defaultim.config", "r",dimConfigPath);
        if (fp) {
            fclose(fp);
            file_comment_modify(*dimConfigPath);
        }
    }

    if (!*imPluginConfigPath) {
        //获取输入法插件配置文件路径
        fp = FcitxXDGGetFileUserWithPrefix("conf", "fcitx-implugin.config", "r",imPluginConfigPath);
        if (fp) {
            fclose(fp);
            file_comment_modify(*imPluginConfigPath);
        }
    }
}
//====================================================================
/* Display information from inotify_event structure */
void display_inotify_event(char** dimConfigPath,char **imPluginConfigPath,struct inotify_event *i)
{
    if (i->cookie > 0)
    get_config_path(dimConfigPath,imPluginConfigPath);
    if (i->mask & IN_MOVED_TO){
        char* imName = NULL;
        printf("dir.path[i->wd] %s",dir.path[i->wd]);
        printf("i->name %s",i->name);
        if((strcmp(dir.path[i->wd],"/usr/share/fcitx/inputmethod") == 0 || strcmp(dir.path[i->wd],"/usr/share/fcitx/table") == 0)
                && str_find_target(".conf",i->name,&imName) == 1){
            printf("imName %s",imName);

            if(!*dimConfigPath){
                ini_puts("DefaultIM", "IMNAME", imName, *dimConfigPath);
            }

            sleep(3);
            fcitx_utils_launch_restart();
//            system("fcitx -r");
            sleep(3);

            char settingWizard[BUFSIZ];
            memset(settingWizard, 0,FCITX_ARRAY_SIZE(settingWizard));
            if(!*dimConfigPath){
                ini_gets(imName, "SettingWizard", "none",settingWizard,FCITX_ARRAY_SIZE(settingWizard),*imPluginConfigPath);
            }
            char *pSettingWizard =malloc((strlen(settingWizard) + 1));
            memset(pSettingWizard, 0,(strlen(settingWizard) + 1));
            strncpy(pSettingWizard, settingWizard,(strlen(settingWizard) + 1));

            char parameter[BUFSIZ];
            memset(parameter, 0, FCITX_ARRAY_SIZE(parameter));
            if(!*dimConfigPath){
                ini_gets(imName, "Parameter", NULL, parameter, FCITX_ARRAY_SIZE(parameter),*imPluginConfigPath);
            }
            char *pParameter = malloc((strlen(parameter) + 1));
            memset(pParameter, 0, (strlen(parameter) + 1));
            strncpy(pParameter, parameter, (strlen(parameter) + 1));

            if (fcitx_utils_strcmp0(pSettingWizard, "none") != 0 &&
                    fcitx_utils_strcmp0(imName, "baidupinyin") != 0 ) {
                if (pParameter) {
                    char *commod[] = {
                        pSettingWizard,
                        (char *)(intptr_t)pParameter,
                        NULL};
                    fcitx_utils_start_process(commod);
                } else {
                    char *commod[] = {
                        pSettingWizard,
                        NULL};
                    fcitx_utils_start_process(commod);
                }
            }
            else if(fcitx_utils_strcmp0(imName, "none") != 0 &&
                    fcitx_utils_strcmp0(dir.path[i->wd],"/usr/share/fcitx/table") != 0){
                sleep(5);
                char *result = NULL;
                fcitx_utils_alloc_cat_str(result, "fcitx-", imName);
                char *commod[] = {
                    "fcitx-config-gtk3",
                    result};
                fcitx_utils_start_process(commod);
                free(result);
            }
            else if(strcmp(dir.path[i->wd],"/usr/share/fcitx/table") == 0){
                char *commod[] = {
                    "fcitx-config-gtk3",
                    "fcitx-table"};
                fcitx_utils_start_process(commod);
            }
            free(pSettingWizard);
            free(pParameter);
            printf("add imname = %s; \n", imName);
        }
        free(imName);
    }
    if (i->mask & IN_DELETE) {
        char* imName = NULL;
        if((strcmp(dir.path[i->wd],"/usr/share/fcitx/inputmethod") == 0 || strcmp(dir.path[i->wd],"/usr/share/fcitx/table") == 0)
                && str_find_target(".conf",i->name,&imName) == 1){

            sleep(3);
            fcitx_utils_launch_restart();
//            system("fcitx -r");
            sleep(3);

            if(!*dimConfigPath){
                char curDeimName[BUFSIZ];
                memset(curDeimName, 0,FCITX_ARRAY_SIZE(curDeimName));
                ini_gets("DefaultIM", "IMNAME", "fcitx-keyboard-us",curDeimName, FCITX_ARRAY_SIZE(curDeimName),*dimConfigPath);
                printf("curDeimName %s,",*curDeimName);

                char *pCurDeimName =malloc((strlen(curDeimName) + 1));
                memset(pCurDeimName, 0, (strlen(curDeimName) + 1));
                strncpy(pCurDeimName, curDeimName,(strlen(curDeimName) + 1));

                if (fcitx_utils_strcmp0(pCurDeimName, imName) == 0) {
                    char *secName;
                    get_curindex_inputmethod(2, &secName);
                    if (fcitx_utils_strcmp0(secName, "") == 0) {
                        secName = "fcitx-keyboard-us";
                    }

                    ini_puts("DefaultIM", "IMNAME", secName, *dimConfigPath);
                    free(secName);
                }
                free(pCurDeimName);
            }
            printf("remove imname = %s; \n", imName);
        }
        free(imName);
    }
}

//====================================================================
int main(int argc, char *argv[])
{
    dir.id = 1;
    dir.path = (char **)malloc(65534);
    char* watchpath = fcitx_utils_get_fcitx_path("pkgdatadir");//"/usr/share/fcitx";
    id_add(watchpath);
    int fd = inotify_init();

    inotify_watch_dir(watchpath, fd);

    int i;
    int len;
    int nread;
    struct stat res;
    char path[BUFSIZ];
    char buf[BUFSIZ];
    struct inotify_event *event;
    buf[sizeof(buf) - 1] = 0;
    char *dimConfigPath = NULL;
    char *imPluginConfigPath = NULL;
    while ((len = read(fd, buf, sizeof(buf) - 1)) > 0) {
        nread = 0;
        while (len > 0) {
            event = (struct inotify_event *)&buf[nread];
            for (i = 0; i < EVENT_NUM; i++) {
                if ((event->mask >> i) & 1) {
                    if (event->len > 0)
                        if (strncmp(event->name, ".", 1)) {
                            display_inotify_event(&dimConfigPath,&imPluginConfigPath,event);
                            if ((strcmp(event_str[i], "IN_CREATE") == 0) || (strcmp(event_str[i], "IN_MOVED_TO") == 0)) {
                                memset(path, 0, sizeof(path));
                                strncat(path, dir.path[event->wd], BUFSIZ);
                                strncat(path, "/", 1);
                                strncat(path, event->name, BUFSIZ);
                                stat(path, &res);
                                if (S_ISDIR(res.st_mode)) {
                                    inotify_add_watch(fd, path, IN_CREATE | IN_ATTRIB | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO);
                                    id_add(path);
                                }
                            }
                        }
                }
            }
            nread = nread + sizeof(struct inotify_event) + event->len;
            len = len - sizeof(struct inotify_event) - event->len;
        }
    }
    free(watchpath);
    free(dimConfigPath);
    free(imPluginConfigPath);
    return 0;
}

