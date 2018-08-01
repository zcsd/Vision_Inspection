import cv2 # Version must be <= 3.1.0
import numpy as np
import tensorflow as tf
import keras
from keras.models import Sequential
from keras.layers import Dropout, Flatten, Dense
from keras import applications
from keras.utils.np_utils import to_categorical
from keras import backend as K
#import PIL
import imutils

def main(nd_data):
    (label, confidence) = indentification(nd_data)
    return (label, confidence)

def indentification(image):
    #cv2.imwrite("abc.jpg", img)
    class_dictionary = np.load('/home/zichun/pylon_cv/python/model/class_indices.npy').item()
    num_classes = len(class_dictionary)
    top_model_weights_path = "/home/zichun/pylon_cv/python/model/bottleneck_fc_model.h5"
    # important! otherwise the predictions will be '0'
    image = image / 255
    image = np.expand_dims(image, axis=0)

    # build the VGG16 network
    model_1 = applications.VGG16(include_top=False, weights='imagenet')
    # get the bottleneck prediction from the pre-trained VGG16 model
    bottleneck_prediction = model_1.predict(image)

    K.clear_session()

    # build top model
    model_2 = Sequential()
    model_2.add(Flatten(input_shape=bottleneck_prediction.shape[1:]))
    model_2.add(Dense(256, activation='relu'))
    model_2.add(Dropout(0.5))
    model_2.add(Dense(num_classes, activation='softmax'))

    model_2.load_weights(top_model_weights_path)

    # use the bottleneck prediction on the top model to get the final
    # classification
    class_predicted = model_2.predict_classes(bottleneck_prediction)

    probabilities = model_2.predict_proba(bottleneck_prediction)

    K.clear_session()

    inID = class_predicted[0]
    confidence = probabilities[0][inID]

    inv_map = {v: k for k, v in class_dictionary.items()}

    label = inv_map[inID]

    #print("Label: {}; Confidence: {:.2f};".format(label, confidence))
    return (label, confidence)
