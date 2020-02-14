#based on prev year 104
def z(x, y):
    return -(x + y + 1)**2
x = 0
y = 0
delta = 0.000001
#x_derivative = -2
x_derivative = ( z(x+delta,y)-z(x,y) )/delta
#y_derivative = -2
y_derivative = ( z(x,y+delta)-z(x,y) )/delta
x = x - delta * x_derivative
y = y - delta * y_derivative
print (x, y, z(x, y))