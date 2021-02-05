import theano
import numpy as np
x = theano.tensor.fscalar('x')
z = (2*x + 3)**3
derivative = theano.tensor.grad(z, x)
differentiate = theano.function([x], derivative)
print (differentiate(1))
assert differentiate(1) == 150 #3*2*(2*x + 3)**2 = 6*25

import theano
x = theano.tensor.fscalar('x')
y = theano.tensor.fscalar('y')
z = theano.tensor.fscalar('z')
v = x * y * z
derivative = theano.tensor.grad(v, [x,y,z])
differentiate = theano.function([x,y,z], derivative)
print (differentiate(1,1,1)[0], differentiate(1,1,1)[1], differentiate(1,1,1)[2])
assert differentiate(1,1,1) == [1,1,1]

W = theano.shared(np.asarray([0.0, 0.0, 0.0]), 'W')
z = -(W[0]+W[1]+W[2]-1)**2
gradients = theano.tensor.grad(z, W)
W_updated = W + (0.1 * gradients)
updates = [(W, W_updated)]
f = theano.function([], [W, z], updates=updates)

for i in range(10):
    output = f()
    print (output)


