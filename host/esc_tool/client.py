"""调试协议客户端"""
from __future__ import annotations

import struct
import time
from dataclasses import dataclass

from .protocol import Cmd, FrameParser, pack_frame
from .transport import Transport


@dataclass
class EscInfo:
    proto_version: int
    mcu_id: int
    product_id: int
    fw_version: int
    target_id: int
    hw_revision: int
    name: str

    @property
    def board_id(self) -> int:
        """兼容旧字段名"""
        return self.product_id


@dataclass
class EscTelem:
    uptime_ms: int
    state: int
    fault_code: int
    vbus_mv: int
    ibus_ma: int
    temp_mos_c10: int
    rpm: int
    iq_ma: int
    throttle_us: int


@dataclass
class FaultLogEntry:
    timestamp_ms: int
    code: int
    vbus_mv: int
    ibus_ma: int
    temp_c10: int
    rpm: int
    throttle_us: int
    state: int


class EscClient:
    def __init__(self, transport: Transport) -> None:
        self._tp = transport
        self._parser = FrameParser()

    def _exchange(self, cmd: int, payload: bytes = b"", timeout: float = 1.0) -> bytes:
        self._tp.write(pack_frame(cmd, payload))
        deadline = time.time() + timeout
        while time.time() < deadline:
            chunk = self._tp.read()
            for rsp_cmd, rsp_pl in self._parser.feed(chunk):
                if rsp_cmd == cmd:
                    return rsp_pl
            time.sleep(0.01)
        raise TimeoutError(f"No response for cmd 0x{cmd:02X}")

    def ping(self) -> int:
        return self._exchange(Cmd.PING)[0]

    def get_info(self) -> EscInfo:
        pl = self._exchange(Cmd.GET_INFO)
        if len(pl) >= 42:
            proto, mcu, product, fw, target, hw = struct.unpack_from("<BBHHHH", pl, 0)
            name = pl[12:28].split(b"\x00")[0].decode(errors="replace")
            return EscInfo(proto, mcu, product, fw, target, hw, name)
        if len(pl) >= 38:
            proto, mcu, product, fw, target, hw = struct.unpack_from("<BBHHHH", pl, 0)
            name = pl[12:28].split(b"\x00")[0].decode(errors="replace")
            return EscInfo(proto, mcu, product, fw, target, hw, name)
        proto, _, board, fw, hw = struct.unpack_from("<BBHHH", pl, 0)
        name = pl[8:24].split(b"\x00")[0].decode(errors="replace")
        return EscInfo(proto, 0, board, fw, 0, hw, name)

    def get_telem(self) -> EscTelem:
        pl = self._exchange(Cmd.GET_TELEM)
        n = struct.calcsize("<IBBHhhihhHB")
        if len(pl) < n:
            pl = pl + bytes(n - len(pl))
        (
            uptime,
            state,
            fault,
            vbus,
            ibus,
            temp,
            rpm,
            _id_ma,
            iq_ma,
            thr,
            _src,
        ) = struct.unpack("<IBBHhhihhHB", pl[:n])
        return EscTelem(uptime, state, fault, vbus, ibus, temp, rpm, iq_ma, thr)

    def list_param(self, index: int) -> tuple[str, float]:
        pl = self._exchange(Cmd.GET_PARAM, struct.pack("<H", index))
        if len(pl) < 6:
            raise ValueError(f"GET_PARAM short response: {len(pl)} bytes")
        val = struct.unpack_from("<f", pl, 2)[0]
        name = pl[6 : min(len(pl), 38)].split(b"\x00")[0].decode()
        return name, val

    def set_param(self, name: str, value: float) -> bool:
        for i in range(64):
            try:
                n, _ = self.list_param(i)
            except TimeoutError:
                break
            if n == name:
                if float(value) == int(value):
                    pl = struct.pack("<HH", i, int(value) & 0xFFFF)
                else:
                    pl = struct.pack("<Hf", i, float(value))
                return self._exchange(Cmd.SET_PARAM, pl)[0] == 0
        pl = bytearray(36)
        nb = name.encode()[:31]
        pl[0 : len(nb)] = nb
        struct.pack_into("<f", pl, 32, float(value))
        return self._exchange(Cmd.SET_PARAM, bytes(pl))[0] == 0

    def load_defaults(self) -> None:
        self._exchange(Cmd.LOAD_DEFAULTS)

    def save_params(self) -> None:
        self._exchange(Cmd.SAVE_PARAMS)

    def arm(self) -> None:
        self._exchange(Cmd.ARM)

    def disarm(self) -> None:
        self._exchange(Cmd.DISARM)

    def set_throttle_us(self, us: int) -> None:
        self._exchange(Cmd.SET_THROTTLE, struct.pack("<H", us))

    def get_fault_log(self, offset: int = 0, count: int = 8) -> list[FaultLogEntry]:
        pl = self._exchange(Cmd.GET_FAULT_LOG, struct.pack("<HB", offset, count))
        total, returned = struct.unpack_from("<HH", pl, 0)
        _ = total
        entries: list[FaultLogEntry] = []
        for i in range(returned):
            o = 4 + i * 18
            if o + 18 > len(pl):
                break
            ts, code, vbus, ibus, temp, rpm, thr, st = struct.unpack_from(
                "<IBhhihHB", pl, o
            )
            entries.append(FaultLogEntry(ts, code, vbus, ibus, temp, rpm, thr, st))
        return entries

    def fw_erase(self) -> None:
        self._exchange(Cmd.FW_ERASE, timeout=3.0)

    def fw_write(self, offset: int, data: bytes) -> None:
        payload = struct.pack("<I", offset) + data
        self._exchange(Cmd.FW_WRITE, payload, timeout=3.0)

    def fw_crc(self, crc32: int) -> None:
        self._exchange(Cmd.FW_CRC, struct.pack("<I", crc32), timeout=5.0)

    def fw_reboot(self) -> None:
        self._exchange(Cmd.FW_REBOOT, timeout=1.0)
