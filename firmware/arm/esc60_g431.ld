/* STM32G431CBU6: 128 KiB Flash, 32 KiB SRAM (CCM excluded for portability). */
MEMORY
{
    FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 128K
    RAM  (xrw) : ORIGIN = 0x20000000, LENGTH = 32K
}
INCLUDE sections.ld
