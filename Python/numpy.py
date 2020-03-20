import numpy
v1 = numpy.asarray([1, 2])
v2 = numpy.asarray([3, 4])
print (numpy.dot(v1, v2))

W = numpy.asarray([[1, 2], [3, 4], [5, 6]])
v3 = numpy.asarray([1, 1])
print (numpy.dot(W, v3))

v4 = v2 + 1
print (v4)