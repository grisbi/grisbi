#!/usr/bin/perl 
# 
#  Copyright 2003 (c) Benjamin Drieu
#  Copyright 2005 (c) Francois Terrot
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
#  along with this program; if not, see <https://www.gnu.org/licenses/>.

#  This program obfuscates Grisbi files so that they won't reveal
#  personnal details to developers such as addresses, names, amounts,
#  balances.

my $file_in  = $ARGV[0];
my $file_out = $ARGV[1];
my $FD_OUT   = STDOUT;   # Default output is stdout ...

die "Usage: obfuscate.pl input_file [output_file]\n" if ($#ARGV <0);
die "It's not a good idea to overwrite the input file !\n" if ("$file_in" eq "$file_out");

open XML, $file_in or die $!;
open $FD_OUT, ">$file_out" or die $! if ($file_out);

my @obfuscate = (
  'Grisbi/Generalites/Backup',
  'Grisbi/Generalites/Titre',
  'Grisbi/Generalites/Adresse_commune',
  'Grisbi/Generalites/Adresse_secondaire',
  'Grisbi/Comptes/Compte/Details/Nom',
  'Grisbi/Comptes/Compte/Details/Adresse_du_titulaire',
  'Grisbi/Comptes/Compte/Details/Commentaires',
  'Grisbi/Comptes/Compte/Details/Titulaire',
  'Grisbi/Comptes/Compte/Details/Guichet',
  'Grisbi/Comptes/Compte/Details/No_compte_banque',
  'Grisbi/Comptes/Compte/Details/Cle_du_compte',
  'Grisbi/Comptes/Compte/Details/Solde_initial',
  'Grisbi/Comptes/Compte/Details/Solde_mini_voulu',
  'Grisbi/Comptes/Compte/Details/Solde_mini_autorise',
  'Grisbi/Comptes/Compte/Details/Solde_courant',
  'Grisbi/Comptes/Compte/Details/Solde_dernier_releve',
  'Grisbi/Comptes/Compte/Details/Commentaires',
  'Grisbi/Comptes/Compte/Details/Adresse_du_titulaire',
  'Grisbi/Comptes/Compte/Detail_de_Types/Type=No_en_cours',
  'Grisbi/Comptes/Compte/Detail_des_operations/Operation=M,N',
  'Grisbi/Tiers/Detail_des_tiers/Tiers=Nom,Informations',
  'Grisbi/Echeances/Detail_des_echeances/Echeance=Montant',
  'Grisbi/Categories/Detail_des_categories/Categorie=Nom',
  'Grisbi/Categories/Detail_des_categories/Categorie/Sous-categorie=Nom',
  'Grisbi/Imputations/Detail_des_imputations/Imputation=Nom',
  'Grisbi/Imputations/Detail_des_imputations/Imputation/Sous-imputation=Nom',
  'Grisbi/Banques/Detail_des_banques/Banque=Nom,Code,Adresse,Tel,Mail,Web,Nom_correspondant,Fax_correspondant,Tel_correspondant,Mail_correspondant,Remarques',
  'Grisbi/Etats/Detail_des_etats/Etat/Nom',
);

my @state;
my %total;

while (my $line = <XML>)
{
    my $last;

    if ($line =~ / *<([-a-z_]+)/gi)
    {
	push @state, $1;
	$last = $1;
	$total{$last}++;
    }
    next if not @state;

    my $path = join '/', @state;
    if (my @var = grep /$path(=.*)?$/, @obfuscate)
    {
	my $var = join "", @var;
	$var =~ s/.*=//g;
	foreach (split /,/, $var)
	{
	    my $repl;
	    if ($line =~ / $_=\"[-0-9\.,]+\"/g)
	    {
		$repl = rand 1000;
	    }
	    else 
	    {
		$repl = "$last $total{$last}";
	    }

	    $line =~ s/ $_=\"[^\"]+\"/ $_=\"$repl\"/g;
	}

	while ( $line =~ /<[-a-z_]+>[^< ]+/gi &&
	       ! ($line =~ /<\/([-a-z_]+)>/gi) &&
	       ! ($line =~ /\/>/gi) )
	{
	    $line .= <XML>;
	}

	$line =~ s/>([^<]*)<\//>$last $total{$last}<\//gi;
    }
    print $FD_OUT $line;

    if ($line =~ /<\/[^>]+>/ || $line =~ /\/>/)
    {
	pop @state;
    }
}

close XML;

__END__

=head1 MAIN

obfuscate.pl - Remove all personal information of a GRISBI account to file.

=head1 SYNOPSIS

obfuscate.pl <input_file> [output_file]

     Options:
       input_file   The GRISBI account file you want to remove the 
                    personal information from.
       output_file  The result file (optional). If this option is not
                    present, the script print of the standard output.

=head1 DESCRIPTION

    obfuscate.pl will read the given input file and remove all
    personal information.  This script can be use when your account
    make GRISBI crash and the development team request to have a copy
    of the account file to fix the problem.

    Note: Sometimes, GRISBI does no more crash with obfuscated file
    version if what failed is caused by anonymized data.

=cut

