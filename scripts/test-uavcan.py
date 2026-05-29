#!/usr/bin/env python3
"""Send a DroneCAN RawCommand to esc32_sim over UDP :7779."""
from __future__ import annotations

import socket
import struct
import sys
import time

CAN_PORT = 7779
RAWCOMMAND_ID = 1030
NODE_ID = 30
PRIORITY = 24


def make_msg_id(priority: int, data_type_id: int, source: int) -> int:
    return (priority << 24) | (data_type_id << 8) | source


def send_frame(sock: socket.socket, can_id: int, data: bytes) -> None:
    pkt = struct.pack("<I", can_id) + bytes([len(data)]) + data
    sock.sendto(pkt, ("127.0.0.1", CAN_PORT))


def main() -> int:
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(0.5)
    cid = make_msg_id(PRIORITY, RAWCOMMAND_ID, 127)
    payload = bytes([1, 0])  # 1 ESC, throttle center
    tail = 0xC0  # start+end, tid=0
    send_frame(sock, cid, payload + bytes([tail]))
    print("Sent RawCommand to 127.0.0.1:7779")
    try:
        for _ in range(5):
            data, _ = sock.recvfrom(256)
            if len(data) >= 5:
                rid = struct.unpack_from("<I", data, 0)[0]
                dlen = data[4]
                print(f"  RX id=0x{rid:08X} len={dlen} data={data[5:5+dlen].hex()}")
    except socket.timeout:
        print("  (no RX — ensure esc32_sim.exe is running)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
