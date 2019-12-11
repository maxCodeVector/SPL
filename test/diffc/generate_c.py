def w(l):
	if len(l) > 7:
		p = 'r'
	else:
		p = 'b'
	for n in l:
		with open("test%s%d.c"%(p,n), "wt") as fw:
			fw.write("#include \"build.h\"\n#include \"../test_3_%s0%d.spl\"\n"%(p, n))

w([1,2,3,4,5])
w(list(range(1,11)))
