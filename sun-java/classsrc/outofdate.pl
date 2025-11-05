#!/usr/local/bin/perl

use v5.36;
use strict;
use warnings;

my $classdir;
if (defined $ARGV[0] && $ARGV[0] eq '-d') {
    $classdir = $ARGV[1];
    $classdir .= "/";
    shift;
    shift;
} else {
    $classdir = "./";
}

foreach my $filename (@ARGV) {
    my $classfilename = $classdir;
    $classfilename .= $filename;
    $classfilename =~ s/.java$/.class/;
    my @statf = stat($filename);
    my $mtime = $statf[9];
    my @statc = stat($classfilename);
    my $classmtime = $statc[9];
    $classmtime = 0 unless defined $classmtime;
    if (defined $mtime && $mtime > $classmtime) {
        print $filename, " ";
    }
}

print "\n";
