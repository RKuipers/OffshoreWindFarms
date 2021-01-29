import numpy as np

#A
#[1,2] * [3,4] = 3 + 8 = 11

#B
v1 = np.asarray([1,2])
v2 = np.asarray([3,4])
print (np.dot(v1, v2))
assert (np.dot(v1, v2) == 11)

#C
#[1,2] * [1,1] = 1 + 2 = 3
#[3,4] * [1,1] = 3 + 4 = 7
#[5,6] * [1,1] = 5 + 6 = 11

#D
W = np.asarray([[1,2],[3,4],[5,6]])
vd = np.asarray([1,1])
print (np.dot(W, vd))
assert (np.dot(W, vd)[0] == 3)
assert (np.dot(W, vd)[1] == 7)
assert (np.dot(W, vd)[2] == 11)

#E
#[3,4] + 1 = [4,5]

#F
vf = np.asarray([3,4])
print (vf + 1)
assert ((vf + 1)[0] == 4)
assert ((vf + 1)[1] == 5)