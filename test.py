f = open ("im5.ppm", "r")
form = f.readline()
n, m = map (int, f.readline().split())
max_pix = f.readline()
a = []

for i in range (n):
	for j in range (m):
		a.append(list (map (int, f.readline().strip().split())))
f.close()

f = open ("asm.txt", "w")
num = 0
for i in range (n):
	for j in range (m):
		pixel = a[i + n * j]
		r = pixel[0]
		g = pixel[1]
		b = pixel[2]
		x = (r << 16) | (g << 8) | b
		f.write("push " + str(x) + "\n")
		f.write("pop [" + str(num % m + i * 1000) +"]" + "\n")
		num += 1
f.write("draw\n")
f.write("hlt")
f.close()
