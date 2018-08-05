import cv2 # Version must be <= 3.1.0
import numpy as np
import tensorflow as tf
import keras
from keras.models import Sequential
from keras.layers import Dropout, Flatten, Dense
from keras import applications
from keras import backend as K
#import PIL
import imutils

def main(nd_data):
    (label, confidence) = indentification(nd_data)
    return (label, confidence)

def indentification(image):
    #cv2.imwrite("abc.jpg", img)
    id_class_dictionary = np.load('/home/zichun/pylon_cv/python/model/id/class_indices.npy').item()
    id_num_classes = len(id_class_dictionary)
    id_top_model_weights_path = "/home/zichun/pylon_cv/python/model/id/bottleneck_fc_model.h5"
    # important! otherwise the predictions will be '0'
    image = image / 255
    image = np.expand_dims(image, axis=0)

    # build the VGG16 network
    model_1 = applications.VGG16(include_top=False, weights='imagenet')
    # get the bottleneck prediction from the pre-trained VGG16 model
    id_bottleneck_prediction = model_1.predict(image)

    K.clear_session()

    # build top model
    model_2 = Sequential()
    model_2.add(Flatten(input_shape=id_bottleneck_prediction.shape[1:]))
    model_2.add(Dense(256, activation='relu'))
    model_2.add(Dropout(0.5))
    model_2.add(Dense(id_num_classes, activation='softmax'))
    model_2.load_weights(id_top_model_weights_path)

    model_2.load_weights(id_top_model_weights_path)

    # use the bottleneck prediction on the top model to get the final
    # classification
    id_class_predicted = model_2.predict_classes(id_bottleneck_prediction)

    id_probabilities = model_2.predict_proba(id_bottleneck_prediction)

    K.clear_session()

    id_inID = id_class_predicted[0]
    id_confidence = id_probabilities[0][id_inID]

    id_inv_map = {v: k for k, v in id_class_dictionary.items()}

    id_label = id_inv_map[id_inID]

    if id_label == "heel_cap":
        model_class_dictionary = np.load('/home/zichun/pylon_cv/python/model/heel_cap/class_indices.npy').item()
        model_top_model_weights_path = "/home/zichun/pylon_cv/python/model/heel_cap/bottleneck_fc_model.h5"
    elif id_label == "heel_lining":
        model_class_dictionary = np.load('/home/zichun/pylon_cv/python/model/heel_lining/class_indices.npy').item()
        model_top_model_weights_path = "/home/zichun/pylon_cv/python/model/heel_lining/bottleneck_fc_model.h5"
    elif id_label == "sockliner":
        model_class_dictionary = np.load('/home/zichun/pylon_cv/python/model/sockliner/class_indices.npy').item()
        model_top_model_weights_path = "/home/zichun/pylon_cv/python/model/sockliner/bottleneck_fc_model.h5"
    elif id_label == "sole":
        model_class_dictionary = np.load('/home/zichun/pylon_cv/python/model/sole/class_indices.npy').item()
        model_top_model_weights_path = "/home/zichun/pylon_cv/python/model/sole/bottleneck_fc_model.h5"
    elif id_label == "vamp":
        model_class_dictionary = np.load('/home/zichun/pylon_cv/python/model/vamp/class_indices.npy').item()
        model_top_model_weights_path = "/home/zichun/pylon_cv/python/model/vamp/bottleneck_fc_model.h5"

    model_num_classes = len(model_class_dictionary)
    # build top model
    model_3 = Sequential()
    model_3.add(Flatten(input_shape=id_bottleneck_prediction.shape[1:]))
    model_3.add(Dense(256, activation='relu'))
    model_3.add(Dropout(0.5))
    model_3.add(Dense(model_num_classes, activation='softmax'))
    model_3.load_weights(model_top_model_weights_path)

    # use the bottleneck prediction on the top model to get the final
    # classification
    model_class_predicted = model_3.predict_classes(id_bottleneck_prediction)

    model_probabilities = model_3.predict_proba(id_bottleneck_prediction)

    K.clear_session()

    model_inID = model_class_predicted[0]
    model_confidence = model_probabilities[0][model_inID]

    model_inv_map = {v: k for k, v in model_class_dictionary.items()}

    model_label = model_inv_map[model_inID]

    #print("id_label: {}; model_label: {}.".format(id_label, model_label))
    if id_label == "heel_lining" or id_label == "heel_cap":
        label = id_label
    else:
        label = model_label + " " + id_label
    #print("id_label: {}; id_confidence: {:.2f};".format(id_label, id_confidence))
    #print("All: {}".format(id_probabilities[0]))
    return (label, id_confidence)
'''
test_img = cv2.imread("/home/zichun/pylon_cv/images/1.jpg", 1)
resized_image = cv2.resize(test_img, (224, 224)) 
(lab, conf) = main(resized_image)
print(lab)
'''
