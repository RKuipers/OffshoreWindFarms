def z(x, y):
    return -(2*x - y + 1)**2
def dz_dx(x, y):
    return -2*2*(2*x - y + 1)
def dz_dy(x, y):
    return -2*(-1)*(2*x - y + 1)
x = 0
y = 0
r = .05 #learning rate #0.01 - .2 give several steps, other values too slow or too quick
delta = 0.001
print(x, y, z(x, y))
for _ in range(10):
	#print(dz_dx(x, y), (z(x + delta, y) - z(x, y)) / delta, dz_dy(x, y), (z(x, y + delta) - z(x, y)) / delta)
	x_derivative = dz_dx(x, y)
	y_derivative = dz_dy(x, y)
	x = x + r * x_derivative
	y = y + r * y_derivative
	print (x, y, z(x, y))
