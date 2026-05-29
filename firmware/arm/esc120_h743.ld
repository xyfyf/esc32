/* STM32H743VIT6: 2 MiB Flash, 1 MiB SRAM (DTCM 128K + AXI/SRAM ~864K).
 * For link verification we use the 512 KiB AXI-SRAM block at 0x24000000. */
MEMORY
{
    FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 2048K
    RAM  (xrw) : ORIGIN = 0x24000000, LENGTH = 512K
}
INCLUDE sections.ld
