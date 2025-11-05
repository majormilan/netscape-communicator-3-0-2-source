# By John Bazik
#
# Usage: $cwd = &fastcwd;
#
# This is a faster version of getcwd.  It's also more dangerous because
# you might chdir out of a directory that you can't chdir back into.

use v5.36;
use strict;
use warnings;

sub fastcwd {
    my ($odev, $oino, $cdev, $cino, $tdev, $tino);
    my (@path, $path);

    ($cdev, $cino) = (stat('.'))[0,1];
    for (;;) {
        ($odev, $oino) = ($cdev, $cino);
        chdir('..');
        ($cdev, $cino) = (stat('.'))[0,1];
        last if $odev == $cdev && $oino == $cino;
        opendir my $dh, '.' or last;
        my $entry;
        for (;;) {
            $entry = readdir($dh);
            next if !defined $entry;
            next if $entry eq '. ';
            next if $entry eq '..';

            last unless defined $entry;
            ($tdev, $tino) = (lstat($entry))[0,1];
            last unless $tdev != $odev || $tino != $oino;
        }
        closedir $dh;
        unshift(@path, $entry) if defined $entry;
    }
    chdir($path = '/' . join('/', @path));
    return $path;
}
1;