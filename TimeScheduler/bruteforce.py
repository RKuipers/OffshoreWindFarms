def calc(minutes: int) -> (int, int):
    lows = [15, 1, 30, 1, 3] #Inclusive
    highs = [26, 4, 51, 5, 6] #Exclusive    
    
    best = 0
    bestset = [-1, -1, -1, -1, -1]
    
    for B in range(lows[0], highs[0]):  
        for S in range(lows[1], highs[1]):  
            for L in range(lows[2], highs[2]):  
                for N in range(lows[3], highs[3]):
                    for P in range(lows[4], highs[4]):
                        pomoblock = P * B
                        fullblock = pomoblock + (P-1) * S
                        program = N * fullblock + (N-1) * L
                        
                        worktime = N * P * B
                        
                        ratio = worktime / program
                        
                        if program <= minutes and ratio <= 2/3 and worktime > best:
                            best = worktime
                            bestset = [B, S, L, N, P]

    print (bestset)
    
    #TODO: Everything after this can be better, pretty sure  the first result of the divmod is N
    #Should deffo print something about ratio too
    
    worku = bestset[0] * bestset[4] + bestset[1] * (bestset[4]-1)
    breaku = bestset[2]

    return divmod((minutes + breaku), worku + breaku)
