import numpy as np

word_embedding_size = 5
word_embeddings = np.zeros((5, 5))

for i in range(5):
    word_embeddings[i][i] = 1
print("word_embeddings: \n", word_embeddings)
contextVectorSize = word_embedding_size
W = np.ones(contextVectorSize, word_embedding_size)

def rnn_step_add(word_vector, context_vector):
    return np.dot(W, context_vector) + word_vector

def map_sequence_add(input_indices):
    input_vectors = word_embeddings[input_indices]
    context_vector = np.zeros(contextVectorSize)
    for x in input_vectors:
        print("context vector:", context_vector)
        print("word_vector", x)
        context_vector = rnn_step_add(x, context_vector)
    print("final context vector:", context_vector)

print("model: adding to the existing context vector")
map_sequence_add([0, 1, 2])
