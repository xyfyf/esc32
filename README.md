# esc32

**Open-source FOC ESC firmware for agricultural / industrial multirotors**
面向农业/工业多旋翼的 **开源 FOC 无刷电调** 固件 · 上位机 · 硬件参考设计

[![License](https://img.shields.io/badge/license-MIT-blue)](#license)
[![Firmware](https://img.shields.io/badge/firmware-C11-brightgreen)]()
[![Host](https://img.shields.io/badge/host-Python%203.10%2B%20%2F%20PyQt6-blue)]()
[![MCUs](https://img.shields.io/badge/MCUs-STM32%20G4%20%2F%20H7%20%E2%80%A2%20AT32-orange)]()
[![Docs](https://img.shields.io/badge/docs-CN%20%2F%20EN-success)](docs/esc32-完整文档.md)

> 中文 README · [English documentation](docs/esc32-full-document-en.md)

---

## 简介

`esc32` 是面向 **农业植保 / 工业吊运** 多旋翼无人机的开源 FOC 矢量电调工程。
工程从一开始就走 **"先软后硬"** 路线：在 PC 上提供完整的仿真闭环（仿真固件 +
上位机 + 端到端测试），无需 PCB 即可调通协议、参数与算法；硬件采用 **Product /
MCU / Target 三层解耦架构**，参考 VESC 的 `hw_*` 思路，单仓库支持多款 MCU、
多个功率档板卡共用同一份核心代码。

| 能力 | esc32 |
|------|-------|
| 控制方式 | 无感 FOC（PLL / SMO 观测器）+ 重载电流环 |
| 油门输入 | PWM 1050–1950 µs · DroneCAN RawCommand · 上位机调试通道 |
| 输出协议 | UAVCAN v0 DSDL（Status / NodeStatus / GetNodeInfo）+ 私有调试协议（UART/UDP） |
| 保护链 | 过/欠压 · 过流 · 过温 · 堵转 · 油门丢失 · 故障黑匣子（64 条） |
| 提示音 | 上电完整旋律 · 首次联机旋律 · 信号丢失"滴滴"报警 |
| 工具链 | PC 仿真器 + PyQt6 上位机 + 电机识别向导 + JSON 预设 + Bootloader/OTA |
| 仓库形态 | 一仓多 Target（ESC-60/80/120/200 × G431/G474/H743/AT32F415） |

> 设计目标是给小批量、专业化的农业/工业整机厂提供 **可二次开发、可量产、可商业闭源**
> 的 ESC 方案，自研 FOC 与协议栈、不依赖 GPL 固件拷贝。

---

## ⚠️ 免责声明 / Disclaimer

`esc32` 是开源项目，固件与上位机仅用于 **学习、研发与功能验证**。
将本固件烧录到任何带电机或大功率电池的硬件之前，请确保：

- 已了解三相 FOC 控制与高压锂电池的安全规范；
- 第一次上电请使用 **限流电源 / 假电池**，电机无桨；
- 请务必先在 PC 仿真模式（`esc32_sim.exe`）下完成参数与协议验证。

由于错误使用导致的电机、电池、机架或人身伤害，作者与贡献者 **不承担任何责任**。

---

## 支持的目标 / Supported Targets

| Target | 产品系列 | MCU | 状态 | 备注 |
|---|---|---|---|---|
| `ESC_SIM` | — | PC (host) | ✅ Ready | UDP 仿真，调试协议 + DroneCAN |
| `ESC80_STM32G474_V1` | ESC-80 | STM32G474RET6 | 🟡 HAL skeleton | 首版生产硬件，HAL 骨架已链接通过 |
| `ESC60_STM32G431_V1` | ESC-60 | STM32G431CBU6 | 🟡 Stub | G4 同系，优先移植，引脚已规划 |
| `ESC120_STM32H743_V1` | ESC-120 | STM32H743 | 🟡 Stub | 大功率档，引脚待原理图定稿 |
| `ESC200_STM32H743_V1` | ESC-200 | STM32H743 | 🟡 Stub | 重载档，35 kg 级单轴 |
| `ESC80_AT32F415_V1` | ESC-80 (cost) | AT32F415CBT7 | 🟡 Stub | 成本档替代，独立 HAL |

> ✅ Ready = 完整可运行 · 🟡 Stub = 链接通过 / 等真机外设填充 · ⏳ Planned = 规划中
> 详细 MCU 能力对比见 [docs/esc32-完整文档.md](docs/esc32-完整文档.md) 中
> "MCU移植与多平台架构" 一章。

---

## 主要特性 / Features

### 控制 & 算法
- 无感 FOC：Clarke/Park/SVPWM、电流环 20 kHz、速度环 1 kHz
- 双观测器：PLL（默认）/ SMO（可切换），三段式启动（对齐 → 开环 → 切换）
- 弱磁、加减速限流、空载检测、过零保护

### 通信
- **UAVCAN v0 DSDL 栈**（DroneCAN 兼容子集）
  - `equipment.esc.RawCommand` (1030)、`equipment.esc.Status` (1034 / 1035)
  - `protocol.NodeStatus` (341)、`protocol.GetNodeInfo` (1) 服务响应
- **私有调试协议**（同步头 `0xEC 0x32`，UART 或 UDP 承载）
- **不**实现双向 DShot（BDShot），转速/状态走 UAVCAN

### 工具与生态
- `esc32_sim.exe`：PC 仿真固件，UDP 7777（调试）+ 7779（DroneCAN）
- `esc_tool`：CLI + PyQt6 GUI + 电机识别向导 + 实时遥测曲线
- 一键 E2E：`scripts/run-closed-loop.ps1` / `scripts/verify-all.ps1`
- 多功率档 JSON 预设：`shared/defaults/{60,80,120,200}.json`
- Bootloader + OTA：擦除 / 写入 / CRC 校验 / 重启（仿真已贯通）

### 量产支撑
- 故障黑匣子：64 条，掉电保留（NVM）
- 标定流程文档、台架验收清单、IPX6 灌封工艺
- 协议层带 `feature_flags` / `port_status`，上位机可识别 stub vs ready

---

## 快速开始 / Quick Start

### 1. 一键搭建编译环境（仅 Windows，首次）
```powershell
powershell -ExecutionPolicy Bypass -File scripts\setup-build-env.ps1
. .\scripts\env.ps1
```
脚本自动安装 / 注册 MinGW、CMake、ARM GCC、Python venv。

### 2. 编译并运行仿真
```powershell
cd firmware
mingw32-make
.\esc32_sim.exe              # UDP 7777 / 7779 监听
```

### 3. 启动上位机 GUI
```powershell
cd ..\host
.\.venv\Scripts\python -m esc_tool --gui
```

### 4. 一键端到端验证
```powershell
powershell -File scripts\run-closed-loop.ps1     # 编译 + 仿真 + E2E
powershell -File scripts\verify-all.ps1          # 上面 + 5 个硬件 Target 链接
```

### 5. 双击发布包（给非开发用户）
```powershell
powershell -File scripts\build-release.ps1       # 生成 dist\esc32\
# dist\esc32\esc32_start.exe   一键启动仿真 + GUI
```

---

## 系列化构建 / Multi-Target Build

`Product × MCU × Target` 三层解耦，详见 [docs](docs/esc32-完整文档.md) 中
"命名与系列规范" 一章。

```powershell
cd firmware
mingw32-make list-targets
mingw32-make esc80         # 仿真 + 产品 0x80
mingw32-make esc200        # 仿真 + 产品 0x200
mingw32-make target80      # G474 stub HAL 链接验证
mingw32-make target80-hal  # G474 生产 HAL 骨架
mingw32-make target60      # ESC-60 + G431
mingw32-make target415     # ESC-80 + AT32F415
mingw32-make target200     # ESC-200 + H743
mingw32-make verify        # 一次性链接全部硬件 Target
```

---

## 硬件 / Hardware

首版参考硬件采用 **STM32G474RET6 + DRV8323 + 三电阻采样 + INA240** 拓扑，
工作电压 6S–12S，连续电流随产品档位 60 / 80 / 120 / 200 A。

- 原理图说明：`docs/esc32-完整文档.md` → "ESC-80硬件原理图"
- 引脚映射：`firmware/boards/mcu/stm32g474/pinmap.h`
- BOM：`docs/esc32-完整文档.md` → "BOM-ESC-80"
- IPX6 灌封工艺：同上文档 → "IPX6结构与灌封工艺"

> **没有自研 PCB？** 推荐起手用 `NUCLEO-G474RE + X-NUCLEO-IHM08M1` 套件，
> 或独立的 `B-G431B-ESC1` 评估板（一周内可点亮电机）。

---

## 配置 / Configuration

参数通过 GUI / CLI 在线读写，断电保存到 NVM（仿真为文件，硬件为 Flash）。
四档默认值见 `shared/defaults/{60,80,120,200}.json`：

| 文件 | 适用场景 |
|------|---------|
| `60.json`  | 5–8 kg 单轴轻载 |
| `80.json`  | 17 kg 级农业植保（默认） |
| `120.json` | 25 kg+ 重载吊运 |
| `200.json` | 35 kg 级特重载 |

GUI 内提供 **电机识别向导**（KV / 极对数 / R-L 估算）与 **JSON 预设一键加载**。

---

## 协议 / Protocols

| 协议 | 承载 | 文档 |
|------|------|------|
| esc32 调试协议 | UART 115200 / UDP 7777 | `shared/protocol/protocol.h` |
| UAVCAN v0 / DroneCAN | CAN 1 Mbps / 仿真 UDP 7779 | `firmware/comm/uavcan/` |

调试协议帧格式：
```
[0xEC][0x32][CMD][LEN_L][LEN_H][PAYLOAD...][CRC16_L][CRC16_H]
```

---

## 路线图 / Roadmap

- ✅ **P0** 仿真 + 调试协议 + 参数 + GUI
- ✅ **P1** 保护状态机 + 故障黑匣子 + Bootloader/OTA（仿真）
- ✅ **P2** 多 Target 架构 + 电机提示音 + JSON 预设
- ✅ **P3** UAVCAN DSDL 栈 + NodeStatus / GetNodeInfo + 多 MCU HAL 骨架
- ⏳ **P4** ESC-80 G474 真机点亮 + 实测调参（待硬件打样）
- ⏳ **P5** Cyphal (UAVCAN v1) 可选支持 + ESC-120/200 大功率验证

完整路线图见 `docs/esc32-完整文档.md` 中 "ROADMAP" 一章。

---

## 文档 / Documentation

| 文档 | 内容 |
|------|------|
| [docs/esc32-完整文档.md](docs/esc32-完整文档.md) | 中文合订本（28 章 · 2369 行） |
| [docs/esc32-full-document-en.md](docs/esc32-full-document-en.md) | English edition (28 chapters · 2372 lines) |

文档涵盖：项目特点、可行性分析、系统闭环、命名规范、MCU 移植、UAVCAN 协议栈、
生产/标定/IPX6 工艺、ESC-60/80/120 硬件概要、各 MCU README 等。

重新合并文档（仅当源 `.md` 重建后）：
```bash
python scripts\merge-md.py
```

---

## 贡献 / Contributing

欢迎提 Issue / PR：

- 新增 MCU Target：在 `firmware/boards/mcu/<family>/` 建 `mcu_conf.h` + `pinmap.h` + `hal_stub.c`，再到 `firmware/boards/targets/<TARGET>/` 建 `target.h`，在 `firmware/Makefile` 加一条 `targetXXX` 规则。
- 新增产品功率档：`firmware/include/product.h` 加 `ESC_PRODUCT_*`，`shared/defaults/<id>.json` 写默认值。
- 协议扩展：保持 `0xEC 0x32` 同步头与 CRC16，向后兼容旧 `proto_version`。

代码风格：内核遵循 K&R + 4 空格；Python 遵循 PEP 8；新增源文件请加文件级 doxygen 注释。

---

## 许可 / License

MIT License。详见 `LICENSE`（如未单独提供，则以仓库根目录最新版本为准）。

> esc32 自研 FOC 与协议栈，**不**包含 BLHeli / AM32 / VESC 等 GPL 项目的任何代码片段，
> 因此可在商业闭源整机中再分发。

---

## 致谢 / Acknowledgements

- ST 微电子：STM32 G4 / H7 系列 MCU 与 STEVAL/NUCLEO 评估板
- DroneCAN / OpenCyphal 社区：DSDL 规范
- VESC Project (Benjamin Vedder)：多硬件目录架构思路（仅参考组织方式，不复用 GPL 代码）
- 国内农业无人机生态对开源工具链的持续推动
