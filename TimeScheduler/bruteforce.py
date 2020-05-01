import numpy as np

#Magic function that generates the Rth set of variables
def getvars(R: int, lows: list, ranges: list) -> list:
    res = []
    
    for i in range(len(lows)):
        (x, R) = divmod(R, int(np.prod(ranges[(i+1):]))) #Arcane magic, based on flexible number systems
        res.append(x + lows[i])
        
    return res

def calc(minutes: int, lows: list, highs: list, target: float) -> (int, int):    
    ranges = [y - x + 1 for x,y in zip(lows, highs)]
    options = np.prod(ranges)
    
    noSol = True
    bestVal = 1.0
    bestFull = 0
    bestSet = [-1, -1, -1, -1, -1]
    
    for i in range(options):
        [B, S, L, N, P] = getvars(i, lows, ranges)
        
        pomoblock = P * B
        fullblock = pomoblock + (P-1) * S
        program = N * fullblock + (N-1) * L
        
        worktime = N * P * B
        
        ratio = worktime / program
        
        if program <= minutes and 0.6 <= ratio <= 0.75: #Check validity     
            penalty = 1 + (pow(minutes - program, 1.5)/100) #Incorporates leftover minutes into objective
            val = (abs(target - ratio) + penalty / 100) * penalty
            #Objective:
            if noSol or (val <= bestVal or (val == bestVal and program > bestFull)):
                bestVal = val
                bestFull = program
                bestSet = [B, S, L, N, P]    
                noSol = False         

    return bestSet