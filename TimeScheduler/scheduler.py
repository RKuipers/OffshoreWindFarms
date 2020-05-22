# -*- coding: utf-8 -*-

import datetime as dt
import numpy as np
import sys

modify = False
#TODO: Set to false/remove
debug = False

class Settings:    
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
    dl = [15, 1, 35, 1, 2]
    dh = [25, 4, 50, 5, 5]
    #"Normal" ranges
    nl = [20, 2, 45, 2, 4]
    nm = [20, 2, 45, 3, 4]
    nh = [20, 2, 45, 4, 4]
    #Target ratios 
    t2 = (160/(2*86 + 45))
    t3 = (240/(3*86 + 90))
    t4 = (320/(4*86 + 135))   
    #Dimension Weights
    dw = [1, 1, 1, 1, 1, 1, 1]
    #Minute range and default
    mr = 15  
    md = 360
    #Ratio range
    rl = 0.6
    rh = 0.75
    
    def __init__(self):
        self.lows = Settings.dl
        self.highs = Settings.dh
        self.target = Settings.t3
        self.weights = Settings.dw
        self.mins = Settings.md
        self.ml = self.mins - Settings.mr
        self.mh = self.mins + Settings.mr
        self.rl = Settings.rl
        self.rh = Settings.rh
        
    def setRanges(self, lows: list, highs: list) -> None:
        self.lows = lows
        self.highs = highs
    
    def setRangesF(self, s: str) -> (list, list):
        if (s == "n"):
            self.lows = Settings.nl
            self.highs = Settings.nh
        else:
            self.lows = Settings.dl
            self.highs = Settings.dh
        return (self.lows, self.highs)
    
    def setTarget(self, f: float) -> None:
        self.target = f
            
    def setTargetF(self, i: int) -> float:
        switcher = {
            2: Settings.t2,
            3: Settings.t3,
            4: Settings.t4
            }
        self.target = switcher.get(i)
        
    def setWeights(self, w: list) -> None:
        self.weights = w
    
    def getMinsWeight(self) -> float:
        return self.weights[5]
        
    def getRatioWeight(self) -> float:
        return self.weights[6]
    
    def setMins1(self, minutes: int) -> None:
        self.mins = minutes        
        self.ml = minutes - Settings.mr
        self.mh = minutes + Settings.mr
        
    def setMins2(self, low: int, high: int) -> None:
        self.ml = low
        self.mh = high
        self.mins = (low + high) / 2
        
    def setMins3(self, minutes: int, low: int, high: int) -> None:
        self.mins = minutes        
        self.ml = low
        self.mh = high

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
        self.score = -1
    
    @staticmethod
    def fromList(l: list) -> None:
        return Solution(l[0], l[1], l[2], l[3], l[4])
    
    @staticmethod
    def fromTuple(t: (int, int, int, int, int)) -> None:
        (B, S, L, N, P) = t
        return Solution(B, S, L, N, P)
    
    def asList(self) -> list:
        return [self.B, self.S, self.L, self.N, self.P]
    
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
    
    def ratioLong(self) -> int:
        return self.workTime() / (self.workTime() + self.L * (self.N - 1))
    
    #Checks validity
    def valid(self, S: Settings) -> bool:
        return S.ml <= self.length() <= S.mh and S.rl <= self.ratio() <= S.rh
    
    def setScore(self, score :float) -> None:
        self.score = score
    
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
    def printOutput(self, available: dt.timedelta, minutes: int) -> None:
        global S
        settings = S
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
        
class ICompare:
    def __init__(self, S: Settings):
        self.settings = S
        
    def compare(self, s1: Solution, s2: Solution) -> Solution:
        if (not s1.valid(self.settings)):            
            return s2
        if (not s2.valid(self.settings)):            
            return s1 
        
        if (s1.score == -1):
            self.calcScore(s1)            
        if (s2.score == -1):
            self.calcScore(s2)
        
        if s1.score < s2.score:
            return s1
        elif s1.score == s2.score:
            return self.tieBreaker(s1, s2)
        else:
            return s2
    
    def calcScore(self, s: Solution) -> float:
        pass
    
    def tieBreaker(self, s1: Solution, s2: Solution) -> Solution:
        pass
    
class RatioCompare(ICompare):
    def calcScore(self, s: Solution) -> float:
        penalty = 1 + (pow(self.settings.mins - s.length(), 1.5)/100)
        val = (abs(self.settings.target - s.ratioLong()) + penalty / 100) * penalty
        s.setScore(val)
        return val
    
    def tieBreaker(self, s1: Solution, s2: Solution) -> Solution:
        global debug
        if debug:
            print ("TIE")
            
        if s1.S < s2.S:
            return s1
        else:
            return s2

class DistanceCompare(ICompare):
    def calcAttributeScore(v: int, n: int, l: int, h: int, w: float) -> float:
        dis = v - n
        if (dis > 0):
            rang = h - n
        else:                
            rang = l - n
        if rang == 0:
            print(f"RANGE 0, {v}, {n}, {l}, {h}, {w}")
            return 0
        normDis = dis / rang
        return normDis * w
    
    def calcScore(self, s: Solution) -> float:
        val = DistanceCompare.calcAttributeScore(s.length(), self.settings.mins, self.settings.ml, self.settings.mh, self.settings.getMinsWeight())
        val += DistanceCompare.calcAttributeScore(s.ratio(), self.settings.target, self.settings.rl, self.settings.rh, self.settings.getRatioWeight())
        sol = s.asList()
        for i in range(5):
            val += DistanceCompare.calcAttributeScore(sol[i], Settings.nm[i], self.settings.lows[i], self.settings.highs[i], self.settings.weights[i])
        
        s.setScore(val)
        return val
    
    def tieBreaker(self, s1: Solution, s2: Solution) -> Solution:
        global debug
        if debug:
            print ("TIE")
            
        if s1.S < s2.S:
            return s1
        else:
            return s2

S = Settings()
comparer = DistanceCompare(S)

def chop_microseconds(delta: dt.timedelta) -> dt.timedelta:
    return delta - dt.timedelta(microseconds=delta.microseconds)

def inpStr2min(inpstr: str, start: dt.datetime.time) -> int():
    end = dt.datetime.strptime(inpstr, "%H:%M").time()            
    available = dt.datetime.combine(dt.date.today(), end) - dt.datetime.combine(dt.date.today(), start)
    available = chop_microseconds(available)
    mins = round(available.seconds / 60)
    return mins

def getInput() -> dt.timedelta: 
    global S
    
    now = dt.datetime.now()

    if not debug:
        print ("What time do you need to be done? (HH:MM)")
        endstr = input()
        start = now.time()
        if endstr.count(":") == 1:
            S.setMins1(inpStr2min(endstr, start))
            
        elif endstr.count(":") == 2:
            print ("Please explain what you have just entered:")
            print ("(1) Ranges for the end time")
            print ("(2) A start and an end time")
            choice = input()
            if (choice == "1"): 
                S.setMins2(inpStr2min(endstr.split(" ")[0], start), inpStr2min(endstr.split(" ")[1], start))
            elif (choice == "2"):
                start = dt.datetime.strptime(endstr.split(" ")[0], "%H:%M").time()
                mins = inpStr2min(endstr.split(" ")[1], start)
                S.setMins1(mins)
        elif endstr.count(":") == 3:
            S.setMins3(inpStr2min(endstr.split(" ")[1], start), inpStr2min(endstr.split(" ")[0], start), inpStr2min(endstr.split(" ")[2], start))
        else:
            print ("Invalid Input") #ERROR
            return getInput()
    else: 
        print ("Enter available minutes (q to exit)")
        mins = input()
        if (mins == "q"):
            sys.exit()
        available = dt.timedelta(minutes = int(mins))
        S.setMins1(round(available.seconds / 60))
        
    return dt.timedelta(minutes = S.mins)

def modifySettings() -> None:
    global S
    
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
            lows.append(Settings.nl[i])
            highs.append(Settings.nh[i])
        elif (inp == "N"):
            lows.extend(Settings.nl[i:])
            highs.extend(Settings.nh[i:])
            break
        elif (inp == "d"):
            lows.append(Settings.dl[i])
            highs.append(Settings.dh[i])
        elif (inp == "D"):
            lows.extend(Settings.dl[i:])
            highs.extend(Settings.dh[i:])
            break
        elif (inp == "k"):
            lows.append(S.lows[i])
            highs.append(S.highs[i])
        elif (inp == "K"):
            lows.extend(S.lows[i:])
            highs.extend(S.highs[i:])
            break
        elif " " in inp:
            (l, h) = inp.split()
            lows.append(int(l))
            highs.append(int(h))
        else:            
            lows.append(int(inp))
            highs.append(int(inp))
            
    S.setRanges(lows, highs)
    
    print ("Enter 2, 3, or 4 to set the target ratio to the corresponding normal ratio, or enter a float to fix the target.")
    
    inp = input()
    if "." in inp:
        S.setTarget(float(inp))
    else:
        S.setTargetF(int(inp))

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

def calc(comp: ICompare, lows: list, highs: list, target: float) -> Solution:    
    ranges = [y - x + 1 for x,y in zip(lows, highs)]
    options = np.prod(ranges)
    
    noSol = True
    bestSol = Solution(-1, -1, -1, -1, -1)
    
    for i in range(options):
        sol = getvars(i, lows, ranges)
        if (not sol.valid(comp.settings)):
            continue
        if (noSol):
            bestSol = sol
            noSol = False
        else:
            bestSol = comp.compare(sol, bestSol)
               
    return bestSol

def main():  
    global modify
    global S
    while True:
        if modify:
            modifySettings()
        
        if debug:
            print (f"Lows {S.lows}")
            print (f"Highs {S.highs}")
            print (f"Target {S.target}")
        
        available = getInput()
        
        sol = calc(comparer, S.lows, S.highs, S.target)

        sol.printOutput(available, S.mins)

        if reRun(sol):
            break
        
if __name__ == '__main__':    
    main()