/* STM32H743VIT6 (ESC-200 variant): same memory map as ESC-120. */
MEMORY
{
    FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 2048K
    RAM  (xrw) : ORIGIN = 0x24000000, LENGTH = 512K
}
INCLUDE sections.ld
