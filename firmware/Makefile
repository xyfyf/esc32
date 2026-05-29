# esc32 firmware 鈥?绯诲垪鍖栨瀯寤?
# ESC_TARGET   瑙?boards/targets/锛堥粯璁?ESC_SIM锛?
# PRODUCT_ID   0x60 / 0x80 / 0x120

MAKE ?= make
CC = gcc
PRODUCT_ID ?= 0x80

CFLAGS_BASE = -std=gnu11 -Wall -Wextra -O2 -g
CFLAGS_BASE += -DESC_PRODUCT_ID=$(PRODUCT_ID) -DESC_BOARD_ID=$(PRODUCT_ID)

INCLUDES = -Iinclude -I. -Iboards -Iboards/targets -Iapp -Icore -Ifoc -Imotor_ctrl \
	-Iprotection -Iparams -Icomm -Icomm/uavcan -Ibootloader -Iplatform -I../shared/protocol

CORE_SRCS = main.c boards/product.c boards/target_meta.c boards/mcu_catalog.c \
	platform/mcu_port.c \
	app/app.c core/fault.c core/state.c core/scheduler.c core/fault_log.c \
	foc/math.c foc/foc.c foc/observer.c motor_ctrl/motor.c motor_ctrl/motor_beep.c \
	protection/protect.c params/params.c comm/pwm.c comm/pwm_in.c comm/comm.c \
	comm/dronecan.c comm/uavcan/uavcan.c comm/uavcan/uavcan_crc.c \
	comm/uavcan/uavcan_dsdl.c bootloader/boot.c ../shared/protocol/protocol.c

LDFLAGS = -lm
# Winsock needs to be linked when building for Windows, either natively or
# via mingw cross-compile from Linux (used by the GitHub Actions release job).
NEED_WINSOCK :=
ifeq ($(OS),Windows_NT)
  NEED_WINSOCK := 1
endif
ifneq (,$(findstring mingw,$(CC)))
  NEED_WINSOCK := 1
endif
ifeq ($(NEED_WINSOCK),1)
  LDFLAGS += -lws2_32
endif

ARM_CC      ?= arm-none-eabi-gcc
ARM_OBJCOPY ?= arm-none-eabi-objcopy
ARM_SIZE    ?= arm-none-eabi-size

# Common Cortex-M flags. Per-target rules append the right -mcpu/-mfpu.
ARM_CFLAGS  = -std=gnu11 -Wall -Wextra -Os -g -ffunction-sections -fdata-sections
ARM_CFLAGS += -DESC_PRODUCT_ID=$(PRODUCT_ID) -DESC_BOARD_ID=$(PRODUCT_ID)
ARM_LDFLAGS  = -nostartfiles -Wl,--gc-sections -specs=nano.specs -specs=nosys.specs \
               -Wl,-Map=$(@:.elf=.map) -Larm
ARM_LDLIBS   = -Wl,--start-group -lm -lc_nano -lgcc -Wl,--end-group
ARM_GLUE    = arm/startup_cortex_m.c arm/syscalls_stub.c

ARM_M4F  = -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
ARM_M4   = -mcpu=cortex-m4 -mthumb -mfloat-abi=soft
ARM_M7F  = -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard

.PHONY: all sim esc60 esc80 esc120 esc200 target60 target80 target415 target120 target200 \
        arm-target60 arm-target80 arm-target415 arm-target120 arm-target200 arm-verify \
        clean list-targets verify

all: sim

list-targets:
	@echo ESC_SIM
	@echo ESC60_STM32G431_V1
	@echo ESC80_STM32G474_V1
	@echo ESC80_AT32F415_V1
	@echo ESC120_STM32H743_V1
	@echo ESC200_STM32H743_V1

# --- ESC_SIM锛圥C 浠跨湡锛?--
sim: PRODUCT_ID=0x80
sim: esc32_sim.exe

esc60: PRODUCT_ID=0x60
esc60: esc32_sim.exe

esc80: PRODUCT_ID=0x80
esc80: esc32_sim.exe

esc120: PRODUCT_ID=0x120
esc120: esc32_sim.exe

esc200: PRODUCT_ID=0x200
esc200: esc32_sim.exe

esc32_sim.exe: $(CORE_SRCS) boards/mcu/sim/hal_sim.c
	$(CC) $(CFLAGS_BASE) -DESC_TARGET_ESC_SIM=1 -DESC_PLATFORM_SIM=1 \
		$(INCLUDES) $(CORE_SRCS) boards/mcu/sim/hal_sim.c -o $@ $(LDFLAGS)

# --- 纭欢 Target锛堝崰浣?HAL锛屼緵閾炬帴楠岃瘉锛?--
target80: PRODUCT_ID=0x80
target80: esc32_ESC80_STM32G474_V1.elf

HAL_G474 ?= boards/mcu/stm32g474/hal_stub.c

esc32_ESC80_STM32G474_V1.elf: $(CORE_SRCS) $(HAL_G474)
	$(CC) $(CFLAGS_BASE) -DESC_TARGET_ESC80_STM32G474_V1=1 \
		$(INCLUDES) $(CORE_SRCS) $(HAL_G474) -o $@ $(LDFLAGS)

target120: PRODUCT_ID=0x120
target120: esc32_ESC120_STM32H743_V1.elf

esc32_ESC120_STM32H743_V1.elf: $(CORE_SRCS) boards/mcu/stm32h743/hal_stub.c
	$(CC) $(CFLAGS_BASE) -DESC_TARGET_ESC120_STM32H743_V1=1 \
		$(INCLUDES) $(CORE_SRCS) boards/mcu/stm32h743/hal_stub.c -o $@ $(LDFLAGS)

target80-hal: HAL_G474=boards/mcu/stm32g474/hal_stm32g474.c
target80-hal: target80

target60: PRODUCT_ID=0x60
target60: esc32_ESC60_STM32G431_V1.elf

esc32_ESC60_STM32G431_V1.elf: $(CORE_SRCS) boards/mcu/stm32g431/hal_stub.c
	$(CC) $(CFLAGS_BASE) -DESC_TARGET_ESC60_STM32G431_V1=1 \
		$(INCLUDES) $(CORE_SRCS) boards/mcu/stm32g431/hal_stub.c -o $@ $(LDFLAGS)

target415: PRODUCT_ID=0x80
target415: esc32_ESC80_AT32F415_V1.elf

esc32_ESC80_AT32F415_V1.elf: $(CORE_SRCS) boards/mcu/at32f415/hal_stub.c
	$(CC) $(CFLAGS_BASE) -DESC_TARGET_ESC80_AT32F415_V1=1 \
		$(INCLUDES) $(CORE_SRCS) boards/mcu/at32f415/hal_stub.c -o $@ $(LDFLAGS)

target200: PRODUCT_ID=0x200
target200: esc32_ESC200_STM32H743_V1.elf

esc32_ESC200_STM32H743_V1.elf: $(CORE_SRCS) boards/mcu/stm32h743/hal_stub.c
	$(CC) $(CFLAGS_BASE) -DESC_TARGET_ESC200_STM32H743_V1=1 \
		$(INCLUDES) $(CORE_SRCS) boards/mcu/stm32h743/hal_stub.c -o $@ $(LDFLAGS)

verify: esc32_sim.exe esc32_ESC60_STM32G431_V1.elf esc32_ESC80_STM32G474_V1.elf \
	esc32_ESC80_AT32F415_V1.elf esc32_ESC120_STM32H743_V1.elf esc32_ESC200_STM32H743_V1.elf
	@echo All firmware targets built OK

# --- ARM cross-compile (real Cortex-M ELF/BIN/HEX) -------------------
# These artifacts are link-verified only; the HAL bodies are still stubs,
# so the firmware boots into Default_Handler on real silicon. Useful for
# CI artifacts, toolchain smoke tests, and downstream flasher pipelines.

arm-target80: PRODUCT_ID=0x80
arm-target80: esc32_ESC80_STM32G474_V1.arm.elf esc32_ESC80_STM32G474_V1.bin esc32_ESC80_STM32G474_V1.hex

esc32_ESC80_STM32G474_V1.arm.elf: $(CORE_SRCS) boards/mcu/stm32g474/hal_stub.c $(ARM_GLUE) arm/sections.ld arm/esc80_g474.ld
	$(ARM_CC) $(ARM_CFLAGS) $(ARM_M4F) -DESC_TARGET_ESC80_STM32G474_V1=1 \
		$(INCLUDES) $(CORE_SRCS) boards/mcu/stm32g474/hal_stub.c $(ARM_GLUE) \
		$(ARM_LDFLAGS) -Tarm/esc80_g474.ld -o $@ $(ARM_LDLIBS)
	$(ARM_SIZE) $@

arm-target60: PRODUCT_ID=0x60
arm-target60: esc32_ESC60_STM32G431_V1.arm.elf esc32_ESC60_STM32G431_V1.bin esc32_ESC60_STM32G431_V1.hex

esc32_ESC60_STM32G431_V1.arm.elf: $(CORE_SRCS) boards/mcu/stm32g431/hal_stub.c $(ARM_GLUE) arm/sections.ld arm/esc60_g431.ld
	$(ARM_CC) $(ARM_CFLAGS) $(ARM_M4F) -DESC_TARGET_ESC60_STM32G431_V1=1 \
		$(INCLUDES) $(CORE_SRCS) boards/mcu/stm32g431/hal_stub.c $(ARM_GLUE) \
		$(ARM_LDFLAGS) -Tarm/esc60_g431.ld -o $@ $(ARM_LDLIBS)
	$(ARM_SIZE) $@

arm-target415: PRODUCT_ID=0x80
arm-target415: esc32_ESC80_AT32F415_V1.arm.elf esc32_ESC80_AT32F415_V1.bin esc32_ESC80_AT32F415_V1.hex

esc32_ESC80_AT32F415_V1.arm.elf: $(CORE_SRCS) boards/mcu/at32f415/hal_stub.c $(ARM_GLUE) arm/sections.ld arm/esc80_at32f415.ld
	$(ARM_CC) $(ARM_CFLAGS) $(ARM_M4) -DESC_TARGET_ESC80_AT32F415_V1=1 \
		$(INCLUDES) $(CORE_SRCS) boards/mcu/at32f415/hal_stub.c $(ARM_GLUE) \
		$(ARM_LDFLAGS) -Tarm/esc80_at32f415.ld -o $@ $(ARM_LDLIBS)
	$(ARM_SIZE) $@

arm-target120: PRODUCT_ID=0x120
arm-target120: esc32_ESC120_STM32H743_V1.arm.elf esc32_ESC120_STM32H743_V1.bin esc32_ESC120_STM32H743_V1.hex

esc32_ESC120_STM32H743_V1.arm.elf: $(CORE_SRCS) boards/mcu/stm32h743/hal_stub.c $(ARM_GLUE) arm/sections.ld arm/esc120_h743.ld
	$(ARM_CC) $(ARM_CFLAGS) $(ARM_M7F) -DESC_TARGET_ESC120_STM32H743_V1=1 \
		$(INCLUDES) $(CORE_SRCS) boards/mcu/stm32h743/hal_stub.c $(ARM_GLUE) \
		$(ARM_LDFLAGS) -Tarm/esc120_h743.ld -o $@ $(ARM_LDLIBS)
	$(ARM_SIZE) $@

arm-target200: PRODUCT_ID=0x200
arm-target200: esc32_ESC200_STM32H743_V1.arm.elf esc32_ESC200_STM32H743_V1.bin esc32_ESC200_STM32H743_V1.hex

esc32_ESC200_STM32H743_V1.arm.elf: $(CORE_SRCS) boards/mcu/stm32h743/hal_stub.c $(ARM_GLUE) arm/sections.ld arm/esc200_h743.ld
	$(ARM_CC) $(ARM_CFLAGS) $(ARM_M7F) -DESC_TARGET_ESC200_STM32H743_V1=1 \
		$(INCLUDES) $(CORE_SRCS) boards/mcu/stm32h743/hal_stub.c $(ARM_GLUE) \
		$(ARM_LDFLAGS) -Tarm/esc200_h743.ld -o $@ $(ARM_LDLIBS)
	$(ARM_SIZE) $@

# Generic .bin / .hex from any .arm.elf
%.bin: %.arm.elf
	$(ARM_OBJCOPY) -O binary $< $@

%.hex: %.arm.elf
	$(ARM_OBJCOPY) -O ihex $< $@

arm-verify: arm-target60 arm-target80 arm-target415 arm-target120 arm-target200
	@echo All ARM firmware artifacts built OK

clean:
ifeq ($(OS),Windows_NT)
	-del /Q esc32_sim.exe esc32_*.elf esc32_*.arm.elf esc32_*.bin esc32_*.hex esc32_*.map esc32_nvm.bin 2>nul
else
	-$(RM) esc32_sim.exe esc32_*.elf esc32_*.arm.elf esc32_*.bin esc32_*.hex esc32_*.map esc32_nvm.bin
endif
