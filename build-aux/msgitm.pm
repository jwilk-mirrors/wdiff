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

sub head($@) {
  my ($this, @newval) = @_;
  return accessor($this, wantarray, 'head', @newval);
}

sub msgid($@) {
  my ($this, @newval) = @_;
  return accessor($this, wantarray, 'msgid', @newval);
}

sub msgstr($@) {
  my ($this, @newval) = @_;
  return accessor($this, wantarray, 'msgstr', @newval);
}

sub block($) {
  my ($this) = @_;
  my $res = $this->head;
  if (@{$this->{msgid}}) {
    $res .= qq{msgid "} . join(qq{"\n"}, $this->msgid) . qq{"\n};
    $res .= qq{msgstr "} . join(qq{"\n"}, $this->msgstr) . qq{"\n};
  }
  return $res;
}

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

sub srcfiles($) {
  my ($this) = @_;
  my @res = ();
  for my $r ($this->srcrefs) {
    $r =~ s/:\d+$//;
    push @res, $r;
  }
  return @res;
}

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

sub add_comments($@) {
  my ($this, @comments) = @_;
  my $i;
  for ($i = $#{$this->{head}}; $i >= 0; --$i) {
    last unless $this->{head}[$i] =~ /^#\S/;
  }
  splice(@{$this->{head}}, $i + 1, 0, map { "# $_\n" } @comments);
}

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

1;
