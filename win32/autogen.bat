@rem = '--*-Perl-*--
@echo off
if "%OS%" == "Windows_NT" goto WinNT
perl -x -S "%0" %1 %2 %3 %4 %5 %6 %7 %8 %9
goto endofperl
:WinNT
perl -x -S %0 %*
if NOT "%COMSPEC%" == "%SystemRoot%\system32\cmd.exe" goto endofperl
if %errorlevel% == 9009 echo You do not have Perl in your PATH.
if errorlevel 1 goto script_failed_so_exit_with_non_zero_val 2>nul
goto endofperl
@rem ';
#!perl -w
#line 15
#  -------------------------------------------------------------------------
#                               GRISBI for Windows
#  -------------------------------------------------------------------------
# $Id: autogen.bat,v 1.1.2.1 2004/08/15 14:24:54 teilginn Exp $
#  -------------------------------------------------------------------------
# 
#  Copyleft 2004 (c) François Terrot
#  
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License as
#  published by the Free Software Foundation; either version 2 of the
#  License, or (at your option) any later version.
#                                                                            
#  This program is distributed in the hope that it will be useful, but
#  WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  General Public License for more details.
#                                                                            
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
#  USA
#  -------------------------------------------------------------------------
#  History:
#
#  $Log: autogen.bat,v $
#  Revision 1.1.2.1  2004/08/15 14:24:54  teilginn
#  autogen script and templates for Windows
#
#
# --------------------------------------------------------------------------
# Todo:
#
# - To implemente cache system for using same value on different projects
# - To try to autodetect used tools location
# - To clean command line args
# - To add auto documentation for the -h/--help args
# - To add a clean command
# - To add the build directory management
# - To manage and explain all error cases
# 
# **************************************************************************
# local $CvsId = '$RevisionName';
# ==========================================================================
# PACKAGES {{{
# ==========================================================================
# **************************************************************************
# @INC {{{ (c) Copyleft francois terrot
# **************************************************************************
# These few lines are here to manage the fact the script may have to use
# not present modules without having to modify the current installation:
# - put your pure perl module in the 'lib' directory
# - put the 'cpan' module required by the script in 'site/lib'
# - put the dll files required by modules in the 'site/arch/5x' directory
#       where <x> depend form the Perl version ( Perl 5.x)
#       Note: it should a good idea to provide Perl 5.6 and 5.8 dlls
#       
# The way you are able to distribution your script for perl 5.6 and 5.8
# without having to ask the user to update his installation.
# If you want to restricted you script to Perl 5.8 modify the require line
BEGIN 
{
    require 5.006;
    use File::Basename; my $INSTDIR = dirname $0; $INSTDIR =~ s/\\/\//g;
    our $LOCALLIBDIR  = $INSTDIR."/lib";
    our $LOCALSITEDIR = $INSTDIR."/site/lib";
    our $LOCALARCHDIR = $INSTDIR."/site/arch/5" . int (($]-5) *1000);
}

use lib($LOCALLIBDIR);
use lib($LOCALSITEDIR);
use lib($LOCALARCHDIR);

( "$^O" =~ m/MSWin32/ ) or die "This script is only designed for MS Win32 operating System\n";
# }}} @INC
# **************************************************************************

use strict;
use Cwd qw(chdir realpath getcwd);
use File::Basename;
use File::Copy;
use File::Path;
use File::Spec qw(File::Spec->abs2rel file_name_is_absolute);
use Pod::Usage;
use Getopt::Long;
use Win32::TieRegistry(Delimiter=>"/");
use TieIniFile;
# }}}
# ==========================================================================
# GLOBALS                                                {{{ PART_1
# ==========================================================================
our $g_debug  = 0;
my $config_ini = "autogen.ini";   # configuration file (config.ini)
my %cache_in;                     # configuration parameters from cache
my %config;                       # configuration parameters from config.ini
my %opt;                          # configuration parameters from arg line
my @targets = ();
my $g_outdir = realpath(getcwd());

my $builddir   ;
my $gtkbindir ;
my $gccbasedir ;
my $grisbidir  ;
my $startdir   ;





# }}} GLOBALS
# ==========================================================================
# FUNCTIONS                                              {{{ PART_2
# ==========================================================================
# --------------------------------------------------------------------------
# Utils                                                             PART_2_1
# --------------------------------------------------------------------------
# \brief List the source files from the given directory
#   Get the source ( ".c", ) files from the given directory, exclude
#   "variable*.c" files and remove the directory name.
#   The result is a Makefile compatible file list
#
# \param  $dir path (relative or absolute) of the directory to search in
# \return $    the source file list, each file is <space> separed
# --------------------------------------------------------------------------
sub _get_c_file_list # {{{
{
    my ($dir) = @_;
    my $srcs  = "";
    opendir DIR,$dir or die "autogen.pl *** WARNING * unable get sources list : $@\n";
    rewinddir DIR;
    LS_DIR: while (defined (my $f = readdir DIR )) 
    { 
        next LS_DIR if ( $f =~ m/^variables.*\.c/ ) ;
        $srcs .= " $f" if ( $f =~ m/\.c$/ ); }
    close DIR;
    return $srcs;
} # }}}
# --------------------------------------------------------------------------
#/**
# * create directy if not exists in the current selected by _cd directory
# *
# * \param $dir directory to create 
# *
# */
sub _mkdir # {{{
{
    my ($dir) = @_;
    $dir = "${g_outdir}/${dir}" unless (File::Spec->file_name_is_absolute($dir));
    print "_mkdir $dir\n" if ($g_debug);
    unless (-d "$dir")
    {
        print "Creating ".File::Spec->abs2rel($dir)."\n";
        eval { mkpath($dir) } or die "*** ERROR * Couldn't create dir : $@\n";
    }
} # }}}
#/**
# * virtually change directory, the new directory is the base of all commands
# *     the directory will be created if not exists
# *     $g_outdir = realpath $g_outdir/$dir if $dir is a relative path
# *     $g_outdir = $dir if $dir is a absolute path
# *
# * \param $dir new directory destination , 
# *
# */
sub _cd # {{{
{
    my ($dir,$quiet) = @_;
    
    $dir = "${g_outdir}/${dir}" unless (File::Spec->file_name_is_absolute(${dir}));
    print "_cd $dir\n" if ($g_debug);
    _mkdir ($dir);

    $dir = realpath($dir); 
    print "_cd $dir\n" if ($g_debug);
    if ( $g_outdir ne $dir)
    {
        die "*** ERROR * Couldn't change directory to ${dir}\n" unless (-d "$dir");
        print "Entering ".File::Spec->abs2rel($dir)."\n" unless (defined($quiet));
        $g_outdir = $dir;
    } 
    return $g_outdir;
} #  }}} 
#/**
# * copy file if it not exists or is out of date (not implemented yet) in the destination directory
# *
# * \param $path directory the file is to be copied from
# * \param $file location to copy form $path to $g_outdir 
# * \param $ref_ignore reference to a list of regexp defining the file to ignore
# *
# */
sub _cp # {{{
{
    my $cp_oldpwd = $g_outdir;
    my ($path,$file,$ref_ignore) = @_;
    eval "\$path = \"$path\";"; 
    eval "\$ref_ignore = $ref_ignore" if (defined ($ref_ignore));
    $path =~ s/\/\//\//g;
    print "_cp ${path}, ${file}\n" if($g_debug);
    # if file is a directory - we copy all the contains
    die "**** CONFIGURATION ERROR * $path/$file does not exist\n" unless ( -d "$path");
    if (defined ($ref_ignore))
    {
        print "is $file matching with $ref_ignore ?\n" if ($g_debug);
        IGNORE: foreach (@$ref_ignore)
        {
            if ($file =~ m/$_/ )
            {
                print "Ignoring $file\n";
                return;
            }
        }
        print "NO\n" if ($g_debug);
    }
    if ( -d "$path/$file")
    {
        print "$path/$file is a directory\n" if ($g_debug);
        _cd ($file);
        opendir DIR, "${path}/${file}";
        rewinddir DIR;
        LS: foreach (readdir DIR)
        {
            next LS if ( /\.{1,2}$/ ) ; # Skip . and ..
            _cp ("$path/$file","$_",$ref_ignore);
        }
        closedir DIR;
        _cd('..','TRUE');
    }
    elsif ( $file =~ m/^\/.*\/$/ ) # regexp
    {
        opendir DIR, "${path}";
        rewinddir DIR;
        LSREGEXP: foreach (readdir DIR)
        {
            next LSREGEXP if ( /\.{1,2}$/ ) ; # Skip . and ..
            _cp ("$path","$_",$ref_ignore) if ( eval ("'$_' =~ m$file") ) ;
        }
        closedir DIR;
    }
    else 
    {
        if ( (!-f "${g_outdir}/${file}") || ( (stat("${g_outdir}/${file}"))[9] <  (stat("${path}/${file}"))[9] ))
        {
            _mkdir ( dirname "${g_outdir}/${file}");
            print "Copying $file\n";
            copy ("${path}/${file}","${g_outdir}/${file}");
        }
        else
        {
            print "$file is uptodate\n";
        }
    }
    _cd ($cp_oldpwd,'quiet');
    
} # }}}
# \brief : compare the last modification time of two files
#   it does $ledt cmp $right
# \param $left 
# \param $right
# \return -1 if $left is older than $right
# \return  0 if $left has the same modification time as $right
# \return  1 if $ledt is more recent than $right
# --------------------------------------------------------------------------
sub _last_modified # {{{
{
    my ($left,$right) = @_;
    my @left_stat = stat($left);
    my @right_stat = stat($right);
    return 1 if ($left_stat[9] > $right_stat[9]);
    return -1 if ($left_stat[9] < $right_stat[9]);
    return 0;
} # }}}
# --------------------------------------------------------------------------
# \brief returns the location of a file from a directory list 
# \param  $prog complete name (with extension) of the file to search for
# \param  $dirs list of directory in a semi columns separed string 
#  \caveas : the function use %PATH% if the $dirs param is omitted or 'false'.
# \return $ the directory name in the Unix way without last dir separator
#           undef is returned is no occurence of the file was found.
# --------------------------------------------------------------------------
sub _which # {{{
{
    my ($prog,$dirs) = @_;
    $dirs = $ENV{'PATH'} unless ($dirs);
    foreach my $dir (split /;/,$dirs)
    {
        (-f "$dir\\$prog") && do { (my $udir = $dir) =~ s/\\/\//g; return $udir; };
    }
    return undef;
} # }}}
sub _dirname # {{{
{
    my ($path,$sep) = @_;
    $sep = '/' unless ($sep);
    $sep =~ s/^/\\/;
    $path =~ s/[\w\d_\.\s-]+$sep?$// if ($path);
    return $path;
}
# }}}
sub _uninstallstring # {{{
{
    my ($uninstallprogkey,$uninstallinfokey) = @_;
    $uninstallinfokey = "UninstallString" unless ($uninstallinfokey);
    ( my $uninstallstring = $Registry->{"HKEY_LOCAL_MACHINE/Software/Microsoft/Windows/CurrentVersion/Uninstall/$uninstallprogkey/$uninstallinfokey"}) =~ s/\"//g; 
    $uninstallstring =~ s/\\/\//g if ($uninstallstring);
    return $uninstallstring;
} # }}} 
# --------------------------------------------------------------------------
# Configuration functions                                           PART_2_2
# --------------------------------------------------------------------------
sub _configuration_autodetect # {{{
{
    # Mingw : in the path
    $config{'directories'}{'mingw'} =  _dirname _which 'gcc.exe'  unless ($config{'directories'}{'mingw'});
    die "*** ERROR *** autogen is not able to detect the location of gcc.exe\n\n \
        Please add the MinGw installation directory in your PATH\n" unless ($config{'directories'}{'mingw'});
    # Perl : in the path
    $config{'directories'}{'perl'}  =  _dirname _which 'perl.exe' unless ($config{'directories'}{'perl'});
    # NSIS from PATH, then from registry 
    $config{'directories'}{'nsis'} = _which 'makensis.exe' unless ($config{'directories'}{'nsis'});
    $config{'directories'}{'nsis'} = _dirname _uninstallstring "NSIS" unless ($config{'directories'}{'nsis'});
    
    #
    # GTK runtime and development files
    # 
    $config{'directories'}{'gtkbin'} = _dirname _uninstallstring "WinGTK-2_is1" unless ($config{'directories'}{'gtkbin'});
    
    my $gtkdevvers ;
    my $gtkbinvers ;

    # Gtk 2.2.4-3 install package ...
    unless ($config{'directories'}{'gtkbin'})
    {
        $config{'directories'}{'gtkbin'} = _dirname _uninstallstring "GTK+ Runtime Environment_is1" unless ($config{'directories'}{'gtkbin'});
        $config{'directories'}{'gtkdev'} = _dirname _uninstallstring "GTK+ Development Environment_is1" unless ($config{'directories'}{'gtkdev'});
        $gtkdevvers = _uninstallstring "GTK+ Development Environment_is1","DisplayVersion";
        $gtkbinvers = _uninstallstring "GTK+ Runtime Environment_is1","DisplayVersion";
    }
    else # package from www.gtk.org/win32
    {
        ( $gtkbinvers = _uninstallstring "WinGTK-2_is1","DisplayName" ) =~ s/^.*(\d\.\d.\d).*$/$1/;
        $gtkdevvers = $gtkbinvers;
        $config{'directories'}{'gtkdev'} = $config{'directories'}{'gtkbin'};
    }

    #
    # Other usefull location
    #
    $config{'directories'}{'prefix'} = "../build" unless ($config{'directories'}{'prefix'});
    $config{'directories'}{'grisbi'} = dirname(getcwd()) unless ($config{'directories'}{'grisbi'});

    # Extract version number from pwd
    (my $core = basename dirname getcwd() ) =~ s/^grisbi-//;
    $config{'grisbi'}{'core'} = $core unless ($config{'grisbi'}{'core'});

    my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime(time);
    $year %= 100;
    $year  =~ s/^(\d)$/0$1/;
    $mon  =~ s/^(\d)$/0$1/;
    $mday  =~ s/^(\d)$/0$1/;

    $config{'grisbi'}{'build'} = "$year$mon$mday" unless ($config{'grisbi'}{'build'});
    $config{'grisbi'}{'patch'} = "$year$mon$mday" unless ($config{'grisbi'}{'patch'});

    unless ($gtkdevvers eq $gtkbinvers)
    {
        die "*** ERROR *** Your GTK Runtime version ($gtkbinvers) is not the same as your GTK Development version ($gtkdevvers)\n" unless ($gtkdevvers eq $gtkbinvers);
    }
    $config{'grisbi'}{'gtk'} = "$gtkbinvers" unless ($config{'grisbi'}{'gtk'});
} # }}}
sub _configuration_check # {{{
{
    unless ($config{'directories'}{'gtkbin'})
    {
        print "*** ERROR *** Unable to detect gtk runtime files location\n";
        print "   Please edit autogen.ini to set the 'gtkbin' of the 'directories section' with the correct value\n";
        die "\n";
    }
    unless ($config{'directories'}{'gtkdev'})
    {
        print "*** ERROR *** Unable to detect gtk development files location\n";
        print "   Please edit autogen.ini to set the 'gtkdev' of the 'directories section' with the correct value\n";
        die "\n";
    }
    die "Unable to determine prefix directory name, please edit $config_ini\n"   unless ( $config{'directories'}{'prefix'} );
    die "gtk runtime directory    is empty, please edit $config_ini\n" unless ( $config{'directories'}{'gtkbin'} );
    die "mingw directory  is empty, please edit $config_ini\n" unless ( $config{'directories'}{'mingw'} );
    #die "no target defined, please use configure.pl or make \n" unless ($targets);

} # }}}

# --------------------------------------------------------------------------
# Template management                                               PART_2_3
# --------------------------------------------------------------------------
# \brief callbacks function use for configuring created files
#   There are use to replace the  @NAME@ variables
#   The call returns the modified version of the string taken as input
#   _cb_default is called when no callback is defined
# \param  $ the line without the last '\n' to modify
# \return $ the modified line
# --------------------------------------------------------------------------
sub _cb_default #{{{
{
    return shift;
} #}}}
sub _cb_config_ini #{{{
{
    my $l = shift;
    READ_INI: {
        last READ_INI if ($l =~ s/GCCBASEDIR/$config{'directories'}{'mingw'}/);
        last READ_INI if ($l =~ s/PERLBASEDIR/$config{'directories'}{'perl'}/);
        last READ_INI if ($l =~ s/NSISBINDIR/$config{'directories'}{'nsis'}/);
        last READ_INI if ($l =~ s/GTKBINDIR/$config{'directories'}{'gtkbin'}/);
        last READ_INI if ($l =~ s/GTKDEVDIR/$config{'directories'}{'gtkdev'}/);
        last READ_INI if ($l =~ s/BUILDDIR/$config{'directories'}{'prefix'}/);
        last READ_INI if ($l =~ s/WINGRISBIDIR/$config{'directories'}{'grisbi'}/);
        last READ_INI if ($l =~ s/CORE/$config{'grisbi'}{'core'}/);
        last READ_INI if ($l =~ s/BUILD/$config{'grisbi'}{'build'}/);
        last READ_INI if ($l =~ s/PATCH/$config{'grisbi'}{'patch'}/);
        last READ_INI if ($l =~ s/GTKVERS/$config{'grisbi'}{'gtk'}/);
    }
    
    $l =~ s/\s+$//;
    return $l;
} #}}}
sub _cb_config_h # {{{
{
     (my $l = shift ) =~ s/\@VERSION\@/$config{'grisbi'}{'core'} for Windows (GTK $config{'grisbi'}{'gtk'})/ ;
    return $l;
} # }}}
sub _cb_config_nsh #{{{
{
    return shift;
} #}}}
sub _cb_grisbi_private_h # {{{
{
    my $l = shift;
    my ($major,$minor,$release ) = split /\./,$config{'grisbi'}{'core'};
    $release =~ s/(\d).*$/$1/;
    PRIVATE_H:
    {
        $l =~ s/\@MAJOR\@/$major/;
        $l =~ s/\@MINOR\@/$minor/;
        $l =~ s/\@RELEASE\@/$release/;
        $l =~ s/\@PATCH\@/$config{'grisbi'}{'patch'}/;
        last PRIVATE_H if ($l =~ s/\@CORE\@/$config{'grisbi'}{'core'}/);
    }
    $l =~ s/\s+$//;
    return $l;
} # }}}
sub _cb_makefile # {{{
{
    my $l = shift;
    READ_MAKE: {
        last READ_MAKE if ($l =~ s/\@GCCBASEDIR\@/$config{'directories'}{'mingw'}/);
        last READ_MAKE if ($l =~ s/\@GTKBASEDIR\@/$config{'directories'}{'gtkbin'}/);
        last READ_MAKE if ($l =~ s/\@GTKDEVDIR\@/$config{'directories'}{'gtkdev'}/);
        last READ_MAKE if ($l =~ s/\@PERLBASEDIR\@/$config{'directories'}{'perl'}/);
        last READ_MAKE if ($l =~ s/\@BUILDDIR\@/$config{'directories'}{'prefix'}/);
        ( $l =~ /\@SRCS\@/ ) && do {
            my $srcs = _get_c_file_list("../src");
            $l =~ s/\@SRCS\@/$srcs/;
            last READ_MAKE;
        };
        ( $l =~ /\@WIN32\@/ ) && do {
            my $srcs = _get_c_file_list("../win32");
            $l =~ s/\@WIN32\@/$srcs/;
            last READ_MAKE;
        };
    }
    
    $l =~ s/\s+$//;
    return $l;
} # }}}
# --------------------------------------------------------------------------
# \brief the _configure function takes the template configuration data from
#   the _DATA_ area of the script and create the corresponding file
# 
# <file\s+name="" [src=""][ callback=""]>[\n.*\n]</file>
#
# \param $target
# \param $callback
#
# \return $ 0 if the target file template as not been found
# --------------------------------------------------------------------------
sub _templates_configure # {{{
{
    my ($target,$callback) = @_;
    my $found = 0;
    my $src   = '__FILE__';
    $callback = '_cb_default' unless (defined($callback));
    
    # 
    READ_DATA: while (<DATA>)
    {
        last READ_DATA if (/^__END__/);

        ( /^\<file\s+name="$target"/ ) && do {
            $found    = 1;
            $src      = $1 if ( /src="([\w\d\._-]*)"/ );
            $callback = $1 if ( /callback="([\w\d\._-]*)"/ );

            print "Generating $target \n" unless ($target eq $config_ini);
            open FD,">$target" or do { warn "*** autogen.pl ERROR * Unable to create $target from $src : $@\n"; return;};

            if ( $src ne '__FILE__')
            {
                open SRC,"$src" or do { warn "*** autogen.pl ERROR * Unable to load template $src : $@\n";return;};
                LOAD_TEMPLATE: while (defined (my $line = <SRC>))
                { 
                    chomp $line; 
                    $line = eval "$callback ".'$line';
                    print FD $line."\n";
                }
                close SRC;
            }
            else
            {
                next READ_DATA;
            }
        };

        ( /\<\/file\>/ ) && do {
            if ($found)
            {
                close FD;
                last READ_DATA;
            }
        };

        next READ_DATA if (!$found);
        
        chomp; 
        $_ = eval "$callback ".'$_';
        print FD $_."\n";
    }

    return ($found);
    
} # }}}
# --------------------------------------------------------------------------
#   Extract from _DATA_ part the file of templates available
#   Exclude the autogen.ini line from the result
# 
# \return @ list of template description lines 
# --------------------------------------------------------------------------
sub _templates_get_targets_list() # {{{
{
    my @templates = ();
    my $data = 0;
    open FD,$0 or die "autogen.pl *** ERROR * Unable to read $0 : $@\n";
    SEEK2DATA: while (defined($_ = <FD>)) { next SEEK2DATA unless ( /^__DATA__/) ; last SEEK2DATA; }

    READ_DATA: while (defined ($_ = <FD>))
    {
        last READ_DATA if ( /^__END__/ ); 
        #push @templates,($1) if ( /^\<file\s+name="([\w\d\._-]*)"/ );
        if ( /^\<file\s+name="([\w\d\._-]*)"/ )
        {
            next READ_DATA if ($1 eq "$config_ini");
            push @templates,($1) ;
        }
    }
    close FD;
    return @templates;

} # }}}

sub _templates_clean
{
    return 0;
}
# ------------------------------------------------------------------------
#   Extract from _RUNTIME_ part the file of targets available
# 
# \return @ list of targets description lines 
# --------------------------------------------------------------------------
sub _runtime_get_targets_list() # {{{
{
    my @templates = ();
    my $data = 0;
    open FD,$0 or die "autogen.pl *** ERROR * Unable to read $0 : $@\n";
    SEEK2DATA: while (defined($_ = <FD>)) { next SEEK2DATA unless ( /^__DATA__/) ; last SEEK2DATA; }

    READ_DATA: while (defined ($_ = <FD>))
    {
        last READ_DATA if ( /^__END__/ ); 
        #push @templates,($1) if ( /^\<file\s+name="([\w\d\._-]*)"/ );
        if ( /^\<file\s+name="([\w\d\._-]*)"/ )
        {
            next READ_DATA if ($1 eq "$config_ini");
            push @templates,($1) ;
        }
    }
    close FD;
    return @templates;

} # }}}
sub _runtime_configure # {{{
{
    my ($targets) = @_;
    $targets = join ':', qw/build obj etc lib dlls pixmaps help it fr/ unless $targets;
    
my $gtkcompliant   = 0;
my $runtime_prefix = 0;
my $target_found   = 0;
my $with_target_dest=0;
my $dest_dir       = "";
    open FD,$0 or die "autogen.pl *** ERROR Unable to read $0 : $@\n";
    SEEK2RUNTIME: while(<FD>) { next SEEK2RUNTIME unless ( /^#__RUNTIME__#/); last SEEK2RUNTIME;};
    READ_RUNTIME: while(<FD>)
    {
        last READ_RUNTIME if ( /^__END__$/);
        ( /^\<gtk\s+version=[\"\']?([\d\.\w-]*)[\"\']?/ ) && do {
            my $gtkversion = $1;
            if (($gtkversion =~ m/all/i )||($gtkversion eq $config{'grisbi'}{'gtk'}))
            {
                $gtkcompliant = 1;
            }
        };
        ( /\<\/gtk\>/ ) && do { $gtkcompliant = 0; };
       next READ_RUNTIME unless($gtkcompliant);

       ( /^\s*\<prefix\>/ )   && do { $runtime_prefix = 1 ; };
       ( /^\s*\<\/prefix\>/ ) && do { $runtime_prefix = 0 ; };
        
       next READ_RUNTIME unless($runtime_prefix);

       ( /^\s*\<target\s+name=([\/\w\d\.-]*)\s+(?:dest=([\/\"\'\$\{\}\w\d\s\.-]*))?/ ) && do {
           my $target_name = $1;
           $with_target_dest = (defined($2));
           ($dest_dir = $2) =~ s/\s*$// if (defined($2));
           $target_found = 1 if (grep { /$target_name/ } $targets);
           if ($target_found)
           {
               eval "_cd(\"${builddir}/${dest_dir}\");" if ($with_target_dest);
           }
       };
       (( $target_found ) && ( /^\s*<copy\s+(.*)\s*>(.*)<\/copy>/ )) && do
       {
           my $dest = $dest_dir;
           my $filter = $2;
           my %F ;
           foreach my $kv (split /\s/,$1)
           {
               my ($k,$v) = split /=/,$kv;
               $F{$k} = $v if defined($v);
           }

           $dest = $F{'dest'} if (defined($F{'dest'}));
           eval "_cd(\"${builddir}/${dest}\")" if (defined($F{'dest'}));
           if (defined($F{'ignore'}))
           {
                eval '_cp("$F{from}",$filter,$F{ignore});';
            }
            else
            {
                eval '_cp("$F{from}",$filter);';
            }
            _cd("..") if ((defined($F{'dest'}))&&($F{'dest'}));

       };
       ( /\<\/target\>/ ) && do {
           if ($target_found)
           {
               _cd("..",'TRUE') if (($with_target_dest)&&($dest_dir));
           }
           $target_found     = 0;
           $with_target_dest = 0;
           $dest_dir         = undef;
       };
       
       next READ_RUNTIME unless($target_found);
    }
    close FD;
    exit;
} # }}}
# }}}
# ==========================================================================
# __MAIN___                                              {{{ PART_3
# ==========================================================================

# Load config.ini if any
tie %config, 'TieIniFile', READ => $config_ini if (-f $config_ini);

my $opt_help      = undef;
my $opt_usage     = undef;
my $opt_force     = undef;
my $opt_prefix    = $config{'directories'}{'prefix'};
my $opt_npatch    = undef;
my $opt_runtime   = undef;
my $opt_templates = undef;
my $opt_clean     = undef;

# Commmand lines arguments management {{{
GetOptions(
    "h|?"         => \$opt_usage,
    "help|aide"   => \$opt_help,
    "force|f"     => \$opt_force,
    "prefix=s"    => \$opt_prefix,
    "new-patch"   => \$opt_npatch,
    "runtime"     => \$opt_runtime,
    "templates"   => \$opt_templates,
) or pod2usage(2);

# Check arguments value
pod2usage(verbose=>1,exit_status=>0) if $opt_usage;
pod2usage(verbose=>2,exit_status=>0) if $opt_help;
pod2usage(verbose=>1,exit_status=>1) if ($opt_runtime and $opt_templates );

#default action is template creation
$opt_templates = 'default' if (!$opt_runtime and !$opt_templates );

# reconfigure configuration depending arg lines
$config{'grisbi'}{'patch'} = undef if ($opt_npatch);

#
# }}}

_configuration_autodetect;

#  * First in a clean directory : create ini and ask to complete it
if ((! -f $config_ini)||($opt_npatch))
{
    _templates_configure($config_ini,'_cb_config_ini') or die "autogen.pl *** ERROR * Unable to create $config_ini: $@\n";
    print "Your $config_ini file has been created, please check it and restart the autogen.bat command\n";
    exit;
}

_configuration_check;

#
# Build runtime environnement in the prefix directory only if $opt_runtime
#
if ($opt_runtime)
{
    $builddir   = $config{'directories'}{'prefix'};
    $gtkbindir  = $config{'directories'}{'gtkbin'};
    $gccbasedir = $config{'directories'}{'mingw'};
    $grisbidir  = $config{'directories'}{'grisbi'};
    $startdir   = $g_outdir;
    _runtime_configure join ':',@ARGV
}

# 
# Configure the files from the template
#
if ($opt_templates)
{
    TEMPLATES_ARGV: foreach (@ARGV)
    {
        ( /^clean$/ ) && do { $opt_clean = 1; next TEMPLATES_ARGV; }; 
        push @targets,($_) ; 
    }
    @targets = _templates_get_targets_list() if ($#targets < 0);

    foreach my $target ( @targets )
    {
        if (($opt_force)&&($opt_clean)&&(-f $target))
        {
            _templates_clean($target) or warn "*** autogen.bat WARNING * Unable to remove $target ***\n";
        }
        elsif (($opt_force)||(!-f $target)|| (_last_modified($config_ini,$target) == 1))
        {
            _templates_configure($target) or warn "*** autogen.bat ERROR * Unable to found a template for $target***\n";
        }
    }
}

0;

# }}}

# ==========================================================================
# FILE TEMPLATES                                         {{{ PART_4
# ==========================================================================
__DATA__
<file name="autogen.ini" callback"_cb_config_ini" >  {{{
[grisbi]
core      = CORE
build     = BUILD
patch     = PATCH
gtk       = GTKVERS
[environment]
compiler  = gcc ; only gcc is supported yet
[directories]
gtkdev    = GTKDEVDIR
gtkbin    = GTKBINDIR
perl      = PERLBASEDIR
mingw     = GCCBASEDIR
nsis      = NSISBINDIR
prefix    = BUILDDIR
grisbi    = WINGRISBIDIR
installer = 
</file> # }}}
<file name="Grisbi_private.rc"> {{{
// THIS FILE WILL BE OVERWRITTEN BY DEV-C++!
// DO NOT EDIT!


A ICON MOVEABLE PURE LOADONCALL DISCARDABLE "Grisbi.ico"
</file> }}}
<file name="Grisbi_private.h" callback="_cb_grisbi_private_h"> {{{
// THIS FILE WILL BE OVERWRITTEN BY DEV-C++!
// DO NOT EDIT!

#ifndef GRISBI_PRIVATE_H
#define GRISBI_PRIVATE_H

// VERSION DEFINITIONS
#define VER_STRING	"@MAJOR@.@MINOR@.@RELEASE@.@PATCH@"
#define VER_MAJOR	@MAJOR@
#define VER_MINOR	@MINOR@
#define VER_RELEASE	@RELEASE@
#define VER_BUILD	@PATCH@
#define COMPANY_NAME	"grisbi.org"
#define FILE_VERSION	""
#define FILE_DESCRIPTION	"Personal accounting software using GTK under GPL Licence (ported on Windows using the Dev-C++ IDE)"
#define INTERNAL_NAME	        "Grisbi"
#define LEGAL_COPYRIGHT	        "2004 grisbi.org"
#define LEGAL_TRADEMARKS	"grisbi.org"
#define ORIGINAL_FILENAME	"grisbi.exe"
#define PRODUCT_NAME	        "Grisbi"
#define PRODUCT_VERSION	        "@CORE@"

#endif //GRISBI_PRIVATE_H
</file> }}}
<file name="config.h"   src="config-win32.in" callback="_cb_config_h"></file>
<file name="Makefile"   src="Makefile-win32.am" callback="_cb_makefile"></file>
<file name="config.nsh" src="config-nsh.in" callback="_cb_config_nsh"></file>
__END__ 
# }}} 
# ==========================================================================
#__RUNTIME__# {{{
<gtk version="all">
<prefix>
    <target name=build dest= ></target>
    <target name=obj   dest=obj></target>
    <target name=help  dest=help>
        <copy from=${grisbidir}/help ignore=['CVS','Makefile','topic']>C</copy>
        <copy from=${grisbidir}/help ignore=['CVS','Makefile','topic']>fr</copy>
    </target>
    <target name=pixmaps  dest=pixmaps>
        <copy from=${grisbidir}/pixmaps ignore=['CVS','Makefile','topic']>.</copy>
    </target>
</prefix>
</gtk>
<gtk version="2.4.3">
<prefix>
    <target name=dlls dest= >
        <copy from=${gtkbindir}/bin >/dll$/</copy>
        <copy from=${gtkbindir}/lib >/dll$/</copy>
    </target>
    <target name=etc dest=etc>
        <copy from=${gtkbindir}/etc ignore=['.*~$']>fonts</copy>
        <copy from=${gtkbindir}/etc ignore=['.*~$']>gtk-2.0</copy>
        <copy from=${gtkbindir}/etc ignore=['.*~$']>pango</copy>
    </target>
    <target name=lib dest=lib>
        <copy from=${gtkbindir}/lib ignore=['include','.*h$']>gtk-2.0</copy>
        <copy from=${gtkbindir}/lib ignore=['include','.*h$']>pango</copy>
    </target>
    <target name=it dest= >
        <copy dest=lib   from=${gtkbindir}/lib >locale/it</copy>
        <copy dest=share from=${gtkbindir}/share >locale/it</copy>
    </target>
    <target name=fr dest= >
        <copy dest=lib   from=${gtkbindir}/lib >locale/fr</copy>
        <copy dest=share from=${gtkbindir}/share >locale/fr</copy>
    </target>
</prefix>
<libs>
</libs>
</gtk>
__END__
# }}}
# ==========================================================================
# DOCUMENTATION                                          {{{ PART_5
# ==========================================================================
=head1 NAME

autogen.bat - Perl script to configure Grisbi for Windows build environment

=head1 SYNOPSIS

autogen.bat [-f] [clean]

autogen.bat [-h|--help] 

autogen.bat [-f] 

autogen.bat [-runtime [[obj] [help] [pixmaps] [dlls] [lib] [it] [fr]]]

=head1 OPTIONS

=over 8

=item B<-h>

Print a brief help message with command line syntax summary and exit.

=item B<--help>

Print this complete help message and exit 

=back

=head1 DESCRIPTION
    
    B<This program> is used to configure the build environment of Grisbi for
    Windows. It creates all needs files and configures them .

=head2 autogen.ini
    All common information about the builded version and tools configuration
    are stored in the autogen.ini file created by the first run of the script.

=head2 created files

=back
=cut

#}}}
# ==========================================================================
# vim:sts=4:ft=perl:

__END__
:endofperl
