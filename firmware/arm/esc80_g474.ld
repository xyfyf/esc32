/* STM32G474RET6: 512 KiB Flash, 128 KiB SRAM (CCM excluded for portability). */
MEMORY
{
    FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 512K
    RAM  (xrw) : ORIGIN = 0x20000000, LENGTH = 128K
}
INCLUDE sections.ld
