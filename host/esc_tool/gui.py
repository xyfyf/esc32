"""PyQt6 上位机：参数 / 遥测曲线 / 电机向导 / 批量配置 / 固件升级"""
from __future__ import annotations

import struct
import sys
import zlib
from pathlib import Path

from PyQt6.QtCore import QTimer
from PyQt6.QtWidgets import (
    QApplication,
    QComboBox,
    QDoubleSpinBox,
    QFileDialog,
    QGridLayout,
    QHBoxLayout,
    QLabel,
    QLineEdit,
    QMainWindow,
    QMessageBox,
    QPushButton,
    QSpinBox,
    QTabWidget,
    QTableWidget,
    QTableWidgetItem,
    QVBoxLayout,
    QWidget,
)

import pyqtgraph as pg

from .client import EscClient
from .motor_wizard import MotorWizardWidget
from .preset_apply import apply_preset
from .transport import SerialTransport, UdpTransport

def _defaults_dir() -> Path:
    if getattr(sys, "frozen", False):
        d = Path(sys.executable).resolve().parent / "defaults"
        if d.is_dir():
            return d
    return Path(__file__).resolve().parents[2] / "shared" / "defaults"


class MainWindow(QMainWindow):
    def __init__(self) -> None:
        super().__init__()
        self.setWindowTitle("esc_tool")
        self.resize(1000, 700)
        self._client: EscClient | None = None
        self._telem_history: dict[str, list] = {
            "rpm": [],
            "ibus": [],
            "vbus": [],
        }

        root = QWidget()
        self.setCentralWidget(root)
        layout = QVBoxLayout(root)

        conn = QHBoxLayout()
        self._conn_type = QComboBox()
        self._conn_type.addItems(["UDP 仿真", "串口"])
        self._conn_addr = QLineEdit("127.0.0.1:7777")
        btn_connect = QPushButton("连接")
        btn_connect.clicked.connect(self._on_connect)
        conn.addWidget(QLabel("连接:"))
        conn.addWidget(self._conn_type)
        conn.addWidget(self._conn_addr, 1)
        conn.addWidget(btn_connect)
        layout.addLayout(conn)

        self._tabs = QTabWidget()
        layout.addWidget(self._tabs, 1)

        self._build_telem_tab()
        self._build_params_tab()
        self._build_fault_tab()
        self._build_batch_tab()
        self._build_fw_tab()
        self._tabs.addTab(MotorWizardWidget(self._get_client), "电机向导")

        ctrl = QHBoxLayout()
        btn_arm = QPushButton("解锁")
        btn_disarm = QPushButton("上锁")
        self._thr_spin = QSpinBox()
        self._thr_spin.setRange(1000, 2000)
        self._thr_spin.setValue(1000)
        btn_thr = QPushButton("设油门")
        btn_arm.clicked.connect(lambda: self._safe(lambda c: c.arm()))
        btn_disarm.clicked.connect(lambda: self._safe(lambda c: c.disarm()))
        btn_thr.clicked.connect(
            lambda: self._safe(lambda c: c.set_throttle_us(self._thr_spin.value()))
        )
        ctrl.addWidget(btn_arm)
        ctrl.addWidget(btn_disarm)
        ctrl.addWidget(QLabel("PWM us"))
        ctrl.addWidget(self._thr_spin)
        ctrl.addWidget(btn_thr)
        ctrl.addStretch()
        layout.addLayout(ctrl)

        self._timer = QTimer()
        self._timer.timeout.connect(self._poll_telem)
        self._timer.setInterval(100)

    def _get_client(self) -> EscClient | None:
        return self._client

    def _build_telem_tab(self) -> None:
        w = QWidget()
        v = QVBoxLayout(w)
        self._plot = pg.PlotWidget(title="遥测")
        self._plot.addLegend()
        self._curve_rpm = self._plot.plot(pen="y", name="RPM")
        self._curve_i = self._plot.plot(pen="c", name="I(mA)")
        self._curve_v = self._plot.plot(pen="g", name="V(mV)")
        v.addWidget(self._plot)
        self._lbl_telem = QLabel("未连接")
        v.addWidget(self._lbl_telem)
        self._tabs.addTab(w, "实时曲线")

    def _build_params_tab(self) -> None:
        w = QWidget()
        v = QVBoxLayout(w)
        self._param_table = QTableWidget(0, 3)
        self._param_table.setHorizontalHeaderLabels(["名称", "值", "操作"])
        row = QHBoxLayout()
        btn_refresh = QPushButton("刷新参数")
        btn_refresh.clicked.connect(self._refresh_params)
        btn_defaults = QPushButton("恢复出厂默认")
        btn_defaults.clicked.connect(self._load_factory_defaults)
        btn_preset = QPushButton("加载预设 JSON…")
        btn_preset.clicked.connect(self._load_preset_file)
        self._preset_combo = QComboBox()
        for n in ("60", "80", "120", "200"):
            p = _defaults_dir() / f"{n}.json"
            if p.is_file():
                self._preset_combo.addItem(f"{n}.json", str(p))
        btn_preset_quick = QPushButton("应用档位预设")
        btn_preset_quick.clicked.connect(self._load_preset_combo)
        row.addWidget(btn_refresh)
        row.addWidget(btn_defaults)
        row.addWidget(btn_preset)
        row.addWidget(self._preset_combo)
        row.addWidget(btn_preset_quick)
        row.addStretch()
        btn_save = QPushButton("保存到 Flash")
        btn_save.clicked.connect(lambda: self._safe(lambda c: c.save_params()))
        v.addLayout(row)
        v.addWidget(self._param_table, 1)
        v.addWidget(btn_save)
        self._tabs.addTab(w, "参数")

    def _build_fault_tab(self) -> None:
        w = QWidget()
        v = QVBoxLayout(w)
        self._fault_table = QTableWidget(0, 6)
        self._fault_table.setHorizontalHeaderLabels(
            ["时间", "故障", "V", "I", "RPM", "油门"]
        )
        btn = QPushButton("读取故障记录")
        btn.clicked.connect(self._refresh_faults)
        v.addWidget(btn)
        v.addWidget(self._fault_table, 1)
        self._tabs.addTab(w, "故障黑匣子")

    def _build_batch_tab(self) -> None:
        w = QWidget()
        g = QGridLayout(w)
        self._batch_node = QSpinBox()
        self._batch_node.setRange(1, 127)
        self._batch_kv = QSpinBox()
        self._batch_kv.setRange(100, 3000)
        self._batch_kv.setValue(170)
        btn = QPushButton("应用批量预设到当前 ESC")
        btn.clicked.connect(self._batch_apply)
        g.addWidget(QLabel("node_id"), 0, 0)
        g.addWidget(self._batch_node, 0, 1)
        g.addWidget(QLabel("motor_kv"), 1, 0)
        g.addWidget(self._batch_kv, 1, 1)
        g.addWidget(btn, 2, 0, 1, 2)
        self._tabs.addTab(w, "批量配置")

    def _build_fw_tab(self) -> None:
        w = QWidget()
        v = QVBoxLayout(w)
        btn_sel = QPushButton("选择固件文件…")
        self._fw_path = QLineEdit()
        btn_erase = QPushButton("1. 擦除")
        btn_write = QPushButton("2. 写入")
        btn_crc = QPushButton("3. 校验并提交")
        btn_reboot = QPushButton("4. 重启")
        btn_sel.clicked.connect(self._pick_fw)
        btn_erase.clicked.connect(lambda: self._safe(lambda c: c.fw_erase()))
        btn_write.clicked.connect(self._fw_write)
        btn_crc.clicked.connect(self._fw_crc)
        btn_reboot.clicked.connect(lambda: self._safe(lambda c: c.fw_reboot()))
        v.addWidget(btn_sel)
        v.addWidget(self._fw_path)
        v.addWidget(btn_erase)
        v.addWidget(btn_write)
        v.addWidget(btn_crc)
        v.addWidget(btn_reboot)
        self._tabs.addTab(w, "固件 OTA")

    def _on_connect(self) -> None:
        try:
            addr = self._conn_addr.text().strip()
            if self._conn_type.currentIndex() == 0:
                host, _, port = addr.partition(":")
                tp = UdpTransport(host, int(port or "7777"))
            else:
                tp = SerialTransport(addr, 115200)
            self._client = EscClient(tp)
            ver = self._client.ping()
            info = self._client.get_info()
            self._lbl_telem.setText(
                f"已连接 {info.name} FW={info.fw_version >> 8}.{info.fw_version & 0xFF} "
                f"产品=0x{info.product_id:04X} Target=0x{info.target_id:04X} "
                f"MCU=0x{info.mcu_id:02X} proto={ver}"
            )
            self._timer.start()
            self._refresh_params()
        except Exception as e:
            QMessageBox.critical(self, "连接失败", str(e))

    def _safe(self, fn) -> None:
        if not self._client:
            QMessageBox.warning(self, "提示", "请先连接")
            return
        try:
            fn(self._client)
        except Exception as e:
            QMessageBox.critical(self, "错误", str(e))

    def _poll_telem(self) -> None:
        if not self._client:
            return
        try:
            t = self._client.get_telem()
            for key, val in (
                ("rpm", t.rpm),
                ("ibus", t.ibus_ma),
                ("vbus", t.vbus_mv),
            ):
                h = self._telem_history[key]
                h.append(val)
                if len(h) > 300:
                    del h[0]
            x = list(range(len(self._telem_history["rpm"])))
            self._curve_rpm.setData(x, self._telem_history["rpm"])
            self._curve_i.setData(x, self._telem_history["ibus"])
            self._curve_v.setData(x, self._telem_history["vbus"])
            self._lbl_telem.setText(
                f"状态={t.state} 故障={t.fault_code} V={t.vbus_mv/1000:.1f}V "
                f"I={t.ibus_ma}mA RPM={t.rpm} 油门={t.throttle_us}us"
            )
        except Exception:
            pass

    def _load_factory_defaults(self) -> None:
        def do(c: EscClient) -> None:
            c.load_defaults()
            self._refresh_params()
            QMessageBox.information(self, "完成", "已恢复固件出厂默认参数（未写入 Flash，需点「保存到 Flash」持久化）")

        self._safe(do)

    def _load_preset_file(self) -> None:
        if not self._client:
            QMessageBox.warning(self, "提示", "请先连接")
            return
        p, _ = QFileDialog.getOpenFileName(
            self,
            "选择预设",
            str(_defaults_dir()),
            "JSON (*.json)",
        )
        if not p:
            return
        self._apply_preset_path(Path(p))

    def _load_preset_combo(self) -> None:
        path = self._preset_combo.currentData()
        if not path:
            return
        self._apply_preset_path(Path(path))

    def _apply_preset_path(self, path: Path) -> None:
        import json

        def do(c: EscClient) -> None:
            preset = json.loads(path.read_text(encoding="utf-8"))
            apply_preset(c, preset)
            self._refresh_params()
            name = preset.get("config_name", path.name)
            QMessageBox.information(self, "完成", f"已加载预设：{name}\n并已保存到仿真 NVM")

        self._safe(do)

    def _refresh_params(self) -> None:
        if not self._client:
            return
        rows: list[tuple[str, float]] = []
        for i in range(128):
            try:
                rows.append(self._client.list_param(i))
            except TimeoutError:
                break
        self._param_table.setRowCount(len(rows))
        for r, (name, val) in enumerate(rows):
            self._param_table.setItem(r, 0, QTableWidgetItem(name))
            spin = QDoubleSpinBox()
            spin.setDecimals(4)
            spin.setRange(-1e6, 1e6)
            spin.setValue(float(val))
            self._param_table.setCellWidget(r, 1, spin)
            btn = QPushButton("写入")

            def make_write(n: str, s: QDoubleSpinBox) -> None:
                def do_write() -> None:
                    if self._client:
                        self._client.set_param(n, s.value())

                return do_write

            btn.clicked.connect(make_write(name, spin))
            self._param_table.setCellWidget(r, 2, btn)

    def _refresh_faults(self) -> None:
        self._safe(self._do_refresh_faults)

    def _do_refresh_faults(self, c: EscClient) -> None:
        entries = c.get_fault_log(0, 16)
        self._fault_table.setRowCount(len(entries))
        for i, e in enumerate(entries):
            self._fault_table.setItem(i, 0, QTableWidgetItem(str(e.timestamp_ms)))
            self._fault_table.setItem(i, 1, QTableWidgetItem(str(e.code)))
            self._fault_table.setItem(i, 2, QTableWidgetItem(str(e.vbus_mv)))
            self._fault_table.setItem(i, 3, QTableWidgetItem(str(e.ibus_ma)))
            self._fault_table.setItem(i, 4, QTableWidgetItem(str(e.rpm)))
            self._fault_table.setItem(i, 5, QTableWidgetItem(str(e.throttle_us)))

    def _batch_apply(self) -> None:
        def apply(c: EscClient):
            c.set_param("node_id", float(self._batch_node.value()))
            c.set_param("motor_kv", float(self._batch_kv.value()))
            c.save_params()

        self._safe(apply)

    def _pick_fw(self) -> None:
        p, _ = QFileDialog.getOpenFileName(self, "固件", "", "Bin (*.bin);;All (*)")
        if p:
            self._fw_path.setText(p)

    def _fw_write(self) -> None:
        path = self._fw_path.text()
        if not path:
            return

        def do(c: EscClient):
            data = Path(path).read_bytes()
            chunk = 256
            for off in range(0, len(data), chunk):
                c.fw_write(off, data[off : off + chunk])

        self._safe(do)

    def _fw_crc(self) -> None:
        path = self._fw_path.text()
        if not path:
            return

        def do(c: EscClient):
            data = Path(path).read_bytes()
            crc = zlib.crc32(data) & 0xFFFFFFFF
            c.fw_crc(crc)

        self._safe(do)


def run_gui() -> None:
    app = QApplication(sys.argv)
    win = MainWindow()
    win.show()
    sys.exit(app.exec())


if __name__ == "__main__":
    run_gui()
