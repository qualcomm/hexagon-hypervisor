#!/usr/bin/perl
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear


# Generates the coverage function list from the built kernel libraries: every
# F-type H2K_*/h2_* symbol in libh2kernel.a (kernel) and libh2.a (h2 user lib,
# e.g. the h2_compat functions exercised by the libs/ tests).  libh2check.a is
# intentionally excluded since it holds test-checker code (checker_*), not
# product code.
#
# Build with OPTIMIZE='-fno-inline' (i.e. TARGET=ref_cov/opt_cov) so inline
# functions appear as normal symbols and can be tracked.
#
# Usage: gen_cov_fns.pl [installpath] [omitfile]
#   installpath  defaults to ./install
#   omitfile     optional list of function names to drop (one per line; blank
#                lines and '#' comments ignored).  See scripts/cov_omit_functions.

use strict;
use warnings;

my $installpath = $ARGV[0] || "./install";
my $omitfile    = $ARGV[1];

my %omit;
if (defined $omitfile && -f $omitfile) {
  open(my $fh, '<', $omitfile) or die "cannot open omit file $omitfile: $!";
  while (my $line = <$fh>) {
    $line =~ s/\s*#.*//;      # strip comments
    $line =~ s/^\s+|\s+$//g;  # trim
    next if $line eq '';
    $omit{$line} = 1;
  }
  close($fh);
}

my %funclist;

my @syms;
foreach my $lib ("libh2kernel.a", "libh2.a") {
  push @syms, `hexagon-llvm-objdump --syms $installpath/lib/$lib|grep F`;
}

foreach my $line (@syms) {
  if ($line =~ /\S+\s+\S+\s+F\s+[A-Za-z0-9_\.]+\s+\S+\s+(H2K_.*|h2_.*)/) {
    my $name = $1;
    # Skip LLVM-generated specialized clones (e.g. foo.llvmint.1.0,
    # bar.llvmint.1.2_i64_0).  Source function names never contain a '.', so a
    # dot in the symbol marks a compiler artifact, not a function we test.
    # These also get dropped later by cov_rpt_tool's \w-only header regex, so
    # excluding them here keeps cov_fns and cov.rpt in agreement.
    next if $name =~ /\./;
    # Skip functions explicitly listed in the omit file.
    next if $omit{$name};
    $funclist{$name}=1;
  }
}

foreach my $key (sort {$a cmp $b} keys(%funclist)) {
  print "$key\n";
}
