#!/usr/bin/perl

# msgitm - gettext po file parsing library
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

msgitm - gettext po file parsing library

=head1 SYNOPSIS

    use msgitm;

=head1 DESCRIPTION

Provides functionality to parse and handle translation message files
in GNU gettext po format.

=head1 API

=cut

use strict;
use warnings;

package msgitm;

sub new() {
  my $this = { head => [], msgid => [], msgstr => [] };
  return bless $this;
}

sub accessor($$$@) {
  my ($this, $wantarray, $field, @newval) = @_;
  @{$this->{$field}} = @newval if (@newval);
  return $wantarray ? @{$this->{$field}} : join('', @{$this->{$field}});
}

=head2 $msgitm->head([@newhead])

Set or retrieve the whitespace and comments leading up to a
message. If a list is passed, it should consist of lines of text,
including terminating newlines. All lines should be whitespace only or
starting in a C<#>. The return value is such a list, if called in list
context, or the concatenation of these lines, if called in scalar
context.

=cut

sub head($@) {
  my ($this, @newval) = @_;
  return accessor($this, wantarray, 'head', @newval);
}

=head2 $msgitm->msgid([@newid]), $msgitm->msgstr([@newstr])

Set or retrieve the untranslated or translated message. Both these
messages are represented as lists of strings, where each string
corresponds to the part inside the quotes of a single line in the po
file. In other words, the logical value of a message is the
concatenation of these strings, and the strings should not contain
unescaped quotation marks or newlines, but instead should use C escape
sequences for these. Giving an array allows setting these values. When
called in list context, the list of lines is returned. In scalar
context they are concatenated to represent the logical value of the
message.

=cut

sub msgid($@) {
  my ($this, @newval) = @_;
  return accessor($this, wantarray, 'msgid', @newval);
}

sub msgstr($@) {
  my ($this, @newval) = @_;
  return accessor($this, wantarray, 'msgstr', @newval);
}

=head2 $msgitm->block

Returns the block corresponding to a given message, including
comments, msgid and msgstr, in a format suitable for generation of a
modified po file.

=cut

sub block($) {
  my ($this) = @_;
  my $res = $this->head;
  if (@{$this->{msgid}}) {
    $res .= qq{msgid "} . join(qq{"\n"}, $this->msgid) . qq{"\n};
    $res .= qq{msgstr "} . join(qq{"\n"}, $this->msgstr) . qq{"\n};
  }
  return $res;
}

=head2 $msgitm->srcrefs

Retrieve a list of source references. This is the content of all lines
starting with C<#:>, split at whitespace.

=cut

sub srcrefs($) {
  my ($this) = @_;
  my @res = ();
  for my $h ($this->head) {
    if ($h =~ s/^#: //) {
      chomp($h);
      push @res, split(/\s+/, $h);
    }
  }
  return @res;
}

=head2 $msgitm->srcfiles

This is like the source references returned from I<srcrefs> but
without the line number part usually contained in these comments.

=cut

sub srcfiles($) {
  my ($this) = @_;
  my @res = ();
  for my $r ($this->srcrefs) {
    $r =~ s/:\d+$//;
    push @res, $r;
  }
  return @res;
}

=head2 $msgitm->set_flag($flag)

Sets a given flag. Ensures the flag is present in a comment starting
with C<#,>. If it already is, nothing will be changed. If a flags
comment already exists, the value will be added to its head. Otherwise
a new comment line will be inserted.

=cut

sub set_flag($$) {
  my ($this, $flag) = @_;
  for (my $i = $#{$this->{head}}; $i >= 0; --$i) {
    my $h = $this->{head}[$i];
    chomp($h);
    if ($h =~ s/^#,\s*//) {
      my @flags = split(/,\s*/, $h);
      return if grep { $_ eq $flag } @flags;
      $this->{head}[$i] = qq{#, } . join(', ', $flag, @flags) . qq{\n};
      return;
    }
  }
  push @{$this->{head}}, "#, $flag\n";
}

=head2 $msgitm->add_comments(@comments)

Adds translator comments starting with C<# > to the list of comments
leading to this message.

=cut

sub add_comments($@) {
  my ($this, @comments) = @_;
  my $i;
  for ($i = $#{$this->{head}}; $i >= 0; --$i) {
    last unless $this->{head}[$i] =~ /^#\S/;
  }
  splice(@{$this->{head}}, $i + 1, 0, map { "# $_\n" } @comments);
}

=head2 msgitm->parse($file)

Parses the named po file. Returns a list of msgitm objects.

=cut

sub parse($) {
  my ($cls, $file) = @_;
  my $itm = new;
  my @res = ( $itm );
  my $mode = "head";
  open IN, "<", $file or die "Error opening $file";
  while (<IN>) {
    $mode = $1 if (s/^(msgid|msgstr)\s+"/"/);
    if (/^"(.*)"\n$/) {
      $_ = $1;
    } elsif ($mode ne "head") {
      $itm = new;
      push @res, $itm;
      $mode = "head";
    }
    push @{$itm->{$mode}}, $_;
  }
  close IN or die "Error closing $file";
  return @res;
}

=head1 HISTORY

This script was originally written for GNU wdiff.
There it's been used for F<print-po.pl> as well as for one-time
migration of usage help messages when these were split into individual
lines.

=head1 AUTHOR

Written 2010 by Martin von Gagern

=head1 COPYRIGHT

Copyright (C) 2010 Free Software Foundation, Inc.

Licensed under the GNU General Public License version 3 or later.

=cut

1;
