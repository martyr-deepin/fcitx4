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
#include "fcitx/instance.h"
#include <ctype.h>
#include <errno.h>
#include <libintl.h>
#include <limits.h>
#include <stdio.h>

#include "fcitx-config/xdg.h"
#include "fcitx-utils/log.h"
#include "fcitx/candidate.h"
#include "fcitx/fcitx.h"
#include "fcitx/hook.h"
#include "fcitx/keys.h"
#include "fcitx/module.h"
#include <fcitx/context.h>

typedef struct _IMSelector IMSelector;

typedef struct _SelectorHandle {
    int idx;
    IMSelector *imselector;
} SelectorHandle;

struct _IMSelector {
    FcitxGenericConfig gconfig;
    FcitxHotkey selectorKey;
    SelectorHandle handle;
    boolean triggered;
    char *commond;
    FcitxInstance *owner;
};

static void *IMSelectorCreate(FcitxInstance *instance);
static boolean IMSelectorPreFilter(void *arg, FcitxKeySym sym,
                                   unsigned int state,
                                   INPUT_RETURN_VALUE *retval);
static void IMSelectorReset(void *arg);
static void IMSelectorReload(void *arg);
static INPUT_RETURN_VALUE IMSelectorSelect(void *arg);
static boolean LoadIMSelectorConfig(IMSelector *imselector);
static void SaveIMSelectorConfig(IMSelector *imselector);

FCITX_DEFINE_PLUGIN(fcitx_onboard, module, FcitxModule) = {
    IMSelectorCreate, NULL, NULL, NULL, IMSelectorReload};

CONFIG_BINDING_BEGIN(IMSelector)
CONFIG_BINDING_REGISTER("Onboard", "HOTKEY", selectorKey)
CONFIG_BINDING_REGISTER("Onboard", "COMMOND", commond)
CONFIG_BINDING_END()

void *IMSelectorCreate(FcitxInstance *instance) {
    IMSelector *imselector = fcitx_utils_malloc0(sizeof(IMSelector));
    imselector->owner = instance;
    if (!LoadIMSelectorConfig(imselector)) {
        free(imselector);
        return NULL;
    }

    FcitxKeyFilterHook hk;
    hk.arg = imselector;
    hk.func = IMSelectorPreFilter;
    FcitxInstanceRegisterPreInputFilter(instance, hk);

    hk.arg = &imselector->triggered;
    hk.func = FcitxDummyReleaseInputHook;
    FcitxInstanceRegisterPreReleaseInputFilter(instance, hk);

    FcitxHotkeyHook hkhk;
    hkhk.arg = imselector;

    /* this key is ignore the very first input method which is for inactive */

    do {
        SelectorHandle *handle = &imselector->handle;
        handle->idx = 0;
        handle->imselector = imselector;
        hkhk.arg = handle;
        hkhk.hotkeyhandle = IMSelectorSelect;
        hkhk.hotkey = &imselector->selectorKey;
        FcitxInstanceRegisterHotkeyFilter(instance, hkhk);
    } while (0);

    FcitxIMEventHook resethk;
    resethk.arg = imselector;
    resethk.func = IMSelectorReset;
    FcitxInstanceRegisterResetInputHook(instance, resethk);
    return imselector;
}

boolean IMSelectorPreFilter(void *arg, FcitxKeySym sym, unsigned int state,
                            INPUT_RETURN_VALUE *retval) {
    IMSelector *imselector = arg;
    FcitxInstance *instance = imselector->owner;
    FcitxInputState *input = FcitxInstanceGetInputState(instance);
    FcitxGlobalConfig *fc = FcitxInstanceGetGlobalConfig(instance);
    if (!imselector->triggered)
        return false;
    FcitxCandidateWordList *candList = FcitxInputStateGetCandidateList(input);
    if (FcitxHotkeyIsHotKey(sym, state, FcitxConfigPrevPageKey(instance, fc))) {
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

INPUT_RETURN_VALUE IMSelectorSelect(void *arg) {
    SelectorHandle *handle = arg;
    IMSelector *imselector = handle->imselector;
    char *commod[] = {imselector->commond, NULL};
    fcitx_utils_start_process(commod);
    return IRV_TO_PROCESS;
}

void IMSelectorReset(void *arg) {
    IMSelector *imselector = arg;
    imselector->triggered = false;
}

void IMSelectorReload(void *arg) {
    IMSelector *imselector = arg;
    LoadIMSelectorConfig(imselector);
}

CONFIG_DESC_DEFINE(GetIMSelectorConfig, "fcitx-onboard.desc")

boolean LoadIMSelectorConfig(IMSelector *imselector) {
    FcitxConfigFileDesc *configDesc = GetIMSelectorConfig();
    if (configDesc == NULL)
        return false;

    FILE *fp;
    fp = FcitxXDGGetFileUserWithPrefix("conf", "fcitx-onboard.config", "r",
                                       NULL);
    if (!fp) {
        if (errno == ENOENT)
            SaveIMSelectorConfig(imselector);
    }

    FcitxConfigFile *cfile = FcitxConfigParseConfigFileFp(fp, configDesc);

    IMSelectorConfigBind(imselector, cfile, configDesc);
    FcitxConfigBindSync((FcitxGenericConfig *)imselector);

    if (fp)
        fclose(fp);

    return true;
}

void SaveIMSelectorConfig(IMSelector *imselector) {
    FcitxConfigFileDesc *configDesc = GetIMSelectorConfig();
    FILE *fp = FcitxXDGGetFileUserWithPrefix("conf", "fcitx-onboard.config",
                                             "w", NULL);
    FcitxConfigSaveConfigFileFpNdc(fp, &imselector->gconfig, configDesc);
    if (fp)
        fclose(fp);
}
