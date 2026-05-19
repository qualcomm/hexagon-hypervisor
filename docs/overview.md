
# Overview

Hexagon Hypervisor is designed to be a small, efficient minimal monitor mode software
for the Qualcomm Hexagon processor.  It contains a scheduler and techniques to interact
with hardware resources, but minimizes functionality in order to focus on simplicity and
security.

# Source Repository Organization

The kernel/ directory contains the code that runs in Monitor Mode, the most privileged mode.
Each module should have documentation and unit tests.  

# Important metrics

## Quality and Security

Having a robust, bug-free environment is very important.  To help this, we want to have 
simple functions that interact in simple ways and where we can easily test the code.

We collect code coverage to help check that our testing is sufficient.

## Performance

Performance is very important.  Scheduling, blocking, unblocking, interrupts,
handling the MMU, and other activities are common and saving cycles helps to
ensure we can meet real-time deadlines and provide an efficient product.

Some interfaces are more important for performance than others, but performance
should always be something considered.


## Code Size

We aim to have small code and data footprint.  This enables the environment to run
even on very small devices, and to work with small memories and small caches.  Things
that increase code size such as duplicated code or inefficient access patterns should
be avoided.  

We monitor code size and periodically will work towards optimizing the size instead of
the typical view of optimizing for cycles.  The best solutions tend to be ones where both
the cycles and the bytes are minimized.

