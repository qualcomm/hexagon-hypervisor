#!/usr/bin/perl
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear


# Generates test.cov_fns for lib tests based on what is called.

my %funclist;

open(TEST, "qdsp6-cpp -w test.c 2>/dev/null |");
while(my $line = <TEST>) {
  next if ($line =~ /^#/);
  if ($line =~ /(h2[^\(]+)\(/) {
    $funclist{$1}=1;
  }
}

close TEST;

foreach my $key (sort {$a <=> $b} keys(%funclist)) {
  print "$key\n";
}
