#! /usr/local/bin/perl

require "fastcwd.pl";

use v5.36;
use strict;
use warnings;

my $cwd = fastcwd();
local $_ = $cwd;
if (m@^/[uh]/@o || s@^/tmp_mnt/@/@o) {
    print("$_
");
} elsif ((my ($user, $rest) = m@^/usr/people/(\\\\w+)/(.*)@o)
      && readlink("/u/$user") eq "/usr/people/$user") {
    print("/u/$user/$rest\n");
} else {
    chomp(my $host = `hostname`);
    print("/h/$host$_
");
}
