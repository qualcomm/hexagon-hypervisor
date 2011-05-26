# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear

import sys
import random

NUM_SCENARIOS = 10000
NUM_THREADS_IN_SCENARIOS = 12  # must match the C code
MAX_PRIOS = 32

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
# In addition to thread states, we generate a priomask and a wait_mask to be set
# before entering H2K_check_sanity.
#
# We also generate an expected output priomask and whether the resched interrupt
# should be raised.  The output priomask does not have to exactly match the
# expected output priomask.  If the input priomask is zero, then at least one
# bit of the output priomask should be set that is also set in the expected
# output priomask.

outfile = open('scenarios.h', 'w')
random.seed(0)
MAX_HTHREADS_possibilities = (3, 6, 8)
print >> outfile, '#if MAX_HTHREADS < %d' % min(MAX_HTHREADS_possibilities)
print >> outfile, '#error MAX_HTHREADS must be at least %d for this test.' % min(MAX_HTHREADS_possibilities)
print >> outfile, '#endif'
for MAX_HTHREADS in MAX_HTHREADS_possibilities:
	print >> outfile, '#if MAX_HTHREADS >= ' + str(MAX_HTHREADS)
	for i in range(NUM_SCENARIOS):
		print >> outfile, '\t // status,  hthread, prio'
		# First we decide how many threads are RUNNING, READY, and BLOCKED.
		num_running_threads = random.randrange(1, MAX_HTHREADS + 1)
		num_ready_threads = random.randrange(NUM_THREADS_IN_SCENARIOS - num_running_threads + 1)
		num_blocked_threads = NUM_THREADS_IN_SCENARIOS - num_running_threads - num_ready_threads
		# Then we determine the hthread and prio of each RUNNING thread.
		available_threads = range(MAX_HTHREADS)
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
			# the worst RUNNING prio and a valid priomask.
			running_hthreads.append(hthread)
			running_prios.append(prio)
			# We then print out the state of the RUNNING thread.
			if thread_id == 0:
				outfile.write('\t{{')
			else:
				outfile.write('\t  ')
			print >> outfile, '{ RUNNING, %2d, %8d },' % (hthread, prio)
		# Next we determine the hthread and prio of each READY thread.  We record the ready_valids
		# and best READY prio in order to later compute whether the resched interrupt should be raised.
		ready_valids = 0
		best_ready_prio = MAX_PRIOS
		for thread_id in range(num_ready_threads):
			hthread = random.randrange(-5, 20)
			prio = random.randrange(MAX_PRIOS)
			ready_valids |= 1 << prio
			if prio < best_ready_prio:
				best_ready_prio = prio
			# We then print out the state of the READY thread.
			print >> outfile, '\t  { READY,   %2d, %8d },' % (hthread, prio)
		# Next we determine the hthread and prio of each BLOCKED thread.
		for thread_id in range(num_blocked_threads):
			hthread = random.randrange(-5, 20)
			prio = random.randrange(MAX_PRIOS)
			# We then print out the state of the BLOCKED thread.
			print >> outfile, '\t  { BLOCKED, %2d, %8d },' % (hthread, prio)
		print >> outfile, '\t },'
		# Next we compute the list of RUNNING threads with the worst prio and the list of non-RUNNING hthreads.
		worst_running_prio = max(running_prios)
		worst_prio_running_threads = []
		non_running_threads = range(MAX_HTHREADS)
		for j in range(num_running_threads):
			if running_prios[j] == worst_running_prio:
				worst_prio_running_threads.append(running_hthreads[j])
			non_running_threads.remove(running_hthreads[j])
		# Then we compute a possible valid input priomask and the expected output priomask.
		priomask = 0
		expected_priomask = 0
		for worst_prio_running_thread in worst_prio_running_threads:
			if random.choice((True, False)):
				priomask |= 1 << worst_prio_running_thread
			expected_priomask |= 1 << worst_prio_running_thread
		# Then we compute a possible valid wait_mask.
		wait_mask = 0
		for non_running_thread in non_running_threads:
			if random.choice((True, False)):
				wait_mask |= 1 << non_running_thread
		# We then compute whether the resched interrupt should be raised.
		should_resched = (worst_running_prio > best_ready_prio or (wait_mask != 0 and ready_valids != 0))
		# Finally, we print out the remaining input and output state.
		print >> outfile, '\t 0x%x, // priomask' % priomask
		print >> outfile, '\t 0x%x, // wait_mask' % wait_mask
		print >> outfile, '\t 0x%x, // expected_priomask' % expected_priomask
		print >> outfile, '\t %d, // should_resched' % should_resched
		print >> outfile, '\t},'
	print >> outfile, '#endif'
# We print one last scenario to terminate the array.
print >> outfile, \
"""	 // status,  hthread, prio
	{{{ RUNNING,  0,        0 },"""
for i in range(1, NUM_THREADS_IN_SCENARIOS):
	print >> outfile, '\t  { BLOCKED, %2d, %8d },' % (i, i)
print >> outfile, \
"""	 },
	 0x0, // priomask
	 0x0, // wait_mask
	 0x1, // expected_priomask
	 0, // should_resched
	}"""
