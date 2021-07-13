import theano
import sys
import numpy
import collections

input_vector = theano.tensor.fvector('input_vector') #theano variable representing image
target_values = theano.tensor.fvector('target_values') #theano variable representing the label of that image

W = theano.shared(numpy.zeros((2, 4096)), 'W')
act = theano.tensor.dot(W, input_vector)
pred_v = theano.tensor.nnet.sigmoid(act)
Accuracy = -theano.tensor.sqr(pred_v - target_values).sum()
grad = theano.tensor.grad(Accuracy, W)
list_of_updates = [(W, W+0.01*grad)]
predicted_class = theano.tensor.argmax(pred_v)
theano.config.on_unused_input='ignore'

# defining Theano functions for training and testing the model:
train = theano.function([input_vector, target_values], [Accuracy, predicted_class], updates=list_of_updates, allow_input_downcast=True)
test = theano.function([input_vector, target_values], [Accuracy, predicted_class], allow_input_downcast=True)
    #'allow_input_downcast=True' is needed to avoid any issues converting between 64 and 32 bit numbers

def read_dataset(path): #The function that reads training or testing data and returns it as an array of data points
    number_of_images = len(open(path).readlines())
    print (number_of_images)
    f = open(path)
    dataset = [] #starts with an empty container
    for i in range(int(number_of_images)): 
        data_vector = [] #we start with empty data vector, and then we read the data for each image from the 28 lines that it takes:
        line = f.readline()
        line_parts = line.split() #split the line into all the numbers
        assert(len(line_parts) == 4098) #should be total of 29: the label + 28 numbers for the image
        label = int(line_parts[0]) #very first number in the file is the label (the digit that the picture represents)
        assert (0 <= label <= 1) #only digits 0-1 are allowed as labels
        #now, we will create "one-hot vector":
        label_vector = [0,0] #a vector of 2 zeroes
        label_vector[label] = 1 #except 1 for the label
        name = line_parts[1]
        for i in range(2, len(line_parts)):
            data_vector.append(float(line_parts[i]))

        dataset.append((label_vector, name, data_vector))
    return dataset


#reading the data:
data_train = read_dataset("train.txt")
print ("Train set read")
data_test = read_dataset("test.txt")
print ("Test set read")

# training
for epoch in range(10):
        cost_sum = 0.0
        correct = 0
        i = 0
        for labelv, name, vector in data_train:
            i += 1
            if i % 800 == 0:
                print (f"Done {i} datapoints")
            Accuracy, predicted_class = train(vector, labelv)
            cost_sum += Accuracy
            if (labelv[predicted_class] == 1):
                correct += 1
        print ("Epoch: " + str(epoch) + ", Accuracy: " + str(cost_sum) + ", %correct: " + str(float(correct) / len(data_train)*100))

# testing:
cost_sum = 0.0
correct = 0
i = 0
for labelv, name, vector in data_test:
    i += 1
    Accuracy, predicted_class = test(vector, labelv)
    cost_sum += Accuracy
    right = labelv[predicted_class] == 1
    if (right):
        correct += 1
    if i % 9 == 0:
        print (f"Name: {name}, prediction: {predicted_class}, correct: {right}")
        
print ("\t%correct on the test set: " + str(float(correct) / len(data_test)*100))
