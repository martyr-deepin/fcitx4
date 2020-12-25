#include <assert.h>
#include "fcitx-utils/utils.h"

#define TESTFILE "t-rwfile.tmp"

int main()
{
    FILE *fp;

    fp = fopen (TESTFILE, "w+");
    fcitx_utils_write_uint16(fp, 48);
    fclose (fp);
    fp = fopen (TESTFILE, "r+");
    uint16_t ret1 = 0;
    fcitx_utils_read_uint16(fp, &ret1);
    fclose (fp);
    assert(ret1 == 48);

    fp = fopen (TESTFILE, "w+");
    fcitx_utils_write_uint32(fp, 23);
    fclose (fp);
    fp = fopen (TESTFILE, "r+");
    uint32_t ret2 = 0;
    fcitx_utils_read_uint32(fp, &ret2);
    fclose (fp);
    assert(ret2 == 23);

    fp = fopen (TESTFILE, "w+");
    fcitx_utils_write_uint64(fp, 96);
    fclose (fp);
    fp = fopen (TESTFILE, "r+");
    uint32_t ret3 = 0;
    fcitx_utils_read_uint64(fp, &ret3);
    fclose (fp);
    assert(ret3 == 96);

    return 0;
}
