#include "STM32Bootloader.h"
#include <cstdio>

int main()
{
    STM32Bootloader stm32bootloader;
    if( stm32bootloader.Open("COM3") != 0 )
    {
        printf("Open COM3 failed!\n");
        return -1;
    }
    stm32_t *stm = stm32bootloader.GetStm32Info();
    printf("Version      : 0x%02x\n", stm->bl_version);
	printf("Device ID    : 0x%04x (%s)\n", stm->pid, stm->dev->name);
	printf("- RAM        : Up to %dKiB  (%db reserved by bootloader)\n", (stm->dev->ram_end - 0x20000000) / 1024, stm->dev->ram_start - 0x20000000);
	printf("- Flash      : Up to %dKiB (size first sector: %dx%d)\n", (stm->dev->fl_end - stm->dev->fl_start ) / 1024, stm->dev->fl_pps, stm->dev->fl_ps[0]);
	printf("- Option bytes  : %db\n", stm->dev->opt_end - stm->dev->opt_start + 1);
	printf("- System memory : %dKiB\n", (stm->dev->mem_end - stm->dev->mem_start) / 1024);

    printf("EraseMemory = %d\n", stm32bootloader.EraseMemory());
    printf("WriteMemory = %d\n", stm32bootloader.WriteMemory(0x08000000, "./Project.bin", 0));
    printf("ReadMemory = %d\n", stm32bootloader.ReadMemory(0x08000000, 0x1000, "test2.bin"));
    
    getchar();
    return 0;
}