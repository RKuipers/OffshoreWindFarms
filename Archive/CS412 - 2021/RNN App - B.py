import random
import theano
import collections
import numpy
#Total number of classes we need to map our sequences to.
n_classes = 2 # For example two classes: positive or negative (or true/false)
# We can change that later when needed
n_words = 10000 # Total number of words we can support - here we are just guessing, we can get it from data later
#We represent the input sequence as a vector of integers (word id-s):
input_indices = theano.tensor.ivector('input_indices') #word sequence input
#We want to predict:
target_class = theano.tensor.iscalar('target_class') #e.g. could be sentiment level
#All words in the language are represented as trainable vectors:
word_embedding_size = 10    #the size of those vectors

rng = numpy.random.RandomState(0)
def random_matrix(num_rows, num_columns): #initilizes a matrix with random numbers around 0 - this is better than all 0-s
    return numpy.asarray(rng.normal(loc=0.0, scale=0.1, size=(num_rows, num_columns)))
word_embeddings = theano.shared(random_matrix(n_words, word_embedding_size), 'word_embeddings')

W0 = theano.shared(random_matrix(n_classes, word_embedding_size), 'W0')
W1 = theano.shared(random_matrix(n_classes, word_embedding_size), 'W1')
W2 = theano.shared(random_matrix(n_classes, word_embedding_size), 'W2')

#This represents the input sequence (e.g. a sentence):
input_vectors = word_embeddings[input_indices]

activations = theano.tensor.dot(W0, input_vectors[0]) + theano.tensor.dot(W1, input_vectors[1]) + theano.tensor.dot(W2, input_vectors[2])

predicted_class = theano.tensor.argmax(activations)
output = theano.tensor.nnet.softmax(activations)[0]
cost = -theano.tensor.log(output[target_class]) #We use cross-entropy: It works better with multiple classes
learning_rate = .1
updates = [ #now reducing cost so using -, not +
    (word_embeddings, word_embeddings - learning_rate*theano.tensor.grad(cost, word_embeddings)),
    (W0, W0 - learning_rate*theano.tensor.grad(cost, W0)),
    (W1, W1 - learning_rate*theano.tensor.grad(cost, W1)),
    (W2, W2 - learning_rate*theano.tensor.grad(cost, W2))
]

train = theano.function([input_indices, target_class], [cost, predicted_class], updates=updates, allow_input_downcast = True)
test = theano.function([input_indices, target_class], [cost, predicted_class], allow_input_downcast = True)

print("Initialization completed")

def read_dataset(path): #reading text data in the simple format: integer score, followed by a tab, followed by the text sequence
    dataset = []
    with open(path, "r") as f:
        for line in f:
         line_parts = line.strip().split("\t")
         if(len(line_parts) > 1): #convering to low case and adding spaces between any punctuation:
            s1 = line_parts[1].lower().replace(',', ' , ').replace(';',' ; ').replace(':',' : ').replace('"',' " ').replace("'"," ' ").replace("-"," - ").replace("("," ( ").replace(")"," ) ").replace("/"," / ").replace("?"," ? ").replace("!"," ! ").replace("."," . ")
            dataset.append((int(line_parts[0]), s1))
    return dataset

def create_dictionary(sentences): #this is indexing! we need to convert all words to their ID-s
    counter = collections.Counter() #Python's class that can count
    for sentence in sentences:
        for word in sentence:
            counter.update([word])

    word2id = collections.OrderedDict() #Python's class that can map words to ID-s
    word2id["<unk>"] = 0    #We reserve this for "uknown words" that we may encounter in the future
    word2id["<s>"] = 1 #Marks beginning of the sentence
    word2id["</s>"] = 2 #Marks the end of the sentence

    word_count_list = counter.most_common() #For every word, we create an entry in  'word2id'
    for (word, count) in word_count_list: #so it can map them to their ID-s
            word2id[word] = len(word2id)

    return word2id

def sentence2ids(words, word2id): #Converts a word sequence (sentence) into a list of ID-s
    ids = [word2id["<s>"],] #marks beginning of the sentence
    for word in words:
        if word in word2id:
            ids.append(word2id[word])
        else:
            ids.append(word2id["<unk>"])
    ids.append(word2id["</s>"]) #marks the end of the sentence
    return ids

print("")
print("Takes ages to run for me; on VM it runs quickly")
print("Template itself already has right output, so pay attention to that.")
print("Template gives 87%; basic implementation of 3rd word gives 95%")
print("Things to look for in marking: new matrix, added to activations, list of updates")
print("")

path_train = "train-shuffle.txt"
path_test = "test-shuffle.txt"

sentences_train = read_dataset(path_train)
sentences_test = read_dataset(path_test)
print("Data read")
word2id = create_dictionary([sentence.split() for label, sentence in sentences_train])
n_words = len(word2id)  # Important to set it c
assert n_words < 10000
data_train = [(score, sentence2ids(sentence.split(), word2id)) for score, sentence in sentences_train] #here we need to convert
data_test = [(score, sentence2ids(sentence.split(), word2id)) for score, sentence in sentences_test] #our data from text to the lists of ID-s
print("Data processed")

#The rest of the code is similar to the MNIST task:

for epoch in range(10):
        cost_sum = 0.0
        correct = 0
        count = 0
        for target_class, sentence in data_train:
            count += 1 
            cost, predicted_class = train(sentence, target_class)
            cost_sum += cost
            if predicted_class == target_class:
                correct += 1
        print ("Epoch: " + str(epoch) + "\tCost: " + str(cost_sum) + "\tAccuracy: " + str(float(correct)/count))
        cost_sum2 = 0.0
        correct2 = 0
for target_class, sentence in data_test:
    cost, predicted_class = test(sentence, target_class)
    cost_sum2 += cost
    if predicted_class == target_class:
                    correct2 += 1
print ("")
print ("Test_cost: " + str(cost_sum2) + "\tTest_accuracy: " + str(float(correct2)/len(data_test)))