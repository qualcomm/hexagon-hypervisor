#!/usr/bin/perl
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear


# Generates test.cov_fns based on what is in h2kernel.a and h2.a

# Build with OPTIMIZE='-O0' so we get inlines as normal symbols...

use strict;
use warnings;

my %funclist;

#my $test = shift @ARGV or die "Provide test on command line!";
#die "$test not found!" unless (-f $test);
#$test =~ s/test\.elf$//;
#$test .= "*.o";

while (my $test = shift @ARGV) {
  my @tobj=`hexagon-llvm-objdump --syms $test`;

  foreach my $line (@tobj) {
    # (g) functions are added to the ignore list
    if ($line =~ /\S+\s+g\s+F\s+[A-Za-z0-9_\.]+\s+\S+\s+(H2K_.*|h2_.*)/) {
      $funclist{$1}=1;
    }
    # Some functions don't have the F?
    elsif ($line =~ /\S+\s+g\s+[A-Za-z0-9_\.]+\s+\S+\s+(H2K_.*|h2_.*)/) {
      $funclist{$1}=1;
    }
  }
}

foreach my $key (sort {$a cmp $b} keys(%funclist)) {
  print "$key\n";
}
