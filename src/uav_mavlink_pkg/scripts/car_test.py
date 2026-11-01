#!/usr/bin/python3
"""
car_test.py - 小车控制测试
直行→拐弯→停止
"""
import asyncio, time
try:
    from bleak import BleakClient, BleakScanner
except ImportError:
    print("pip3 install bleak"); exit(1)

CAR_MAC = "C4:25:07:09:07:02"
CHR_UUID = "0000ffe1-0000-1000-8000-00805f9b34fb"

async def send(client, cmd):
    """发ASCII指令到小车"""
    data = cmd.encode() + b'\r\n'
    await client.write_gatt_char(CHR_UUID, data, response=False)
    print(f"  Sent: {cmd}")

async def main():
    print(f"=== Car BLE Test ===")
    print(f"[1] Connecting {CAR_MAC}...")
    async with BleakClient(CAR_MAC, timeout=8.0) as c:
        if not c.is_connected: print("FAIL"); return
        print("    Connected!")

        # 等2秒
        await asyncio.sleep(2)

        # ── 前进2秒 ──
        print("[2] Forward 2s...")
        await send(c, "F")
        await asyncio.sleep(2)

        # ── 右转 ──
        print("[3] Turn Right...")
        await send(c, "R")
        await asyncio.sleep(1)

        # ── 停止 ──
        print("[4] Stop...")
        await send(c, "S")

        # ── 定距直行 40cm (D4=40cm @ 10cm/单位) ──
        print("[5] Move 40cm...")
        await send(c, "D4-30")
        await asyncio.sleep(3)

        # ── 陀螺仪转90度 ──
        print("[6] Turn 90 degree...")
        await send(c, "T90-30")
        await asyncio.sleep(3)

        # ── 前进回去 ──
        print("[7] Forward 2s...")
        await send(c, "F")
        await asyncio.sleep(2)

        print("[8] Stop")
        await send(c, "S")
        print("Done!")

if __name__ == "__main__":
    asyncio.run(main())
