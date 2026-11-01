#!/usr/bin/env python3
import cv2
import queue
import time
import threading
import numpy as np
import signal
import sys
import rospy
from sensor_msgs.msg import Image, CompressedImage
from cv_bridge import CvBridge, CvBridgeError

# Initialize ROS node
rospy.init_node('cam_publish_display')
bridge = CvBridge()
image_pub = rospy.Publisher('/image', Image, queue_size=1, latch=True)
image2_pub = rospy.Publisher('/image2', Image, queue_size=1, latch=True)
image3_pub = rospy.Publisher('/image3', Image, queue_size=1, latch=True)

# Shared status variable for threads
is_alive = True
threads = []

def cam_worker(camera_id, publisher):
    """Generic camera worker function"""
    global is_alive
    
    # Attempt to open the camera
    cap = cv2.VideoCapture(camera_id, cv2.CAP_V4L2)
    
    # Check if camera opened successfully
    if not cap.isOpened():
        rospy.logerr(f"Failed to open camera {camera_id}")
        return
    
    rospy.loginfo(f"Camera {camera_id} opened successfully")
    
    try:
        while cap.isOpened() and is_alive:
            ret, frame = cap.read()
            
            # Check if frame was successfully read
            if not ret:
                rospy.logwarn(f"Failed to read frame from camera {camera_id}")
                time.sleep(0.1)  # Short delay to avoid high CPU usage
                continue
            
            # Publish image message
            try:
                publisher.publish(bridge.cv2_to_imgmsg(frame, "bgr8"))
            except CvBridgeError as e:
                rospy.logerr(f"Error converting image: {e}")
    
    finally:
        # Ensure camera resources are released
        cap.release()
        rospy.loginfo(f"Camera {camera_id} closed")

def signal_handler(sig, frame):
    """Handle interrupt signals"""
    global is_alive
    print("Interrupted by user, shutting down...")
    is_alive = False
    
    # Wait for all threads to finish
    for thread in threads:
        thread.join(timeout=2.0)
    
    cv2.destroyAllWindows()
    rospy.loginfo("Program exited safely")
    sys.exit(0)

if __name__ == '__main__':
    # Set up signal handler
    signal.signal(signal.SIGINT, signal_handler)
    
    # Create and start camera threads
    camera_configs = [
        (0, image_pub),
        (2, image2_pub),
        (4, image3_pub)
    ]
    
    for camera_id, publisher in camera_configs:
        thread = threading.Thread(target=cam_worker, args=(camera_id, publisher))
        thread.daemon = True  # Set as daemon thread to auto-terminate on main exit
        thread.start()
        threads.append(thread)
        rospy.loginfo(f"Camera {camera_id} thread started")
    
    # Main loop
    try:
        rospy.spin()  # Use rospy.spin() instead of empty loop for better ROS event handling
    except KeyboardInterrupt:
        pass
    finally:
        is_alive = False
        for thread in threads:
            thread.join(timeout=2.0)
        cv2.destroyAllWindows()






# #!/usr/bin/env python3
# import cv2
# import queue
# import time
# import threading
# import numpy as np
# import signal
# import sys
# import rospy
# from sensor_msgs.msg import Image, CompressedImage
# from cv_bridge import CvBridge, CvBridgeError


# rospy.init_node('cam_publish_display')
# bridge = CvBridge()
# image_pub = rospy.Publisher('/image', Image,queue_size=1,latch=True)
# image2_pub = rospy.Publisher('/image2', Image,queue_size=1,latch=True)
# image3_pub = rospy.Publisher('/image3', Image,queue_size=1,latch=True)


# def cam_1():
#     global is_alive
#     cap_1 = cv2.VideoCapture(0, cv2.CAP_V4L2)
#     while cap_1.isOpened() and is_alive:
#         ret, frame = cap_1.read()
#         # cv2.imshow("test", frame)
#         image_pub.publish(bridge.cv2_to_imgmsg(frame, "bgr8"))
#         if cv2.waitKey(10) & 0xFF == ord('q'):
#             break
#     rospy.loginfo("cam1 quit!")
#     # cap_1.close()

# def cam_2():
#     cap_2 = cv2.VideoCapture(2, cv2.CAP_V4L2)
#     while cap_2.isOpened() and is_alive:
#         ret2, frame2 = cap_2.read()
#         # cv2.imshow("test2", frame2)
#         image2_pub.publish(bridge.cv2_to_imgmsg(frame2, "bgr8"))
#         if cv2.waitKey(10) & 0xFF == ord('q'):
#             break
#     rospy.loginfo("cam2 quit!")
#     # cap_2.close()

# def cam_3():
#     cap_3 = cv2.VideoCapture(4, cv2.CAP_V4L2)
#     while cap_3.isOpened() and is_alive:
#         ret3, frame3 = cap_3.read()
#         # cv2.imshow("test3", frame3)
#         image3_pub.publish(bridge.cv2_to_imgmsg(frame3, "bgr8"))
#         if cv2.waitKey(10) & 0xFF == ord('q'):
#             break
#     rospy.loginfo("cam3 quit!")
#     # cap_3.close()

# def signal_handler(sig, frame):
#     print("Interrupted by user, shutting down...")
#     global is_alive
    
#     is_alive = False
#     rospy.loginfo("End!!!")
#     sys.exit(0)

# if __name__ == '__main__':

#     global is_alive
#     is_alive = True
#     p1 = threading.Thread(target=cam_1, args=())
#     p1.start()
#     p2 = threading.Thread(target=cam_2, args=())
#     p2.start()
#     p3 = threading.Thread(target=cam_3, args=())
#     p3.start()
    
#     signal.signal(signal.SIGINT, signal_handler)
#     # cv2.namedWindow('test', cv2.WINDOW_NORMAL)
#     # cv2.namedWindow('test2', cv2.WINDOW_NORMAL)
#     # cv2.namedWindow('test3', cv2.WINDOW_NORMAL)
#     while is_alive:
#         pass
#         # Exit if ESC pressed
#         k = cv2.waitKey(1) & 0xff
#         if k == 27:
#             break

#     cv2.destroyAllWindows()
#     # p1.paused = True
#     p1.join()
#     p2.join()
#     p3.join()
    