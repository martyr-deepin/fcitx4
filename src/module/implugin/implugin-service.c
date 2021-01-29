#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>

#include "fcitx-utils/utils.h"
#include "minIni.h"

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

//字符串向前移动
void fcitx_util_strc_to_pop(char *imname, int size, int pop) {
    for (int i = pop; i <= size; i++) {
        imname[i - pop] = imname[i];
    }
    return;
}

int main(int argc, char *argv[]) {
    fcitx_utils_init_as_daemon();

    int fd;
    int wd;
    int len;
    int nread;
    char buf[BUFSIZ];
    struct inotify_event *event;
    int i;

    char *fcitxlibpath =
        fcitx_utils_get_fcitx_path_with_filename("libdir", "fcitx");
    char defaultimconfigpath[50];
    char impluginconfigpath[50];

    //获取默认输入法配置文件路径
    FILE *fp;
    fp = FcitxXDGGetFileUserWithPrefix("conf", "fcitx-defaultim.config", "r",
                                       &defaultimconfigpath);
    if (fp)
        fclose(fp);

    //获取输入法插件配置文件路径
    fp = FcitxXDGGetFileUserWithPrefix("conf", "fcitx-implugin.config", "r",
                                       &impluginconfigpath);
    if (fp)
        fclose(fp);

    printf("%s", fcitxlibpath);
    if (!fcitxlibpath)
        return -1;

    // 初始化
    fd = inotify_init();
    if (fd < 0) {
        fprintf(stderr, "inotify_init failed\n");
        return -1;
    }

    /* 增加监听事件
     * 监听所有事件：IN_ALL_EVENTS
     * 监听文件是否被创建,删除,移动：IN_CREATE|IN_DELETE|IN_MOVED_FROM|IN_MOVED_TO
     */
    wd = inotify_add_watch(fd, fcitxlibpath,
                           IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO);
    if (wd < 0) {
        fprintf(stderr, "inotify_add_watch %s failed\n", argv[1]);
        return -1;
    }

    buf[sizeof(buf) - 1] = 0;
    while ((len = read(fd, buf, sizeof(buf) - 1)) > 0) {
        nread = 0;
        while (len > 0) {
            event = (struct inotify_event *)&buf[nread];
            for (i = 0; i < EVENT_NUM; i++) {
                if ((event->mask >> i) & 1) {
                    if (event->len > 0) {
                        //获取输入法名称
                        char imname[50];
                        memset(imname,0,FCITX_ARRAY_SIZE(imname));
                        strcpy(imname, event->name);
                        fcitx_util_strc_to_pop(imname, strlen(imname), 6);
                        strstr(imname, ".so")[0] = '\0';

                        if (fcitx_utils_strcmp0(event_str[i], "IN_CREATE") ==
                                0 &&
                            fcitx_utils_strcmp0("fcitx-baidupinyin.so",
                                                event->name) != 0) {
                            ini_puts("GlobalSelector", "IMNAME", imname,
                                     defaultimconfigpath);
                            fprintf(stdout, "%s --- %s --- %s\n", " ",
                                    event_str[i], event->name, imname);

                            sleep(10);
                            fcitx_utils_launch_restart();
                            sleep(5);

                            char settingwizard[50];
                            memset(settingwizard,0,FCITX_ARRAY_SIZE(settingwizard));
                            ini_gets(imname, "SettingWizard", "none",
                                     settingwizard, FCITX_ARRAY_SIZE(settingwizard), impluginconfigpath);
                            if (fcitx_utils_strcmp0(settingwizard, "none") !=
                                0) {
                               fcitx_utils_launch_tool(settingwizard, NULL);
                            }

                        } else if (fcitx_utils_strcmp0(event_str[i],
                                                       "IN_DELETE") == 0) {
                            char curdeimname[50];
                            memset(curdeimname,0,FCITX_ARRAY_SIZE(curdeimname));
                            ini_gets("GlobalSelector", "IMNAME",
                                     "fcitx-keyboard-us", curdeimname, FCITX_ARRAY_SIZE(curdeimname),
                                     defaultimconfigpath);

                            if (fcitx_utils_strcmp0(curdeimname, imname) == 0) {
                                ini_puts("GlobalSelector", "IMNAME",
                                         "fcitx-keyboard-us",
                                         defaultimconfigpath);
                            }
                            ini_puts("GlobalSelector", "IMLOG", curdeimname,
                                     defaultimconfigpath);

                            fprintf(stdout, "%s --- %s --- %s\n", " ",
                                    event_str[i], event->name, imname);
                            sleep(3);

                            fcitx_utils_launch_restart();
                        }
                    } else
                        fprintf(stdout, "%s --- %s\n", " ", event_str[i]);
                }
            }
            nread = nread + sizeof(struct inotify_event) + event->len;
            len = len - sizeof(struct inotify_event) - event->len;
        }
    }

    return 0;
}
