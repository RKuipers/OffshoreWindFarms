import random
import theano
import collections
import numpy

#Total number of classes we need to map our sequences to.
n_words = 400 #Total number of words we can support: 200 sufficient for our task since NLVR vocabulary is small
n_classes = n_words # Since we are predicting a word to generate based on already generated words

input_indices = theano.tensor.ivector('input_indices') #the indices of the words already generated
target_class = theano.tensor.iscalar('target_class') #here is it the ID of the next word to generate

#All words in the language are represented as trainable vectors:
word_embedding_size = 10    #the size of those vectors

rng = numpy.random.RandomState(0)
def random_matrix(num_rows, num_columns):
    return numpy.asarray(rng.normal(loc=0.0, scale=0.1, size=(num_rows, num_columns)))

word_embeddings = theano.shared(random_matrix(n_words, word_embedding_size), 'word_embeddings')
input_vectors = word_embeddings[input_indices]
recurrent_size = 10
W_xr = theano.shared(random_matrix(recurrent_size, word_embedding_size), 'W_xr')
W_hr = theano.shared(random_matrix(recurrent_size, recurrent_size), 'W_hr')
W_xz = theano.shared(random_matrix(recurrent_size, word_embedding_size), 'W_xz')
W_hz = theano.shared(random_matrix(recurrent_size, recurrent_size), 'W_hz')
W_x = theano.shared(random_matrix(recurrent_size, word_embedding_size), 'W_x')
W_h = theano.shared(random_matrix(recurrent_size, recurrent_size), 'W_h')

def rnn_step(x, h_prev, W_xr, W_hr, W_xz, W_hz, W_x, W_h): #implements GRU
    r = theano.tensor.nnet.sigmoid(theano.tensor.dot(W_xr, x) + theano.tensor.dot(W_hr, h_prev))
    z = theano.tensor.nnet.sigmoid(theano.tensor.dot(W_xz, x) + theano.tensor.dot(W_hz, h_prev))
    _h = theano.tensor.tanh(theano.tensor.dot(W_x, x) + theano.tensor.dot(W_h, r * h_prev))
    return z * h_prev + (1.0-z) * _h

context_vector, other_info = theano.scan(
            rnn_step,
            sequences = input_vectors,
            outputs_info=numpy.zeros(recurrent_size),
            non_sequences = [W_xr, W_hr, W_xz, W_hz, W_x, W_h]
        )
context_vector = context_vector[-1]

W_output = theano.shared(random_matrix(n_classes, recurrent_size), 'W_output')
activations = theano.tensor.dot(W_output, context_vector)

predicted_class = theano.tensor.argmax(activations)
output = theano.tensor.nnet.softmax(activations)[0]
cost = -theano.tensor.log(output[target_class]) #We use cross-entropy: It works better with multiple classes
learning_rate = .003
updates = [
    #make sure all the new trainable parameters added here:
    (word_embeddings, word_embeddings - learning_rate*theano.tensor.grad(cost, word_embeddings)),
    (W_output, W_output - learning_rate*theano.tensor.grad(cost, W_output)),
    (W_xr, W_xr - learning_rate*theano.tensor.grad(cost, W_xr)),
    (W_hr, W_hr - learning_rate * theano.tensor.grad(cost, W_hr)),
    (W_xz, W_xz - learning_rate*theano.tensor.grad(cost, W_xz)),
    (W_hz, W_hz - learning_rate * theano.tensor.grad(cost, W_hz)),
    (W_x, W_x - learning_rate*theano.tensor.grad(cost, W_x)),
    (W_h, W_h - learning_rate * theano.tensor.grad(cost, W_h))
]

train = theano.function([input_indices, target_class], [cost, predicted_class], updates=updates, allow_input_downcast = True)
test = theano.function([input_indices, target_class], [cost, predicted_class], allow_input_downcast = True)

def read_dataset(path):
    """Read a dataset: same format as in shuffle detection template. But some asserts are added to verify the integrity of the data intended for text generation.
    """
    dataset = []
    max_word_count = 0
    with open(path, "r") as f:
        for line in f:
            assert len(line) >= 4
            assert '\t' in line
            line_parts = line.strip().split("\t")
            assert len(line_parts) == 2
            assert len(line_parts[0]) > 0
            assert len(line_parts[1]) > 2
            if max_word_count < len(line_parts[1].split()):
                max_word_count = len(line_parts[1].split())
            #We don't add <s> and </s> as in shuffling detection, because they are now included in our datasets
            #We verify that just in case by using assert statements
            assert line_parts[0] == "<s>" or "<s>" in line_parts[1]
            dataset.append((line_parts[0], line_parts[1]))
            if "</s>" in line_parts[0]: #We also expect </s> to be included in some inputs
                sentenceEndFound = True

    assert sentenceEndFound
    assert max_word_count > 10
    return dataset

id2word  = [] #Mapping back from word ID-s to actual words
def create_dictionary(sentences): #this is indexing! we need to convert all words to their ID-s
    counter = collections.Counter() #Python's class that can count
    for sentence in sentences:
        for word in sentence:
            counter.update([word])
    word2id = collections.OrderedDict() #Python's class that can map words to ID-s
    word2id["<unk>"] = 0    #We reserve this for "uknown words" that we may encounter in the future
    word2id["<s>"] = 1 #Marks beginning of the sentence
    word2id["</s>"] = 2 #Marks the end of the sentence
    id2word.append("<unk>")
    id2word.append("<s>")
    id2word.append("</s>")
    word_count_list = counter.most_common() #For every word, we create an entry in  'word2id'
    for (word, count) in word_count_list: #so it can map them to their ID-s
            if word not in word2id: #this check important for consistency of mapping
                word2id[word] = len(word2id)
                id2word.append(word)

    for word in word2id: #Verifying that our mapping between words and their id-s is consistent
        assert id2word[word2id[word]] == word

    return word2id

def sentence2ids(words, word2id): #Converts a word sequence (sentence) into a list of ID-s
    #Note that we don't add <s> and </s> here anymore since they are already included in our data
    ids = []
    for word in words:
        if word in word2id:
            ids.append(word2id[word])
        else:
            ids.append(word2id["<unk>"])
    return ids


path_train = "train-translate.txt"
path_test = "test-translate.txt"
sentences_train = read_dataset(path_train)
sentences_test = read_dataset(path_test)
print ("Data read completed")

word2id = create_dictionary([sentence.split() for label, sentence in sentences_train+sentences_test])

#Convereting from text to word ID-s:
data_train = [(word2id[label], sentence2ids(sentence.split(), word2id))
              for label, sentence in sentences_train]
data_test = [(word2id[label], sentence2ids(sentence.split(), word2id))
             for label, sentence in sentences_test]

random.shuffle(data_train) #useful thing to do

def generate_sentence(starting_text): #The function that generates some text based on trained LM and supplied starting string
    number_of_words_generated = 0
    already_generated_text = "<s> " + starting_text + " "
    already_generated_IDs = sentence2ids(already_generated_text.split(), word2id)
    while  True:
              cost, id_to_generate = test(already_generated_IDs, 0)  #any class label ok to give here, so we are giving 0
              number_of_words_generated += 1
              if id_to_generate == word2id["</s>"] or number_of_words_generated > 100 or id_to_generate >= len(id2word):
                   break
              word = id2word[id_to_generate] #convert the id of the next word to generate back to the word
              already_generated_IDs.append(id_to_generate)
              already_generated_text += word + " "
    print (already_generated_text)

print ("Starting epochs")
for epoch in range(100):
        cost_sum = 0.0
        correct = 0
        count = 0
        for target_class, sentence in data_train:
            count += 1 
            cost, predicted_class = train(sentence, target_class)
            cost_sum += cost
            if predicted_class == target_class:
                correct += 1
            if count % 100 == 0:
                print("count: ", count)
        print("Epoch: " + str(epoch) + "\tAverage Cost: " + str(cost_sum/count) + "\tAccuracy: " + str(float(correct)/count))
            #Now we report the cost averaged across the training examples
        cost_sum2 = 0.0
        correct2 = 0
        for target_class, sentence in data_test:
              cost, predicted_class = test(sentence, target_class)
              cost_sum2 += cost
              if predicted_class == target_class:
                correct2 += 1
        print ("\t\t\t\t\t\t\tTest_cost: " + str(cost_sum2/len(data_test)) + "\tTest_accuracy: " + str(float(correct2)/len(data_test)))
