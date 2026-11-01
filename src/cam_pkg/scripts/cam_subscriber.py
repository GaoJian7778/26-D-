#!/usr/bin/env python3
import time
import rospy
import sys
import cv2
import numpy as np
import math
from sensor_msgs.msg import Image
from cv_bridge import CvBridge, CvBridgeError
image_path  = '/home/mr_wind/working/ros_project/hello_workspace/'
count = 0

def image_callback(image_data):
    global bridge,count
    #timestr = "%.6f" % image_data.header.stamp.to_sec()
    frame = bridge.imgmsg_to_cv2(image_data, "bgr8")
    #image_name = str(count) + ".jpg"  # 图像命名：时间戳.jpg
    #cv2.imwrite(image_path + image_name, frame)  # 保存；
    cv2.imshow("subscriber",frame)
    cv2.waitKey(1)
    #rospy.Rate(20).sleep()
    count+=1

if __name__ == "__main__":
    global bridge,cap,image_pub
    bridge = CvBridge()
    #2.初始化 ROS 节点:命名(唯一)
    rospy.init_node("cam_subscriber")
    #3.实例化 订阅者 对象
    sub = rospy.Subscriber("/image",Image,image_callback,queue_size=1)
    #4.处理订阅的消息(回调函数)
    #5.设置循环调用回调函数
    rospy.spin()
