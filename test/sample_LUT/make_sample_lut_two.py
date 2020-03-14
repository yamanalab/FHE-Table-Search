n = 256
total = n * n

print("%d, %d" % (2, total))

for i in range(n):
    for j in range(n):
        x0 = i+1
        x1 = j+1
        print("%d, %d, %d" % (x0, x1, x0+x1))
