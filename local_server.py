import socket
import select
import sys

if len(sys.argv) != 2:
    print(f"Usage: {sys.argv[0]} EVEN_PORT", file=sys.stderr)
    sys.exit(1)

even = int(sys.argv[1])
odd = even + 1

s_even = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s_odd = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

s_even.bind(("0.0.0.0", even))
s_odd.bind(("0.0.0.0", odd))


peer_even = None
peer_odd = None

while True:
    readdata, _, _ = select.select([s_even, s_odd], [], [])
    for s in readdata:
        data, addr = s.recvfrom(65535)
        if s is s_even:
            peer_even = addr
            if peer_odd is not None and data:
                s_odd.sendto(data, peer_odd)
        if s is s_odd:
            peer_odd = addr
            if peer_even is not None and data:
                s_even.sendto(data, peer_even)