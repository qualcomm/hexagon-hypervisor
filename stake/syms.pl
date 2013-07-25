#!/usr/bin/perl
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear


use strict;

my %base_syms;
my %add_syms;
my %ig;

my ($basefile, $addfile, $outfile, @ignore) = @ARGV;

foreach my $s (@ignore) {
	$ig{$s} = 1;
}

open(BASE, "hexagon-nm $basefile|")
	or die "Can't hexagon-nm $basefile";

while (<BASE>) {
	chomp;
	my ($addr, undef, $sym) = split /\s+/;
	$base_syms{$sym} = "0x$addr";
}
close BASE;

open (ADD, "hexagon-nm --defined-only $addfile|")
	or die "Can't hexagon-nm $addfile";

while (<ADD>) {
	chomp;
	my ($addr, undef, $sym) = split /\s+/;
	unless (exists $ig{$sym} or exists $base_syms{$sym}) {
		$add_syms{$sym} = "0x$addr";
	}
}
close ADD;

open(OUT, ">$outfile")
	or die "Can't open $outfile for write";

foreach my $s (keys %add_syms) {
	print OUT "--defsym=$s=$add_syms{$s}\n";
}
close OUT;
