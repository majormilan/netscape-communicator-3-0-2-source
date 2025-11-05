#! /usr/local/bin/perl

use v5.36;
use strict;
use warnings;

my $table = $ARGV[0];
shift(@ARGV);

print "/* Automatically generated file; do not edit */\n\n";

print "#include \"prtypes.h\"\n\n";
print "#include \"prlink.h\"\n\n";

foreach my $symbol (@ARGV) {
    print "extern void ", $symbol, "();\n";
}
print "\n";

print "PRStaticLinkTable ", $table, "[] = {\n";
foreach my $symbol (@ARGV) {
    print "  { \"", $symbol, \"\", ", $symbol, " },\n";
}
print "  { 0, 0, },\n";
print "};\n";
