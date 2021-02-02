/*
* Copyright (C) 2021 ~ 2025 Uniontech Software Technology Co.,Ltd.
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
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <fcntl.h>
#include <unistd.h>

#include "fcitx-utils/utils.h"
#include "minIni.h"

// \brief 检索目标内容 fcitx-vvv.soxxxx中间内容
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
    memset(localName, 0, BUFSIZ * sizeof(char));

    if (inputFIleInfo == NULL) {
        return NULL;
    }
    //[1] 第一次与 "fcitx-" 不匹配的字段位置
    const char * sPointPosition = strstr(inputFIleInfo, "fcitx-");
    if(sPointPosition == NULL){
        return NULL;
    }
    int startPosition = (sPointPosition - inputFIleInfo ) + strlen("fcitx-");
    //[2] 第一次与 ".so" 匹配的字段位置
    const char *ePointPosition = strstr(inputFIleInfo, ".so");
    if(ePointPosition == NULL){
        return NULL;
    }
    size_t endPosition = ePointPosition - inputFIleInfo;

    //确保是 开头是 "fcitx-" , 后缀是 ".so"
    if (endPosition > startPosition && startPosition > 0) {
        for (int i = startPosition; i < (int)endPosition; i++) {
            localName[i - startPosition] = *(inputFIleInfo + i);
        }

        result = malloc((strlen(localName) + 1) * sizeof(char));
        memset(result, 0, (strlen(localName) + 1) * sizeof(char));
        strncpy(result, localName, strlen(localName));
        return result;
    }
    return NULL;
}

void delete_comment(char str[]){
    int i,j;
    for(i=j=0;str[i]!='\0';i++){
        if(str[i]!='\#' || str[i+1]==' ')
        {
            str[j++]=str[i];
        }
    }
    str[j]='\0';
}

void file_checkout(const char * filename) {
    int fd,len;
    char str[BUFSIZ];

    fd=open(filename,O_RDWR,O_CREAT|O_RDWR,S_IRUSR|S_IWUSR);
    if(fd){
    len=read(fd,str,BUFSIZ);
    str[len]='\0';
    delete_comment(str);
    ftruncate(fd, 0);
    lseek(fd, 0, SEEK_SET);
    write(fd,str,strlen(str));

    }
    close(fd);
}

#define EVENT_NUM 12

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


int main(int argc, char *argv[]) {

    int inotifyFd = -1;
    int inotifyWd = -1;
    int len;
    int nread;
    char buf[BUFSIZ];
    struct inotify_event *event;
    int i;

    char *fcitxLibPath =
        fcitx_utils_get_fcitx_path_with_filename("libdir", "fcitx");
    char *dimConfigPath = NULL;
    char *imPluginConfigPath = NULL;

    //获取默认输入法配置文件路径
    FILE *fp;
    fp = FcitxXDGGetFileUserWithPrefix("conf", "fcitx-defaultim.config", "r",
                                       &dimConfigPath);
    if (fp)
        fclose(fp);
    file_checkout(dimConfigPath);

    //获取输入法插件配置文件路径
    fp = FcitxXDGGetFileUserWithPrefix("conf", "fcitx-implugin.config", "r",
                                       &imPluginConfigPath);
    if (fp)
        fclose(fp);
    file_checkout(imPluginConfigPath);

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
    inotifyWd = inotify_add_watch(inotifyFd, fcitxLibPath,
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
                        if(imName == NULL){
                            continue;
                        }
                        if (fcitx_utils_strcmp0(event_str[i], "IN_CREATE") ==
                                0 &&
                            fcitx_utils_strcmp0("baidupinyin", imName) != 0) {
                            ini_puts("DefaultIM", "IMNAME", imName,
                                     dimConfigPath);

                            sleep(10);
                            fcitx_utils_launch_restart();
                            sleep(5);

                            char settingWizard[BUFSIZ];
                            memset(settingWizard, 0,
                                   FCITX_ARRAY_SIZE(settingWizard));
                            ini_gets(imName, "SettingWizard", "none",
                                     settingWizard,
                                     FCITX_ARRAY_SIZE(settingWizard),
                                     imPluginConfigPath);

                            char *pSettingWizard = malloc(
                                (strlen(settingWizard) + 1) * sizeof(char));
                            memset(pSettingWizard, 0,
                                   (strlen(settingWizard) + 1) * sizeof(char));
                            strncpy(pSettingWizard, settingWizard,
                                    (strlen(settingWizard) + 1) * sizeof(char));

                            char parameter[BUFSIZ];
                            memset(parameter, 0,
                                   FCITX_ARRAY_SIZE(parameter));
                            ini_gets(imName, "Parameter", NULL,
                                     parameter,
                                     FCITX_ARRAY_SIZE(parameter),
                                     imPluginConfigPath);

                            char *pParameter = malloc(
                                (strlen(parameter) + 1) * sizeof(char));
                            memset(pParameter, 0,
                                   (strlen(parameter) + 1) * sizeof(char));
                            strncpy(pParameter, parameter,
                                    (strlen(parameter) + 1) * sizeof(char));

                            if (fcitx_utils_strcmp0(pSettingWizard, "none") !=
                                0) {

                                if(pParameter){
                                    char* commod[] = {
                                        pSettingWizard,
                                        (char*)(intptr_t)pParameter
                                    };
                                    fcitx_utils_start_process(commod);
                                }
                                else {
                                    char* commod[] = {
                                        pSettingWizard,
                                        NULL
                                    };
                                    fcitx_utils_start_process(commod);
                                }

                            }
                            free(pSettingWizard);
                            free(pParameter);

                        } else if (fcitx_utils_strcmp0(event_str[i],
                                                       "IN_DELETE") == 0) {
                            char curDeimName[BUFSIZ];
                            memset(curDeimName, 0,
                                   FCITX_ARRAY_SIZE(curDeimName));
                            ini_gets("DefaultIM", "IMNAME",
                                     "fcitx-keyboard-us", curDeimName,
                                     FCITX_ARRAY_SIZE(curDeimName),
                                     dimConfigPath);

                            char *pCurDeimName = malloc(
                                (strlen(curDeimName) + 1) * sizeof(char));
                            memset(pCurDeimName, 0,
                                   (strlen(curDeimName) + 1) * sizeof(char));
                            strncpy(pCurDeimName, curDeimName,
                                    (strlen(curDeimName) + 1) * sizeof(char));

                            if (fcitx_utils_strcmp0(pCurDeimName, imName) ==
                                0) {
                                ini_puts("DefaultIM", "IMNAME",
                                         "fcitx-keyboard-us",
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
