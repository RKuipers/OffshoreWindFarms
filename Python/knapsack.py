def knapSack(budget, costs, values, n):

	if budget == 0 or n == 0:
		return []
	
	if (costs[n-1] > budget):
		return knapSack(budget, costs, values, n-1)
	
	incl = knapSack(budget - costs[n-1], costs, values, n-1)
	incl.append(n-1)
	excl = knapSack(budget, costs, values, n-1)
	
	ival = values[n-1] + checkVal(incl, values)
	eval = checkVal(excl, values)
	
	if ival > eval:
		return incl
	else:
		return excl
		

def checkVal(indices, values):
	sum = 0
	for i in indices:
		sum += values[i]		
	return sum

print(knapSack(50, [10, 20, 30], [50, 10, 20], 3))