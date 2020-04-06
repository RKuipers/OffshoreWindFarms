import theano
import numpy

word_embedding_size = 5
input_indices = theano.tensor.ivector("input_indices")

vals = numpy.zeros((5, 5))
for i in range(5):
    vals[i][i] = 1
print("word_embeddings: \n", vals)
word_embeddings = theano.shared(vals, 'word_embeddings')
input_vectors = word_embeddings[input_indices]
recurrent_size = 5
W_x = theano.shared(numpy.ones((recurrent_size, word_embedding_size)), "W_x")

def rnn_step(x, h_prev, W_x):
    return theano.tensor.dot(W_x, h_prev) + x

context_vector, other_info = theano.scan(
    rnn_step,
    sequences=input_vectors,
    outputs_info=numpy.zeros(recurrent_size),
    non_sequences=[W_x]
)

context_vector = context_vector[-1]

map_sequence = theano.function([input_indices],
                               [input_vectors, context_vector],
                               updates=[])

vectors, context = map_sequence([0, 1, 2])
print("input_vectors: \n", vectors, " are mapped into context vector: \n", context)