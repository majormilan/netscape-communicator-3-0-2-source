#! /usr/local/bin/perl

unshift(@INC, '/usr/lib/perl');
unshift(@INC, '/usr/local/lib/perl');

require "fastcwd.pl";

use v5.36;
use strict;
use warnings;

my $cur = fastcwd();
chdir($ARGV[0]);
my $newcur = fastcwd();
my $newcurlen = length($newcur);

# Skip common separating / unless $newcur is "/"
$cur = substr($cur, $newcurlen + ($newcurlen > 1));
print $cur;
