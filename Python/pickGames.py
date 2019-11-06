import pandas as pd
import random as ra
import datetime as dt

def knapSack(budget, costs, values, n):

    if budget == 0 or n == 0:
        return []
	
    if (costs[n-1] > budget):
        return []
	
    incl = knapSack(budget - costs[n-1], costs, values, n-1)
    incl.append(n-1)
    excl = knapSack(budget, costs, values, n-1)
	
    iVal = checkVal(incl, values)
    eVal = checkVal(excl, values)
    
    if iVal > eVal:
        return incl
    else:
        return excl
    
def knapSackDP(budget, costs, values, n):
    vals = []
    for i in range(0, n):
        vals.append([-1] * (budget+1))
            
    def m(i, j):
        if j <= 0:
            return 0
        
        if i == 0:
            if costs[i] > j:
                return 0
            else:
                return values[i]
        
        if(vals[i-1][j] == -1):
            vals[i-1][j] = m(i-1, j)
        
        if costs[i] > j:
            vals[i][j] = vals[i-1][j]
        else:
            if(vals[i-1][j-costs[i]] == -1):
                vals[i-1][j-costs[i]] = m(i-1, j-costs[i])
            vals[i][j] = max(vals[i-1][j], vals[i-1][j-costs[i]] + values[i])
    
        return vals[i][j]
    
    def backtrack(i, j):
        if i == 0:
            if costs[i] > j:
                return []
            else:
                return [i]
        
        if vals[i][j] == vals[i-1][j]:
            return backtrack(i-1, j)
        if vals[i][j] == vals[i-1][j-costs[i]] + values[i]:
            rest = backtrack(i-1, j-costs[i])
            rest.append(i)
            return rest
        else:
            print("PANIC1")
            return []
    
    m(n-1, budget)
    result = backtrack(n-1, budget)
    
    return result
		

def checkVal(indices, values):
	s = 0
	for i in indices:
		s += values[i]		
	return s

def openFile(fn, s):
    df = pd.read_excel(fn, s)
    return df.iloc[0:,2]

def getData(file_name, sheet):
    column = openFile(file_name, sheet)

    for cell in column:
        games = cell.split(", ")
        for game in games:
            if game in names:
                votes[names.index(game)] += 1
            else:
                price = ra.randrange(15, 50)
                ind = insort(prices, price)
                names.insert(ind, game)
                votes.insert(ind, 1)
                
def insort(l, x):
    i = 0
    while i < len(l) and l[i] > x:
        i += 1
    l.insert(i, x)
    return i

def run(budget):
    print("")
    start = dt.datetime.now()   
    #games = knapSack(budget, prices, votes, len(names))
    games = knapSackDP(budget, prices, votes, len(names))
    #print("DP RESULT: " + str([names[i] for i in gamesDP]) + " therefore same? " + str(games == gamesDP))
    print("Budget: " + str(budget) + "...................................................")
    print(str([names[i] for i in games]))
    print("Total: " + str(sum([prices[i] for i in games])))
    end = dt.datetime.now() - start
    print("Duration: " + str(end))


ra.seed(42)

file_name = r'test.xlsx'
sheet = "Sheet1"

names = []
votes = []
prices = []
                
getData(file_name, sheet)
for i in range(len(names)):
    print("Name: " + str(names[i]) + " , votes: " + str(votes[i]) + " , price: " + str(prices[i]))

run(50)
run(75)
run(100)
run(150)
run(200)
run(250)
run(300)
run(500)
run(750)
