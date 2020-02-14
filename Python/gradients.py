def z(x, y):
    return -(x + y + 1)**2
    #return (x + 2*y + 1)**2

def xder(x, y, delta):
    return (z(x+delta,y)-z(x,y))/delta
    #return -2 * (x + y + 1)

def yder(x, y, delta):
    return (z(x,y+delta)-z(x,y))/delta
    #return -2 * (x + y + 1)

def GA(x, y, d, pm):    
    for _i in range(10):
        print(_i, xder(x, y, d), yder(x, y, d))
        
        xd = xder(x, y, d)
        yd = yder(x, y, d)
        
        x = x + pm * d * xd
        y = y + pm * d * yd
        
        print(_i, x, y, z(x, y))

        #step = max(x - x, y - y)
        #if abs(step) <= 0.000001:
            #break

def GAA(x, y, d, depth=0):
    xd = xder(d)
    yd = yder(d)
    xn = x + (xd * d)
    yn = y + (yd * d)
    print ('Depth: ' +  str(depth) + ', Value: ' + str(z(xn,yn)) + ' , XY: (' + str(xn) + ', ' + str(yn) + ')')
    if depth < 50:
        GAA(xn, yn, d, depth + 1)


GA(0, 0, 0.1, 1)