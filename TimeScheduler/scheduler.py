# -*- coding: utf-8 -*-

import datetime as dt
import numpy as np
import sys

modify = False
#TODO: Set to false/remove
debug = False

class Input:    
    #The Low/High ranges are (in order) for the following 5 values
    #B: Length of a work block in minutes
    #S: Short break length in minutes
    #L: Long break length in minutes
    #N: Number of blocks (full repeats)
    #P: Number of work blocks (pomodoros) before a long break

    #From this the following values can be calcuated
    #WorkTime = B * P * N
    #PomoBlock = B * P + S * (P-1)
    #TotalLength = PomoBlock * N + L * (N-1)
    #Ratio = WorkTime / TotalLength
    
    #Default ranges
    dl = [15, 1, 30, 1, 2]
    dh = [25, 4, 50, 5, 5]
    #"Normal" ranges
    nl = [20, 2, 45, 2, 4]
    nh = [20, 2, 45, 4, 4]
    #Target ratios 
    t2 = (160/(2*86 + 45))
    t3 = (240/(3*86 + 90))
    t4 = (320/(4*86 + 135))    
    
    def __init__(self):
        self.lows = Input.dl
        self.highs = Input.dh
        self.target = Input.t3
        
    def setRanges(self, lows: list, highs: list) -> None:
        self.lows = lows
        self.highs = highs
    
    def setRangesF(self, s: str) -> (list, list):
        if (s == "n"):
            self.lows = Input.nl
            self.highs = Input.nh
        else:
            self.lows = Input.dl
            self.highs = Input.dh
        return (self.lows, self.highs)
    
    def setTarget(self, f: float) -> None:
        self.target = f
            
    def setTargetF(self, i: int) -> float:
        switcher = {
            2: Input.t2,
            3: Input.t3,
            4: Input.t4
            }
        self.target = switcher.get(i)

I = Input()    

class Solution:
    #B: Length of a work block in minutes
    #S: Short break length in minutes
    #L: Long break length in minutes
    #N: Number of blocks (full repeats)
    #P: Number of work blocks (pomodoros) before a long break

    def __init__(self, B: int, S: int, L: int , N: int, P: int):
        self.B = B
        self.S = S
        self.L = L
        self.N = N
        self.P = P
    
    @staticmethod
    def fromList(l: list):
        return Solution(l[0], l[1], l[2], l[3], l[4])
    
    @staticmethod
    def fromTuple(t: (int, int, int, int, int)):
        (B, S, L, N, P) = t
        return Solution(B, S, L, N, P)
    
    def asTuple(self) -> (int, int, int, int, int):
        return (self.B, self.S, self.L, self.N, self.P)

    #From this the following values can be calcuated
    #WorkTime = B * P * N
    #PomoBlock = B * P + S * (P-1)
    #TotalLength = PomoBlock * N + L * (N-1)
    #Ratio = WorkTime / TotalLength
    
    def workTime(self) -> int:
        return self.B * self.P * self.N
    
    def pomoBlock(self) -> int:
        return self.B * self.P + self.S * (self.P-1)
    
    def length(self) -> int:
        return self.pomoBlock() * self.N + self.L * (self.N-1)
    
    def ratio(self) -> int:
        return self.workTime() / self.length()
    
    #Return a string to put into pomodoro-timers
    def pomo(self) -> str:
        res = ""
        for n in range(self.N):
            if n > 0:
                res = res + "+" + str(self.L)
            for p in range(self.P):
                if p > 0:
                    res = res + "+" + str(self.S)
                res = res + "+" + str(self.B)
        return res[1:]
    
    #Print the values to the console
    def printOutput(self, available: dt.timedelta, minutes: int):
        (B, S, L, N, P) = self.asTuple()
        wt = self.workTime()
        length = self.length()
        
        print (f"Available time: {available} ({minutes} minutes)")
        print (f"Block length: {self.B}m")
        print (f"Full blocks: {N} ({P} pomos each)")
        print (f"Breaks: {L}m long, {S}m short")
        print (f"Worktime: {wt}, Breaktime: {length - wt}, Ratio: {wt/length}")
        print (f"Leftover minutes: {minutes - length}")
        if debug:
            print (f"B:{B} N:{N} P:{P} L:{L} S:{S}") 
        
def chop_microseconds(delta: dt.timedelta) -> dt.timedelta:
    return delta - dt.timedelta(microseconds=delta.microseconds)

def getInput() -> (dt.timedelta, int):    
    now = dt.datetime.now()

    if not debug:
        print ("What time do you need to be done? (HH:MM)")
        endstr = input()
        if endstr.count(":") == 1:
            start = now.time()
            end = dt.datetime.strptime(endstr, "%H:%M").time()
        elif endstr.count(":") == 2:
            start = dt.datetime.strptime(endstr.split(" ")[0], "%H:%M").time()
            end = dt.datetime.strptime(endstr.split(" ")[1], "%H:%M").time()
        else:
            print ("Invalid Input") #ERROR
            endstr = input()
        available = dt.datetime.combine(dt.date.today(), end) - dt.datetime.combine(dt.date.today(), start)
        available = chop_microseconds(available)
    else: 
        print ("Enter available minutes (q to exit)")
        mins = input()
        if (mins == "q"):
            sys.exit()
        available = dt.timedelta(minutes = int(mins))
        
    return (available, round(available.seconds / 60))

def modifySettings() -> None:
    global I
    
    lows = []
    highs = []
    vs = ["B (block length)", "S (short break length)", "L (long break length)", "N (number of blocks)", "P (number of pomodoros per block)"]
    
    print ("For each of the following variables enter a range (L H),")
    print ("or a fixed value (F).")
    print ("Enter \"n\" to fix it to the normal value.")
    print ("Enter \"d\" to fix it to the default value.")
    print ("Enter \"k\" to keep it to the current value.")
    print ("Enter a capital letter to do that action for all remaining values")
    
    for i in range(5):
        print (vs[i])
        inp = input()
        
        if (inp == "n"):
            lows.append(Input.nl[i])
            highs.append(Input.nh[i])
        elif (inp == "N"):
            for j in range(i, 5):
                lows.append(Input.nl[j])
                highs.append(Input.nh[j])
            break
        elif (inp == "d"):
            lows.append(Input.dl[i])
            highs.append(Input.dh[i])
        elif (inp == "D"):
            for j in range(i, 5):
                lows.append(Input.dl[j])
                highs.append(Input.dh[j])
            break
        elif (inp == "k"):
            lows.append(I.lows[i])
            highs.append(I.highs[i])
        elif (inp == "K"):
            for j in range(i, 5):
                lows.append(I.lows[j])
                highs.append(I.highs[j])
            break
        elif " " in inp:
            (l, h) = inp.split()
            lows.append(int(l))
            highs.append(int(h))
        else:            
            lows.append(int(inp))
            highs.append(int(inp))
            
    I.setRanges(lows, highs)
    
    print ("Enter 2, 3, or 4 to set the target ratio to the corresponding normal ratio, or enter a float to fix the target.")
    
    inp = input()
    if "." in inp:
        I.setTarget(float(inp))
    else:
        I.setTargetF(int(inp))

def reRun(sol: Solution) -> bool:
    global modify
    
    print ("Press Enter to run again, m to modify, or q to quit")
    inp = input()
    if inp == "pomo" or inp == "Pomo":
        print (sol.pomo())
        return reRun(sol)
    modify = inp == "m" or inp == "M"
    return inp == "q" or inp == "Q"

def getvars(R: int, lows: list, ranges: list) -> Solution:
    #Magic function that generates the Rth set of variables
    res = []
    
    for i in range(len(lows)):
        (x, R) = divmod(R, int(np.prod(ranges[(i+1):]))) #Arcane magic, based on flexible number systems
        res.append(x + lows[i])
        
    return Solution.fromList(res)

def calc(minutes: int, lows: list, highs: list, target: float) -> (int, int):    
    ranges = [y - x + 1 for x,y in zip(lows, highs)]
    options = np.prod(ranges)
    
    noSol = True
    bestVal = 1.0
    bestFull = 0
    bestSol = Solution(-1, -1, -1, -1, -1)
    
    for i in range(options):
        sol = getvars(i, lows, ranges)
        
        if sol.length() <= minutes and 0.6 <= sol.ratio() <= 0.75: #Check validity     
            penalty = 1 + (pow(minutes - sol.length(), 1.5)/100) #Incorporates leftover minutes into objective
            val = (abs(target - sol.ratio()) + penalty / 100) * penalty
            #Objective:
            if noSol or (val <= bestVal or (val == bestVal and sol.length() > bestFull)):
                bestVal = val
                bestFull = sol.length()
                bestSol = sol
                noSol = False         

    return bestSol

def main():  
    global modify
    global I
    while True:
        if modify:
            modifySettings()
        
        if debug:
            print (f"Lows {I.lows}")
            print (f"Highs {I.highs}")
            print (f"Target {I.target}")
        
        (available, minutes) = getInput()
        
        sol = calc(minutes, I.lows, I.highs, I.target)

        sol.printOutput(available, minutes)

        if reRun(sol):
            break
        
if __name__ == '__main__':    
    main()