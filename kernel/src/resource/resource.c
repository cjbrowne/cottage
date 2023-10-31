#include <resource/resource.h>
#include <ioctl/ioctl.h>

uint64_t dev_id_counter = 0;

uint64_t resource_create_dev_id()
{
    return dev_id_counter++;
}

int resource_default_ioctl(__attribute__((unused)) void* handle, __attribute__((unused)) uint64_t request,__attribute__((unused)) void* argp)
{
    switch(request)
    {
        case IOCTL_TCGETS    :
        case IOCTL_TCSETS    :
        case IOCTL_TCSETSW   :
        case IOCTL_TCSETSF   :
        case IOCTL_TIOCSCTTY :
        case IOCTL_TIOCGWINSZ:
        // todo: set errno ENOTTY
        return -1;
        break;
        default:
        // todo: set errno EINVAL
        return -1;
        break;
    }
}
