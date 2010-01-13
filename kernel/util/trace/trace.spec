
:mod:`trace` -- kernel logging facility
=========================================

.. module:: trace

SUMMARY
--------

The kernel provides a logging facility for kernel information.

Kernel logging is not an optional feature. 

In V4, we will use the ETM for kernel logging.  V4 will allow the ETM to be
read by software during a crash.  For V3, we will log to a small memory.

For both V3 and V4, the trace format is a 32-bit word.  It has the structure::

	typedef union {
		u32_t raw;
		struct {
			s16_t delta;
			u8_t tid;
			u8_t hwtnum:4;
			s8_t id:4;
		};
	} H2K_trace_entry_t;

The buffer is defined as:

u32_t H2K_trace_index;
H2K_trace_entry_t H2K_trace[MAX_TRACE_ENTRIES];

Fatal Kernel Errors have negative Trace IDs.  Informational kernel messages
have positive Trace IDs.  MAX_TRACE_LEVEL may be used to filter out kernel
messages.  Recommended MAX_TRACE_LEVEL values are zero for maximum performance,
or 1000 for maximum tracing.

The delta (in processor cycles) between the last trace event and the current
trace event is logged in the "delta" field.  The value is kept in a signed 
16-bit field, as events from different threads in close proximity may be placed
in the buffer in different order than their pcycle capture time.

If the delta time does not fit in a signed, 16-bit value, the delta is saturated
to signed 16-bits.  The delta time is compted only from pcyclelo, events spaced
more than a few seconds apart may have incorrect deltas.  

Time may also not be counted while all threads are in wait mode if clock 
gating is enabled.

H2K_trace
---------

.. cfunction:: void H2K_trace(s8_t id, u8_t hwtnum, u8_t tid, u16_t pcyclelo)

Description
~~~~~~~~~~~

If an event is less than MAX_TRACE_LEVEL, log a kernel event.

Input
~~~~~

.. InputAssert::
	assert(H2K_trace_index < MAX_TRACE_ENTRIES)

Argument 0: Message ID.  See Message ID Format below.
Argument 1: Additional information to log
Argument 2: Additional information to log
Argument 3: Additional information to log
Argument 4: Hardware Thread Number.

Output
~~~~~~


Functionality
~~~~~~~~~~~~~

If type is greater than MAX_TRACE_LEVEL, do nothing.

H2K_trace_index points to the next trace entry to write.  We read the value, 
increment it, reset to zero if greater than MAX_TRACE_ENTRIES, and store the
pointer back atomically.  We use the old value to compute the pointer to the
trace entry to write.  

The first argument contains the Message ID, which is really a signed 5-bit
value.  We combine this with the hardware thread number.  The resulting value
is the most significant byte of the trace entry.  The next byte is the TID 
of the appropriate thread.  The final sixteen bits are low-order bits from 
PCYCLELO.

In V3, this word is added to the trace buffer in memory.  In V4, this word is
added to the ETM trace.



