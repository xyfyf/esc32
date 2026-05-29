# esc32

农业/工业多旋翼 **FOC 无刷电调**（固件 + 上位机 + 硬件文档）。

**全部文档已合并为一册** → **[docs/esc32-完整文档.md](docs/esc32-完整文档.md)**（约 2300+ 行，含快速开始、架构、硬件、协议、路线图等）。

## 快速开始

```powershell
powershell -File scripts\setup-build-env.ps1
. .\scripts\env.ps1
cd firmware && mingw32-make && .\esc32_sim.exe
```

一键验证：`powershell -File scripts\verify-all.ps1`  
重新合并文档：`python scripts\merge-md.py`
