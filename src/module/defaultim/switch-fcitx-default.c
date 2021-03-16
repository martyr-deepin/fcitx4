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

#include "minIni.h"

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

int main(int argc, char *argv[]) {

    FILE *fp = NULL;
    char *dimConfigPath = NULL;
    fp = FcitxXDGGetFileUserWithPrefix(
        "conf", "fcitx-defaultim.config", "r",
        &dimConfigPath);
    if (fp) {
        fclose(fp);
        file_comment_checkout(dimConfigPath);
    }
    char curDeimName[BUFSIZ];
    memset(curDeimName, 0,
           FCITX_ARRAY_SIZE(curDeimName));
    ini_gets("DefaultIM", "IMNAME", "fcitx-keyboard-us",
             curDeimName, FCITX_ARRAY_SIZE(curDeimName),
             dimConfigPath);
    free(dimConfigPath);

    char* args[] = {
        "/usr/bin/fcitx-remote",
        "-s",
        curDeimName, /* parent process haven't even touched this... */
        NULL
    };
    fcitx_utils_start_process(args);
    return 0;
}
