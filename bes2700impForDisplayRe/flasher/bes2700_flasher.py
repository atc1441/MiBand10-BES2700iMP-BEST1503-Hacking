#!/usr/bin/env python3
"""
BES best1503 (BES2700iMP / MiBand 9 and 10) Firmware flasher by ATC1441 at 20.06.2026
Hardware required:
  - 3.3 V USB-UART adapter (CP2102 / CH340 / FT232)
  - Both UART-TX and UART-RX pins of the MiBand 9 connected
  - Way to power-cycle (disconnect/reconnect charging power)

On any error the script resets and prompts for a power-cycle — no restart needed.
Partial output files are resumed automatically.

Usage:
  python bes1503_read_flash.py --port COM3 --output firmware.bin
  python bes1503_read_flash.py --port COM3 --output boot.bin --address 0x2C000000 --size 0x100000
  python bes1503_read_flash.py --port COM3 --output app.bin  --address 0x2C100000 --size 0x300000
  python bes1503_read_flash.py --port COM3 --output firmware.bin --no-log   # silent
"""

import serial, struct, time, sys, argparse, os, zlib

# ── BESLink constants ──────────────────────────────────────────────────────
BES_SYNC         = 0xBE
MSG_SYNC         = 0x50
MSG_DEVICE_CMD   = 0x00   # DeviceCommand (reboot: payload 00 01 F1)
MSG_START_PROG   = 0x53
MSG_PROG_RUNNING = 0x54
MSG_PROG_START   = 0x55
MSG_PROG_INIT    = 0x60
MSG_FLASH_READ   = 0x03
MSG_ERASE_BURN   = 0x61   # EraseBurnStart (write)
MSG_BURN_DATA    = 0x62   # FlashBurnData  (write, 32 KB chunks)
MSG_FLASH_CMD    = 0x65
MSG_EXTEND_CMD   = 0x83

MSG_NAMES = {
    0x03: "FlashRead",   0x50: "Sync",      0x53: "StartProgrammer",
    0x54: "ProgRunning", 0x55: "ProgStart",  0x60: "ProgInit",
    0x61: "EraseBurnStart", 0x62: "FlashBurnData", 0x65: "FlashCmd",
    0x83: "ExtendCmd",
}

FLASH_BUFFER_SIZE = 0x8000   # 32 KB burn chunk (matches factory tool)

BAUD             = 921600

# ── best1503 / MiBand 9 specific ──────────────────────────────────────────
# EXACT protocol from a real DldProductLine UART capture of THIS MiBand 9
# (working/Uart CMDs.txt). The StartProgrammer (0x53) message is:
#   BE 53 00 0C | 58 08 0A 24 | 18 B9 01 00 | AA 37 5F 26 | 1C
#   = 00 0C | addr=0x240A0858 (file trailer raw[-4:]) | len=0x1B918 (112920) |
#     field3 = CRC32 | checksum.
# field3 is a STANDARD CRC-32 (zlib) over raw[0x61C:-4] — i.e. from code_offset
# 0x61C (1564, incl. the 4 zero bytes before 0x620) up to but excluding the
# 4-byte trailer. CRACKED & verified: crc32(orig[0x61C:-4]) == 0x265F37AA. The
# ROM verifies it; a wrong value gives ProgRunning ACK 0x23 (not 0x20) and the
# programmer won't start. It is auto-computed below, so you can freely EDIT the
# programmer code and it stays valid. (--field3 can still force a value.)
# Transmitted code = raw[0x620:-4] (the 4-byte trailer is NOT sent as data).
PROG_PATH   = os.path.join(os.path.dirname(__file__), r"programmer.bin")
PROG_HDR    = 0x620        # code data starts here; the trailing 4 B are not sent
PROG_CRC_OFF = 0x61C       # CRC32 (field3) is taken from here to -4
FLASH_BASE  = 0x28000000   # XIP cached flash base
FLASH_SIZE  = 0x400000     # 4 MB single die (confirmed: GET_FLASH_SIZE=4194304, FLASH_ID:1=00-00-00)
CHUNK_SIZE  = 0x1000       # 4 KB per FlashRead request

CHUNK_RETRIES   = 10
SESSION_RETRIES = 99

# Boot-ROM advertisement: BE 50 00 03 00 00 01 ED
BES_ROM_ADV = bytes([0xBE, 0x50, 0x00, 0x03, 0x00, 0x00, 0x01, 0xED])


# ── UART logging proxy ─────────────────────────────────────────────────────
class LoggingPort:
    """
    Transparent wrapper around serial.Serial.
    Prints every TX and RX byte as a hex+ASCII dump.
    Set port.bulk = True to suppress raw bulk data (programmer binary,
    flash chunk data) and print a one-line summary instead.
    """

    _COL  = 16    # bytes per hex row
    _ANSI = {
        'TX':    '\033[33m',   # yellow
        'RX':    '\033[36m',   # cyan
        'INFO':  '\033[90m',   # dark grey
        'RESET': '\033[0m',
    }

    def __init__(self, port, enabled: bool = True):
        object.__setattr__(self, '_p',       port)
        object.__setattr__(self, '_enabled', enabled)
        object.__setattr__(self, '_t0',      time.time())
        object.__setattr__(self, 'bulk',     False)   # set True to suppress bulk data
        # ANSI: detect Windows console support
        try:
            import ctypes
            k = ctypes.windll.kernel32
            k.SetConsoleMode(k.GetStdHandle(-11), 7)
            object.__setattr__(self, '_ansi', True)
        except Exception:
            object.__setattr__(self, '_ansi', sys.stdout.isatty())

    # ── attribute pass-through ──────────────────────────────────────────
    def __getattr__(self, name):
        return getattr(object.__getattribute__(self, '_p'), name)

    def __setattr__(self, name, value):
        if name in ('bulk', '_p', '_enabled', '_t0', '_ansi'):
            object.__setattr__(self, name, value)
        else:
            setattr(object.__getattribute__(self, '_p'), name, value)

    # ── serial API ──────────────────────────────────────────────────────
    def read(self, size=1):
        data = object.__getattribute__(self, '_p').read(size)
        if data and not object.__getattribute__(self, 'bulk'):
            self._dump('RX', data)
        return data

    def write(self, data):
        b = bytes(data)
        if not object.__getattribute__(self, 'bulk'):
            self._dump('TX', b)
        return object.__getattribute__(self, '_p').write(b)

    def flush(self):
        return object.__getattribute__(self, '_p').flush()

    def reset_input_buffer(self):
        return object.__getattribute__(self, '_p').reset_input_buffer()

    def reset_output_buffer(self):
        return object.__getattribute__(self, '_p').reset_output_buffer()

    # ── helper to print a bulk summary (called by caller code) ──────────
    def log_bulk(self, direction: str, n_bytes: int, note: str = ''):
        if not object.__getattribute__(self, '_enabled'):
            return
        t   = time.time() - object.__getattribute__(self, '_t0')
        clr = self._color(direction)
        rst = self._color('RESET')
        tag = f"[{direction} bulk {n_bytes} B{(' ' + note) if note else ''}]"
        print(f"  {t:8.3f}  {clr}{direction}{rst}  {tag}")

    # ── internal hex dump ───────────────────────────────────────────────
    def _dump(self, direction: str, data: bytes):
        if not object.__getattribute__(self, '_enabled') or not data:
            return
        t   = time.time() - object.__getattribute__(self, '_t0')
        col = self._COL
        clr = self._color(direction)
        rst = self._color('RESET')

        # Annotate first byte if it looks like a BES packet start
        annotation = ''
        if len(data) >= 2 and data[0] == BES_SYNC:
            mtype = data[1]
            name  = MSG_NAMES.get(mtype, f'0x{mtype:02X}')
            annotation = f'  ← {name}'
        elif len(data) == 1 and data[0] == BES_SYNC:
            annotation = '  ← BES_SYNC'

        for i in range(0, len(data), col):
            row     = data[i:i + col]
            hex_s   = ' '.join(f'{b:02X}' for b in row)
            asc_s   = ''.join(chr(b) if 32 <= b < 127 else '.' for b in row)
            if i == 0:
                ann = annotation
                ts  = f"{t:8.3f}"
                lbl = f"{clr}{direction}{rst}"
            else:
                ann = ''
                ts  = ' ' * 8
                lbl = '  '
            print(f"  {ts}  {lbl}  {hex_s:<{col*3-1}}  {asc_s}{ann}")

    def _color(self, key: str) -> str:
        if object.__getattribute__(self, '_ansi'):
            return self._ANSI.get(key, '')
        return ''


# ── Protocol helpers ───────────────────────────────────────────────────────
def checksum(data: bytes) -> int:
    return (0xFF - sum(data)) & 0xFF

def make_packet(msg_type: int, payload: bytes) -> bytes:
    body = bytes([BES_SYNC, msg_type]) + payload
    return body + bytes([checksum(body)])

def read_packet(port, expect_type: int, timeout: float = 3.0) -> bytes:
    """
    Read one BESLink packet.
    Format: [0xBE][type][p0][len_indicator][payload...][checksum]
    Total length = 5 + packet[3]
    Returns payload bytes (between type byte and checksum).
    """
    port.timeout = timeout
    for _ in range(10000):
        b = port.read(1)
        if not b:
            raise TimeoutError(f"Timeout waiting for 0x{expect_type:02X} packet")
        if b[0] == BES_SYNC:
            break
    else:
        raise TimeoutError("Could not find BES_SYNC byte")

    hdr = port.read(3)
    if len(hdr) < 3:
        raise TimeoutError("Incomplete packet header")

    total_len = 5 + hdr[2]
    remaining = total_len - 4
    rest = port.read(remaining)
    if len(rest) < remaining:
        raise TimeoutError(f"Short packet: got {len(rest)}, need {remaining}")

    full = bytes([BES_SYNC]) + hdr + rest
    got_cs = full[-1]
    exp_cs = checksum(full[:-1])
    if got_cs != exp_cs:
        print(f"  [!] Checksum mismatch: got {got_cs:02X}, expected {exp_cs:02X}")

    if hdr[0] != expect_type:
        print(f"  [!] Type mismatch: got 0x{hdr[0]:02X}, expected 0x{expect_type:02X}")

    return full[2:-1]


# ── Phase 1: Boot-ROM sync ─────────────────────────────────────────────────
def sync_bootrom(port) -> None:
    """
    Synchronise with the BES boot ROM.

    Boot ROM sequence:
      1. Chip power-on → ROM asserts UART BREAK on TX (= null bytes)
      2. ROM sends advertisement: BE 50 00 03 00 00 01 ED
      3. PC responds with sync ACK:   BE 50 00 01 01 EF
      4. ROM confirms:               BE 50 00 03 02 00 01 EB

    We send sync packets continuously so the ROM enters download mode
    as soon as it boots, AND we accumulate all incoming bytes into a
    growing buffer so the advertisement is never lost.
    """
    SYNC_PKT = make_packet(MSG_SYNC, bytes([0x00, 0x01, 0x01]))
    ADV_HDR  = bytes([BES_SYNC, MSG_SYNC])   # BE 50

    while True:
        print("[1] Sending sync — power-cycle MiBand NOW "
              "(disconnect + reconnect charger)...")
        # Give the OS serial driver time to fully initialise before power-cycle
        time.sleep(0.5)
        port.reset_input_buffer()
        port.reset_output_buffer()
        print("    (port ready — power-cycle now if you haven't yet)")

        buf      = b''
        deadline = time.time() + 20.0
        last_tx  = 0.0
        break_sent = False

        while time.time() < deadline:
            # ── Send BREAK once at start (triggers ROM advertisement) ──
            if not break_sent and time.time() - (deadline - 20.0) > 0.3:
                try:
                    port._p.send_break(duration=0.1)
                    print("    [BREAK sent]")
                except Exception:
                    pass
                break_sent = True

            # ── Send sync every 50 ms ──────────────────────────────────
            now = time.time()
            if now - last_tx >= 0.05:
                port.timeout = 0
                port.write(SYNC_PKT)
                port.flush()
                last_tx = now

            # ── Read whatever arrived, accumulate ──────────────────────
            port.timeout = 0.01
            d = port._p.read(256)
            if d:
                if object.__getattribute__(port, '_enabled'):
                    port._dump('RX', d)
                buf += d

            # ── Strip leading UART BREAK null bytes (do NOT clear buf) ─
            buf = buf.lstrip(b'\x00')

            # ── Search accumulated buffer for BE 50 ────────────────────
            idx = buf.find(ADV_HDR)
            if idx < 0:
                # Prevent buf growing unboundedly if only garbage arrives
                if len(buf) > 256:
                    buf = buf[-32:]
                continue

            buf = buf[idx:]   # discard anything before BE 50

            # Make sure we have all 8 bytes of the advertisement
            while len(buf) < 8:
                port.timeout = 0.2
                d = port._p.read(8 - len(buf))
                if not d:
                    break
                if object.__getattribute__(port, '_enabled'):
                    port._dump('RX', d)
                buf += d

            adv = buf[:8]
            print(f"  ROM advertisement: {adv.hex()}")

            # ── ACK ────────────────────────────────────────────────────
            time.sleep(0.02)
            port.reset_input_buffer()
            port.write(SYNC_PKT)
            port.flush()

            # ── Read confirm (BE 50 00 03 02 ...) — optional ───────────
            confirm = b''
            t_end   = time.time() + 1.0
            while time.time() < t_end and len(confirm) < 16:
                port.timeout = 0.1
                d = port._p.read(16)
                if d:
                    if object.__getattribute__(port, '_enabled'):
                        port._dump('RX', d)
                    confirm += d
                    if ADV_HDR in confirm:
                        break
            if confirm:
                print(f"  Confirm: {confirm.hex()}")
            print("  Sync complete")
            return

        print("  No ROM advertisement after 20 s.")
        print("  Possible causes: wrong port, voltage mismatch (1.8V?), wrong pin")
        print("  Power-cycle MiBand and try again...\n")
        time.sleep(1)


# ── Phase 2: Upload programmer1503.bin ─────────────────────────────────────
def load_programmer(port) -> None:
    if not os.path.exists(PROG_PATH):
        raise FileNotFoundError(f"Programmer not found:\n  {PROG_PATH}")

    with open(PROG_PATH, 'rb') as f:
        raw = f.read()

    trailer  = raw[-4:]
    code     = raw[PROG_HDR:-4]          # raw[0x620:-4] = 112916 B transmitted
    code_len = len(code)

    # ── StartProgrammer (0x53), exactly as captured ──
    #   00 0C | addr=trailer(0x240A0858) | len=code_len+4 (112920) | field3 CRC
    code_info_addr = int.from_bytes(trailer, 'little')   # file trailer = 0x240A0858
    info_len       = code_len + 4                         # captured len = 112920
    # field3 = CRC-32 (zlib) over raw[0x61C:-4]; auto-computed so EDITED
    # programmer binaries stay valid (no manual checksum needed).
    field3 = struct.pack('<I', zlib.crc32(raw[PROG_CRC_OFF:-4]) & 0xFFFFFFFF)

    print(f"[2] Loading {os.path.basename(PROG_PATH)} "
          f"({code_len} B, addr=0x{code_info_addr:08X} len={info_len} "
          f"crc32={field3[::-1].hex()})")

    sp_payload = (bytes([0x00, 0x0C]) +
                  struct.pack('<I', code_info_addr) +
                  struct.pack('<I', info_len) +
                  field3)
    port.write(make_packet(MSG_START_PROG, sp_payload))
    port.flush()

    resp = read_packet(port, MSG_START_PROG, timeout=3)
    if resp and resp[0] != 0x00:
        raise RuntimeError(f"StartProgrammer NAK: {resp.hex()}")

    # ── ProgrammerRunning (0x54): captured 10-byte leader (NO checksum) + code ──
    #   BE 54 A2 03 00 00 00 48 00 00 00 00  <code bytes …>
    leader = bytes([BES_SYNC, MSG_PROG_RUNNING,
                    0xA2, 0x03, 0x00, 0x00, 0x00, 0x48, 0x00, 0x00, 0x00, 0x00])
    port.write(leader)
    port.flush()

    # Suppress per-byte logging for the 112 KB binary; print one summary line
    port.bulk = True
    blk = 4096
    for off in range(0, code_len, blk):
        port.write(code[off:off + blk])
        port.flush()
        pct = min(100, (off + blk) * 100 // code_len)
        print(f"\r    Sending binary... {pct:3d}%", end='', flush=True)
    print()
    port.bulk = False
    port.log_bulk('TX', code_len, 'programmer binary')

    # Captured ACK: BE 54 A2 01 20 2A  → payload A2 01 20 means success.
    resp = read_packet(port, MSG_PROG_RUNNING, timeout=10)
    if resp and list(resp[:3]) == [0xA2, 0x01, 0x20]:
        print("    Programmer loaded OK (ACK A2 01 20)")
    else:
        print(f"  [!] Unexpected ProgrammerRunning ACK: {resp.hex() if resp else 'none'} "
              f"(expected A2 01 20)")


# ── Phase 3: Start programmer ──────────────────────────────────────────────
def start_programmer(port) -> None:
    print("[3] Starting programmer...")
    port.write(make_packet(MSG_PROG_START, bytes([0x01, 0x00])))
    port.flush()
    # Captured reply: BE 10 00 06 ... (status) then BE 60 01 06 05 01 00 D0 03 00 01
    # (ProgrammerInit: version 0x0105, sector 0x0003D000). Read until the 0x60.
    want = bytes([BES_SYNC, MSG_PROG_INIT])
    deadline = time.time() + 5
    port.timeout = 0.2
    buf = b''
    while time.time() < deadline:
        d = port._p.read(64)
        if d:
            if object.__getattribute__(port, '_enabled'):
                port._dump('RX', d)
            buf += d
        i = buf.find(want)
        if i >= 0 and len(buf) - i >= 5 and len(buf) - i >= 5 + buf[i + 3]:
            payload = buf[i + 2:i + 5 + buf[i + 3] - 1]
            ver = (payload[3] << 8 | payload[2]) if len(payload) >= 4 else 0
            sec = struct.unpack_from('<I', payload, 4)[0] if len(payload) >= 8 else 0
            print(f"    Programmer v0x{ver:04X}, sector 0x{sec:X} — RUNNING")
            return
    print("    (no ProgrammerInit 0x60 — continuing)")


# ── Phase 4: Verify flash ID ───────────────────────────────────────────────
def query_flash_id(port) -> None:
    print("[4] Flash ID...")
    port.write(make_packet(MSG_FLASH_CMD, bytes([0x02, 0x01, 0x11])))
    port.flush()
    try:
        resp = read_packet(port, MSG_FLASH_CMD, timeout=2)
        if resp and len(resp) >= 6:
            fid = resp[3:6]
            mfr = {0xC8: "GigaDevice", 0xEF: "Winbond", 0x20: "Micron"}.get(fid[0], "Unknown")
            print(f"    {mfr} {'-'.join(f'{b:02X}' for b in fid)}")
            if list(fid[:3]) == [0xC8, 0x63, 0x16]:
                print("    Confirmed: GigaDevice C8-63-16 = 4 MB single die")
    except TimeoutError:
        print("    (flash ID timeout — continuing)")


# ── Diagnostic: dump the raw reply to one FlashRead ────────────────────────
def probe_flash_read(port, base: int) -> None:
    """Send one FlashRead (0x03) and dump the raw response so we can see the
    exact reply format of THIS programmer (or confirm 0x03 is not supported)."""
    print(f"[probe] FlashRead @ 0x{base:08X} (len 0x100) — raw reply:")
    pkt = make_packet(MSG_FLASH_READ,
                      bytes([0x05, 0x08]) + struct.pack('<I', base) +
                      struct.pack('<I', 0x100))
    port.reset_input_buffer()
    port.bulk = True                      # our own dump below; skip per-byte log
    port.write(pkt)
    port.flush()
    buf = b''
    end = time.time() + 1.5
    port.timeout = 0.1
    while time.time() < end:
        d = port._p.read(1024)
        if d:
            buf += d
    port.bulk = False
    if not buf:
        print("[probe]   NO RESPONSE — this programmer likely uses a different "
              "read command than FlashRead 0x03. Share this output.")
    else:
        print(f"[probe]   got {len(buf)} bytes; first 80:")
        print("   " + ' '.join(f'{b:02X}' for b in buf[:80]))


# ── Phase 5: Read flash ────────────────────────────────────────────────────
def read_flash(port, out_path: str,
               base: int = FLASH_BASE, size: int = FLASH_SIZE) -> bool:
    """
    Read flash in 4 KB chunks. Resumes from existing file size.
    Returns True when complete, False if session should restart.
    """
    resume_offset = 0
    if os.path.exists(out_path):
        existing      = os.path.getsize(out_path)
        resume_offset = (existing // CHUNK_SIZE) * CHUNK_SIZE
        if resume_offset > 0:
            print(f"[5] Resuming from offset 0x{resume_offset:X} "
                  f"({resume_offset // 1024} KB already done)")
            # Truncate to ensure alignment if the file was partially written
            if existing > resume_offset:
                with open(out_path, 'ab') as f:
                    f.truncate(resume_offset)
        else:
            print(f"[5] Reading 0x{size:X} bytes from 0x{base:08X}...")
    else:
        print(f"[5] Reading 0x{size:X} bytes from 0x{base:08X}...")

    if resume_offset >= size:
        print("    Already complete.")
        return True

    total_chunks = (size + CHUNK_SIZE - 1) // CHUNK_SIZE
    start_chunk  = resume_offset // CHUNK_SIZE
    written      = resume_offset
    t_start      = time.time()

    mode = 'ab' if resume_offset > 0 else 'wb'

    # Clear any stray bytes left by start_programmer / query_flash_id so the very
    # first FlashRead response is not shifted by a leftover byte.
    port.reset_input_buffer()

    with open(out_path, mode) as f:
        for ci in range(start_chunk, total_chunks):
            addr = base + ci * CHUNK_SIZE

            pkt = make_packet(MSG_FLASH_READ,
                              bytes([0x05, 0x08]) +
                              struct.pack('<I', addr) +
                              struct.pack('<I', CHUNK_SIZE))

            chunk_ok = False
            for attempt in range(CHUNK_RETRIES):
                port.write(pkt)
                port.flush()
                try:
                    port.timeout = 3.0
                    found = False
                    for _ in range(500):
                        b = port.read(1)
                        if b and b[0] == BES_SYNC:
                            # Consume the FULL response header packet, whose length
                            # is variable (5 + header[3]). Reading a fixed 4 bytes
                            # here leaked header bytes into the data and shifted the
                            # dump by 1 (first byte doubled). Read BE+3, then the
                            # rest of the header per its length byte.
                            hdr = port.read(3)            # type, sub, len
                            if len(hdr) < 3:
                                raise TimeoutError("short FlashRead header")
                            total = 5 + hdr[2]            # full header packet length
                            if total > 4:
                                port.read(total - 4)      # BE + 3 already consumed
                            found = True
                            break
                    if not found:
                        raise TimeoutError("no FlashRead response header")

                    # Read raw 4 KB chunk — suppress per-byte log, print summary
                    chunk = b''
                    port.bulk = True
                    port.timeout = 2.0
                    while len(chunk) < CHUNK_SIZE:
                        d = port.read(CHUNK_SIZE - len(chunk))
                        if not d:
                            port.bulk = False
                            raise TimeoutError(f"partial chunk ({len(chunk)}/{CHUNK_SIZE})")
                        chunk += d
                    port.bulk = False
                    port.log_bulk('RX', len(chunk), f'@ 0x{addr:08X}')

                    f.write(chunk)
                    f.flush()
                    written   += len(chunk)
                    chunk_ok   = True
                    break

                except TimeoutError as e:
                    port.bulk = False
                    port.reset_input_buffer()
                    if attempt < CHUNK_RETRIES - 1:
                        wait = 0.2 * (attempt + 1)
                        print(f"\n  [!] Chunk 0x{addr:08X} attempt {attempt+1}/{CHUNK_RETRIES}: "
                              f"{e} — retry in {wait:.1f}s")
                        time.sleep(wait)

            if not chunk_ok:
                print(f"\n  [!] Chunk 0x{addr:08X} failed after {CHUNK_RETRIES} attempts.")
                print(f"      Saved {written//1024} KB so far. Session will restart.")
                return False

            elapsed     = max(0.1, time.time() - t_start)
            net_written = written - resume_offset
            speed       = net_written / elapsed / 1024
            pct         = written * 100 // size
            remain      = (size - written) / (net_written / elapsed) if net_written > 0 else 0
            print(f"\r    {pct:3d}%  {written//1024:5d}/{size//1024} KB  "
                  f"{speed:5.1f} KB/s  ETA {remain/60:.1f} min  addr=0x{addr:08X}",
                  end='', flush=True)

    elapsed     = max(0.1, time.time() - t_start)
    net_written = written - resume_offset
    print(f"\n    Done: {written} bytes in {elapsed:.1f}s "
          f"({net_written/elapsed/1024:.1f} KB/s avg)")
    return True


def read_memory_4b(port, out_path: str, base: int, size: int) -> bool:
    """
    Read arbitrary memory (e.g. ROM) 4 bytes at a time using register-read command.
    Befehl: BE 83 00 06 A4 02 <addr:4LE> <cs>
    Antwort: BE 83 00 05 00 <val:4LE> <cs>
    """
    resume_offset = 0
    if os.path.exists(out_path):
        existing      = os.path.getsize(out_path)
        resume_offset = (existing // 4) * 4
        if resume_offset > 0:
            print(f"[5] Resuming memory read from offset 0x{resume_offset:X}")
            if existing > resume_offset:
                with open(out_path, 'ab') as f:
                    f.truncate(resume_offset)
        else:
            print(f"[5] Reading 0x{size:X} bytes memory from 0x{base:08X} (4-byte words)...")
    else:
        print(f"[5] Reading 0x{size:X} bytes memory from 0x{base:08X} (4-byte words)...")

    if resume_offset >= size:
        print("    Already complete.")
        return True

    t_start = time.time()
    mode = 'ab' if resume_offset > 0 else 'wb'

    with open(out_path, mode) as f:
        for addr in range(base + resume_offset, base + size, 4):
            # msg[1]=0x83, payload=[p0=00, len=06, sub=A4, op=02, addr:4LE]
            pkt = make_packet(MSG_EXTEND_CMD,
                              bytes([0x00, 0x06, 0xA4, 0x02]) + struct.pack('<I', addr))

            val = None
            for attempt in range(CHUNK_RETRIES):
                port.write(pkt)
                port.flush()
                try:
                    # Expect response: BE 83 [p0] [len] [status] [val:4] [cs]
                    # read_typed returns payload between TYPE and CS: [p0, len, status, payload...]
                    resp = read_typed(port, MSG_EXTEND_CMD, timeout=1.0)
                    if len(resp) >= 6 and resp[2] == 0x00:
                        val = resp[3:7]
                        break
                    else:
                        if resp:
                            print(f"\n  [!] Unexpected status/len: {resp.hex()}")
                except TimeoutError:
                    if attempt < CHUNK_RETRIES - 1:
                        time.sleep(0.1)
            
            if val is None:
                print(f"\n  [!] Failed to read memory at 0x{addr:08X} after {CHUNK_RETRIES} retries.")
                return False

            f.write(val)
            f.flush()

            written = addr - base + 4
            if (addr % 0x100) == 0 or written == size:
                elapsed = max(0.1, time.time() - t_start)
                speed = (written - resume_offset) / elapsed
                pct = written * 100 // size
                remain = (size - written) / speed if speed > 0 else 0
                print(f"\r    {pct:3d}%  {written//1024:5d}/{size//1024} KB  "
                      f"{speed:.1f} B/s  ETA {remain/60:.1f} min  addr=0x{addr:08X}",
                      end='', flush=True)

    print(f"\n    Memory read complete.")
    return True


def probe_bases(port):
    """Try to read the first 16 bytes from several common memory bases."""
    bases = [0x00000000, 0x00100000, 0x0C000000, 0x1C000000,
             0x20000000, 0x24000000, 0x3C000000, 0xFFFF0000]
    print("[probe] Probing potential memory bases (4 words each)...")
    for b in bases:
        print(f"  0x{b:08X}: ", end='', flush=True)
        row = []
        for i in range(4):
            addr = b + i*4
            pkt = make_packet(MSG_EXTEND_CMD,
                              bytes([0x00, 0x06, 0xA4, 0x02]) + struct.pack('<I', addr))
            port.write(pkt); port.flush()
            try:
                # Need to use a shorter timeout for probing to avoid waiting 1s per miss
                resp = read_typed(port, MSG_EXTEND_CMD, timeout=0.3)
                if len(resp) >= 6 and resp[2] == 0x00:
                    row.append(resp[3:7].hex())
                else:
                    row.append("ERR")
            except TimeoutError:
                row.append("TMO")
        print(' '.join(row))
    print("[probe] Done.")


# ── Read a packet of a specific type (skip others, e.g. stray BE 50) ───────
def read_typed(port, want_type: int, timeout: float = 5.0) -> bytes:
    """Read BES packets until one of `want_type` arrives; return its payload."""
    port.timeout = timeout
    end = time.time() + timeout
    while time.time() < end:
        b = port.read(1)
        if not b:
            continue
        if b[0] != BES_SYNC:
            continue
        hdr = port.read(3)
        if len(hdr) < 3:
            continue
        total = 5 + hdr[2]
        rest = port.read(total - 4)
        full = bytes([BES_SYNC]) + hdr + rest
        if hdr[0] == want_type:
            return full[2:-1]      # payload (between type byte and checksum)
    raise TimeoutError(f"Timeout waiting for 0x{want_type:02X} packet")


# ── Phase 5 (write): burn an image to flash ────────────────────────────────
def write_flash(port, data: bytes, base: int = FLASH_BASE) -> bool:
    """
    Write `data` to flash starting at `base`, using the BES burn protocol
    (EraseBurnStart 0x61 → FlashBurnData 0x62 in 32 KB chunks, max 2 unacked →
    FlashCommand commit 0x65). The region is padded to a 32 KB multiple with
    0xFF. CRC of each chunk is CRC-32/ISO-HDLC (== zlib.crc32).
    """
    if len(data) % FLASH_BUFFER_SIZE:
        pad = FLASH_BUFFER_SIZE - (len(data) % FLASH_BUFFER_SIZE)
        data += b'\xFF' * pad
    nchunks = len(data) // FLASH_BUFFER_SIZE
    print(f"[5] Writing {len(data)} B to 0x{base:08X} "
          f"({nchunks} x {FLASH_BUFFER_SIZE//1024} KB chunks)")

    # ── EraseBurnStart (0x61): erase the target region + prepare burn ──
    erase = make_packet(MSG_ERASE_BURN,
                        bytes([0x05, 0x0C]) +
                        struct.pack('<I', base) +
                        struct.pack('<I', len(data)) +
                        bytes([0x00, 0x80, 0x00, 0x00]))
    port.write(erase)
    port.flush()
    resp = read_typed(port, MSG_ERASE_BURN, timeout=30)
    if list(resp[:3]) != [0x05, 0x01, 0x00]:
        raise RuntimeError(f"EraseBurnStart NAK: {resp.hex()}")
    print("    Region erased, burn ready")

    # ── FlashBurnData (0x62) chunks, pipelined (max 2 unacked) ──
    MAX_UNACKED = 2
    outstanding = 0
    t_start = time.time()
    for ci in range(nchunks):
        # Drain acks until we may send again
        while outstanding >= MAX_UNACKED:
            read_typed(port, MSG_BURN_DATA, timeout=30)
            outstanding -= 1

        chunk = data[ci * FLASH_BUFFER_SIZE:(ci + 1) * FLASH_BUFFER_SIZE]
        crc = zlib.crc32(chunk) & 0xFFFFFFFF
        seq = ci & 0xFF
        hdr = make_packet(MSG_BURN_DATA,
                         bytes([(0xC1 + seq) & 0xFF, 0x0B]) +
                         struct.pack('<H', FLASH_BUFFER_SIZE) +
                         bytes([0x00, 0x00]) +
                         struct.pack('<I', crc) +
                         bytes([seq, 0x00, 0x00]))
        port.bulk = True
        port.write(hdr)
        port.write(chunk)
        port.flush()
        port.bulk = False
        if ci == 0:
            time.sleep(0.411)          # first chunk: device finishes erase
        outstanding += 1

        done = ci + 1
        speed = (done * FLASH_BUFFER_SIZE) / max(0.1, time.time() - t_start) / 1024
        print(f"\r    {done*100//nchunks:3d}%  {done}/{nchunks} chunks  "
              f"{speed:5.1f} KB/s", end='', flush=True)

    while outstanding > 0:
        read_typed(port, MSG_BURN_DATA, timeout=30)
        outstanding -= 1
    print()

    # NO commit command: the best1503 programmer erases+burns+VERIFIES every chunk
    # in its FLASH_TASK as it receives it (log: "Erase done / Burn done / Verify
    # done"), so the data is already committed once the last chunk is acked. The
    # bestool finalize (0x65 06 09 22 … 1C EC 57 BE) is a BES2300 command; on this
    # chip it is parsed as a bogus BURN_DATA that verifies a RAM address and
    # asserts ("addr not in flash"). So we stop here — the write is complete.
    print("    Write complete (all chunks erased+burned+verified by the programmer)")
    return True


# ── Reboot the device (start the newly-flashed firmware) ───────────────────
def send_reboot(port) -> None:
    """
    Send the DeviceCommand reboot (BE 00 00 01 F1 cs) — triggers the PMU watchdog
    reboot so the SoC resets and boots the firmware just written. The device
    resets immediately, so a reply may not arrive.
    """
    print("[6] Rebooting device to start the new firmware...")
    port.write(make_packet(MSG_DEVICE_CMD, bytes([0x00, 0x01, 0xF1])))
    port.flush()
    try:
        resp = read_typed(port, MSG_DEVICE_CMD, timeout=2)
        print(f"    reboot ack: {resp.hex()}")
    except TimeoutError:
        print("    (no ack — device is rebooting)")


# ── Main ──────────────────────────────────────────────────────────────────
def main():
    ap = argparse.ArgumentParser(
        description="BES best1503 (MiBand 9) flash read/write over UART")
    ap.add_argument('--port',    required=True, help="Serial port, e.g. COM3")
    ap.add_argument('--address', default=f'0x{FLASH_BASE:08X}',
                    help="Flash address, hex (default flash base)")
    ap.add_argument('--output',  default='firmware_dump.bin',
                    help="READ mode: output file")
    ap.add_argument('--size',    default=f'0x{FLASH_SIZE:08X}',
                    help="READ mode: bytes to read, hex")
    ap.add_argument('--mem',     action='store_true',
                    help="Memory read mode (4-byte words, e.g. for ROM)")
    ap.add_argument('--mem-size', default='0x10000',
                    help="Memory mode: bytes to read, hex (default 64KB)")
    ap.add_argument('--write',   default=None, metavar='FILE',
                    help="WRITE mode: erase+burn FILE to --address")
    ap.add_argument('--no-reboot', action='store_true',
                    help="WRITE mode: do NOT reboot the device after writing")
    ap.add_argument('--no-log',  action='store_true',
                    help="Disable UART hex dump (quiet mode)")
    ap.add_argument('--probe',   action='store_true',
                    help="Diagnostic: probe various memory bases for ROM")
    args = ap.parse_args()

    addr    = int(args.address, 16)
    size    = int(args.mem_size, 16) if args.mem else int(args.size, 16)
    writing = args.write is not None

    try:
        import serial as _s   # noqa: F401
    except ImportError:
        sys.exit("ERROR: pyserial not installed — run: pip install pyserial")

    payload = None
    if writing:
        if not os.path.exists(args.write):
            sys.exit(f"ERROR: file not found: {args.write}")
        with open(args.write, 'rb') as f:
            payload = f.read()

    print("=" * 60)
    print("  BES best1503 / MiBand 9  Flash " + ("Writer" if writing else "Reader"))
    print("=" * 60)
    print(f"  Port:    {args.port}  @  {BAUD} baud")
    if writing:
        print(f"  Write:   {args.write}  ({len(payload)} B)  →  0x{addr:08X}")
    else:
        mode_str = "Memory (4b words)" if args.mem else "Flash (4KB chunks)"
        print(f"  Mode:    {mode_str}")
        print(f"  Read:    0x{addr:08X}  +  {size // 1024} KB  →  {args.output}")
    print()

    # Single attempt: sync → load programmer → start → read/write.
    # We deliberately do NOT auto-retry the whole sequence: once the programmer
    # is running, re-sending sync (0x50) / StartProgrammer (0x53) to it desyncs
    # it ("Invalid message type: 0x50/0x53"). On any failure, power-cycle the
    # MiBand and re-run (reads resume from the existing output file).
    done = False
    try:
        with serial.Serial(args.port, BAUD, timeout=1) as raw_port:
            port = LoggingPort(raw_port, enabled=not args.no_log)
            port.reset_input_buffer()
            port.reset_output_buffer()

            sync_bootrom(port)
            load_programmer(port)
            start_programmer(port)
            query_flash_id(port)

            if args.probe:
                probe_bases(port)
                done = True
            elif writing:
                done = write_flash(port, payload, addr)
                if done and not args.no_reboot:
                    send_reboot(port)
            elif args.mem:
                done = read_memory_4b(port, args.output, addr, size)
            else:
                done = read_flash(port, args.output, addr, size)

    except FileNotFoundError as e:
        sys.exit(f"\nERROR: {e}")

    except serial.SerialException as e:
        sys.exit(f"\n  [!] Serial port error: {e}\n  Check USB connection and port name.")

    except KeyboardInterrupt:
        print("\n\nAborted by user.")
        if not writing:
            existing = os.path.getsize(args.output) if os.path.exists(args.output) else 0
            if existing > 0:
                print(f"Partial dump: {existing // 1024} KB in {args.output} (re-run to resume).")
        sys.exit(0)

    except Exception as e:
        print(f"\n  [!] Error: {type(e).__name__}: {e}")
        print("  POWER-CYCLE the MiBand, then re-run "
              "(do NOT re-run without a power-cycle — the programmer is live).")
        sys.exit(1)

    if not done:
        print("\n  Did not complete. Power-cycle the MiBand and re-run to resume.")
        sys.exit(1)

    if writing:
        print("\nFlash write complete!")
    else:
        print("\nFirmware extraction complete!")

if __name__ == '__main__':
    main()
