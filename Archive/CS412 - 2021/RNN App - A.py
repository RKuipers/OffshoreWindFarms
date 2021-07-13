import theano
import sys
import numpy
import collections

input_vector = theano.tensor.fvector('input_vector') #theano variable representing image
target_values = theano.tensor.fvector('target_values') #theano variable representing the label of that image

input_size = 28 * 28
l1_size = 100
l2_size = 100
number_of_classes = 10
rng = numpy.random.RandomState(0)

# We initialize trainable weights randomly:
W = theano.shared(numpy.asarray(rng.normal(loc=0.0, scale=0.1, size=(l1_size, input_size)), dtype=theano.config.floatX), 'W')
W1 = theano.shared(numpy.asarray(rng.normal(loc=0.0, scale=0.1, size=(l2_size, l1_size)), dtype=theano.config.floatX), 'W1')
W2 = theano.shared(numpy.asarray(rng.normal(loc=0.0, scale=0.1, size=(number_of_classes, l2_size)), dtype=theano.config.floatX), 'W2')

activations_l1 = theano.tensor.dot(W, input_vector)
outputs_l1 = theano.tensor.nnet.sigmoid(activations_l1)

activations_l2 = theano.tensor.dot(W1, outputs_l1)
outputs_l2 = theano.tensor.nnet.sigmoid(activations_l2)

activations = theano.tensor.dot(W2, outputs_l2)
predicted_values = theano.tensor.nnet.sigmoid(activations)
predicted_class = theano.tensor.argmax(predicted_values)
Accuracy = -theano.tensor.sqr(predicted_values - target_values).sum()
gradients = theano.tensor.grad(Accuracy, [W, W1, W2])
learning_rate = 0.3
list_of_updates  = [
    (W, W + learning_rate * theano.tensor.grad(Accuracy, W)),
    (W1, W1 + learning_rate * theano.tensor.grad(Accuracy, W1)), 
    (W2, W2 + learning_rate * theano.tensor.grad(Accuracy, W2))
]

# defining Theano functions for training and testing the model:
train = theano.function([input_vector, target_values], [Accuracy, predicted_class], updates=list_of_updates, allow_input_downcast=True)
test = theano.function([input_vector, target_values], [Accuracy, predicted_class], allow_input_downcast=True)
    #'allow_input_downcast=True' is needed to avoid any issues converting between 64 and 32 bit numbers

def read_dataset(path): #The function that reads training or testing data and returns it as an array of data points
    number_of_images = len(open(path).readlines()) / 28 #28 lines per each image
    f = open(path)
    dataset = [] #starts with an empty container
    for i in range(int(number_of_images)): 
     data_vector = [] #we start with empty data vector, and then we read the data for each image from the 28 lines that it takes:
     for l in range(28): #each image takes 28 lines
            line = f.readline()
            line_parts = line.split() #split the line into all the numbers
            assert(len(line_parts) == 29) #should be total of 29: the label + 28 numbers for the image
            label = int(line_parts[0]) #very first number in the file is the label (the digit that the picture represents)
            assert (0 <= label <= 9) #only digits 0-9 are allowed as labels
            #now, we will create "one-hot vector":
            label_vector = [0,0,0,0,0,0,0,0,0,0] #a vector of 10 zeroes
            label_vector[label] = 1 #except 1 for the label
            data_vector += [float(line_parts[i])/255. for i in range(1, len(line_parts))]
            #we divide by 255 so the pixel brightness is represented by a number between 0 and 1

     dataset.append((label_vector, data_vector))
    return dataset

print("")
print("Takes ages to run for me; on VM it runs quickly")
print("Template itself already has right output, so pay attention to that.")
print("Template gives 94.7%; basic implementation of 3rd layer gives 94.9%")
print("Adding epochs easily pushes it over 95%")
print("Things to look for in marking: new matrix, correct changing of sizes and activation/output pipeline, list of updates")
print("")

#reading the data:
data_train = read_dataset("train.txt")
data_test = read_dataset("test.txt")
print("Completed reading input")

# training
for epoch in range(15):
        cost_sum = 0.0
        correct = 0
        for labelv, vector in data_train:
            Accuracy, predicted_class = train(vector, labelv)
            cost_sum += Accuracy
            if (labelv[predicted_class] == 1):
                correct += 1
        print ("Epoch: " + str(epoch) + ", Accuracy: " + str(cost_sum) + ", %correct: " + str(float(correct) / len(data_train)*100))

# testing:
cost_sum1 = 0.0
correct1 = 0
for labelv, vector in data_test:
                Accuracy1, predicted_class = test(vector, labelv)
                cost_sum1 += Accuracy
                if (labelv[predicted_class] == 1):
                            correct1 += 1
print ("\t%correct on the test set: " + str(float(correct1) / len(data_test)*100))
