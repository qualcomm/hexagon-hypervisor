# Hexagon Hypervisor

This software comprises:
* A hypervisor kernel that provides a stable interface to the Qualcomm® Hexagon™ processor.
* A virtual-machine model that allows multiple guest operating systems to run concurrently using virtualized system resources.
* A sample guest operating system that allows application software to run in a virtual machine.

## Branches

**master**: Primary development branch. Contributors should develop submissions based on this branch, and submit pull requests to this branch. CI tests this branch; merging pull requests requires passing status and approval.

**stable**: Release and hot-fix branch. We merge master to this branch to create a release candidate. Developers may also base pull requests for simple hot fixes on stable. CI tests this branch; merging pull requests and tagging for release versions requires passing status and approval.

## Requirements

An installation of the [Hexagon SDK](https://softwarecenter.qualcomm.com/catalog/item/Hexagon_SDK).

## Build Instructions

* Modify scripts/Makefile.inc.config as needed.
* `make` in the top-level directory.

make options:
* ARCHV=\<hexagon architecture version\>. List of available versions is given by ARCHV_LIST in top makefile.
* TARGET=ref // for debug

## Usage

Run with hexagon-sim from Hexagon tools:

* hexagon-sim \<options\> -- install/bin/booter \<options\> \<application executable\>
* hexagon-sim \<options\> -- install/bin/booter --help  // list available booter options

## Development

See [CONTRIBUTING.md file](CONTRIBUTING.md).

## Getting in Contact

* [Report an Issue on GitHub](../../issues)
* [Open a Discussion on GitHub](../../discussions)
* [E-mail us](mailto:h2_dev@qti.qualcomm.com) for general questions

## License

Hexagon Hypervisor is licensed under the [BSD-3-clause License](https://spdx.org/licenses/BSD-3-Clause.html). See [LICENSE.txt](LICENSE.txt) for the full license text.
