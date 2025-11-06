#!/usr/bin/perl
#
# fix_deps.pl - Netscape 3.0 / Mozilla classic dependency cleaner
# Called exactly like the original:  fix_deps.pl $(DEPENDENCIES) $<
#

my $depfile = $ARGV[0];   # $(DEPENDENCIES)
my $target  = $ARGV[1];   # $<

# If no dependency file yet (first build, or after clean), do nothing silently
exit 0 unless $depfile && -f $depfile;

my $found = 0;
my $basename = $target;
$basename =~ s@.*/@@;   # strip path like " \.*/*$< "

open(MD, "< $depfile") || exit 0;
while (<MD>) {
    if (/\s\.*\/\Q$basename\E\s/) {
        $found = 1;
        last;
    }
}
close(MD);

if ($found) {
    print "Removing stale dependency $target from $depfile\n";

    my $tmpname = "$depfile.fix.tmp.$$";
    open(TMD, "> $tmpname") || die "Can't write $tmpname\n";

    # Re-read and filter
    open(MD, "< $depfile") || die "Can't re-open $depfile\n";
    while (<MD>) {
        s@\s\.*\/\Q$basename\E\s@ @g;
        print TMD $_ || die "Write failed to $tmpname\n";
    }
    close(MD);
    close(TMD);

    if (!rename($tmpname, $depfile)) {
        unlink($tmpname);
        die "Failed to rename $tmpname → $depfile\n";
    }
}
elsif ($target ne $depfile && $target ne "") {
    print "$(MAKE): *** No rule to make target $target. Stop.\n";
    exit 1;

}
