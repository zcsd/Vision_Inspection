import os
import time
import math
import numpy as np
import tensorflow as tf
from keras.preprocessing.image import ImageDataGenerator, img_to_array, load_img
from keras.models import Sequential
from keras.layers import Dropout, Flatten, Dense
from keras import applications
from keras.utils.np_utils import to_categorical
from keras import backend as K
import matplotlib.pyplot as plt
import cv2

# dimensions of our images.
img_width, img_height = 224, 224

top_model_weights_path = 'bottleneck_fc_model.h5'
train_data_dir = 'data/train'
validation_data_dir = 'data/validation'

# number of epochs to train top model
epochs = 100
# batch size used by flow_from_directory and predict_generator
batch_size = 16

mode = 0 # 0 for training, 1 for single predict, 2 for batch predict
total_counter = 0
correct_counter = 0
total_time = 0.0

def save_bottlebeck_features():
    # build the VGG16 network
    model = applications.VGG16(include_top=False, weights='imagenet')

    datagen = ImageDataGenerator(rescale=1. / 255)

    generator = datagen.flow_from_directory(
        train_data_dir,
        target_size=(img_width, img_height),
        batch_size=batch_size,
        class_mode=None,
        shuffle=False)

    print(len(generator.filenames))
    print(generator.class_indices)
    print(len(generator.class_indices))

    nb_train_samples = len(generator.filenames)
    num_classes = len(generator.class_indices)

    predict_size_train = int(math.ceil(nb_train_samples / batch_size))

    bottleneck_features_train = model.predict_generator(
        generator, predict_size_train)

    np.save('bottleneck_features_train.npy', bottleneck_features_train)

    generator = datagen.flow_from_directory(
        validation_data_dir,
        target_size=(img_width, img_height),
        batch_size=batch_size,
        class_mode=None,
        shuffle=False)

    nb_validation_samples = len(generator.filenames)

    predict_size_validation = int(
        math.ceil(nb_validation_samples / batch_size))

    bottleneck_features_validation = model.predict_generator(
        generator, predict_size_validation)

    np.save('bottleneck_features_validation.npy',
            bottleneck_features_validation)


def train_top_model():
    datagen_top = ImageDataGenerator(rescale=1. / 255)
    generator_top = datagen_top.flow_from_directory(
        train_data_dir,
        target_size=(img_width, img_height),
        batch_size=batch_size,
        class_mode='categorical',
        shuffle=False)

    nb_train_samples = len(generator_top.filenames)
    num_classes = len(generator_top.class_indices)

    # save the class indices to use use later in predictions
    np.save('class_indices.npy', generator_top.class_indices)

    # load the bottleneck features saved earlier
    train_data = np.load('bottleneck_features_train.npy')

    # get the class lebels for the training data, in the original order
    train_labels = generator_top.classes

    # https://github.com/fchollet/keras/issues/3467
    # convert the training labels to categorical vectors
    train_labels = to_categorical(train_labels, num_classes=num_classes)

    generator_top = datagen_top.flow_from_directory(
        validation_data_dir,
        target_size=(img_width, img_height),
        batch_size=batch_size,
        class_mode=None,
        shuffle=False)

    nb_validation_samples = len(generator_top.filenames)

    validation_data = np.load('bottleneck_features_validation.npy')

    validation_labels = generator_top.classes
    validation_labels = to_categorical(
        validation_labels, num_classes=num_classes)

    model = Sequential()
    model.add(Flatten(input_shape=train_data.shape[1:]))
    model.add(Dense(256, activation='relu'))
    model.add(Dropout(0.5))
    model.add(Dense(num_classes, activation='softmax'))

    model.compile(optimizer='rmsprop',
                  loss='categorical_crossentropy', metrics=['accuracy'])

    history = model.fit(train_data, train_labels,
                        epochs=epochs,
                        batch_size=batch_size,
                        validation_data=(validation_data, validation_labels))

    model.save_weights(top_model_weights_path)

    (eval_loss, eval_accuracy) = model.evaluate(
        validation_data, validation_labels, batch_size=batch_size, verbose=1)

    print("[INFO] accuracy: {:.2f}%".format(eval_accuracy * 100))
    print("[INFO] Loss: {}".format(eval_loss))

    plt.figure(1)

    # summarize history for accuracy

    plt.subplot(211)
    plt.plot(history.history['acc'])
    plt.plot(history.history['val_acc'])
    plt.title('model accuracy')
    plt.ylabel('accuracy')
    plt.xlabel('epoch')
    plt.legend(['train', 'test'], loc='upper left')

    # summarize history for loss

    plt.subplot(212)
    plt.plot(history.history['loss'])
    plt.plot(history.history['val_loss'])
    plt.title('model loss')
    plt.ylabel('loss')
    plt.xlabel('epoch')
    plt.legend(['train', 'test'], loc='upper left')
    plt.show()


def predict(image_path):
    
    time_start = time.time()
    #orig = cv2.imread(image_path)

    image = load_img(image_path, target_size=(224, 224))
    image = img_to_array(image)

    # important! otherwise the predictions will be '0'
    image = image / 255
    image = np.expand_dims(image, axis=0)

    # build the VGG16 network
    time_build_start = time.time()
    model_1 = applications.VGG16(include_top=False, weights='imagenet')
    time_build_end = time.time()
    # get the bottleneck prediction from the pre-trained VGG16 model
    #global model_1
    time_feat_start = time.time()
    bottleneck_prediction = model_1.predict(image)
    time_feat_end = time.time()

    K.clear_session()
    # build top model
    time_top_start = time.time()
    model_2 = Sequential()
    model_2.add(Flatten(input_shape=bottleneck_prediction.shape[1:]))
    model_2.add(Dense(256, activation='relu'))
    model_2.add(Dropout(0.5))
    model_2.add(Dense(num_classes, activation='softmax'))

    model_2.load_weights(top_model_weights_path)
    time_top_end = time.time()

    # use the bottleneck prediction on the top model to get the final
    # classification
    time_pre_start = time.time()
    class_predicted = model_2.predict_classes(bottleneck_prediction)

    probabilities = model_2.predict_proba(bottleneck_prediction)
    time_pre_end = time.time()
    K.clear_session()

    inID = class_predicted[0]
    confidence = probabilities[0][inID]

    inv_map = {v: k for k, v in class_dictionary.items()}

    label = inv_map[inID]

    result = "Wrong"
    time_end = time.time()
    time_interval = time_end - time_start
    time_build = time_build_end - time_build_start
    time_feat = time_feat_end - time_feat_start
    time_top = time_top_end - time_top_start
    time_pre = time_pre_end - time_pre_start
    
    print("  Total time: {:.2f}s; Build VGG Time: {:.2f}s; Feature Extract Time: {:.2f}s; Build Top Time: {:.2f}s; Predict Time: {:.2f}s".format(time_interval, time_build, time_feat, time_top, time_pre))
    global correct_counter
    if label in image_path:
        correct_counter+=1
        result = "Correct"
    
    # get the prediction label
    print("  Result: {}; Time: {:.2f}s; Label: {}; Confidence: {:.3f}; All: {}".format(result, time_interval, label, confidence, probabilities[0]))
    print("  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~")
    
    global total_time
    total_time += time_interval
    # display the predictions with the image
    #cv2.putText(orig, "Predicted: {}".format(label), (10, 30), cv2.FONT_HERSHEY_PLAIN, 1.5, (43, 99, 255), 2)

    #cv2.imshow("Classification", orig)
    #cv2.waitKey(0)
    #cv2.destroyAllWindows()

#model_1 = applications.VGG16(include_top=False, weights='imagenet')
if mode == 0:
    time_start = time.time()
    save_bottlebeck_features()
    time_mid = time.time()
    train_top_model()
    time_end = time.time()
    time_bb = time_mid - time_start
    time_interval = time_end - time_start
    time_top = time_end - time_mid
    print("Total training time: {:.2f}s".format(time_interval))
    print("Feature Extract time: {:.2f}s".format(time_bb))
    print("Top model training time: {:.2f}s".format(time_top))
elif mode == 1:
    # load the class_indices saved in the earlier step
    class_dictionary = np.load('class_indices.npy').item()
    num_classes = len(class_dictionary)
    image_path = "test+/gray+/heel_lining_red-CL_gray-BG_2.jpg"
    predict(image_path)
elif mode == 2:

    base_test_image_path = "test+"
    # load the class_indices saved in the earlier step
    class_dictionary = np.load('class_indices.npy').item()
    num_classes = len(class_dictionary)
    for folder in os.listdir(base_test_image_path):
        print("----------------------------------------")
        print("  Testing in " + folder + " folder...")
        current_folder_path = base_test_image_path + "/" + folder
        for file in os.listdir(current_folder_path):
            total_counter+=1
            print("  Testing " + file + " in " + folder + " folder...")
            current_file_path = current_folder_path + "/" + file
            predict(current_file_path)
        
    print("Accuracy: " + "{0:.0%}, ".format(correct_counter/total_counter) + str(correct_counter) + "/" + str(total_counter) + "; Average time: {:.0f}ms".format(total_time/total_counter*1000))
    
