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
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>

#include <dbus/dbus-glib.h>
#include <dbus/dbus.h>

#include "minIni.h"

// \brief 检索目标内容 fcitx-vvv.mbxxxx中间内容
// \param input: 传入等待检测的字段
// \return 返回 传出中间字段的内容， NULL
// \note
// 用法
// const char *strexam = "fcitx-s1.sodpkgcccc";
// char* result = find_target(strexam);
// printf("%s", result);

char *find_target(const char *inputFIleInfo) {
    char localName[BUFSIZ];
    char *result = NULL;
    memset(localName, 0, BUFSIZ);

    if (inputFIleInfo == NULL) {
        return NULL;
    }
    int startPosition = 0;
    //第一次与 ".mb" 匹配的字段位置
    const char *ePointPosition = strstr(inputFIleInfo, ".mb");
    if (ePointPosition == NULL) {
        return NULL;
    }
    size_t endPosition = ePointPosition - inputFIleInfo;

    //确保是 后缀是 ".mb"
    if (endPosition > startPosition) {
        for (int i = startPosition; i < (int)endPosition; i++) {
            localName[i - startPosition] = *(inputFIleInfo + i);
        }

        result = malloc(strlen(localName) + 1);
        memset(result, 0, (strlen(localName) + 1));
        strncpy(result, localName, strlen(localName));
        return result;
    }
    return NULL;
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
void file_comment_checkout(const char *filename) {
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

int send_a_method(int32_t sigvalue, char **imname) {
    DBusError err;
    DBusConnection *connection;
    DBusMessage *msg;
    DBusMessageIter arg;
    DBusPendingCall *pending;
    int ret;

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

#define EVENT_NUM 12

char *event_str[EVENT_NUM] = {"IN_ACCESS",
                              "IN_MODIFY", //文件修改
                              "IN_ATTRIB",        "IN_CLOSE_WRITE",
                              "IN_CLOSE_NOWRITE", "IN_OPEN",
                              "IN_MOVED_FROM", //文件移动from
                              "IN_MOVED_TO",   //文件移动to
                              "IN_CREATE",     //文件创建
                              "IN_DELETE",     //文件删除
                              "IN_DELETE_SELF",   "IN_MOVE_SELF"};

int main(int argc, char *argv[]) {

    int inotifyFd = -1;
    int inotifyWd = -1;
    int len = 0;
    int nread = 0;
    char buf[BUFSIZ];
    struct inotify_event *event;
    int i = 0;

    char *fcitxLibPath =
        fcitx_utils_get_fcitx_path_with_filename("tabledir", "");
    char *dimConfigPath = NULL;
    char *imPluginConfigPath = NULL;
    FILE *fp = NULL;

    if (!fcitxLibPath)
        return -1;

    // 初始化
    inotifyFd = inotify_init();
    if (inotifyFd < 0) {
        fprintf(stderr, "inotify_init failed\n");
        return -1;
    }

    /* 增加监听事件
     * 监听所有事件：IN_ALL_EVENTS
     * 监听文件是否被创建,删除,移动：IN_CREATE|IN_DELETE|IN_MOVED_FROM|IN_MOVED_TO
     */
    inotifyWd =
        inotify_add_watch(inotifyFd, fcitxLibPath,
                          IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO);
    if (inotifyWd < 0) {
        fprintf(stderr, "inotify_add_watch %s failed\n", argv[1]);
        return -1;
    }

    buf[sizeof(buf) - 1] = 0;
    while ((len = read(inotifyFd, buf, sizeof(buf) - 1)) > 0) {
        nread = 0;
        while (len > 0) {
            event = (struct inotify_event *)&buf[nread];
            for (i = 0; i < EVENT_NUM; i++) {
                if ((event->mask >> i) & 1) {
                    if (event->len > 0) {
                        //获取输入法名称
                        char *imName = find_target(event->name);
                        if (imName == NULL) {
                            continue;
                        }

                        if (!dimConfigPath) {
                            //获取默认输入法配置文件路径
                            fp = FcitxXDGGetFileUserWithPrefix(
                                "conf", "fcitx-defaultim.config", "r",
                                &dimConfigPath);
                            if (fp) {
                                fclose(fp);
                                file_comment_checkout(dimConfigPath);
                            }
                        }

                        if (!imPluginConfigPath) {
                            //获取输入法插件配置文件路径
                            fp = FcitxXDGGetFileUserWithPrefix(
                                "conf", "fcitx-implugin.config", "r",
                                &imPluginConfigPath);
                            if (fp) {
                                fclose(fp);
                                file_comment_checkout(imPluginConfigPath);
                            }
                        }

                        if (fcitx_utils_strcmp0(event_str[i], "IN_CREATE") ==
                                0 &&
                            (fcitx_utils_strcmp0("wbpy", imName) == 0 ||
                             fcitx_utils_strcmp0("wbx", imName) == 0)) {
                            ini_puts("DefaultIM", "IMNAME", imName,
                                     dimConfigPath);

                            sleep(5);
                            fcitx_utils_launch_restart();
                            sleep(5);

                            char settingWizard[BUFSIZ];
                            memset(settingWizard, 0,
                                   FCITX_ARRAY_SIZE(settingWizard));
                            ini_gets(imName, "SettingWizard", "none",
                                     settingWizard,
                                     FCITX_ARRAY_SIZE(settingWizard),
                                     imPluginConfigPath);

                            char *pSettingWizard =
                                malloc((strlen(settingWizard) + 1));
                            memset(pSettingWizard, 0,
                                   (strlen(settingWizard) + 1));
                            strncpy(pSettingWizard, settingWizard,
                                    (strlen(settingWizard) + 1));

                            char parameter[BUFSIZ];
                            memset(parameter, 0, FCITX_ARRAY_SIZE(parameter));
                            ini_gets(imName, "Parameter", NULL, parameter,
                                     FCITX_ARRAY_SIZE(parameter),
                                     imPluginConfigPath);

                            char *pParameter = malloc((strlen(parameter) + 1));
                            memset(pParameter, 0, (strlen(parameter) + 1));
                            strncpy(pParameter, parameter,
                                    (strlen(parameter) + 1));

                            if (fcitx_utils_strcmp0(pSettingWizard, "none") !=
                                0) {

                                if (pParameter) {
                                    char *commod[] = {
                                        pSettingWizard,
                                        (char *)(intptr_t)pParameter, NULL};
                                    fcitx_utils_start_process(commod);
                                } else {
                                    char *commod[] = {pSettingWizard, NULL};
                                    fcitx_utils_start_process(commod);
                                }
                            }
                            free(pSettingWizard);
                            free(pParameter);

                        } else if (fcitx_utils_strcmp0(event_str[i],
                                                       "IN_DELETE") == 0 &&
                                   (fcitx_utils_strcmp0("wbpy", imName) == 0 ||
                                    fcitx_utils_strcmp0("wbx", imName) == 0)) {
                            char curDeimName[BUFSIZ];
                            memset(curDeimName, 0,
                                   FCITX_ARRAY_SIZE(curDeimName));
                            ini_gets("DefaultIM", "IMNAME", "fcitx-keyboard-us",
                                     curDeimName, FCITX_ARRAY_SIZE(curDeimName),
                                     dimConfigPath);

                            char *pCurDeimName =
                                malloc((strlen(curDeimName) + 1));
                            memset(pCurDeimName, 0, (strlen(curDeimName) + 1));
                            strncpy(pCurDeimName, curDeimName,
                                    (strlen(curDeimName) + 1));

                            if (fcitx_utils_strcmp0(pCurDeimName, imName) ==
                                0) {
                                char *secName;
                                send_a_method(1, &secName);
                                if (fcitx_utils_strcmp0(secName, "") == 0) {
                                    secName = "fcitx-keyboard-us";
                                }

                                ini_puts("DefaultIM", "IMNAME", secName,
                                         dimConfigPath);
                            }

                            free(pCurDeimName);

                            sleep(3);

                            fcitx_utils_launch_restart();
                        }
                        free(imName);
                    } else
                        fprintf(stdout, "%s --- %s\n", " ", event_str[i]);
                }
            }
            nread = nread + sizeof(struct inotify_event) + event->len;
            len = len - sizeof(struct inotify_event) - event->len;
        }
    }

    free(imPluginConfigPath);
    free(dimConfigPath);
    close(inotifyFd);

    return 0;
}
