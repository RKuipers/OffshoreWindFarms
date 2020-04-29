import numpy as np

#Magic function that generates the Rth set of variables
def getvars(R: int, lows: list, ranges: list) -> list:
    res = []
    
    for i in range(len(lows)):
        (x, R) = divmod(R, int(np.prod(ranges[(i+1):]))) #Arcane magic, based on flexible number systems
        res.append(x + lows[i])
        
    return res

def calc(minutes: int, lows: list, highs: list) -> (int, int):    
    ranges = [y - x + 1 for x,y in zip(lows, highs)]
    options = np.prod(ranges)
    
    bestWT = 0
    bestFull = 0
    bestset = [-1, -1, -1, -1, -1]
    
    for i in range(options):
        [B, S, L, N, P] = getvars(i, lows, ranges)
        
        pomoblock = P * B
        fullblock = pomoblock + (P-1) * S
        program = N * fullblock + (N-1) * L
        
        worktime = N * P * B
        
        ratio = worktime / program
        
        if program <= minutes and ratio <= 3/4:
            if worktime > bestWT:
                bestWT = worktime
                bestFull = program
                bestset = [B, S, L, N, P]
            elif worktime == bestWT and bestFull < program:                
                bestFull = program
                bestset = [B, S, L, N, P]

    return bestset