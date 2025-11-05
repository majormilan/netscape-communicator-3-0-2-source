#!/usr/usc/bin/perl
#
# $Id: rdistvf.pl,v 1.1 1996/01/31 23:30:33 dkarlton Exp $
#
# rdistvfilter - Verbose filter of rdist messages.  Takes the output from
#	 rdist and outputs a much nicer, though verbose, form.
#
# [mcooper] 5/9/88
#

use v5.36;
use strict;
use warnings;

my $tmp = "/tmp/rdistfilter.$$";
open my $outf, "|sort >$tmp" or die "Can not open tmp file.\n";

while (<>) {
    chomp;

    # Remove any garbage we might find
    s/[\\000-\\007\\016-\\037]//g;

    #
    # The point of the below code is to try to extract and save the
    # name of the host messages are for.  If an input line doesn't
    # look like a normal rdist message, it's probably something like
    # "Permission denied".  In this case, we assume the last saved host
    # name is associated with this message and print the line with that
    # host name.
    #
    if ((/updating of /) || (/updating host /)) {
        my @Fields = split;
        my $Host = $Fields[2];
        $Host =~ s/\..*//;    # Strip domain name
    } elsif (/:/) {
        my @Fields = split;
        my $Host = $Fields[0];
        $Host =~ s/://;
        $Host =~ s/\..*//;    # Strip domain name

        my $tmpname = $Host . ":";
        printf {$outf} "%-12s", $tmpname;
        for (my $i = 1; $i <= $#Fields; $i++) {
            printf {$outf} " %s", $Fields[$i];
        }
        printf {$outf} "\n";
    } elsif ($_) {
        if (defined $Host) {
            my $tmpname = $Host . ":";
            printf {$outf} "%-12s", $tmpname;
        }
        printf {$outf} "%s\n", $_;
    }
}

close $outf;
open my $inf, "$tmp" or die "Cannot open tmp file.\n";
my $last = "";
my $ll = "";
my $lc = 0;

while (<$inf>) {
    my ($current) = split(/\t| /);
    if ($last && ($last ne $current)) {
        printf "\n";
    }
    $last = $current;
    $_ =~ s/\n//;
    if ($ll eq $_) {
        $lc++;
    } else {
        printf $_;
        if ($lc > 1) {
            my $cs = sprintf(" (x%d)", $lc);
        } else {
            my $cs = "";
        }
        printf "\n"; # printf "%s\n", $cs;
        $lc = 0;
        $ll = $_;
    }
}

unlink $tmp;

exit 0;
