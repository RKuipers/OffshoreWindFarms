# -*- coding: utf-8 -*-

import datetime as dt
import bruteforce

#TODO: Set to false/remove
debug = True

def chop_microseconds(delta: dt.timedelta) -> dt.timedelta:
    return delta - dt.timedelta(microseconds=delta.microseconds)

def getInput() -> (dt.timedelta, int):
    now = dt.datetime.now()
    start = now.time()

    print ("What time do you need to be done? (HH:MM)")
    if not debug:
        endstr = input()
    else:
        endstr = "18:00"
    end = dt.datetime.strptime(endstr, "%H:%M").time()
    available = dt.datetime.combine(dt.date.today(), end) - dt.datetime.combine(dt.date.today(), start)
    available = chop_microseconds(available)
    
    return (available, round(available.seconds / 60))

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

def main():
    while True:
        (available, minutes) = getInput()
    
        res = tuple(bruteforce.calc(minutes))

        printOutput(available, minutes, res)

        if debug:
            break

        print ("Press Enter to run again or q to quit")
        inp = input()
        if inp == "q" or inp == "Q":
            break

if __name__ == '__main__':
    main()