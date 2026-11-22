:mod:`cache` -- Cache maintenance functions
=============================================

.. module:: cache

These routines perform whole-cache maintenance using the cache geometry for
the selected architecture.  They wait for the relevant hardware operation to
complete where required and synchronize before returning.


H2K_cache_l2_cleaninv
---------------------

.. c:function:: void H2K_cache_l2_cleaninv(void) IN_SECTION(".text.misc.cache")

Cleans and invalidates the L2 cache.


H2K_cache_d_cleaninv
--------------------

.. c:function:: void H2K_cache_d_cleaninv(void) IN_SECTION(".text.misc.cache")

Cleans and invalidates every data-cache set and way using indexed operations.


H2K_cache_d_clean
-----------------

.. c:function:: void H2K_cache_d_clean(void) IN_SECTION(".text.misc.cache")

Cleans every data-cache set and way without invalidating the lines.


H2K_cache_i_inv
---------------

.. c:function:: void H2K_cache_i_inv(void) IN_SECTION(".text.misc.cache")

Invalidates every instruction-cache set and way using indexed operations.
