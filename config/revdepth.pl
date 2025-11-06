#! /usr/bin/perl

use strict;
use warnings;
use Cwd 'fastcwd';
use v5.36;

# Historical include paths
unshift(@INC, '/usr/lib/perl');
unshift(@INC, '/usr/local/lib/perl');

# Default to current directory if no argument is given
my $target = shift @ARGV // '.';

my $cur = fastcwd();
chdir($target) or die "Cannot chdir to $target: $!";
my $newcur = fastcwd();

# Split both paths into components
my @cur_parts = split('/', $cur);
my @new_parts = split('/', $newcur);

# Remove common prefix
while (@cur_parts && @new_parts && $cur_parts[0] eq $new_parts[0]) {
    shift @cur_parts;
    shift @new_parts;
}

# For each remaining dir in $new_parts, we need to go up one level
my $revdepth = ('../' x scalar(@new_parts));

print $revdepth;
