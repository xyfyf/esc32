# esc32 firmware — 系列化构建
# ESC_TARGET   见 boards/targets/（默认 ESC_SIM）
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
ifeq ($(OS),Windows_NT)
LDFLAGS += -lws2_32
endif

.PHONY: all sim esc60 esc80 esc120 esc200 target60 target80 target415 target120 target200 clean list-targets verify

all: sim

list-targets:
	@echo ESC_SIM
	@echo ESC60_STM32G431_V1
	@echo ESC80_STM32G474_V1
	@echo ESC80_AT32F415_V1
	@echo ESC120_STM32H743_V1
	@echo ESC200_STM32H743_V1

# --- ESC_SIM（PC 仿真）---
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

# --- 硬件 Target（占位 HAL，供链接验证）---
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

clean:
ifeq ($(OS),Windows_NT)
	-del /Q esc32_sim.exe esc32_*.elf esc32_nvm.bin 2>nul
else
	-$(RM) esc32_sim.exe esc32_*.elf esc32_nvm.bin
endif
