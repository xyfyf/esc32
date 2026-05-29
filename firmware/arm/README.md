# ARM cross-build glue

This directory holds the **minimum** glue required to cross-compile the
esc32 core into a Cortex-M ARM ELF / BIN / HEX artifact:

| File | Purpose |
|------|---------|
| `startup_cortex_m.c` | Vector table + `Reset_Handler` (zero `.bss`, copy `.data`, call `main`) |
| `syscalls_stub.c`    | Newlib/nano syscall stubs so libc links cleanly |
| `sections.ld`        | Shared `SECTIONS` layout, included by every target |
| `esc60_g431.ld`      | MEMORY map for STM32G431CBU6 (128K F / 32K R) |
| `esc80_g474.ld`      | MEMORY map for STM32G474RET6 (512K F / 128K R) |
| `esc80_at32f415.ld`  | MEMORY map for AT32F415CBT7 (256K F / 32K R) |
| `esc120_h743.ld`     | MEMORY map for STM32H743 (2M F / 512K AXI-SRAM) |
| `esc200_h743.ld`     | Same map as ESC-120 |

The HAL bodies under `firmware/boards/mcu/<family>/` are still **stubs**
(no peripheral init), so the resulting `.elf` / `.bin` / `.hex` will
**boot to a tight loop** on real silicon. The artifacts are useful for:

- **Link verification** — confirms the codebase builds cleanly with
  `arm-none-eabi-gcc` and respects the per-target memory budget.
- **Toolchain smoke test** for hardware engineers — proof that the build
  pipeline produces the file format their flasher expects.
- **CI artifacts** for upcoming releases.

When the production HAL lands (CubeMX-generated TIM/ADC/FDCAN init merged
with `hal_stm32g474.c`), drop in the vendor-supplied `startup_<dev>.s`
and `<dev>_flash.ld`, then wire them through `firmware/Makefile`. The
glue files in this directory can be removed at that point or kept as a
fallback for new MCUs.

> ⚠️ **Disclaimer**: ARM artifacts published from this minimal glue have
> **not** been validated on real hardware. Do not flash them to a board
> you care about until the per-target HAL is filled in and tested on a
> bench rig.
