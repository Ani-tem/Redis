"""Minimal client for the server's binary protocol.

Usage: python3 client.py set name anirudh
"""
import socket
import struct
import sys


def recv_exact(sock, n):
    buf = b""
    while len(buf) < n:
        chunk = sock.recv(n - len(buf))
        if not chunk:
            raise ConnectionError("server closed the connection")
        buf += chunk
    return buf


def decode(b):
    """Decode one tagged value; return (value, remaining bytes)."""
    tag, b = b[0], b[1:]
    if tag == 0:  # nil
        return None, b
    if tag == 1:  # error: code + message
        code, n = struct.unpack("<II", b[:8])
        return ("ERR", code, b[8:8 + n].decode()), b[8 + n:]
    if tag == 2:  # string
        (n,) = struct.unpack("<I", b[:4])
        return b[4:4 + n].decode(), b[4 + n:]
    if tag == 3:  # int64
        return struct.unpack("<q", b[:8])[0], b[8:]
    if tag == 4:  # double
        return struct.unpack("<d", b[:8])[0], b[8:]
    if tag == 5:  # array
        (n,) = struct.unpack("<I", b[:4])
        b, out = b[4:], []
        for _ in range(n):
            value, b = decode(b)
            out.append(value)
        return out, b
    raise ValueError(f"unknown tag {tag}")


def call(sock, *args):
    body = struct.pack("<I", len(args))
    for arg in args:
        data = arg.encode()
        body += struct.pack("<I", len(data)) + data
    sock.sendall(struct.pack("<I", len(body)) + body)
    (size,) = struct.unpack("<I", recv_exact(sock, 4))
    return decode(recv_exact(sock, size))[0]


if __name__ == "__main__":
    with socket.create_connection(("127.0.0.1", 1234)) as s:
        print(call(s, *sys.argv[1:]))
