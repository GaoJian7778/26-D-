#!/usr/bin/env python3
import time
import rospy
import sys
import cv2
import numpy as np
import math
from sensor_msgs.msg import Image, CompressedImage
from cv_bridge import CvBridge, CvBridgeError


def main():
    try:
        while not rospy.is_shutdown():
            ret, frame = cap.read()
            # frame = cv2.flip(frame, 0)  ##图像上下颠倒
            if not ret:
                rospy.logwarn("Failed to read frame from camera 0")
                rospy.sleep(0.1)
                continue
            image_pub.publish(bridge.cv2_to_imgmsg(frame, "bgr8"))
            
            
            # 创建压缩图像消息
            # 可以手动调节压缩图像的相关参数，如调整 jpg 图像压缩质量为 5（默认为 95）
            compressed_msg = CompressedImage()
            compressed_msg.header.stamp = rospy.Time.now()
            compressed_msg.format = "jpg"
            compressed_msg.data = np.array(cv2.imencode('.jpg', frame, [cv2.IMWRITE_JPEG_QUALITY, 70])[1]).tobytes()  
            #compressed_msg.data = np.array(cv2.imencode('.jpg', frame)[1]).tobytes()
            
            
            
            image_pub.publish(bridge.cv2_to_imgmsg(frame, "bgr8"))
            # cimage_pub.publish(bridge.cv2_to_compressed_imgmsg(frame, "jpg"))
            cimage_pub.publish(compressed_msg)

            #cv2.imshow('publish_img', frame)
            cv2.waitKey(10)
            #if cv2.waitKey(1) & 0xFF == 27:
            #    break
        cap.release()
        #cv2.destroyAllWindows()
    except rospy.ROSInterruptException:
        pass


if __name__ == '__main__':
    try:
        rospy.init_node('cam_publish', anonymous=True)  # 初始化节点
        rospy.logwarn("cam_publish_node start...")
        global bridge,cap,image_pub
        bridge = CvBridge()
        cap = cv2.VideoCapture(0)
        #cap = cv2.VideoCapture(0, cv2.CAP_V4L2)
        if not cap.isOpened():
            rospy.logerr("Failed to open camera 0")
            sys.exit(1)
        cap.set(cv2.CAP_PROP_FOURCC,cv2.VideoWriter_fourcc('M','J','P','G'))
        image_pub = rospy.Publisher('/image', Image,queue_size=1,latch=True)#发布话题
        cimage_pub = rospy.Publisher("/image/compressed", CompressedImage, queue_size=10)
        main()
        rospy.spin()
    except rospy.ROSInterruptException:
        pass


