# -*- coding: utf-8 -*-

import datetime as dt
import bruteforce
import sys

modify = False
#TODO: Set to false/remove
debug = False
autoRerun = True

def chop_microseconds(delta: dt.timedelta) -> dt.timedelta:
    return delta - dt.timedelta(microseconds=delta.microseconds)

def getInput() -> (dt.timedelta, int, list, list):
    if(modify):
        (l, h) = modifyRanges()
    else:
        (l, h) = getRanges("")
    
    now = dt.datetime.now()
    start = now.time()

    if not debug:
        print ("What time do you need to be done? (HH:MM)")
        endstr = input()
        end = dt.datetime.strptime(endstr, "%H:%M").time()
        available = dt.datetime.combine(dt.date.today(), end) - dt.datetime.combine(dt.date.today(), start)
        available = chop_microseconds(available)
    else: 
        print ("Enter available minutes (q to exit)")
        mins = input()
        if (mins == "q"):
            sys.exit()
        available = dt.timedelta(minutes = int(mins))
        
    return (available, round(available.seconds / 60), l, h)

def printOutput(available: dt.timedelta, minutes: int, res: (int, int, int, int, int)):
    (B, S, L, N, P) = res
        
    program = N * (P * B + (P-1) * S) + (N-1) * L   
    wt = N * B * P
    
    print (f"Available time: {available} ({minutes} minutes)")
    print (f"Block length: {B}m")
    print (f"Full blocks: {N} ({P} pomos each)")
    print (f"Breaks: {L}m long, {S}m short")
    print (f"Worktime: {wt}, Breaktime: {program - wt}, Ratio: {wt/program}")
    print (f"Leftover minutes: {minutes - program}")
    if debug:
        print (f"B:{B} N:{N} P:{P} L:{L} S:{S}")

#Returns (inclusive) ranges for the following 5 values:
#B: Length of a work block in minutes
#S: Short break length in minutes
#L: Long break length in minutes
#N: Number of blocks (full repeats)
#P: Number of work blocks (pomodoros) before a long break
def getRanges(s: str) -> (list, list):
    if (s == "n"):
        lows = [20, 2, 45, 2, 4]
        highs = [20, 2, 45, 4, 4]
    else:
        lows = [15, 1, 30, 1, 3]
        highs = [25, 4, 50, 5, 5]
    return (lows, highs)
        
def modifyRanges() -> (list, list):
    lows = []
    highs = []
    vs = ["B (block length)", "S (short break length)", "L (long break length)", "N (number of blocks)", "P (number of pomodoros per block)"]
    (bl, bh) = getRanges("n")
    
    print ("For each of the following variables enter a range (L H), fixed value (F), or enter \"n\" to fix it to the normal value. Enter \"N\" to fix all remaining variables to their normal values.")
    
    for i in range(5):
        print (vs[i])
        inp = input()
        
        if (inp == "n"):
            lows.append(bl[i])
            highs.append(bh[i])
        elif (inp == "N"):
            for j in range(i, 5):
                lows.append(bl[j])
                highs.append(bh[j])
            break
        elif " " in inp:
            (l, h) = inp.split()
            lows.append(int(l))
            highs.append(int(h))
        else:            
            lows.append(int(inp))
            highs.append(int(inp))
    return (lows, highs)

def main():    
    global modify
    while True:
        (available, minutes, l, h) = getInput()
        
        res = tuple(bruteforce.calc(minutes, l, h))

        printOutput(available, minutes, res)

        if debug:
            if not autoRerun:
                break
            else:
                continue

        print ("Press Enter to run again, m to modify, or q to quit")
        inp = input()
        if inp == "q" or inp == "Q":
            break
        modify = inp == "m" or inp == "M"
        
if __name__ == '__main__':    
    main()