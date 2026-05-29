"""esc32 debug protocol (mirrors shared/protocol/protocol)."""
from __future__ import annotations

import struct
from enum import IntEnum

SYNC0, SYNC1 = 0xEC, 0x32
HEADER_SIZE = 5
CRC_SIZE = 2
MAX_PAYLOAD = 512


class Cmd(IntEnum):
    PING = 0x01
    GET_INFO = 0x02
    GET_TELEM = 0x03
    GET_PARAM = 0x10
    SET_PARAM = 0x11
    SAVE_PARAMS = 0x12
    LOAD_DEFAULTS = 0x13
    ARM = 0x20
    DISARM = 0x21
    SET_THROTTLE = 0x22
    GET_FAULT_LOG = 0x30
    FW_ERASE = 0xF0
    FW_WRITE = 0xF1
    FW_CRC = 0xF2
    FW_REBOOT = 0xF3


def crc16(data: bytes) -> int:
    crc = 0xFFFF
    for b in data:
        crc ^= b
        for _ in range(8):
            crc = (crc >> 1) ^ 0xA001 if (crc & 1) else (crc >> 1)
    return crc & 0xFFFF


def pack_frame(cmd: int, payload: bytes = b"") -> bytes:
    body = bytes([cmd, len(payload) & 0xFF, (len(payload) >> 8) & 0xFF]) + payload
    c = crc16(body)
    return bytes([SYNC0, SYNC1]) + body + struct.pack("<H", c)


class FrameParser:
    def __init__(self) -> None:
        self._buf = bytearray()
        self._state = 0
        self._cmd = 0
        self._len = 0
        self._crc_lo = 0

    def feed(self, data: bytes) -> list[tuple[int, bytes]]:
        out: list[tuple[int, bytes]] = []
        for byte in data:
            r = self._feed_byte(byte)
            if r is not None:
                out.append(r)
        return out

    def _feed_byte(self, byte: int) -> tuple[int, bytes] | None:
        if self._state == 0:
            self._state = 1 if byte == SYNC0 else 0
        elif self._state == 1:
            self._state = 2 if byte == SYNC1 else (1 if byte == SYNC0 else 0)
        elif self._state == 2:
            self._cmd = byte
            self._state = 3
        elif self._state == 3:
            self._len = byte
            self._state = 4
        elif self._state == 4:
            self._len |= byte << 8
            self._buf = bytearray()
            self._state = 5 if self._len else 6
        elif self._state == 5:
            self._buf.append(byte)
            if len(self._buf) >= self._len:
                self._state = 6
        elif self._state == 6:
            self._crc_lo = byte
            self._state = 7
        elif self._state == 7:
            crc = self._crc_lo | (byte << 8)
            body = bytes([self._cmd, self._len & 0xFF, (self._len >> 8) & 0xFF]) + bytes(self._buf)
            self._state = 0
            if crc16(body) == crc:
                return self._cmd, bytes(self._buf)
        return None
