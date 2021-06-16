import math
import numpy as np

#Base parameters
batchInterval = 3
maxAmounts = [360, 672, 720, 360]

#Defining options
duration = [6, 12, 21, 18, 24, 33, 30]
turbines = ['a', 'h', 's', 'b', 'e']
vessels = [[1,1,1,3],[1,0,1,3],[1,0,0,3],[1,0,0,0],[1,1,1,0],[0,0,0,0]]
percentage = [5, 10, 15, 0]
size = [80, 120, 150, 200]

#Names
dNames = ["Su", "Ye", "2SS", "2SL", "2Y", "3SS", "3SL"] #Summer, Year, 2 summers short, 2 summers long, 2 years, 3 summers short, 3 summer long
tNames = ["Al", "Ha", "Se", "Ba", "Eq"]                 #All, Half, Season, Batch, Equal
vNames = ["Al", "NF", "Cr", "CT", "NT", "NO"]           #All, No Field Vessel, Crew (CTV + Technicians), CTV only, No Technicians, None
pNames = list(map(str, percentage))                     #Percentage of time a vessel is available
sNames = list(map(str, size))                           #Number of turbines

#Read in base file
f = open("C:\\Users\\Robin\\OneDrive\\Documenten\\GitHub\\OWFSim\\Code\\Overlap\\Input Files\\yearDinwoodie.dat")
baseLines = f.readlines()

def formatList(l):
    if l[:-1] == l[1:]:
        return "U\t" + str(l[0])
    S = "\t".join(["S"] + list(map(str, l)))
    arr = []
    j = 0
    for i in range(len(l) + 1):
        if i == len(l) or l[i] != l[j]:
            arr = arr + [i - j, l[j]]
            j = i
    A = "\t".join(["A"] + list(map(str, arr)))
    return A if len(A) < len(S) else S

def getSeasons(d):
    if d == 6 or d == 12 or d == 24:
        return [1] * d
    winter = 3 if d == 21 or d == 33 else 6
    summer = 12 - winter
    res = [1] * summer
    while len(res) < d:
        res = res + ([0] * winter) + ([1] * summer)
    return res

def getSeasonPeaks(l):
    peaks = []
    for x in range(len(l)-1):
        if l[x] > 0 and l[x+1] == 0:
            peaks.append(x)
    peaks[0] = 0
    peaks = peaks + [len(l) - 1]
    return [1 if i in peaks else 0 for i in range(len(l))]

def getBatches(l):
    if len(l) % batchInterval != 0:
        print ("Oops BATCHES")
    j = 0
    peaks = [0] * len(l)
    for i in range(len(l)):
        if l[i] > 0:
            j = j + 1
            if j % batchInterval == 0:
                peaks[i] = 1
    peaks[0] = 1
    return peaks

def divTurbs(l, s):
    amount = math.floor(s / sum(l))
    res = [i * amount for i in l]
    i = 1
    while sum(res) < s:
        if (res[-i] > 0):
            res[-i] = res[-i] + 1
        i = i + 1
    return res

def getTurbs(s, t, d):
    if t == 'a':
        return [s] + ([0] * (d - 1))
    if t == 'h':
        return [math.ceil(s / 2)] + ([0] * (d - 2)) + [math.floor(s / 2)]
    if t == 's':
        return divTurbs(getSeasonPeaks(getSeasons(d)), s)
    if t == 'b':
        return divTurbs(getBatches(getSeasons(d)), s)
    if t == 'e':
        return divTurbs(getSeasons(d), s)

def getVessels(d, v, p, m):
    seasons = getSeasons(d)
    amount = [seasons[i] * v for i in range(d)]
    time = [amount[i] * p * 0.01 * m for i in range(d)]
    return time, amount

def getName(l, v, names):
    for i in range(len(l)):
        if l[i] == v:
            return names[i]
        
def setup(s, t, d, v, p):
    lines = baseLines.copy()
    
    #Months
    line = lines[3].split()
    line[1] = str(d)
    lines[3] = "\t".join(line) + "\n"
    
    #Turbines
    turbs = getTurbs(s, t, d)
    lines[38] = formatList(turbs) + "\n"
    
    #Vessels
    for y in range(7, 11):
        vesselsT, vesselsA = getVessels(d, v[y - 7], p, maxAmounts[y - 7])
        lines[y] = lines[y][:-9] + "\t" + formatList(vesselsT) + "\t" + formatList(vesselsA) + "\n"
    
    #Write result
    name = "_".join([getName(turbines, t, tNames), getName(duration, d, dNames), getName(vessels, v, vNames), getName(percentage, p, pNames), getName(size, s, sNames)])
    f = open("Input Files/GeneratedFiles/" + name + ".dat", "w")
    f.write("".join(lines))
    f.close()

for s in size:
    for t in turbines:
        for d in duration:
            if t == 'a':
                if d == 6 or d == 12 or d == 24:
                    setup(s, t, d, [0, 0, 0, 0], 0)
                continue
            if t == 's' and (d == 6 or d == 12 or d == 24):
                continue
            for v in vessels:
                if v == [0, 0, 0, 0]:
                    setup(s, t, d, v, 0)
                    continue
                for p in percentage:
                    if p == 0:
                        continue
                    setup(s, t, d, v, p)