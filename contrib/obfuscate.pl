#!/usr/bin/perl
# 
#  Copyright 2003 (c) Benjamin Drieu
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

#  This program obfuscates Grisbi files so that they won't reveal
#  personnal details to developers such as addresses, names, amounts,
#  balances.


open XML, $ARGV[0] or die $!;

my @obfuscate = (
  'Grisbi/Generalites/Backup',
  'Grisbi/Generalites/Titre',
  'Grisbi/Generalites/Adresse_commune',
  'Grisbi/Generalites/Adresse_secondaire',
  'Grisbi/Comptes/Compte/Details/Nom',
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
  'Grisbi/Comptes/Compte/Detail_des_operations/Operation=M,N',
  'Grisbi/Tiers/Detail_des_tiers/Tiers=Nom,Informations',
  'Grisbi/Echeances/Detail_des_echeances/Echeance=Montant',
  'Grisbi/Imputations/Detail_des_imputations/Imputation=Nom',
  'Grisbi/Imputations/Detail_des_imputations/Imputation/Sous-imputation=Nom',
  'Grisbi/Banques/Detail_des_banques/Banque=Nom,Code,Adresse,Tel,Mail,Web,Nom_correspondant,Fax_correspondant,Tel_correspondant,Mail_correspondant,Remarques',
  'Grisbi/Etats/Detail_des_etats/Etat/Nom',
);

my @state;
my %total;

foreach my $line (<XML>)
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
	$line =~ s/>([^<]*)<\//>$last $total{$last}<\//gi;
    }
    print $line;

    if ($line =~ /<\/[^>]+>/ || $line =~ /\/>/)
    {
	pop @state;
    }
}

close XML;
