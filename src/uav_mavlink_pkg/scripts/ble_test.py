#!/usr/bin/python3
"""
ble_test.py - HC-04 BLE 连通性测试 (不依赖ROS)
用法: python3 ble_test.py
"""
import asyncio
import sys

HC04_MAC = "C4:25:07:09:07:02"
CHR_UUID = "0000ffe1-0000-1000-8000-00805f9b34fb"

try:
    from bleak import BleakClient, BleakScanner
except ImportError:
    print("ERROR: pip3 install bleak")
    sys.exit(1)


def notify_cb(sender, data):
    """收到数据回调"""
    print(f"\n>>> RX [{len(data)}B]: {data.hex()} ")
    # 尝试解码ASCII
    try:
        text = data.decode('ascii', errors='replace')
        print(f"    ASCII: {text}")
    except:
        pass


async def main():
    print(f"=== HC-04 BLE Test ===")
    print(f"MAC: {HC04_MAC}")

    # Step 1: 扫描
    print("\n[1] Scanning 3s...")
    devices = await BleakScanner.discover(timeout=3.0)
    found = False
    for d in devices:
        if d.address.upper() == HC04_MAC.upper():
            print(f"    FOUND: {d.name} [{d.address}]")
            found = True
            break
    if not found:
        print(f"    NOT FOUND in scan. Trying direct connect anyway...")

    # Step 2: 连接
    print(f"\n[2] Connecting...")
    try:
        async with BleakClient(HC04_MAC, timeout=8.0) as client:
            if not client.is_connected:
                print("    FAILED to connect!"); return
            print("    Connected!")

            # Step 3: 订阅通知
            print(f"\n[3] Subscribing to FFE1 notify...")
            await client.start_notify(CHR_UUID, notify_cb)
            print("    OK, waiting for data from ground station...")
            print("    (Press Ctrl+C to stop)")

            # Step 4: 发测试帧
            print(f"\n[4] Sending test command...")
            test = bytes.fromhex("AA 55 00 01 3F")
            # 后面不接63字节数据, 仅测试链路
            await client.write_gatt_char(CHR_UUID, test, response=False)
            print(f"    Sent: {test.hex()}")

            # Step 5: 循环接收
            print(f"\n[5] Listening (30s)...")
            for i in range(300):
                await asyncio.sleep(0.1)

    except Exception as e:
        print(f"    ERROR: {e}")

    print("\nDone.")


if __name__ == "__main__":
    asyncio.run(main())
