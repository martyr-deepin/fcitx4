/***************************************************************************
 *   Copyright (C) 2002~2005 by Yuking                                     *
 *   yuking_net@sohu.com                                                   *
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
#include <stdio.h>
#include <limits.h>
#include <ctype.h>
#include <libintl.h>
#include <errno.h>
#include "fcitx/instance.h"

#include "fcitx/fcitx.h"
#include "fcitx/module.h"
#include "fcitx/hook.h"
#include "fcitx/candidate.h"
#include "fcitx/keys.h"
#include <fcitx/context.h>
#include "fcitx-config/xdg.h"
#include "fcitx-utils/log.h"

typedef struct _IMOnboard IMOnboard;

typedef struct _OnboardHandle {
    int idx;
    IMOnboard* imonboard;
} OnboardHandle;

struct _IMOnboard {
    FcitxGenericConfig gconfig;
    FcitxHotkey onboardKey;
    OnboardHandle handle;
    boolean triggered;
    char* commond;
    char* log;
    FcitxInstance* owner;
};

static void* IMOnboardCreate(FcitxInstance* instance);
static boolean IMOnboardPreFilter(void* arg, FcitxKeySym sym,
                                   unsigned int state,
                                   INPUT_RETURN_VALUE *retval
                                   );
static  void IMOnboardReset(void* arg);
static  void IMOnboardReload(void* arg);
static INPUT_RETURN_VALUE IMOnboardSelect(void* arg);
static boolean LoadIMOnboardConfig(IMOnboard* imonboard);
static void SaveIMOnboardConfig(IMOnboard* imonboard);

FCITX_DEFINE_PLUGIN(fcitx_onboard, module, FcitxModule) = {
    IMOnboardCreate,
    NULL,
    NULL,
    NULL,
    IMOnboardReload
};

CONFIG_BINDING_BEGIN(IMOnboard)
CONFIG_BINDING_REGISTER("GlobalOnboard", "IM", onboardKey)
CONFIG_BINDING_REGISTER("GlobalOnboard", "IMNAME", commond)
CONFIG_BINDING_REGISTER("GlobalOnboard", "IMLOG", log)
CONFIG_BINDING_END()

void* IMOnboardCreate(FcitxInstance* instance)
{
    IMOnboard* imonboard = fcitx_utils_malloc0(sizeof(IMOnboard));
    imonboard->owner = instance;
    if (!LoadIMOnboardConfig(imonboard)) {
        free(imonboard);
        return NULL;
    }

    FcitxKeyFilterHook hk;
    hk.arg = imonboard;
    hk.func = IMOnboardPreFilter;
    FcitxInstanceRegisterPreInputFilter(instance, hk);

    hk.arg = &imonboard->triggered;
    hk.func = FcitxDummyReleaseInputHook;
    FcitxInstanceRegisterPreReleaseInputFilter(instance, hk);

    FcitxHotkeyHook hkhk;
    hkhk.arg = imonboard;

    /* this key is ignore the very first input method which is for inactive */

    do {
        OnboardHandle* handle = &imonboard->handle;
        handle->idx = 0;
        handle->imonboard = imonboard;
        hkhk.arg = handle;
        hkhk.hotkeyhandle = IMOnboardSelect;
        hkhk.hotkey = &imonboard->onboardKey;
        FcitxInstanceRegisterHotkeyFilter(instance, hkhk);
    } while(0);

    FcitxIMEventHook resethk;
    resethk.arg = imonboard;
    resethk.func = IMOnboardReset;
    FcitxInstanceRegisterResetInputHook(instance, resethk);
    return imonboard;
}

boolean IMOnboardPreFilter(void* arg, FcitxKeySym sym, unsigned int state, INPUT_RETURN_VALUE* retval)
{
    IMOnboard* imonboard = arg;
    FcitxInstance* instance = imonboard->owner;
    FcitxInputState *input = FcitxInstanceGetInputState(instance);
    FcitxGlobalConfig* fc = FcitxInstanceGetGlobalConfig(instance);
    if (!imonboard->triggered)
        return false;
    FcitxCandidateWordList* candList = FcitxInputStateGetCandidateList(input);
    if (FcitxHotkeyIsHotKey(sym, state,
                            FcitxConfigPrevPageKey(instance, fc))) {
        FcitxCandidateWordGoPrevPage(candList);
        *retval = IRV_DISPLAY_MESSAGE;
    } else if (FcitxHotkeyIsHotKey(sym, state,
                                   FcitxConfigNextPageKey(instance, fc))) {
        FcitxCandidateWordGoNextPage(candList);
        *retval = IRV_DISPLAY_MESSAGE;
    } else if (FcitxHotkeyIsHotKey(sym, state, FCITX_SPACE)) {
        if (FcitxCandidateWordPageCount(candList) != 0)
            *retval = FcitxCandidateWordChooseByIndex(candList, 0);
    } else if (FcitxHotkeyIsHotKeyDigit(sym, state)) {
        int iKey = FcitxHotkeyCheckChooseKey(sym, state, DIGIT_STR_CHOOSE);
        if (iKey >= 0)
            *retval = FcitxCandidateWordChooseByIndex(candList, iKey);
    } else if (FcitxHotkeyIsHotKey(sym, state, FCITX_ESCAPE)) {
        *retval = IRV_CLEAN;
    }
    if (*retval == IRV_TO_PROCESS)
        *retval = IRV_DO_NOTHING;
    return true;
}

INPUT_RETURN_VALUE IMOnboardSelect(void* arg)
{
    OnboardHandle* handle = arg;
    IMOnboard* onboard = handle->imonboard;
    char* commod[] = {
        onboard->commond,
        NULL
    };
    fcitx_utils_start_process(commod);
    return IRV_TO_PROCESS;
}

void IMOnboardReset(void* arg)
{
    IMOnboard* imonboard = arg;
    imonboard->triggered = false;
}

void IMOnboardReload(void* arg)
{
    IMOnboard* imonboard = arg;
    LoadIMOnboardConfig(imonboard);
}

CONFIG_DESC_DEFINE(GetIMOnboardConfig, "fcitx-onboard.desc")

boolean LoadIMOnboardConfig(IMOnboard* imonboard)
{
    FcitxConfigFileDesc* configDesc = GetIMOnboardConfig();
    if (configDesc == NULL)
        return false;

    FILE *fp;
    fp = FcitxXDGGetFileUserWithPrefix("conf", "fcitx-onboard.config", "r", NULL);
    if (!fp) {
        if (errno == ENOENT)
            SaveIMOnboardConfig(imonboard);
    }

    FcitxConfigFile *cfile = FcitxConfigParseConfigFileFp(fp, configDesc);

    IMOnboardConfigBind(imonboard, cfile, configDesc);
    FcitxConfigBindSync((FcitxGenericConfig*)imonboard);

    if (fp)
        fclose(fp);

    return true;
}

void SaveIMOnboardConfig(IMOnboard* imonboard)
{
    FcitxConfigFileDesc* configDesc = GetIMOnboardConfig();
    FILE *fp = FcitxXDGGetFileUserWithPrefix("conf", "fcitx-onboard.config", "w", NULL);
    FcitxConfigSaveConfigFileFpNdc(fp, &imonboard->gconfig, configDesc);
    if (fp)
        fclose(fp);
}
