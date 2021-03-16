import numpy as np

#A
#Instead of [0,1,2] we add [3,2,1]
#Those numbers represent indices of words
#The corresponding word embeddings are [0,0,0,1,0], [0,0,1,0,0], and [0,1,0,0,0] respectively
#Since the starting context vector is [0,0,0,0,0], adding the above word vectors in order gives
#[0,1,1,1,0]

#B
#Step 1: W * [0,0,0,0,0] + [0,0,0,1,0] = [0,0,0,1,0]
#Step 2: W * [0,0,0,1,0] + [0,0,1,0,0] = [1,1,2,1,1]
#Step 3: W * [1,1,2,1,1] + [0,1,0,0,0] = [6,7,6,6,6]

#C
print("C")
wes = 5
w_emb = np.zeros((5,5))
for i in range(5):
    w_emb[i][i] = 1
print ("word embeddings: \n", w_emb, "\n")
cvs = wes

def rnn_step(word_vector, context_vector):
    #return np.dot(np.ones((5,5)), context_vector) + word_vector
    return np.dot(np.ones((5,5)), context_vector) + word_vector
def map_sequence(input_indices):
    input_vectors = w_emb[input_indices]
    context_vector = np.zeros(cvs)
    for x in input_vectors:
        print ("Context Vector: ", context_vector)
        print ("Word Vector: ", x)
        context_vector = rnn_step(x, context_vector)
        print ("Resulting context vector", context_vector, "\n")
    print ("Final context vector: ", context_vector)

map_sequence([3,2,1])
    