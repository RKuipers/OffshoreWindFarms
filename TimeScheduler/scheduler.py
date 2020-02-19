# -*- coding: utf-8 -*-

from datetime import datetime
from datetime import date

now = datetime.now()
start = now.time()

print ("What time do you need to be done? (HH:MM)")
endstr = input()
end = datetime.strptime(endstr, "%H:%M").time()
available = datetime.combine(date.today(), end) - datetime.combine(date.today(), start)
minutes = available.seconds / 60

worku = 86
breaku = 40

blocks = divmod((minutes-worku), (worku + breaku))
print ("Full blocks: ", blocks[0] + 1)
print ("Leftover minutes: ", blocks[1])