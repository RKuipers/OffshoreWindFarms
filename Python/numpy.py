import numpy
v1 = numpy.asarray([3, 4])
v2 = v1 + 1
print (numpy.dot(v1, v2))
assert (v2[0] == 4)
assert (v2[1] == 5)