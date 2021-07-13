import random

output = []
oneCount = 0
zeroCount = 0
with open("test.json") as f:
    for line in f:
        idSt = line.find("identifier") + 13
        idEn = line.find("\"", idSt)
        ident = line[idSt:idEn]
        
        shapesSt = line.find("[[")
        shapes = line[shapesSt:].replace("],[", "},{").split("},{")
        label = 0
        for shape in shapes:
            if shape.find("triangle") != -1 and shape.find("#0099ff") != -1:
                label = 1
                break
        if (label == 0):
            zeroCount += 1
        else:
            oneCount += 3
        
        inp = open("all/test-" + ident + "-0.txt")
        pixels = inp.readlines()[0][21:].split(", ")
        inp.close()
        pixels[-1] = pixels[-1][:-3]
        
        output.append(str(label) + "\t" + ident + "\t" + ' '.join(pixels))
        if label == 1:
            output.append(str(label) + "\t" + ident + "\t" + ' '.join(pixels))
            output.append(str(label) + "\t" + ident + "\t" + ' '.join(pixels))

testsize = int(len(output) / 10)

random.shuffle(output)

test = open("test.txt", "w")
train = open("train.txt", "w")

for line in output[:testsize]:
    test.write(line + "\n")
test.close()

for line in output[testsize:]:
    train.write(line + "\n")
train.close()