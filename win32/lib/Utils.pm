#!perl -w
# 
#  Copyright 2004 (c) François Terrot
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
#  This script is a part of the windows configure.pl script for Windows
#  Please do not use it directly but use configure.pl instead
# --------------------------------------------------------------------------
use strict;
package Utils;
use Cwd qw(chdir realpath getcwd);
use File::Basename;
use File::Copy;
use File::Path;
use File::Spec qw(File::Spec->abs2rel file_name_is_absolute);
use IniFile;


my $g_force_mkdir = 1;
my $g_debug  = 0;
my $g_outdir = realpath(getcwd());
my $config_ini = "config.ini";

sub TieConfig
{
    my %config ;
    tie %config, 'IniFile', READ => "$config_ini" if (-f $config_ini);
    return %config;
}

sub _debug
{
    my ($a_force_mkdir) = @_;
    $g_force_mkdir = $a_force_mkdir if (defined($a_force_mkdir));
    return $g_force_mkdir;
}

sub _force_mkdir
{
    my ($a_force_mkdir) = @_;
    $g_force_mkdir = $a_force_mkdir if (defined($a_force_mkdir));
    return $g_force_mkdir;
}

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
    _mkdir ($dir) if ($g_force_mkdir);

    $dir = realpath($dir); 
    if ( $g_outdir ne $dir)
    {
        die "*** ERROR * Couldn't change directory to ${dir}\n" unless (-d "$dir");
        print "Entering ".File::Spec->abs2rel($dir)."\n" unless (defined($quiet));
        $g_outdir = $dir;
    } 
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
    print "_cp ${path}, ${file}\n" if($g_debug);
    # if file is a directory - we copy all the contains
    die "**** CONFIGURATION ERROR * $path/$file does not exist\n" unless ( -d "$path");
    if (defined ($ref_ignore))
    {
        IGNORE: foreach (@$ref_ignore)
        {
            if ($file =~ m/$_/ )
            {
                print "Ignoring $file\n";
                return;
            }
        }
    }
    if ( -d "$path/$file")
    {
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
            _mkdir ( dirname ("${g_outdir}/${file}"));
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
sub _pwd
{
    return $g_outdir;
}

sub _addFileToZip
{
    my($zip,$path,$file,$ref_ignore) = @_;
    my $member = undef;
    my $res    = undef;

    if (defined ($ref_ignore))
    {
        IGNORE: foreach (@$ref_ignore)
        {
            if ($file =~ m/$_/ )
            {
                print "Ignoring $file\n";
                return '-1';
            }
        }
    }

    if ( -d "${g_outdir}/$file") # $file is a directory, we add all sub directory files
    {
        _cd ($file);
        opendir DIR, "${g_outdir}";
        rewinddir DIR;
        LS: foreach (readdir DIR)
        {
            next LS if ( /\.{1,2}$/ ) ; # Skip . and ..
            $res = _addFileToZip ($zip,"$path/$file","$_",$ref_ignore);
            $member = $res unless ($res == '-1');
        }
        closedir DIR;
        _cd('..','TRUE');
    }
    elsif ( $file =~ m/^\/.*\/$/ ) # regexp
    {
        opendir DIR, "${g_outdir}";
        rewinddir DIR;
        LSREGEXP: foreach (readdir DIR)
        {
            next LSREGEXP if ( /\.{1,2}$/ ) ; # Skip . and ..
            $res = _addFileToZip ($zip,"$path","$_",$ref_ignore) if ( eval ("'$_' =~ m$file") ) ;
            $member = $res unless ($res == '-1');
        }
        closedir DIR;
    }

    elsif (-f "${g_outdir}/${file}")
    {
        print "adding ${file} in ${path}\n";
        $member = $zip->addFile("${g_outdir}/${file}","${path}/${file}");
    }
    return $member;
}
1;
