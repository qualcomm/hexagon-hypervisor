#!/usr/local/bin/perl
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear


use strict;

my ($op, $image, $addrbits, $pgsize, $heapsize, $linkaddr, $loadaddr) = @ARGV;

my $page_bits = $addrbits + (2 * $pgsize);
my $heap_size = (oct $heapsize) * 4; # size is in words
my $v2p_offset = (oct $linkaddr) - (oct $loadaddr);

my $end = `hexagon-nm $image | egrep \' end\$\'`;
$end =~ s/\s.*//;
$end = oct "0x$end";
$end = (($end + 31) / 32) * 32;	# align for allocator heap
$end += $heap_size;
#print STDERR sprintf("end: 0x%0x\n", $end);

my $vpage = (($end >> $page_bits) + 1);
#print STDERR sprintf("vpage: 0x%0x\n", $vpage);

my $ppage = $vpage - ($v2p_offset >> $page_bits);
#print STDERR sprintf("ppage: 0x%0x\n", $ppage);

for ($op) {

	/^bootvm_offset/ and do {
		my $addr = $ppage << $page_bits;

		print sprintf("0x%0x\n", $addr);
		last;
	};

	/^bootvm_start/ and do {
		my $addr = $vpage << $page_bits;

		print sprintf("0x%0x\n", $addr);
		last;
	};

	/^v2p_offset/ and do {
		print sprintf("0x%0x\n", $v2p_offset);
		last;
	};

	/^kernel_npages/ and do {
		print sprintf("0x%0x\n", $vpage - (0xff000000 >> $page_bits));
		last;
	};
}

