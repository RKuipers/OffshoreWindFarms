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
    targets = {
    2: (160/(2*86 + 45)),
    3: (240/(3*86 + 90)),
    4: (320/(4*86 + 135)) 
    }
    #Dimension Weights (6th is Mins, 7th is Ratio)
    dw = [1, 2, 1, 1, 1.5, 1.5, 3]
    #Minute range and default
    mr = 15  
    md = 360
    #Ratio range
    rl = 0.6
    rh = 0.8
    
    def __init__(self):
        self.lows = Settings.dl
        self.highs = Settings.dh
        self.target = Settings.targets.get(3)
        self.weights = Settings.dw
        self.mins = Settings.md
        self.ml = self.mins - Settings.mr
        self.mh = self.mins + Settings.mr
        self.rl = Settings.rl
        self.rh = Settings.rh
        
    def setRanges(self, lows: list, highs: list, startIndex: int = 0) -> None:
        self.lows[startIndex:] = lows
        self.highs[startIndex:] = highs
        
    def setRange(self, index: int, low: int, high: int) -> None:
        self.lows[index] = low
        self.highs[index] = high
    
    def setRatioTarget(self, f: float) -> None:
        self.target = f
    
    def setRatioValues(self, l: float, t: float, h: float) -> None:
        self.rl = l
        self.target = t
        self.rh = h
        
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
        global SETS
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
        elif (dis < 0):                
            rang = l - n
        else:
            return 0
        normDis = dis / rang
        score = pow(2, (1 + normDis))
        return score * w
    
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

SETS = Settings()
comparer = DistanceCompare(SETS)

def chop_microseconds(delta: dt.timedelta) -> dt.timedelta:
    return delta - dt.timedelta(microseconds=delta.microseconds)

def inpStr2min(inpstr: str, start: dt.datetime.time) -> int():
    end = dt.datetime.strptime(inpstr, "%H:%M").time()            
    available = dt.datetime.combine(dt.date.today(), end) - dt.datetime.combine(dt.date.today(), start)
    available = chop_microseconds(available)
    mins = round(available.seconds / 60)
    return mins

def getInput() -> dt.timedelta: 
    global SETS
    
    now = dt.datetime.now()

    if not debug:
        print ("What time do you need to be done? (HH:MM)")
        endstr = input()
        start = now.time()
        if endstr.count(":") == 1:
            SETS.setMins1(inpStr2min(endstr, start))
            
        elif endstr.count(":") == 2:
            print ("Please explain what you have just entered:")
            print ("(1) Ranges for the end time")
            print ("(2) A start and an end time")
            choice = input()
            if (choice == "1"): 
                SETS.setMins2(inpStr2min(endstr.split(" ")[0], start), inpStr2min(endstr.split(" ")[1], start))
            elif (choice == "2"):
                start = dt.datetime.strptime(endstr.split(" ")[0], "%H:%M").time()
                mins = inpStr2min(endstr.split(" ")[1], start)
                SETS.setMins1(mins)
        elif endstr.count(":") == 3:
            SETS.setMins3(inpStr2min(endstr.split(" ")[1], start), inpStr2min(endstr.split(" ")[0], start), inpStr2min(endstr.split(" ")[2], start))
        else:
            print ("Invalid Input") #ERROR
            return getInput()
    else: 
        print ("Enter available minutes (q to exit)")
        mins = input()
        if (mins == "q"):
            sys.exit()
        available = dt.timedelta(minutes = int(mins))
        SETS.setMins1(round(available.seconds / 60))
        
    return dt.timedelta(minutes = SETS.mins)

def modifySettings() -> None:
    global SETS
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
            SETS.setRange(i, Settings.nl[i], Settings.nh[i])
        elif (inp == "N"):
            SETS.setRanges(Settings.nl[i:], Settings.nh[i:], i)
            break
        elif (inp == "d"):
            SETS.setRange(i, Settings.dl[i], Settings.dh[i])
        elif (inp == "D"):
            SETS.setRanges(Settings.dl[i:], Settings.dh[i:], i)
            break
        elif (inp == "k"):
            continue
        elif (inp == "K"):
            break
        elif " " in inp:
            (l, h) = inp.split()
            SETS.setRange(i, int(l), int(h))
        else:
            SETS.setRange(i, int(inp), int(inp))    
    
    print ("Enter 2, 3, or 4 to set the target ratio to the corresponding normal ratio, or enter a float to fix the target.")
    print ("Enter three values to serve as the low, target and high ratios respectively")
    
    inp = input()
    if not inp == "k":
        if " " in inp:
            (l, t, h) = inp.split()
            if not "." in l:
                l = Settings.targets.get(int(l))
            if not "." in t:
                l = Settings.targets.get(int(t))
            if not "." in h:
                l = Settings.targets.get(int(h))
            SETS.setRatioValues(float(l), float(t), float(h))
        elif "." in inp:
            SETS.setRatioTarget(float(inp))
        else:
            SETS.setRatioTarget(Settings.targets.get(int(inp)))
        
    
    print ("Enter the weights as seven values separated by spaces")
    print ("Block Short Long Number Pomos Mins Ratio")
    print (SETS.weights)
    
    inp = ""
    while inp.count(" ") < 6:
        if inp == "k":
            break
        if not inp == "":
            print ("Invalid input")
        inp = input()
    
    if not inp == "k":
        spl = inp.split(" ")
        SETS.setWeights(list(map(float, spl)))

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
    
    print(comp.calcScore(bestSol))        
    
    return bestSol

def main():  
    global modify
    global SETS
    while True:
        if modify:
            modifySettings()
        
        if debug:
            print (f"Lows {SETS.lows}")
            print (f"Highs {SETS.highs}")
            print (f"Target {SETS.target}")
        
        available = getInput()
        
        sol = calc(comparer, SETS.lows, SETS.highs, SETS.target)

        sol.printOutput(available, SETS.mins)

        if reRun(sol):
            break
        
if __name__ == '__main__':    
    main()