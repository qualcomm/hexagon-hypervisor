# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear

import sys
import random

NUM_SCENARIOS = 10000
NUM_THREADS_IN_SCENARIOS = 12  # must match the C code
MAX_PRIOS = 256  # must match max.h

# This script generates scenarios for H2K_check_sanity.  The scenarios are in
# the form of an initializer for a C array of structs.  The initializer is
# included in test.c.  test.c uses the information in the struct to set up the
# state before entering H2K_check_sanity, and to check the results of
# H2K_check_sanity.
#
# For each scenario, we generate a list of threads, each with three properties,
# status (RUNNING, READY, or BLOCKED), hthread, and prio.  There may be at most
# MAX_HTHREADS RUNNING threads, and each RUNNING thread must have a valid
# hthread.  No two RUNNING threads may have the same hthread.
#
#
# We also generate whether the resched interrupt should be raised.  

with open('scenarios.h', 'w') as outfile:
	random.seed(0)
	MAX_HTHREADS_possibilities = (2, 3, 4, 6, 8)

	print('#if MAX_HTHREADS < %d' % min(MAX_HTHREADS_possibilities), file=outfile)
	print('#error MAX_HTHREADS must be at least %d for this test.' % min(MAX_HTHREADS_possibilities), file=outfile)
	print('#endif', file=outfile)
	for MAX_HTHREADS in MAX_HTHREADS_possibilities:
		print('#if MAX_HTHREADS >= ' + str(MAX_HTHREADS), file=outfile)
		for i in range(NUM_SCENARIOS):
			print('\t // status,  hthread, prio', file=outfile)
			# First we decide how many threads are RUNNING, READY, and BLOCKED.
			num_running_threads = random.randrange(1, MAX_HTHREADS + 1)
			num_ready_threads = random.randrange(NUM_THREADS_IN_SCENARIOS - num_running_threads + 1)
			num_blocked_threads = NUM_THREADS_IN_SCENARIOS - num_running_threads - num_ready_threads
			# Then we determine the hthread and prio of each RUNNING thread.
			available_threads = list(range(MAX_HTHREADS))
			running_hthreads = []
			running_prios = []
			for thread_id in range(num_running_threads):
				# For each RUNNING thread, we choose its hthread from the list of available hthreads.
				hthread = random.choice(available_threads)
				# Then we remove that hthread from the list of available hthreads so that no two
				# RUNNING threads get assigned the same hthread.
				available_threads.remove(hthread)
				prio = random.randrange(MAX_PRIOS)
				# We record the hthread and prio of each RUNNING thread in order to later compute
				# the worst RUNNING prio.
				running_hthreads.append(hthread)
				running_prios.append(prio)
				# We then print out the state of the RUNNING thread.
				if thread_id == 0:
					outfile.write('\t{{')
				else:
					outfile.write('\t  ')
				print('{ RUNNING, %2d, %8d },' % (hthread, prio), file=outfile)
			# Next we determine the hthread and prio of each READY thread.  We record the READY prio
			# and best in order to later compute whether the resched interrupt should be raised.
			best_ready_prio = MAX_PRIOS
			for thread_id in range(num_ready_threads):
				hthread = random.randrange(-5, 20)
				prio = random.randrange(MAX_PRIOS)
				if prio < best_ready_prio:
					best_ready_prio = prio
				# We then print out the state of the READY thread.
				print('\t  { READY,   %2d, %8d },' % (hthread, prio), file=outfile)
			# Next we determine the hthread and prio of each BLOCKED thread.
			for thread_id in range(num_blocked_threads):
				hthread = random.randrange(-5, 20)
				prio = random.randrange(MAX_PRIOS)
				# We then print out the state of the BLOCKED thread.
				print('\t  { BLOCKED, %2d, %8d },' % (hthread, prio), file=outfile)
			print('\t },', file=outfile)
			# Next we compute the list of RUNNING threads with the worst prio and the list of non-RUNNING hthreads.
			worst_running_prio = max(running_prios)
			worst_prio_running_threads = []
			non_running_threads = list(range(MAX_HTHREADS))
			for j in range(num_running_threads):
				if running_prios[j] == worst_running_prio:
					worst_prio_running_threads.append(running_hthreads[j])
				non_running_threads.remove(running_hthreads[j])
			# We then compute whether the resched interrupt should be raised.
			should_resched = (worst_running_prio > best_ready_prio)
			# Finally, we print out the remaining input and output state.
			print('\t %d, // should_resched' % should_resched, file=outfile)
			print('\t %d, // hthreads' % MAX_HTHREADS, file=outfile)
			print('\t},', file=outfile)
		print('#endif', file=outfile)
	# We print one last scenario to terminate the array.
	print("""	 // status,  hthread, prio
	{{{ RUNNING,  0,        0 },""", file=outfile)
	for i in range(1, NUM_THREADS_IN_SCENARIOS):
		print('\t  { BLOCKED, %2d, %8d },' % (i, i), file=outfile)
	print("""	 },
	 0, // should_resched
	}""", file=outfile)
