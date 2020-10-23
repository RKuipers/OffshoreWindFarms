import numpy as np

wes = 5
mat = np.zeros((wes, wes))
W = np.ones((wes, wes))
for i in range(wes):
    mat[i][i] = 1
    
print (f"Word Embeddings: {mat}")
    
def rnn(w, c):
    return np.dot(W, c) + w

def map_sec(inp):
    iv = mat[inp]
    cv = np.zeros(wes)
    for x in iv:
        print (f"Context vector: {cv}, Word vector: {x}")
        cv = rnn(x, cv)
    print (f"Final: {cv}")
    
map_sec([0, 1, 2])