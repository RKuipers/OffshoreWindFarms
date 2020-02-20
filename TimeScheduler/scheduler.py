# -*- coding: utf-8 -*-

import datetime as dt

def chop_microseconds(delta):
    return delta - dt.timedelta(microseconds=delta.microseconds)

now = dt.datetime.now()
start = now.time()

print ("What time do you need to be done? (HH:MM)")
endstr = input()
end = dt.datetime.strptime(endstr, "%H:%M").time()
available = dt.datetime.combine(dt.date.today(), end) - dt.datetime.combine(dt.date.today(), start)
available = chop_microseconds(available)
minutes = round(available.seconds / 60)

worku = 86
breaku = 40

blocks = divmod((minutes-worku), (worku + breaku))

print ("Available time: " + str(available) + " (" + str(minutes) + " minutes)")
print ("Full blocks: ", blocks[0] + 1)
print ("Leftover minutes: ", blocks[1])

print ("Press Enter to quit")
_ = input()