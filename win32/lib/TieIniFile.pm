# Based on Tie::Cfg 0.32 from Hans Oesterholt-Dijkema <hans@oesterholt-dijkema.emailt.nl>
# The modification I did are mainly : (original lines are commented by #-Tie::Cfg)
# - Package renaming to avoid confusion in original packe is installed from Tie::Cfg to TieIniFile
# - Removing lock management LockFile is not installed by default on Windows
# - add a special section named _keys_ which contzins the ordered list of the sections of the ini file 
 
#-Tie::Cfg package Tie::Cfg;
package TieIniFile;

use strict;
#-Tie::Cfg use LockFile::Simple;
use vars qw($VERSION %cnf);

$VERSION="0.32";

sub TIEHASH {
  my $class = shift;
  my $args  = { READ       => undef,
                WRITE      => undef,
                MODE       => 0640,
                LOCK       => undef,
                SEP		   => "=",
                REGSEP     => undef,
                COMMENT    => ";",
                REGCOMMENT => undef,
		CHANGE     => undef,
                @_
              };
  my $file    = $args->{READ};
  my $outfile = $args->{WRITE};
  my $lock    = $args->{LOCK};
  my $mode    = $args->{MODE};
  my $sep     = $args->{SEP};
  my $splitsep= $args->{REGSEP};
  my $comment = $args->{COMMENT};
  my $regcmnt = $args->{REGCOMMENT};
  
  if (not $splitsep) { $splitsep=$sep; }
  if (not $regcmnt)  { $regcmnt=$comment; }

  my %cnf = ();
  my $fh;
  my $val;
  my $key;
  my $prekey="";
  
  $outfile="" if (not $outfile);

  my $node = {
     CNF   => {},
     FILE  => $outfile,
     MODE  => $mode,
     LOCK  => undef,
     SEP   => $sep,
     SSEP  => $splitsep,
     CMNT  => $comment,
     SCMNT => $regcmnt,
  };

  if ($lock and $outfile) {
#-Tie::Cfg $node->{LOCK}=LockFile::Simple->make(-max => 30, -delay => 1, -nfs => 1);
#-Tie::Cfg $node->{LOCK}->lock($outfile);
  }

  if (-e $file) {

    my $section="";
    #+TieIniFile
    my @Keys =();
    #+TieIniFile
    
    open $fh, $file;
    
    while (<$fh>) {
      next if /^\s*$/;
      next if /^\s*$regcmnt/;

      if (/^\s*\[.*\]\s*$/) {
          #+TieIniFile
          eval('$cnf'."$prekey".'{\'_keys_\'}=""') if($prekey);
          #+TieIniFile
	      $section=$_;
	      $section=~s/^\s*\[//;
	      $section=~s/\]\s*$//;
	      $section=~s/^\s+//;
	      $section=~s/\s+$//;
	      
	      $prekey="";
	      for (split /\./,$section) {
		      $prekey.="{$_}";
	      }
	      
	      next;
      }
      
      ($key,$val) = split /$splitsep/,$_,2;
      $key=~s/^\s+//;$key=~s/\s+$//;
      $val=~s/^\s+//;$val=~s/\s+$//;

      for my $chg (@{$args->{CHANGE}}) {
        $val=__change($chg,$val);
      }

      if ($key=~/([\[][0-9]+[\]])$/) {
	      my $index;
	      $index=$key;
	      
	      $key=~s/([\[][0-9]+[\]])$//;
	      
	      $index=substr($index,length($key));
	      $index=~s/[\[]//;
	      $index=~s/[\]]//;
	      
	      eval('$cnf'."$prekey"."{$key}[$index]=".'"'."$val".'"');
      }
      else {
	      eval('$cnf'."$prekey"."{$key}=".'"'."$val".'"');
              #+TieIniFile
              eval('$cnf'."$prekey"."{\'_keys_\'}.=".'"'."$key".';"');
              #+TieIniFile
      }
    }
    close $fh;
    
    $node->{CNF}=\%cnf;
  }

  my $this=bless $node, $class;


return $this;
}

sub getHash {
	my $s=shift;
	my $r;
	($s,$r)=split /\./,$s,2;
	my %c;
	
	return $c{s}
}

sub __change {
  my $change=shift;
  my $value=shift;
  my $from;
  my $to;
  my $g;

  if (not $to) {
    if ($change=~/g$/) {
      $g="g";
    }
    $change=~s/^s//;
    $change=~s/^\///;
    $change=~s/g$//;
    $change=~s/\/$//;
    ($from,$to)=split /\//,$change,2;
  }

  if (not defined $g) { $g=""; }

  if ($g eq "g") {
    $value=~s/$from/$to/g;
  }
  else {
    $value=~s/$from/$to/;
  }


$value;
}


sub FETCH {
  my $self = shift;
  my $key  = shift;
  return $self->{CNF}->{$key};
}

sub STORE {
  my $self = shift;
  my $key  = shift;
  my $val  = shift;
  $self->{CNF}->{$key}=$val;
return $val;
}

sub DELETE {
  my $self = shift;
  my $key  = shift;
  delete $self->{CNF}->{$key};
}

sub EXISTS {
  my $self = shift;
  my $key  = shift;
  return exists $self->{CNF}->{$key};
}

sub FIRSTKEY {
  my $self = shift;
  my $temp = keys %{$self->{CNF}};
  return scalar each %{$self->{CNF}};
}

sub NEXTKEY {
  my $self = shift;
  return scalar each %{$self->{CNF}};
}

sub DESTROY {
  my $self = shift;
  my $fh;
  
  if ($self->{FILE}) {
    open $fh,">",$self->{FILE};
    
    print $fh $self->{CMNT}," Tie::Cfg version $VERSION (c) H. Oesterholt-Dijkema, license perl\n";
    print $fh "\n";
    
    __write_self($self->{CNF},$fh,0,$self->{SEP},$self->{CMNT},"");
    close $fh;
    chmod $self->{MODE},$self->{FILE};

    if ($self->{LOCK}) {
#-Tie::Cfg $self->{LOCK}->unlock($self->{FILE});
    }
  }
}


sub __write_self {
	my $cfg     = shift;
	my $fh      = shift;
	my $depth   = shift;
	my $sep	    = shift;
	my $cmnt    = shift;
	my $section = shift;
	
	my $key;
	my $value;
	
    # Pass 1, Keys that are no sections
    
    if ($section) {
	    print $fh "[$section]\n";
    }
    
    while (($key,$value) = each %{$cfg}) {
	    if (ref($value) ne "HASH") {
		    if (ref($value) eq "ARRAY") {
			    my $idx=0;
			    for my $element (@{$value}) {
				    print $fh "$key","[$idx]","$sep","$element\n";
				    $idx+=1;
			    }
		    }
		    else {
			    print $fh "$key","$sep","$value\n";
		    }
	    }
    }
    
    # Pass 2, keys that are sections
    
    while (($key,$value) = each %{$cfg}) {
		if (ref($value) eq "HASH") {
		    # OK, It's a section
		    
		    if ($depth==0) {
		        __write_self($value,$fh,$depth+1,$sep,$cmnt,$key);
	        }
	        else {
		        __write_self($value,$fh,$depth+1,$sep,$cmnt,$section.".".$key);
	        }
		    
	    }
    }
    
}

=pod

=head1 NAME

TieIniFile - Ties simple configuration (.ini) files to hashes.
           Handles arrays and recurrent sections.

=head1 WARNING

This version breaks previous versions as the default mode is '.ini' mode.

=head1 SYNOPSIS

  use TieIniFile;
  
  ### Sample 1

  tie my %conf, 'Tie::Cfg',
    READ   => "/etc/connect.cfg",
    WRITE  => "/etc/connect.cfg",
    MODE   => 0600,
    LOCK   => 1;

  $conf{test}="this is a test";

  untie %conf;
  
  ### Sample 2

  my $limit="10000k";

  tie my %files, 'Tie::Cfg',
    READ  => "find $dirs -xdev -type f -size +$limit -printf \"%h/%f:%k\\n\" |", SEP => ':';

  if (exists $files{"/etc/passwd"}) {
    print "You've got a /etc/passwd file!\n";
  }

  while (($file,$size) = each %newdb) {
    print "Wow! Another file bigger than $limit ($size)\n";
  }
  
  untie %files;
  
  ### Sample 3
  
  tie my %cfg, 'TieIniFile', READ => "config.cfg", WRITE => "config.cfg", SEP => ':', COMMENT => '#';
  
  my $counter=$cfg{"counter"};
  $counter+=1;
  $cfg{"counter"}=$counter;
  $cfg{"counter"}+=1;
  
  untie %cfg;
  
  ### Reading and writing an INI file
  
  tie my %ini, 'TieIniFile', READ => "config.ini", WRITE => "config.ini";
  
  my $counter=$ini{"section1"|{"counter1"};
  $counter+=1;
  $ini{"section1"}{"counter1"}=$counter;

  untie %ini;

  ### INI file with subsections
  
  tie my %ini, 'TieIniFile', READ => "config.ini";
  
  my $counter=$ini{"section1"}{"counter1"};
  $counter+=1;
  $ini{"section1"}{"counter1"}=$counter;
  
  $ini{"section1"}{"subsection1"}{"parameter"}="value";
  
  my @array;
  for(1..10) { push @array,$_; }
  $ini{"section1"}{"array"}{"a"}=@array;

  untie %ini;

  ### CHANGE option

  tie my %paths, 'Tie::Cfg', READ => "paths.ini", 
                             CHANGE => [ "s/PREFIX/$myprefix/", "s/CONF/$myconfidir/" ];

  # Do something here
  
  untie %paths;


=head1 DESCRIPTION

This module reads in a configuration file at 'tie' and writes it at 'untie'.

You can use file locking to prevent others from accessing the configuration file,
but this should only be used if the configuration file is used as a small data file 
to hold a few entries that can be concurrently accessed.
Note! In this case a persistent ".lock" file will be created.

Mode is used to set access permissions; defaults to 0640. It's only set
if a file should be written (i.e. using the WRITE keyword).

INIMODE lets you choose between Windows alike .ini configuration files and simple 
key[:=]value entried files.

Sections are addressed using a hash within a hash: For a tied %cfg the assignment:

  $cfg{"section"}{"key"}="value"
  
will write in the configuration file:

  [section]
  key=value

Keys that end on [\[][0-9]+[\]] will be interpreted as arrays and will show up
in the tied hash as an array element. For example:

  [array-section]
  var[0]=1
  var[1]=2
  var[2]=3

will show up in a tied %cfg hash like:

  for (0..2) {
    print $cfg{"array-section"}{"var"}[$_],"\n";
  }
  
Hashes of hashes are permitted:

	$cfg{"key"}{"subkey"}{"subsubkey"}{"subsubsubkey"}{"par"}="value";
	
will show up in the configuration file as:  

	[key.subkey.subsubkey.subsubsubkey]
	par=value


=head1 PREREQUISITE

Perl's Version >= 5.6.0! Please don't test this module with
anything earlier. 

=head1 AUTHOR

Original package Hans Oesterholt-Dijkema <hans@oesterholt-dijkema.emailt.nl>
Modified by Francois Terrot (francois.terrot@grisbi.org)

=head1 BUGS

Probably.

=head1 LICENCE

Perl.

=end


