#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>

#include "fcitx-utils/utils.h"
#include "minIni.h"

#define EVENT_NUM 12

char *event_str[EVENT_NUM] =
{
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

void fcitx_util_strc_to_imname(char* imname,int size)
{
    for (int i = 6; i <= size; i++) {
        imname[i-6]=imname[i];
    }
    return;
}

int main(int argc, char *argv[])
{
    fcitx_utils_init_as_daemon();

    int fd;
    int wd;
    int len;
    int nread;
    char buf[BUFSIZ];
    struct inotify_event *event;
    int i;

    char* fcitxlibpath = fcitx_utils_get_fcitx_path_with_filename("libdir", "fcitx");
    char* defaultimconfigpath = fcitx_utils_malloc0(50*sizeof(char));
    char* impluginconfigpath = fcitx_utils_malloc0(50*sizeof(char));

    //获取默认输入法配置文件路径
    FILE *fp;
    fp = FcitxXDGGetFileUserWithPrefix("conf", "fcitx-defaultim.config", "r", &defaultimconfigpath);
    if (fp)
        fclose(fp);

    //获取输入法插件配置文件路径
    fp = FcitxXDGGetFileUserWithPrefix("conf", "fcitx-implugin.config", "r", &impluginconfigpath);
    if (fp)
        fclose(fp);

    printf("%s",fcitxlibpath);
    if(!fcitxlibpath)
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
    wd = inotify_add_watch(fd, fcitxlibpath, IN_CREATE|IN_DELETE|IN_MOVED_FROM|IN_MOVED_TO);
    if(wd < 0) {
        fprintf(stderr, "inotify_add_watch %s failed\n", argv[1]);
        return -1;
    }

    buf[sizeof(buf) - 1] = 0;
    while( (len = read(fd, buf, sizeof(buf) - 1)) > 0 ) {
        nread = 0;
        while(len> 0) {
            event = (struct inotify_event *)&buf[nread];
            for(i=0; i<EVENT_NUM; i++) {
                if((event->mask >> i) & 1) {
                    if(event->len > 0)
                    {
                        char* imname= fcitx_utils_malloc0(50*sizeof(char));
                        strcpy(imname, event->name);
                        fcitx_util_strc_to_imname(imname,strlen(imname));
                        imname[strlen(imname)-3] = '\0';

                        char* curdeimname= fcitx_utils_malloc0(50*sizeof(char));

                        if(fcitx_utils_strcmp0(event_str[i],"IN_CREATE") == 0 && strcmp("fcitx-baidupinyin.so",event->name)!=0 )
                        {
                            ini_puts("GlobalSelector", "IMNAME", imname, defaultimconfigpath);
                            fprintf(stdout, "%s --- %s --- %s\n", " ", event_str[i],event->name,imname);

                            fcitx_utils_launch_restart();

                            char* settingwizard = fcitx_utils_malloc0(50*sizeof(char));
                            ini_gets(imname, "SettingWizard", "none",settingwizard, FCITX_ARRAY_SIZE(settingwizard), impluginconfigpath);
                            if(strcmp(settingwizard,"none")!=0)
                            {
                               fcitx_utils_start_process(settingwizard);
                            }
                            free(settingwizard);

                        }
                        else if(fcitx_utils_strcmp0(event_str[i],"IN_DELETE") == 0)
                        {
                            ini_gets("GlobalSelector", "IMNAME", "fcitx-keyboard-us",curdeimname, FCITX_ARRAY_SIZE(curdeimname), defaultimconfigpath);
                            if(strcmp(curdeimname,imname)==0)
                            {
                                ini_puts("GlobalSelector", "IMNAME", "fcitx-keyboard-us", defaultimconfigpath);
                            }

                            fprintf(stdout, "%s --- %s --- %s\n", " ", event_str[i],event->name,imname);

                            fcitx_utils_launch_restart();
                        }

                        free(imname);
                    }
                    else
                        fprintf(stdout, "%s --- %s\n", " ", event_str[i]);
                }
            }
            nread = nread + sizeof(struct inotify_event) + event->len;
            len = len - sizeof(struct inotify_event) - event->len;
        }
    }
    free(defaultimconfigpath);

    return 0;
}
