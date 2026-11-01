#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import bluetooth
import time
import threading

# Target Bluetooth device MAC addres
TARGET_MAC = "98:DA:20:00:E5:69"
# TARGET_MAC = "6A:A6:06:94:C2:1B"


def connect_bluetooth():
    # Maximum number of connection attempts
    max_attempts = 3
    for attempt in range(max_attempts):
        try:
            # Find available services on the target device
            services = bluetooth.find_service(address=TARGET_MAC)
            if not services:
                print(f"Attempt {attempt + 1}: No services found, trying default port 1")
                port = 1
            else:
                port = services[0]['port']
                print(f"Attempt {attempt + 1}: Service found, using port {port}")

            # Create a Bluetooth socket
            sock = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
            # Connect to the target device
            sock.connect((TARGET_MAC, port))

            # Get the local address of the socket
            local_addr = sock.getsockname()
            if not local_addr:
                print(f"Attempt {attempt + 1}: False connection, closing socket and retrying")
                sock.close()
                continue

            print(f"Successfully connected to Bluetooth device {TARGET_MAC} using port {port}")

            # Send wake-up command (depends on the slave device)
            wakeup_command = b'\r\n'
            sock.send(wakeup_command)
            time.sleep(0.1)

            return sock
        except bluetooth.btcommon.BluetoothError as e:
            print(f"Attempt {attempt + 1} to connect failed: {e}")
            if 'sock' in locals():
                sock.close()
            if attempt < max_attempts - 1:
                time.sleep(0.5)  # Wait 1 seconds between each attempt
    print("Failed to connect to Bluetooth after three attempts.")
    return None


def check_connection(sock):
    """
    Check if the Bluetooth connection is normal.
    :param sock: Bluetooth socket object.
    :return: Returns True if the connection is normal, otherwise returns False.
    """
    try:
        # Send simple test data.
        sock.send("test".encode())
        return True
    except bluetooth.btcommon.BluetoothError:
        return False


def auto_connect_bluetooth():
    """
    Automatically connect to the Bluetooth device and perform five checks.
    :return: Returns the socket object if the final connection is successful, otherwise returns None.
    """
    sock = None
    for i in range(1):
        print(f"Attempt {i + 1} to connect to Bluetooth...")
        sock = connect_bluetooth()
        if sock:
            if check_connection(sock):
                print("Bluetooth connection successful!")
                return sock
            else:
                print("Bluetooth connection is unstable. Closing the connection.")
                sock.close()
    print("Failed to connect to Bluetooth after multiple attempts.")
    return None


def send_data(sock, data, mtu=20):
    # 定义内部发送函数
    def _send_inner():
        nonlocal success, error_msg
        try:
            # 处理列表类型数据
            if isinstance(data, list):
                # 将列表元素转换为字节
                data_bytes = bytes(data)
            elif isinstance(data, str):
                # Remove spaces and convert hex string to bytes
                data_clean = data.replace(" ", "")
                try:
                    data_bytes = bytes.fromhex(data_clean)
                except ValueError:
                    error_msg = "Invalid hexadecimal string."
                    return
            else:
                data_bytes = data
                
            # data_bytes = data_bytes + b'\n'
            # Send data in packets
            for i in range(0, len(data_bytes), mtu):
                chunk = data_bytes[i:i + mtu]
                sock.send(chunk)
                time.sleep(0.01)  # Give the slave device time to process data
            
            success = True
            error_msg = data_bytes.hex()
            
        except bluetooth.btcommon.BluetoothError as e:
            error_msg = f"Bluetooth error: {e}"
        except Exception as e:
            error_msg = f"Error: {e}"

    if sock:
        # 初始化结果变量
        success = False
        error_msg = ""
        
        # 创建并启动线程
        send_thread = threading.Thread(target=_send_inner)
        send_thread.start()
        
        # 等待线程完成，最多等待5秒（超时时间）
        send_thread.join(5.0)
        
        # 检查线程是否超时
        if send_thread.is_alive():
            print("Failed to send data: Timeout after 5 seconds")
            return False
            
        if success:
            print(f"Data sent successfully: {error_msg}")
            return True
        else:
            print(f"Failed to send data: {error_msg}")
            return False
    return False
    

def hex_to_decimal(hex_str):
    """
    Convert a hexadecimal string to a decimal integer.
    
    Args:
        hex_str (str): Hexadecimal string (e.g., "1A", "FF", "123ABC").
    
    Returns:
        int: Decimal integer value.
    """
    try:
        # Remove any spaces and convert to uppercase
        hex_str = hex_str.replace(" ", "").upper()
        return int(hex_str, 16)
    except ValueError:
        print(f"Error: Invalid hexadecimal string '{hex_str}'")
        return None


def decimal_to_hex(decimal_num):
    """
    Convert a decimal integer to a hexadecimal integer.
    
    Args:
        decimal_num (int): Decimal integer value.
    
    Returns:
        int: Hexadecimal integer (e.g., 10 → 0xA → 10 in decimal representation).
             Returns None if conversion fails.
    """
    try:
        # 确保输入是整数
        decimal_num = int(decimal_num)
        # 转换为十六进制整数（本质是带0x前缀的整数，Python中与十进制整数等价）
        return int(format(decimal_num, 'X'), 16)
    except ValueError:
        print(f"Error: Invalid decimal number '{decimal_num}'")
        return None


def receive_data(sock, buffer_size=1024, timeout=5):
    """
    Receive data from the Bluetooth socket and return as hex string.
    
    Args:
        sock (bluetooth.BluetoothSocket): Connected Bluetooth socket.
        buffer_size (int): Maximum number of bytes to receive.
        timeout (float): Timeout in seconds for receiving data.
    
    Returns:
        str: Received data as a hexadecimal string, or None if error.
    """
    if not sock:
        print("Error: Bluetooth socket is not initialized.")
        return None
    
    try:
        # Set socket timeout
        sock.settimeout(timeout)
        
        # Receive data
        data = sock.recv(buffer_size)
        
        if not data:
            print("No data received.")
            return None
        
        # Convert bytes to hex string
        hex_data = data.hex()
        print(f"Received data (hex): {hex_data}")
        
        return hex_data
    
    except bluetooth.btcommon.BluetoothError as e:
        print(f"Bluetooth error while receiving data: {e}")
        return None
    except Exception as e:
        print(f"Unexpected error while receiving data: {e}")
        return None


if __name__ == "__main__":
    bluetooth_socket = connect_bluetooth()
    
    if bluetooth_socket:
        # Example: Send a hex command
        message = "FF 11 EE 12 12 FE"
        send_data(bluetooth_socket, message)
        
        # Example: Receive data
        received_hex = receive_data(bluetooth_socket)
        if received_hex:
            # Example: Convert hex to decimal
            decimal_value = hex_to_decimal(received_hex)
            if decimal_value is not None:
                print(f"Converted decimal value: {decimal_value}")
        
        bluetooth_socket.close()