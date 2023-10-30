#include <stat/stat.h>

bool stat_is_dir(int mode)
{
    return (mode & STAT_IFDIR) == STAT_IFDIR;
}

bool stat_is_reg(int mode)
{
    return (mode & STAT_IFREG) == STAT_IFREG;
}

bool stat_is_lnk(int mode)
{
    return (mode & STAT_IFLNK) == STAT_IFLNK;
}
