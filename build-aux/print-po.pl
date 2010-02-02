#!/usr/bin/perl

# print-po.pl - print translations in the order in which they appear in sources
# Copyright (C) 2010 Free Software Foundation, Inc.
# 2010  Martin von Gagern
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

=pod

=head1 NAME

print-po.pl - print translations in the order in which they appear in sources

=head1 SYNOPSIS

B<build-aux/print-po.pl> I<po/*.po>

=head1 DESCRIPTION

The script expects a list of po or pot files as command line
arguments.  For every source file mentioned in the C<#:> comments it
prints all messages in the order in which they appear in the source
file.  The first column of the output gives the number of times a
message was used, so that one can adjust the less-often used messages
to match the style of the more often used ones. An C<X> indicates a
missing translation string, a C<-> a continuation line and a C<+> more
than 9 occurrences of a message. Output will always be in UTF-8.

=head1 HISTORY

This script was originally written for GNU wdiff.
Its main application is to check formatting of usage help screens.

=head1 AUTHOR

Written 2010 by Martin von Gagern

=head1 COPYRIGHT

Copyright (C) 2010 Free Software Foundation, Inc.

Licensed under the GNU General Public License version 3 or later.

=cut

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
      $cnt = '+' if $cnt > 9;
      s/^-/$cnt/;
      print;
    }
  }
}
