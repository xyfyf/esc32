"""Serial / UDP transports."""
from __future__ import annotations

import socket
from typing import Protocol


class Transport(Protocol):
    def write(self, data: bytes) -> int: ...
    def read(self, max_size: int = 256) -> bytes: ...


class UdpTransport:
    def __init__(self, host: str, port: int, local_port: int = 0) -> None:
        """local_port=0 lets the OS assign a port and avoids WinError 10048 when multiple GUIs run."""
        self._addr = (host, port)
        self._sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self._sock.bind(("", local_port))
        self._sock.settimeout(0.3)

    def write(self, data: bytes) -> int:
        return self._sock.sendto(data, self._addr)

    def read(self, max_size: int = 256) -> bytes:
        try:
            data, _ = self._sock.recvfrom(max_size)
            return data
        except socket.timeout:
            return b""

    def close(self) -> None:
        self._sock.close()


class SerialTransport:
    def __init__(self, port: str, baud: int = 115200) -> None:
        import serial

        self._ser = serial.Serial(port, baud, timeout=0.3)

    def write(self, data: bytes) -> int:
        return self._ser.write(data)

    def read(self, max_size: int = 256) -> bytes:
        return self._ser.read(max_size)

    def close(self) -> None:
        self._ser.close()
