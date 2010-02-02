#!/usr/bin/perl

use strict;
use warnings;

use Encode;

BEGIN {
  my $incdir = __FILE__;
  $incdir = '.' unless $incdir =~ s:/[^/]+$::;
  unshift @INC, $incdir;
}

use msgitm;

binmode STDOUT, ':utf8';
for my $pofile (@ARGV) {
  my @po = msgitm->parse($pofile);
  print "========== $pofile ==========\n";
  my %src = ();
  my %cnt = ();
  my $encoding = "utf8";
  for my $itm (@po) {
    my $id = $itm->msgid;
    my $str = $itm->msgstr;
    $encoding = $1 if $id eq '' && $str =~ /; charset=(.*?)\\n/;
    $str = $id if $pofile =~ /\.pot$/;
    my @refs = $itm->srcrefs;
    my $cnt = scalar(@refs);
    for my $ref (@refs) {
      next unless $ref =~ /^(.*):(\d+)$/;
      my $file = $1;
      my $line = $2;
      push @{$src{$file}}, { line => $line, cnt => $cnt,
                             id => $id, str => $str };
    }
  }
  for my $file (sort keys %src) {
    print "---------- $file ($pofile) ----------\n";
    for my $m (sort { $a->{line} <=> $b->{line} } @{$src{$file}}) {
      my $str = $m->{str};
      my $cnt = $m->{cnt};
      my $id = $m->{id};
      $_ = $str;
      $_ = $id if $_ eq '';
      $_ = decode($encoding, $_);
      s/(\\[\\nt"])/eval qq{"$1"}/eg;
      s/\n?$/\n/;
      s/^([^\n]{79})([^\n])/$1| !!! |$2/mg;
      s/^/-|/mg;
      s/^-/X/ if $str eq '';
      s/^-/$cnt/;
      print;
    }
  }
}
