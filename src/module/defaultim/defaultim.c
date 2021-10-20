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
#include <ctype.h>
#include <errno.h>
#include <libintl.h>
#include <limits.h>
#include <stdio.h>

#include "fcitx-config/xdg.h"
#include "fcitx-utils/log.h"
#include "fcitx/candidate.h"
#include "fcitx/context.h"
#include "fcitx/fcitx.h"
#include "fcitx/hook.h"
#include "fcitx/instance.h"
#include "fcitx/keys.h"
#include "fcitx/module.h"

typedef struct _DefaultIM DefaultIM;

typedef struct _SelectorHandle {
    int idx;
    DefaultIM *defaultim;
} SelectorHandle;

struct _DefaultIM {
    FcitxGenericConfig gconfig;
    FcitxHotkey selectorKey;
    SelectorHandle handle;
    boolean triggered;
    char *imname;
    FcitxInstance *owner;
};

static void *DefaultIMCreate(FcitxInstance *instance);
static boolean DefaultIMPreFilter(void *arg, FcitxKeySym sym,
                                  unsigned int state,
                                  INPUT_RETURN_VALUE *retval);
static void DefaultIMReset(void *arg);
static void DefaultIMReload(void *arg);
static INPUT_RETURN_VALUE DefaultIMSelect(void *arg);
static boolean LoadDefaultIMConfig(DefaultIM *defaultim);
static void SaveDefaultIMConfig(DefaultIM *defaultim);

FCITX_DEFINE_PLUGIN(fcitx_defaultim, module, FcitxModule) = {
    DefaultIMCreate, NULL, NULL, NULL, DefaultIMReload};

CONFIG_BINDING_BEGIN(DefaultIM)
CONFIG_BINDING_REGISTER("DefaultIM", "HOTKEY", selectorKey)
CONFIG_BINDING_REGISTER("DefaultIM", "IMNAME", imname)
CONFIG_BINDING_END()

void *DefaultIMCreate(FcitxInstance *instance) {
    DefaultIM *defaultim = fcitx_utils_malloc0(sizeof(DefaultIM));
    defaultim->owner = instance;
    if (!LoadDefaultIMConfig(defaultim)) {
        free(defaultim);
        return NULL;
    }

    FcitxKeyFilterHook hk;
    hk.arg = defaultim;
    hk.func = DefaultIMPreFilter;
    FcitxInstanceRegisterPreInputFilter(instance, hk);

    hk.arg = &defaultim->triggered;
    hk.func = FcitxDummyReleaseInputHook;
    FcitxInstanceRegisterPreReleaseInputFilter(instance, hk);

    FcitxHotkeyHook hkhk;
    hkhk.arg = defaultim;

    /* this key is ignore the very first input method which is for inactive */
    SelectorHandle *handle = &defaultim->handle;
    handle->idx = 0;
    handle->defaultim = defaultim;
    hkhk.arg = handle;
    hkhk.hotkeyhandle = DefaultIMSelect;
    hkhk.hotkey = &defaultim->selectorKey;
    FcitxInstanceRegisterHotkeyFilter(instance, hkhk);

    FcitxIMEventHook resethk;
    resethk.arg = defaultim;
    resethk.func = DefaultIMReset;
    FcitxInstanceRegisterResetInputHook(instance, resethk);
    return defaultim;
}

boolean DefaultIMPreFilter(void *arg, FcitxKeySym sym, unsigned int state,
                           INPUT_RETURN_VALUE *retval) {
    DefaultIM *defaultim = arg;
    if (!defaultim->triggered)
        return false;
    FcitxInstance *instance = defaultim->owner;
    FcitxInputState *input = FcitxInstanceGetInputState(instance);
    FcitxGlobalConfig *fc = FcitxInstanceGetGlobalConfig(instance);
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

INPUT_RETURN_VALUE DefaultIMSelect(void *arg) {
    SelectorHandle *handle = arg;
    DefaultIM *defaultim = handle->defaultim;
    FcitxInstance *instance = defaultim->owner;
    handle->idx = FcitxInstanceGetIMIndexByName(instance, defaultim->imname);

    if (handle->idx == -1) {
        handle->idx = 0;
    }
    FcitxIM *im = FcitxInstanceGetIMByIndex(instance, handle->idx);
    if (!im)
        return IRV_TO_PROCESS;
    FcitxInstanceSwitchIMByIndex(instance, handle->idx);
    return IRV_CLEAN;
}

void DefaultIMReset(void *arg) {
    DefaultIM *defaultim = arg;
    defaultim->triggered = false;
}

void DefaultIMReload(void *arg) {
    DefaultIM *defaultim = arg;
    LoadDefaultIMConfig(defaultim);
}

CONFIG_DESC_DEFINE(GetDefaultIMConfig, "fcitx-defaultim.desc")

boolean LoadDefaultIMConfig(DefaultIM *defaultim) {
    FcitxConfigFileDesc *configDesc = GetDefaultIMConfig();
    if (configDesc == NULL)
        return false;

    FILE *fp;
    fp = FcitxXDGGetFileUserWithPrefix("conf", "fcitx-defaultim.config", "r",
                                       NULL);
    if (!fp) {
        if (errno == ENOENT)
            SaveDefaultIMConfig(defaultim);
    }

    FcitxConfigFile *cfile = FcitxConfigParseConfigFileFp(fp, configDesc);

    DefaultIMConfigBind(defaultim, cfile, configDesc);
    FcitxConfigBindSync((FcitxGenericConfig *)defaultim);

    if (fp)
        fclose(fp);

    return true;
}

void SaveDefaultIMConfig(DefaultIM *defaultim) {
    FcitxConfigFileDesc *configDesc = GetDefaultIMConfig();
    FILE *fp = FcitxXDGGetFileUserWithPrefix("conf", "fcitx-defaultim.config",
                                             "w", NULL);
    FcitxConfigSaveConfigFileFp(fp, &defaultim->gconfig, configDesc);
    if (fp)
        fclose(fp);
}
