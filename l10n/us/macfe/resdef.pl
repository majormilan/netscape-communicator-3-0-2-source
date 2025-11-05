#! /usr/local/bin/perl

# This script reads a series of lines of the form:
# resource 'STR ' ( id+100, "name", purgeable)(NAME, ID, "STRING")
# and generates stuff like:

use v5.36;
use strict;
use warnings;

open my $out, '>', 'xpstring.r' or die "Can't open xpstring.r: $!";
print $out "// This is a generated file, do not edit it\n";
print $out "#include \"Types.r\"\n";
while (my $line = <>) {
  my ($enum, $eid) =
    $line =~ /[ \t]*([A-Za-z0-9_]+)[ \t]*\=[ \t]*([A-Za-z_x0-9]+),/;
   if (defined $enum && $enum ne "") {
    print $out "#define $enum $eid \n";
  }
  my ($name, $id, $string) =
    $line =~ /ResDef[ \t]*\([ \t]*([A-Za-z0-9_]+)[ \t]*,[ \t]*([\(\)0-9A-Za-z_x\-+ ]+)[ \t\01]*,[ \t]*(".*")[ \t]*\)/;
   if (defined $name && $name ne "") {
    print $out "resource 'STR ' (($id)+7000, \"$name\", purgeable)\n{\n\t";

    my $s = $string;
    $s =~ s/([^.:])\\n/$1 /g;
    $s =~ s/(\\n) /$1/g;
    print $out $s;
    
    print $out ";\n};\n";
  }
}
close($out);
