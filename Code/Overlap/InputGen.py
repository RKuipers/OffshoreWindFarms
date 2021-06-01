#Base parameters
nTurbs = 80
batchInterval = 6
avgPercent = 10.0
prog = [2.0, 1.0, 0.75, 0.5, 0.75, 1.0]
maxAmounts = [360, 672, 720, 360]

#Defining options
duration = [18, 24, 36]
vessels = [[1,1,1,1],[1,0,1,1],[1,0,0,1],[1,0,0,0],[1,1,1,0],[0,0,0,0]]
pattern = ['c', 'w', 'b','n']
turbines = ['a', 'e', 'r', 'b']
mode = ['m', 'o']

#Names
dNames = ["18", "24", "36"]                   #Number of months
vNames = ["Al", "NF", "Cr", "CT", "NT", "NO"] #All, No Field Vessel, Crew (CTV + Technicians), CTV only, No Technicians, None
pNames = ["Co", "Wa", "Bi", "No"]             #Constant, Wave, Binary, None
tNames = ["Ha", "Eq", "Ra", "Ba"]             #Half, Equal, Ramp, Batch
mNames = ["Ma", "Ov"]                         #Maintenance, Overlap

#Read in base file
f = open("C:\\Users\\Robin\\OneDrive\\Documenten\\GitHub\\OWFSim\\Code\\Overlap\\Input Files\\yearDinwoodie.dat")
baseLines = f.readlines()

def inc(x):
    return x + 1

def getTurbs(t, d):
    if (t == 'a'):
        s = "\t".join(["A\t1", str(int(nTurbs / 2)), str(d-2), "0\t1", str(int(nTurbs / 2))])
        l = [int(nTurbs / 2)] + ([0] * (d - 2)) + [int(nTurbs / 2)]
        return l, s
    if (t == 'e'):
        amount = int(nTurbs / d)
        leftover = nTurbs % d
        s = "\t".join(["A", str(d-1), str(amount), "1", str(amount + leftover)])
        l = ([amount] * (d-1)) + [amount + leftover]
        return l, s
    if (t == 'r'):
        monthsLeft = d
        amount = 1
        amountLeft = nTurbs
        block = 3 if d == 18 else 4 if d == 24 else 6
        res = []
        while monthsLeft * amount < amountLeft and monthsLeft > 0:
            res = res + ([amount] * block)
            amountLeft = amountLeft - (block * amount)
            amount = amount + 1
            monthsLeft = monthsLeft - block
        res = res + ([amount-1] * monthsLeft)
        diff = nTurbs - sum(res)       
        res = res[:-diff] + list(map(inc, res[-diff:]))
        return (res, "\t".join(["S"] + list(map(str, res))))
    if (t == 'b'):
        batches = int(d / batchInterval)+1
        batchSize = int(nTurbs / batches)
        leftover = nTurbs % batches
        s = ["A"] + (["1", str(batchSize), str(batchInterval - 1), "0"] * (batches - 1)) + ["1", str(batchSize + leftover)]
        s[-4] = str(batchInterval - 2)
        s = "\t".join(s)
        l = ([batchSize] + ([0] * (batchInterval - 1))) * batches
        l[-1] = batchSize + leftover
        return l, s
    print ("oops")
    return "oops"

def getSeq(p, d, t, turbs):
    if p == 'c':
        return [avgPercent] * d
    if p == 'n':
        return [0] * d
    peaks = []
    if t == 'b':
        peaks = [i for i in range(d) if turbs[i] != 0]
    else:
        peaks = [i for i in range(d) if i % batchInterval == 0]
    if p == 'b':
        return [(avgPercent if i+1 in peaks else 0) for i in range(d)]
    return [(avgPercent * prog[(i - peaks[0] + 1) % batchInterval]) for i in range(d)]

def seq2Lines(lines, v, seq):
    res = lines.copy()
    for i in range(4):
        if v[i] == 0:
            continue
        amount = "U\t1"
        if 0 in seq:
            amount = "\t".join(["S"] + ["1" if x > 0 else "0" for x in seq])
        dur = "\t".join(["S"] + [str(maxAmounts[i] * s * 0.01) for s in seq])
        res[i] = lines[i][:-9] + "\t" + dur + "\t" + amount + "\n"
    return res

def getName(l, v, names):
    for i in range(len(l)):
        if l[i] == v:
            return names[i]

for m in mode:
    for v in vessels:
        for p in pattern:
            if (p == 'n') != (v == [0,0,0,0]) or (p == 'n') != (m == 'm'):
                continue            
            for d in duration:
                for t in turbines:
                    lines = baseLines.copy()
                    
                    #Months
                    line = lines[3].split()
                    line[1] = str(d)
                    lines[3] = "\t".join(line) + "\n"
                    
                    #Turbines
                    turbs, line = getTurbs(t, d)
                    lines[38] = line + "\n"
                    
                    #Vessels
                    seq = getSeq(p, d, t, turbs)
                    lines[7:11] = seq2Lines(lines[7:11], v, seq)
                    
                    #Write result
                    name = "_".join([getName(mode, m, mNames), getName(vessels, v, vNames), getName(pattern, p, pNames), getName(turbines, t, tNames), getName(duration, d, dNames)])
                    f = open("Input Files/GeneratedFiles/" + name + ".dat", "w")
                    f.write("".join(lines))
                    f.close()