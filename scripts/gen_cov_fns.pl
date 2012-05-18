#!/usr/bin/perl
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear


# Generates test.cov_fns based on what is in h2kernel.a and h2.a

# Build with OPTIMIZE='-O0' so we get inlines as normal symbols...

use strict;
use warnings;

my %funclist;

my @h2k=`qdsp6-objdump --syms ./install/lib/libh2kernel.a|grep F`;
#my @h2=`qdsp6-objdump --syms ./install/lib/libh2.a|grep F`;
#skip h2 lib functions for now.
my @h2;

foreach my $line (@h2k,@h2) {
  if ($line =~ /\S+\s+\S+\s+F\s+[A-Za-z0-9_\.]+\s+\S+\s+(H2K_.*|h2_.*)/) {
    $funclist{$1}=1;
  }
}

foreach my $key (sort {$a cmp $b} keys(%funclist)) {
  print "$key\n";
}
