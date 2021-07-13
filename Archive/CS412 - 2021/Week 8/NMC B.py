import random

output = []
with open("geo-train-unique-spelled.txt") as f:
    for line in f:
        words = ['<s>'] + line.split() + ['</s>'] 
        i = 1
        for w in words[1:]:
            output.append(w + "\t" + ' '.join(words[:i]))
            i += 1

random.shuffle(output)

test = open("geo-test-lm.txt", "w")
train = open("geo-train-lm.txt", "w")

for line in output[:1000]:
    test.write(line + "\n")
test.close()

for line in output[1000:]:
    train.write(line + "\n")
train.close()