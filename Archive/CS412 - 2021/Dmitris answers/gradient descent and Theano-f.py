import theano
import numpy
texts = numpy.asarray([[1,1],[2,0],[1,0],[0,1],[0,1]])
scores = numpy.asarray([-2,2,0,1,-1])

W = theano.shared(numpy.asarray([0.0, 0.0]), 'W') # The weights for our linear model, [0,0] are initial values
predictions = theano.tensor.dot(texts, W) #this is our linear model
Accuracy = -theano.tensor.sqr(predictions - scores).sum() + 10
gradients = theano.tensor.grad(Accuracy, [W])

W_updated = W + (0.1 * gradients[0])
updates = [(W, W_updated)]

f = theano.function([], Accuracy, updates=updates)
for i in range(10):
    output = f()
    print (output)







