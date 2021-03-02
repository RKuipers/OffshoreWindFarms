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
input_vector2 = th.tensor.fvector('input_vector')
target_values2 = th.tensor.fvector('target_values')
W2 = th.shared(np.zeros((5,2)), 'W2')
act2 = th.tensor.dot(W2, input_vector2)
pred_v2 = th.tensor.nnet.sigmoid(act2)
acc2 = -th.tensor.sqr(pred_v2 - target_values2).sum()
pred_c2 = th.tensor.argmax(pred_v2)
grad2 = th.tensor.grad(acc2, W2)
ups2 = [(W2, W2+1*grad2)]
train2 = th.function([input_vector2, target_values2], [W2, act2, pred_v2, pred_c2, acc2, grad2], updates = ups2, allow_input_downcast=True)


data2 = [0., 1.]
target_vec2 = [1,0,0,0,0]
W2, act2, pred_v2, pred_c2, acc2, grad_W2 = train2(data2, target_vec2)
print ("W: ", W2)
print ("Acts: ", act2)
print ("Pred vals: ", pred_v2)
print ("Pred class: ", pred_c2)
print ("Acc: ", acc2)
print ("Grad: ", grad_W2)