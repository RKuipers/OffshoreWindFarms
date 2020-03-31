import theano
import numpy
word_embedding_size = 5 #the size of those vectors
input_indices = theano.tensor.ivector('input_indices')

#We set our embeddings here to one-hot vectors, so we can reasily recognize them
vals = numpy.zeros((5,5))
for i in range(5):
    vals[i][i] = 1
print ("word_embeddings:\n", vals)
word_embeddings  = theano.shared(vals, 'word_embeddings')
input_vectors = word_embeddings[input_indices]
recurrent_size = 5
W_h = theano.shared(numpy.ones((recurrent_size, word_embedding_size)), 'W_h')

def rnn_step(x, h_prev, W_x):
    return theano.tensor.dot(W_h, h_prev) + x

context_vector, other_info = theano.scan(
            rnn_step,
            sequences = input_vectors,
            outputs_info=numpy.zeros(recurrent_size),
            non_sequences = [W_h]
        )
context_vector = context_vector[-1]

map_sequence = theano.function([input_indices], #inputs
                               [input_vectors, context_vector], #outputs
                               updates=[])

vectors, context = map_sequence([0,1,2]) #should print [1,1,1,0,0]
print ("input vectors:\n",vectors, " are mapped into the context vector:\n",context)

