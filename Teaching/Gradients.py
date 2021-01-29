#A

def z(x, y):
    return (x + 2 * y + 1)**2
    
x = 0
y = 0
delta = 0.1
print ("z(0,0):" , z(0,0))
print ("z(delta,0):" , z(delta,0))
print ("z(-delta,0):" , z(-delta,0))
print ("z(0,delta):" , z(0,delta))
print ("z(0,-delta):" , z(0,-delta))

x_der = (z(delta,0)-z(0,0)) / delta
print ("delta:", delta, "x_derivative:", x_der)

delta = 0.01
x_der = (z(delta,0)-z(0,0)) / delta
print ("delta:", delta, "x_derivative:", x_der)

delta = 0.001
x_der = (z(delta,0)-z(0,0)) / delta
print ("delta:", delta, "x_derivative:", x_der)

delta = 0.0001
x_der = (z(delta,0)-z(0,0)) / delta
print ("delta:", delta, "x_derivative:", x_der)

delta = 0.1
y_der = (z(0,delta)-z(0,0)) / delta
print ("delta:", delta, "y_derivative:", y_der)

delta = 0.01
y_der = (z(0,delta)-z(0,0)) / delta
print ("delta:", delta, "y_derivative:", y_der)

delta = 0.001
y_der = (z(0,delta)-z(0,0)) / delta
print ("delta:", delta, "y_derivative:", y_der)

delta = 0.0001
y_der = (z(0,delta)-z(0,0)) / delta
print ("delta:", delta, "y_derivative:", y_der)

#B
#3*x -> 3
#3*x + 2 -> 3
#x^4 -> 4*x^3 -> 4
#(2*x+3)^3 ->6*(2*x+3)^2 -> 150

delta = 0.0001

print ("Comparing analytical and numerical differentiation")
print ("3*x")
def f1(x):
    return 3*x
def f1p(x):
    return 3
x = 1
f1_der = (f1(x + delta) - f1(x))/delta
print ("x:", x, "delta:", delta, "numerical:", f1_der, "analytical:", f1p(x))

print ("3*x + 2")
def f2(x):
    return 3*x + 2
def f2p(x):
    return 3
x = 1
f2_der = (f2(x + delta) - f2(x))/delta
print ("x:", x, "delta:", delta, "numerical:", f2_der, "analytical:", f2p(x))

print ("x^4")
def f3(x):
    return x**4
def f3p(x):
    return 4*(x**3)
x = 1
f3_der = (f3(x + delta) - f3(x))/delta
print ("x:", x, "delta:", delta, "numerical:", f3_der, "analytical:", f3p(x))

print ("(2*x+3)^3")
def f4(x):
    return (2*x+3)**3
def f4p(x):
    return 6*((2*x+3)**2)
x = 1
f4_der = (f4(x + delta) - f4(x))/delta
print ("x:", x, "delta:", delta, "numerical:", f4_der, "analytical:", f4p(x))

#C
delta = 0.001

print ("Comparing analytical and numerical differentiation")
print ("z(x) = 3 * x^2")
def v1(x):
    return 3 * x**2
def v1x(x):
    return 6 * x
x = 1
v1x_der = (v1(x + delta) - v1(x))/delta
print ("x:", x, "delta:", delta, "numerical:", v1x_der, "analytical:", v1x(x))

print ("z(x,y) = x * y^2")
def v2(x, y):
    return x * y**2
def v2x(x, y):
    return y ** 2
def v2y(x, y):
    return 2 * x * y
x = 1
y = 1
v2x_der = (v2(x + delta, y) - v2(x, y))/delta
v2y_der = (v2(x, y + delta) - v2(x, y))/delta
print ("x:", x, "y:", y, "delta:", delta, "numerical:", v2x_der, v2y_der, "analytical:", v2x(x,y), v2y(x,y))

print ("v(x,y,z) = x * y * z")
def v3(x, y, z):
    return x * y * z
def v3x(x, y, z):
    return y * z
def v3y(x, y, z):
    return x * z
def v3z(x, y, z):
    return x * y
x = 1
y = 1
z = 1
v3x_der = (v3(x + delta, y, z) - v3(x, y, z))/delta
v3y_der = (v3(x, y + delta, z) - v3(x, y, z))/delta
v3z_der = (v3(x, y, z + delta) - v3(x, y, z))/delta
print ("x:", x, "y:", y, "z:", z, "delta:", delta, "numerical:", v3x_der, v3y_der, v3z_der, "analytical:", v3x(x,y, z), v3y(x,y,z), v3z(x,y,z))