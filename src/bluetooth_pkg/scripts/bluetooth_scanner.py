#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import rospy
import bluetooth
from bluetooth_pkg.msg import BluetoothData

def bluetooth_scan():
    rospy.init_node('bluetooth_scanner', anonymous=True)
    pub = rospy.Publisher('bluetooth_data', BluetoothData, queue_size=10)
    rate = rospy.Rate(1)  # 1 Hz

    while not rospy.is_shutdown():
        nearby_devices = bluetooth.discover_devices()
        for addr in nearby_devices:
            device_name = bluetooth.lookup_name(addr)
            data = BluetoothData()
            data.device_name = device_name
            data.device_address = addr
            # Here it is simply assumed that rssi is 0, but in practice it needs to be obtained by other methods
            data.rssi = 0
            pub.publish(data)
        rate.sleep()

if __name__ == '__main__':
    try:
        bluetooth_scan()
    except rospy.ROSInterruptException:
        pass