# coding: utf-8
"""
摄像头/视频源封装，方便后续替换为真实无人机图传。
"""
import cv2

from config import CAMERA_INDEX, VIDEO_PATH


class Camera:
    """
    统一视频源接口：支持本地摄像头和视频文件。
    """

    def __init__(self, source=None):
        if source is None:
            source = VIDEO_PATH if VIDEO_PATH else CAMERA_INDEX
        self.source = source
        self.cap = cv2.VideoCapture(source)
        if not self.cap.isOpened():
            raise RuntimeError(f'无法打开视频源: {source}')

        self.width = int(self.cap.get(cv2.CAP_PROP_FRAME_WIDTH))
        self.height = int(self.cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
        self.fps = self.cap.get(cv2.CAP_PROP_FPS)

    def read(self):
        ret, frame = self.cap.read()
        return frame if ret else None

    def release(self):
        self.cap.release()

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.release()
