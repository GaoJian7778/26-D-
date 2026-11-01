"""
animal_detector.py - 动物识别 (队友实现)
接口: detect_animals(cv_image) → (animal_name, confidence) 或 None
"""
import rospy


def detect_animals(cv_image):
    """
    输入: OpenCV BGR 图像 (numpy array)
    返回:
        ("elephant", 95)  检测到大象，置信度95%
        ("tiger", 87)
        ("wolf", 92)
        ("monkey", 78)
        ("peacock", 90)
        None              未检测到动物
    """
    # TODO: 队友实现
    rospy.logdebug("[DETECTOR] stub called")
    return None
