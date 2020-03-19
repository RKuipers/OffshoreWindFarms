import numpy
v1 = numpy.asarray([5, 4])
v2 = v1 + 1
print (numpy.dot(v1, v2))
assert (v2[0] == 6)
assert (v2[1] == 5)