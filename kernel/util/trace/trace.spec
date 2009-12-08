
UNIT: trace

SUMMARY:

The kernel provides a logging facility for kernel information.

Kernel logging is not an optional feature. 

In V4, we will use the ETM for kernel logging.  V4 will allow the ETM to be
read by software during a crash.  For V3, we will log to a small memory.

For V3, the trace buffer structure is::

	typedef struct {
		u32_t word0;
		u32_t word1;
		u32_t word2;
		u32_t word3;
	} __attribute__((aligned(16))) H2K_trace_entry_t;

The buffer is defined as:

u32_t H2K_trace_index;
H2K_trace_entry_t H2K_trace[MAX_TRACE_ENTRIES];

Fatal Kernel Errors have negative Trace IDs.  Informational kernel messages
have positive Trace IDs.  MAX_TRACE_LEVEL may be used to filter out kernel
messages.  Recommended MAX_TRACE_LEVEL values are zero for maximum performance,
or 1000 for maximum tracing.

FUNCTION: static inline void H2K_trace(s16_t type, u32_t arg1, u32_t arg2, u32_t arg3, u32_t hwtnum)

DESCRIPTION:

If an event is less than DEBUG_LEVEL, log a kernel event.

INPUT:

INPUT_ASSERT(H2K_trace_index < MAX_TRACE_ENTRIES)

Argument 0: Message ID.  See Message ID Format below.
Argument 1: Additional information to log
Argument 2: Additional information to log
Argument 3: Additional information to log
Argument 4: Hardware Thread Number.

OUTPUT:


FUNCTIONALITY:

(V3 Implementation)

If type is greater than MAX_TRACE_LEVEL, do nothing.

H2K_trace_index points to the next trace entry to write.  We read the value, 
increment it, reset to zero if greater than MAX_TRACE_ENTRIES, and store the
pointer back atomically.  We use the old value to compute the pointer to the
trace entry to write.  

The first argument contains the Message ID, which is a 16 bit value.  We combine
this with the hardware thread number.  The resulting value is written to the
entry word0 in the trace buffer.  Word1, word2, and word3 are written as the 
input arguments without modification.


Defined Trace Message IDs:

TRACE_MESSAGE(0, "No Message", "N/A", "N/A", "N/A")
TRACE_MESSAGE(-1, "Fatal Error", "N/A", "PCYCLELO", "PCYCLEHI")

TRACE_MESSAGE(1, "Thread Switch", "New Thread", "PCYCLELO", "PCYCLEHI")
TRACE_MESSAGE(2, "Interrupt", "Interrupt Number", "PCYCLELO", "PCYCLEHI")
TRACE_MESSAGE(4, "Futex Wait", "Futex Address", "PCYCLELO", "PCYCLEHI")
TRACE_MESSAGE(5, "Futex Resume", "Futex Address", "PCYCLELO", "PCYCLEHI")


