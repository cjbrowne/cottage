#include <resource/resource.h>

uint64_t dev_id_counter = 0;

uint64_t resource_create_dev_id()
{
    return dev_id_counter++;
}

