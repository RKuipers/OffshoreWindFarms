import pandas as pd

def openFile(fn, s):
    df = pd.read_excel(fn, s)
    return df.iloc[0:,2]

file_name = r'test.xlsx'
sheet = "Sheet1"

column = openFile(file_name, sheet)

names = []
votes = []

for cell in column:
    games = cell.split(", ")
    for game in games:
        if game in names:
            votes[names.index(game)] += 1
        else:
            names.append(game)
            votes.append(1)
            
for i in range(len(names)):
    print("Name: " + str(names[i]) + " , votes: " + str(votes[i]))