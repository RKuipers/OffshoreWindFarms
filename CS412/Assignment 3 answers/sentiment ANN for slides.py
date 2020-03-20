import numpy
import theano

texts = numpy.asarray([[1,1],[2,0],[1,0],[0,1],[0,1]])
scores = numpy.asarray([-2,2,0,1,-1])

input_vector = theano.tensor.fvector('input_vector') #theano variable representing image
target_values = theano.tensor.fvector('target_values') #theano variable representing the label of that image

W = theano.shared(numpy.zeros((5,2)), 'W')
activations = theano.tensor.dot(W, input_vector)
predicted_values = theano.tensor.nnet.sigmoid(activations)
predicted_class = theano.tensor.argmax(predicted_values)
Accuracy = -theano.tensor.sqr(predicted_values - target_values).sum()
gradients = theano.tensor.grad(Accuracy, W)
list_of_updates  = [(W, W + 1 * gradients)]
train = theano.function(
    [input_vector, target_values],
    [W, activations, predicted_values, predicted_class, Accuracy, gradients],
    updates = list_of_updates , allow_input_downcast=True)

data_vector = [1., 0.]
target_vector = [0,0,0,0,1]
W, activations, predicted_values, predicted_class, Accuracy, gradients_W  \
    = train(data_vector, target_vector)
print (W, activations, predicted_values, predicted_class, Accuracy, '\n', gradients_W)


