# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear

import random

f = open('generated_tests.dat','w')

SIZE = 700
N = 15

NAME = "big_inorder"
MIN = 0
MAX = SIZE-1
print >>f, "TEST(%s,%d,%d,%d,%s)" % (NAME,SIZE,MIN,MAX,"{ %s }" % (",".join([ str(x) for x in range(SIZE)])))

NAME = "big_reverse"
MIN = 1
MAX = SIZE
print >>f, "TEST(%s,%d,%d,%d,%s)" % (NAME,SIZE,MIN,MAX,"{ %s }" % (",".join([ str(x) for x in range(SIZE,0,-1)])))

for i in range(N):
	NAME = "big_unique_random_%d" % i
	arr = range(SIZE)
	random.shuffle(arr)
	MIN = min(arr)
	MAX = max(arr)
	print >>f, "TEST(%s,%d,%d,%d,%s)" % (NAME,SIZE,MIN,MAX,"{ %s }" % (",".join([ str(x) for x in arr])))

for i in range(N):
	NAME = "big_nonunique_random_%d" % i
	arr = [ random.randrange(SIZE) for i in range(SIZE) ]
	MIN = min(arr)
	MAX = max(arr)
	print >>f, "TEST(%s,%d,%d,%d,%s)" % (NAME,SIZE,MIN,MAX,"{ %s }" % (",".join([ str(x) for x in arr])))

