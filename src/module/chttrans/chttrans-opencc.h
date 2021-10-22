#ifndef CHTTRANS_OPENCC_H
#define CHTTRANS_OPENCC_H

#include "chttrans_p.h"
#include "fcitx-config/fcitx-config.h"

boolean OpenCCInit(FcitxChttrans *transState);
char *OpenCCConvert(void *od, const char *str, size_t size);

#endif // CHTTRANS_OPENCC.H
