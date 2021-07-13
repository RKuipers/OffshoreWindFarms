import random
import itertools

output = []

fd = open("geo-dutch.txt")
dutch = fd.readlines()
print(dutch[0])
print(len(dutch))

with open("geo-train-unique-spelled.txt") as f:
    l = 0
    for line in f:
        words = line.split() + ['</s>']
        i = 1
        for w in words[0:]:
            output.append(w + "\t" + dutch[l][:-1] +  ' <s> '  + ' '.join(words[:i-1]))
            i += 1
        l += 1

print(len(output))

random.shuffle(output)

test = open("test-translate.txt", "w")
train = open("train-translate.txt", "w")

for line in output[:1000]:
    test.write(line + "\n")
test.close()

for line in output[1000:]:
    train.write(line + "\n")
train.close()