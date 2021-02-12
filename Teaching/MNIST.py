import numpy as np
import theano as th

#A
print ("A")

input_vector = th.tensor.fvector('input_vector')
target_values = th.tensor.fvector('target_values')
W_initial = np.zeros((5,2))
W = th.shared(W_initial, 'W')
act = th.tensor.dot(W, input_vector)
pred_v = th.tensor.nnet.sigmoid(act)
acc = -th.tensor.sqr(pred_v - target_values).sum()
pred_c = th.tensor.argmax(pred_v)
grad = th.tensor.grad(acc, W)
ups = [(W, W+1*grad)]
train = th.function([input_vector, target_values], [W, act, pred_v, pred_c, acc, grad], updates = ups, allow_input_downcast=True)

data = [1., 0.]
target_vec = [0,0,0,0,1]
W, act, pred_v, pred_c, acc, grad_W = train(data, target_vec)
print ("W: ", W)
print ("Acts: ", act)
print ("Pred vals: ", pred_v)
print ("Pred class: ", pred_c)
print ("Acc: ", acc)
print ("Grad: ", grad_W)

#B
print ("B")

data = [0., 1.]
target_vec = [1,0,0,0,0]
W, act, pred_v, pred_c, acc, grad_W = train(data, target_vec)
print ("W: ", W)
print ("Acts: ", act)
print ("Pred vals: ", pred_v)
print ("Pred class: ", pred_c)
print ("Acc: ", acc)
print ("Grad: ", grad_W)