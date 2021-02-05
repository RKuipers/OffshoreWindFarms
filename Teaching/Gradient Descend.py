import numpy as np
import theano as th

#A
#z = -(2x -y +1)^2
#x_der = -8x + 4y - 4 -> at (0,0) = -4
#y_der = -2y + 4x + 2 -> at (0,0) = 2
#x = 0 + 0.01 * -4 = -0.04
#y = 0 + 0.01 * 2 = 0.02
#z(-0.04, 0.02) = -0.81

#B
print ("B")
def z(x,y):
    return -((2*x -y +1)**2)
def dx(x,y):
    return -8*x + 4*y - 4
def dy(x,y):
    return -2*y + 4*x + 2
x=0
y=0
r=0.05
print(x, y, z(x,y))
for _ in range(10):
    x_up = x + r * dx(x,y)
    y_up = y + r * dy(x,y)
    x = x_up
    y = y_up
    print(x, y, z(x,y))
#Maximum is 0 at (-0.4, 0.2) (or any point where 2x - y = -1)
    
#C
print ("C")
x = th.tensor.fscalar()
z = 2*x**3
der = th.tensor.grad(z,x)
diff = th.function([x], der)
print (diff(1))
assert diff(1) == 6

#D
print ("D")
x = th.tensor.fscalar()
y = th.tensor.fscalar()
z = th.tensor.fscalar()
v = x * y * z
grad = th.tensor.grad(v,[x,y,z])
diff = th.function([x,y,z], grad)
print (diff(1,1,1)[0], diff(1,1,1)[1], diff(1,1,1)[2])
assert diff(1,1,1) == [1,1,1]

#E
print ("E")
sp = np.asarray([0.0, 0.0, 0.0])
W = th.shared(sp, 'W')
v = -(W[0] + W[1] + W[2] - 1)**2
grad = th.tensor.grad(v, [W])
delta = 0.1
W_up = W + (delta * grad[0])
lou = [(W, W_up)]
f = th.function([], [W, v], updates = lou)
for _ in range(10):
    output = f()
    print (output)
#Maximum is 0 at (0.33, 0.33, 0.33) (or any point where x+y+z = 1)
    
#F
print ("F")
texts = np.asarray([[1,1], [2,0], [1,0], [0,1], [0,1]])
scores = np.asarray([-2,2,0,1,-1])

W = th.shared(np.asarray([0.0, 0.0]), 'W')
pred = th.tensor.dot(texts, W)
acc = -th.tensor.sqr(pred - scores).sum() + 10
grad = th.tensor.grad(acc, [W])
delta = 0.1
W_up = W + (delta * grad[0])
ups = [(W, W_up)]
f = th.function([], acc, updates = ups)
for _ in range(10):
    output = f()
    print (output)