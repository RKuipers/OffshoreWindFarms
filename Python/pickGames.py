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
		
def voteSat(budget, n):
    vals = []
    games = []
    for i in range(0, n):
        vals.append([-1] * (budget+1))
        games.append([[]] * (budget+1))
    
    def m(i, j):
        if j <= 0:
            return 0
        
        if i == 0:
            if prices[i] > j:
                return 0
            else:
                games[i][j] = [names[i]]
                #print("Calling CALCVAL1 on: " + str([names[i]]))
                return calcVal([names[i]], votesets)
        
        if(vals[i-1][j] == -1):
            vals[i-1][j] = m(i-1, j)
        
        chosen = False
        
        if prices[i] > j:
            vals[i][j] = vals[i-1][j]
        else:
            if(vals[i-1][j-prices[i]] == -1):
                vals[i-1][j-prices[i]] = m(i-1, j-prices[i])
            newGameList = games[i-1][j-prices[i]].copy()
            newGameList.append(names[i])
            #print("Calling CALCVAL2 on: " + str(newGameList))
            newVal = calcVal(newGameList, votesets)
            if(vals[i-1][j] > newVal):
                vals[i][j] = vals[i-1][j]
            else:
                vals[i][j] = newVal
                chosen = True
        

        if chosen:
            games[i][j] = newGameList
        else:
            games[i][j] = games[i-1][j].copy()
        return vals[i][j]
    
    m(n-1, budget)
    return games[n-1][budget]

def calcVal(setOfGames, setOfVotes):
    numSat = 0
    for vote in setOfVotes:
        satisfied = 0
        for game in vote:
            if (game in setOfGames):
                satisfied += 1
        if satisfied > 0:
            numSat += 49 + satisfied
    return numSat

def checkVal(indices, values):
	s = 0
	for i in indices:
		s += values[i]		
	return s

def openFile(fn, s):
    df = pd.read_excel(fn, s)
    return df.iloc[0:,2]

def getData(file_name, sheet1, sheet2):
    df = pd.read_excel(file_name, sheet1)
    names.extend([j for i in pd.DataFrame(df, columns= ['Name']).values.tolist() for j in i])
    votes.extend([j for i in pd.DataFrame(df, columns= ['Votes']).values.tolist() for j in i])
    prices.extend([j for i in pd.DataFrame(df, columns= ['Price']).values.tolist() for j in i])
    votesets.extend([i[0].split(', ') for i in pd.read_excel(file_name, sheet2).values.tolist()])
                
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
    #games = knapSackDP(budget, prices, votes, len(names))
    games = voteSat(budget, len(names))
    #print("DP RESULT: " + str([names[i] for i in gamesDP]) + " therefore same? " + str(games == gamesDP))
    print("Budget: " + str(budget) + "...................................................")
    print(str(games))
    print("Total: " + str(sum([prices[names.index(i)] for i in games])))
    print("Satisfied voters: " + str(calcVal(games, votesets)))
    end = dt.datetime.now() - start
    print("Duration: " + str(end))

ra.seed(42)

file_name = r'results.xlsx'
sheet1 = "Sheet1"
sheet2 = "Sheet2"

names = []
votes = []
prices = []
votesets = []
                
getData(file_name, sheet1, sheet2)
prices = [round(x) for x in prices]
for i in range(len(names)):
    print("Name: " + str(names[i]) + " , votes: " + str(votes[i]) + " , price: " + str(prices[i]))

run(50)
run(75)
run(100)
run(125)
run(150)
run(160)
run(165)
run(170)
run(175)
run(180)
run(185)
run(190)
run(200)
run(225)
run(250)
run(500)
