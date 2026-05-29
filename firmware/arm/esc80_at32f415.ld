/* AT32F415CBT7: 256 KiB Flash, 32 KiB SRAM (assumes -CB suffix). */
MEMORY
{
    FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 256K
    RAM  (xrw) : ORIGIN = 0x20000000, LENGTH = 32K
}
INCLUDE sections.ld
