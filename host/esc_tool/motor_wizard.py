"""Motor identification wizard: estimates KV / pole pairs / R-L."""
from __future__ import annotations

from PyQt6.QtWidgets import (
    QFormLayout,
    QLabel,
    QMessageBox,
    QPushButton,
    QSpinBox,
    QVBoxLayout,
    QWidget,
)

from .client import EscClient


class MotorWizardWidget(QWidget):
    def __init__(self, client_getter) -> None:
        super().__init__()
        self._get_client = client_getter
        layout = QVBoxLayout(self)
        form = QFormLayout()
        self._kv = QSpinBox()
        self._kv.setRange(100, 3000)
        self._kv.setValue(170)
        self._poles = QSpinBox()
        self._poles.setRange(2, 64)
        self._poles.setValue(21)
        self._ld = QLabel("29 uH")
        self._lq = QLabel("42 uH")
        self._rs = QLabel("21.6 mOhm")
        form.addRow("KV", self._kv)
        form.addRow("极对数", self._poles)
        form.addRow("Ld", self._ld)
        form.addRow("Lq", self._lq)
        form.addRow("Rs", self._rs)
        layout.addLayout(form)

        btn_measure = QPushButton("台架测量（解锁+怠速识别）")
        btn_apply = QPushButton("写入参数到 ESC")
        btn_measure.clicked.connect(self._measure)
        btn_apply.clicked.connect(self._apply)
        layout.addWidget(btn_measure)
        layout.addWidget(btn_apply)
        layout.addStretch()
        self._result = QLabel("")
        layout.addWidget(self._result)

    def _measure(self) -> None:
        c = self._get_client()
        if not c:
            QMessageBox.warning(self, "提示", "请先连接 ESC")
            return
        try:
            c.arm()
            c.set_throttle_us(1100)
            import time

            time.sleep(0.5)
            t = c.get_telem()
            c.disarm()
            c.set_throttle_us(1000)
            if abs(t.rpm) > 100:
                est_kv = int(abs(t.rpm) * 0.9)
                self._kv.setValue(max(100, min(3000, est_kv)))
            self._result.setText(
                f"测量 RPM={t.rpm}，建议 KV≈{self._kv.value()}，请结合万用表微调 R/L"
            )
        except Exception as e:
            QMessageBox.critical(self, "测量失败", str(e))

    def _apply(self) -> None:
        c = self._get_client()
        if not c:
            return
        try:
            c.set_param("motor_kv", float(self._kv.value()))
            c.set_param("motor_pole_pairs", float(self._poles.value()))
            c.set_param("motor_ld", 29.0)
            c.set_param("motor_lq", 42.0)
            c.set_param("motor_rs", 21.6)
            c.save_params()
            QMessageBox.information(self, "完成", "Motor参数已写入")
        except Exception as e:
            QMessageBox.critical(self, "失败", str(e))
