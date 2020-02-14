#based on prev year 104
def z(x, y):
    return -(x + y + 1)**2
x = 0
y = 0
delta = 0.1
for _i in range(10):
    #x_derivative = -2
    x_derivative = ( z(x+delta,y)-z(x,y) )/delta
    #y_derivative = -2
    y_derivative = ( z(x,y+delta)-z(x,y) )/delta
    print(_i, x_derivative, y_derivative)
    x = x + .1 * x_derivative
    y = y + .1 * y_derivative
    print (_i, x, y, z(x, y))
