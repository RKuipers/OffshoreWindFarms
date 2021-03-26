import random
import itertools

output = []
with open("geo-train-unique-spelled.txt") as f:
    for line in f:
        if (len(line.split()) > 10):
            continue
        perms = list(itertools.permutations(line.split()))
        random.shuffle(perms)
        shuffles = perms[:10]
        for shuff in shuffles:
            words = line.split() + ['</s>']
            i = 1
            for w in words[0:]:
                output.append(w + "\t" + ' '.join(shuff) + ' <s> ' + ' '.join(words[:i-1]))
                i += 1
                
print(len(output))

random.shuffle(output)

test = open("test-shuffle.txt", "w")
train = open("train-shuffle.txt", "w")

for line in output[:1000]:
    test.write(line + "\n")
test.close()

for line in output[1000:]:
    train.write(line + "\n")
train.close()