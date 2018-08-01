import cv2 # Version must be <= 3.1.0
import numpy
import tensorflow as tf
import keras
#import PIL
import imutils

def main(nd_data):
    print(type(nd_data))
    cv2.imwrite("abc.jpg", nd_data)
    b = 1 + 2
    return ("hello", b)

