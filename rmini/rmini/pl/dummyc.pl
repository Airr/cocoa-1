#!/usr/bin/env perl
use strict;
use warnings;

my $cfile = $ARGV[0];
my $lc = 1;
my $kindPrev = "comment";
my $lnPrev = 1;
my $line = 1;
open(CFILE, $cfile) or die "could not open $cfile: $!";
open(PIPE, "/usr/local/Cellar/ctags/5.8/bin/ctags -xu $cfile|") or die "couldn't start pipe: $!";
while ($line)
{
  last unless $line;
  # R_USE_SIGNALS    macro        24 errors.c         #define R_USE_SIGNALS 1
  $line = <PIPE>;
  my $name;
  my $kind;
  my $ln;
  my $ls;
  if ($line)
  {
    $line =~ /^(\S+)\s+(\w+)\s+(\d+)\s+$cfile\s+(.+)/;
    $name = $1;
    $kind = $2;
    $ln = $3;
    $ls = $4;
  }
  else
  {
    $ln = 1000000;
  }

  if ($kindPrev eq "function") 
  {
    my $isFunctionBody = 0;
    my $hasStartBrace = 0;
    my $hasReturnValue = 1;
    for (my $i = $lnPrev; $i < $ln; $i++)
    {
      my $cline = <CFILE>;
      last unless $cline;
       
      if ($cline =~ /void/
          or $cline =~ /NORET/)
      {
        $hasReturnValue = 0;  
      }
      if ($isFunctionBody == 0 and $cline =~ /\{/)
      {
        $isFunctionBody = 1;
        unless ($cline =~ /^\{/)
        {
          $hasStartBrace = 1;
          print $cline;
        }
      }
      elsif ($cline =~ /^\}/)
      {
        $isFunctionBody = 0;
        print "{\n" if $hasStartBrace == 0;
        if ($hasReturnValue == 1)
        {
          print "  return 0;\n";
        }
        else
        {
          print "  return;\n";
        }
      }
      unless ($isFunctionBody == 1)
      {
        print $cline;
      }
    }
  }
  else
  {
    for (my $i = $lnPrev; $i < $ln; $i++)
    {
      my $cline = <CFILE>;
      last unless $cline;
      print $cline;
    }
  }
  $kindPrev = $kind;
  $lnPrev = $ln;
}
close(PIPE) or die "couldn't close pipe: $! $?";
close(CFILE) or die "couldn't close $cfile: $! $?";
