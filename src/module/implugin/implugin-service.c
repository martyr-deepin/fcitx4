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

// \brief 检索目标内容 fcitx-vvv.soxxxx中间内容
// \param input: 传入等待检测的字段
// \return 返回 传出中间字段的内容， NULL
// \note
// 用法
// const char *strexam = "fcitx-s1.sodpkgcccc";
// char* result = find_target(strexam);
// printf("%s", result);

char *find_target(const char *input) {
    char localname[BUFSIZ];
    char *output = NULL;
    memset(localname, 0, BUFSIZ * sizeof(char));

    if (input == NULL) {
        return NULL;
    }
    //[1] 第一次与 "fcitx-" 不匹配的字段位置
    const char * start_point_position = strstr(input, "fcitx-");
    int start_position = (start_point_position - input ) + strlen("fcitx-");
    //[2] 第一次与 ".so" 匹配的字段位置
    const char *end_point_position = strstr(input, ".so");
    size_t end_position = end_point_position - input;

    //确保是 开头是 "fcitx-" , 后缀是 ".so"
    if (end_position > start_position && start_position > 0) {
        for (int i = start_position; i < (int)end_position; i++) {
            localname[i - start_position] = *(input + i);
        }

        output = malloc((strlen(localname) + 1) * sizeof(char));
        memset(output, 0, (strlen(localname) + 1) * sizeof(char));
        strncpy(output, localname, strlen(localname));
        return output;
    }
    return NULL;
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
    char *defaultimconfigpath = NULL;
    char *impluginconfigpath = NULL;

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
                        char *imname = find_target(event->name);
                        if(imname == NULL)
                        {
                            continue;
                        }
                        if (fcitx_utils_strcmp0(event_str[i], "IN_CREATE") ==
                                0 &&
                            fcitx_utils_strcmp0("baidupinyin", imname) != 0) {
                            ini_puts("GlobalSelector", "IMNAME", imname,
                                     defaultimconfigpath);

                            sleep(10);
                            fcitx_utils_launch_restart();
                            sleep(5);

                            char settingwizard[BUFSIZ];
                            memset(settingwizard, 0,
                                   FCITX_ARRAY_SIZE(settingwizard));
                            ini_gets(imname, "SettingWizard", "none",
                                     settingwizard,
                                     FCITX_ARRAY_SIZE(settingwizard),
                                     impluginconfigpath);

                            char *psettingwizard = malloc(
                                (strlen(settingwizard) + 1) * sizeof(char));
                            memset(psettingwizard, 0,
                                   (strlen(settingwizard) + 1) * sizeof(char));
                            strncpy(psettingwizard, settingwizard,
                                    (strlen(settingwizard) + 1) * sizeof(char));

                            if (fcitx_utils_strcmp0(psettingwizard, "none") !=
                                0) {
                                char* commod[] = {
                                    psettingwizard,
                                    NULL
                                };
                                fcitx_utils_start_process(commod);
                            }
                            free(psettingwizard);

                        } else if (fcitx_utils_strcmp0(event_str[i],
                                                       "IN_DELETE") == 0) {
                            char curdeimname[BUFSIZ];
                            memset(curdeimname, 0,
                                   FCITX_ARRAY_SIZE(curdeimname));
                            ini_gets("GlobalSelector", "IMNAME",
                                     "fcitx-keyboard-us", curdeimname,
                                     FCITX_ARRAY_SIZE(curdeimname),
                                     defaultimconfigpath);

                            char *pcurdeimname = malloc(
                                (strlen(curdeimname) + 1) * sizeof(char));
                            memset(pcurdeimname, 0,
                                   (strlen(curdeimname) + 1) * sizeof(char));
                            strncpy(pcurdeimname, curdeimname,
                                    (strlen(curdeimname) + 1) * sizeof(char));

                            if (fcitx_utils_strcmp0(pcurdeimname, imname) ==
                                0) {
                                ini_puts("GlobalSelector", "IMNAME",
                                         "fcitx-keyboard-us",
                                         defaultimconfigpath);
                            }
                            ini_puts("GlobalSelector", "IMLOG", pcurdeimname,
                                     defaultimconfigpath);

                            free(pcurdeimname);

                            sleep(3);

                            fcitx_utils_launch_restart();
                        }
                        free(imname);
                    } else
                        fprintf(stdout, "%s --- %s\n", " ", event_str[i]);
                }
            }
            nread = nread + sizeof(struct inotify_event) + event->len;
            len = len - sizeof(struct inotify_event) - event->len;
        }
    }

    free(impluginconfigpath);
    free(defaultimconfigpath);

    return 0;
}
