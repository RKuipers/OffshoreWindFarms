#Base parameters
nTurbs = 80
batchInterval = 6

#Defining options
duration = [18, 24, 36]
vessels = [[1,1,1,1],[1,0,1,1],[0,0,1,1],[0,0,1,0]]
amount = [0.05, 0.1, 0.25]
turbines = ['a', 'e', 'r', 'b']

#Read in base file
f = open("C:\\Users\\Robin\\OneDrive\\Documenten\\GitHub\\OWFSim\\Code\\Overlap\\Input Files\\yearDinwoodie.dat")
baseLines = f.readlines()

def inc(x):
    return x + 1

def getTurbs(t, d : int):
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

for d in duration:
    for v in vessels:
        for a in amount:
            for t in turbines:
                lines = baseLines.copy()
                
                #Months
                line = lines[3].split()
                line[1] = str(d)
                lines[3] = "\t".join(line)
                
                #Turbines
                turbs, line = getTurbs(t, d)
                lines[35] = line
                
                #Vessels