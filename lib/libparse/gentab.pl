#! /usr/local/bin/perl
use v5.36;
use strict;
warnings;

open my $pa_tags, '<', 'pa_tags.h' or die "Can't open pa_tags.h: $!";
open my $hash, "|/usr/local/bin/gperf -T -t -l -Npa_LookupTag -p -k1,\$,2,3 > gperf.out.$$" or die "Can't run gperf: $!";
print $hash "struct pa_TagTable { char *name; int id; };\n%%\n\n";

open my $rmap, '>', 'pa_hash.rmap' or die "Can't open pa_hash.rmap: $!";
my $nextval = 0;

my %strings;
while (my $line = <$pa_tags>) {
  if ($line =~ /^#[ \t]*define[ \t]*([A-Z_][A-Z0-9_]+)[ \t]*(.*)/) {
    my $var = $1;
    my $val = $2;
    $val =~ s/\"//g;
    my $pre = $var;
    $pre =~ s/_.*//;
    my $post = $var;
    $post =~ s/$pre//;
    $post =~ s/_//;
    if ($pre eq "PT") {
      $strings{$post} = $val;
    } elsif ($pre eq "P") {
      if ($strings{$post} ne "") {
        print $hash $strings{$post} . ", $var\n";
      }
      if ($var ne "P_UNKNOWN" && $var ne "P_MAX") {
        while ($nextval < $val) {
          print $rmap "/* $nextval */\t\"\",\n";
          $nextval++;
        }
        print $rmap "/*$val*/\t\"$strings{$post}\",\n";
        $nextval = $val + 1;
      }
    }
  }
}
close $pa_tags;
close $hash;
close $rmap;
open my $gperf_out, '<', "gperf.out.$$" or die "Can't open gperf output: $!";
unlink("gperf.out.$$);
open my $template, '<', "pa_hash.template" or die "Can't open template: $!";

my %template;
while (my $tline = <$template>) {
  if ($tline =~ /^@begin/) {
    my ($name, $start, $end) =
      $tline =~ m#\@begin[ \t]*([A-Za-z0-9_]+)[ \t]*/([^/]*)/[ \t]*/([^/]*)/#;
    my $line;
    $line = <$gperf_out> until (eof($gperf_out) || (defined $line && $line =~ /$start/));
    if (defined $line && $line =~ /$start/) {
      $template{$name} .= $line;
      do {
        $line = <$gperf_out>;
        $template{$name} .= $line;
      } until ($line =~ /$end/ || eof($gperf_out));
    }
  } elsif ($tline =~ /^@include/) {
    my ($name) = $tline =~ /\@include[ \t]*(.*)$/;
    print $template{$name};
  } elsif ($tline =~ /^@sub/) {
    my ($name, $old, $new) =
      $tline =~ m#\@sub[ \t]*([A-Za-z0-9_]*)[ \t]/([^/]*)/([^/]*)/#;
    $template{$name} =~ s/$old/$new/g;
  } elsif ($tline =~ /^@/) {
    ;
  } else {
    print $tline;
  }
}
close $gperf_out;
close $template;
