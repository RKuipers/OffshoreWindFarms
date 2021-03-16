# The template for the lab with Neural Memory Control (gates)
import numpy
word_embedding_size = 5 #for simulation only: in real applications it is 50+
recurrent_size = word_embedding_size #using the same size to avoid complexity

#We use one-hot encoding for the word embeddings so it will be easier to recognize them:
word_embeddings = numpy.zeros((5,5))
for i in range(5):
    word_embeddings[i][i] = 1
print ("word_embeddings:\n", word_embeddings)

#various trainable matrices possibly involved in the gates:
W_xr = numpy.zeros((recurrent_size, word_embedding_size)) #using all 0s to keep the numbers simple
W_hr = numpy.zeros((recurrent_size, recurrent_size))
W_xz = numpy.zeros((recurrent_size, word_embedding_size))
W_hz = numpy.zeros((recurrent_size, recurrent_size))


def sigmoid(x): #Use this only in simulation, don't use in Theano!
    return 1./(1.+numpy.exp(-x))
def tanh(x):  #Use this only in simulation, don't use in Theano!
    return (numpy.exp(x) - numpy.exp(-x))/(numpy.exp(x) + numpy.exp(-x))

def rnn_step(x, h_prev): #Note in Theano you need to pass the trainable matrics here as well
    #Adding vectors as in previous RNN exercise, but now with a forget gate
    r = sigmoid(numpy.dot(W_xr, x) + numpy.dot(W_hr, h_prev))
    return x + r * h_prev

def map_sequence(input_indices): #follows same outline as Theano model
    input_vectors = word_embeddings[input_indices]
    context_vector = numpy.zeros(recurrent_size)
    for x in input_vectors:
        print ("context vector:", context_vector, "word vector:", x)
        context_vector = rnn_step(x, context_vector) #function to update 'context_vector'
    print ("final context vector:", context_vector)

map_sequence([0,1,2])
