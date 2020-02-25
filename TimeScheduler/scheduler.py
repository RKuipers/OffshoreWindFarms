# -*- coding: utf-8 -*-

import datetime as dt

#TODO: Set to false/remove
debug = True

def chop_microseconds(delta):
    return delta - dt.timedelta(microseconds=delta.microseconds)

def getInput():
    now = dt.datetime.now()
    start = now.time()

    print ("What time do you need to be done? (HH:MM)")
    if not debug:
        endstr = input()
    else:
        endstr = "19:00"
    end = dt.datetime.strptime(endstr, "%H:%M").time()
    available = dt.datetime.combine(dt.date.today(), end) - dt.datetime.combine(dt.date.today(), start)
    available = chop_microseconds(available)
    
    return (available, round(available.seconds / 60))

def calc(minutes):
    worku = 86
    breaku = 40

    return divmod((minutes-worku), (worku + breaku))

def printOutput(available, minutes, blocks):
    print ("Available time: " + str(available) + " (" + str(minutes) + " minutes)")
    print ("Full blocks: ", blocks[0] + 1)
    print ("Leftover minutes: ", blocks[1])

def main():
    while True:
        (available, minutes) = getInput()
    
        blocks = calc(minutes)

        printOutput(available, minutes, blocks)

        if debug:
            break

        print ("Press Enter to run again or q to quit")
        inp = input()
        if inp == "q" or inp == "Q":
            break

if __name__ == '__main__':
    main()