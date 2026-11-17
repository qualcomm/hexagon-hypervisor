:mod:`spinlock` -- Ticket spin locks
=======================================

.. module:: spinlock

``H2K_spinlock_t`` is a 32-bit ticket lock.  The upper half is the next ticket
to allocate and the lower half is the ticket currently being served.


H2K_spinlock_init
-----------------

.. c:function:: static inline void H2K_spinlock_init(H2K_spinlock_t *lock)

	:param lock: Lock to initialize

Initializes an unlocked lock by storing zero.


H2K_spinlock_lock
-----------------

.. c:function:: void H2K_spinlock_lock(H2K_spinlock_t *lock)

	:param lock: Lock to acquire

Atomically obtains a ticket and spins until that ticket is being served.  The
function does not return until the caller owns the lock.


H2K_spinlock_trylock
--------------------

.. c:function:: int H2K_spinlock_trylock(H2K_spinlock_t *lock)

	:param lock: Lock to acquire
	:returns: 0 when acquired, 1 when the lock was already held

Attempts to acquire the next ticket without waiting.  On failure the lock is
unchanged; on success the caller owns the lock and must unlock it.


H2K_spinlock_unlock
-------------------

.. c:function:: void H2K_spinlock_unlock(H2K_spinlock_t *lock)

	:param lock: Lock owned by the caller

Advances the serving ticket and releases the lock.  The caller must own the
lock.
