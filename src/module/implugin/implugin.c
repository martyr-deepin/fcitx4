/***************************************************************************
 *   Copyright (C) 2021~2025 by chenshijie                                  *
 *   chenshijie@uniontech.com                                                   *
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

#include "fcitx/instance.h"
#include "fcitx/fcitx.h"
#include "fcitx/module.h"
typedef struct _IMPlugin IMPlugin;

typedef struct _ClosedSourceIM {
    char *swizardpath;//SettingWizard 设置向导软件目录
    char *spath;//Setting 设置属性软件目录
} ClosedSourceIM;

struct _IMPlugin {
    FcitxGenericConfig gconfig;
    ClosedSourceIM chineseime;
    ClosedSourceIM iflyime;
    ClosedSourceIM huayupy;
    FcitxInstance* owner;
};

static void* IMPluginCreate(FcitxInstance* instance);

static  void IMPluginReload(void* arg);
static boolean LoadIMPluginConfig(IMPlugin* implugin);
static void SaveIMPluginConfig(IMPlugin* implugin);

FCITX_DEFINE_PLUGIN(fcitx_implugin, module, FcitxModule) = {
    IMPluginCreate,
    NULL,
    NULL,
    NULL,
    IMPluginReload
};

CONFIG_BINDING_BEGIN(IMPlugin)
CONFIG_BINDING_REGISTER("chineseime", "SettingWizard", chineseime.swizardpath)
CONFIG_BINDING_REGISTER("chineseime", "Setting", chineseime.spath)
CONFIG_BINDING_REGISTER("iflyime", "SettingWizard", iflyime.swizardpath)
CONFIG_BINDING_REGISTER("iflyime", "Setting", iflyime.spath)
CONFIG_BINDING_REGISTER("huayupy", "SettingWizard", huayupy.swizardpath)
CONFIG_BINDING_REGISTER("huayupy", "Setting", huayupy.spath)
CONFIG_BINDING_END()

void* IMPluginCreate(FcitxInstance* instance)
{
    IMPlugin* implugin = fcitx_utils_malloc0(sizeof(IMPlugin));
    implugin->owner = instance;
    if (!LoadIMPluginConfig(implugin)) {
        free(implugin);
        return NULL;
    }

    return implugin;
}

void IMPluginReload(void* arg)
{
    IMPlugin* implugin = arg;
    LoadIMPluginConfig(implugin);
}

CONFIG_DESC_DEFINE(GetIMPluginConfig, "fcitx-implugin.desc")

boolean LoadIMPluginConfig(IMPlugin* implugin)
{
    FcitxConfigFileDesc* configDesc = GetIMPluginConfig();
    if (configDesc == NULL)
        return false;
    FILE *fp;
    fp = FcitxXDGGetFileUserWithPrefix("conf", "fcitx-implugin.config", "r", NULL);
    if (!fp) {
        if (errno == ENOENT)
            SaveIMPluginConfig(implugin);
    }
    FcitxConfigFile *cfile = FcitxConfigParseConfigFileFp(fp, configDesc);
    IMPluginConfigBind(implugin, cfile, configDesc);
    FcitxConfigBindSync((FcitxGenericConfig*)implugin);
    if (fp)
        fclose(fp);
    return true;
}

void SaveIMPluginConfig(IMPlugin* implugin)
{
    FcitxConfigFileDesc* configDesc = GetIMPluginConfig();
    FILE *fp = FcitxXDGGetFileUserWithPrefix("conf", "fcitx-implugin.config", "w", NULL);
    FcitxConfigSaveConfigFileFp(fp, &implugin->gconfig, configDesc);
    if (fp)
        fclose(fp);
}
