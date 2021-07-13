data = []

with open("test.json") as f:
    for line in f:
        idSt = line.find("identifier") + 13
        idEn = line.find("\"", idSt)
        ident = line[idSt:idEn]
        
        inp = open("all/test-" + ident + "-0.txt")
        pixels = inp.readlines()[0][21:].split(", ")
        inp.close()
        pixels[-1] = pixels[-1][:-3]
        data.append(pixels)

print("Datareading done")
        
duplicates = 0
for x in range(0, 989):
    if (x % 100 == 0):
        print (x)
    image = data[x]
    for y in range(x+1, 990):
        image2 = data[y]
        same = True
        for p in range(0, 4096):
            if image[p] != image2[p]:
                same = False
                break
        if same:
            duplicates = duplicates + 1
            break
            
print(duplicates)