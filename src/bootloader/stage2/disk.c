#include "disk.h"
#include "x86.h"
#include "stdio.h"

bool DISK_init(DISK* disk, uint8_t drive_number)
{
    uint8_t drive_type;
    uint16_t cylinders, sectors, heads;

    if(!x86_Disk_GetDriveParams(disk->id, &drive_type, &cylinders, &sectors, &heads))
        return false;

    disk->id = drive_number;
    disk->cylinders = cylinders;
    disk->sectors = sectors;
    disk->heads = heads;
    return true;
}

void DISK_LBA2CHS(DISK* disk, uint32_t lba, uint16_t *cylinders, uint16_t* sectors, uint16_t* heads)
{
    *sectors = lba % disk->sectors + 1;
    *cylinders = (lba / disk->sectors) / disk->heads;
    *heads = (lba / disk->sectors) % disk->heads;
}

bool DISK_read_sectors(DISK* disk, uint32_t lba, uint8_t sectors, uint8_t* buf)
{
    uint16_t cylinder, sector, head;

    DISK_LBA2CHS(disk, lba, &cylinder, &sector, &head);

//    printf("Reading from disk lba=%d cylinder=%d sector=%d head=%d\r\n", lba, cylinder, sector, head);

    for (int i = 0; i < 3; i++)
    {
        if(x86_Disk_Read(disk->id, cylinder, sector, head, sectors, buf))
            return true;

        x86_Disk_Reset(disk->id);
    }

    return false;
}
