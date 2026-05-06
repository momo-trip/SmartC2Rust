#! /usr/bin/perl

$src = ".";
$src = $ARGV[0] if @ARGV == 1;

open(LANGUAGES, '<', "$src/nls/LANGUAGES");
s!#.*!! for (@languages);

@languages = <LANGUAGES>;
%lang2 = ();

for $lang (@languages)
{
   chomp($lang);
   next if $lang =~ m/ -> /;
   next if $lang =~ m/^$/;

   $lang2 = $lang;
   $lang2 =~ s/[^a-zA-Z_0-9]/_/g;
   print "static String gv_nls_${lang2}_dat [] = {\n";
   (system "$src/ad2c '$src/nls/$lang'.dat") == 0 or die "failed to process $src/nls/${lang}.dat" ;
   print "0};\n\n";
}

print "static String* getI18N(char* locale)\n{\n";
for $lang (@languages)
{
   next if $lang =~ m/^$/;

   $lang2 = $lang;
   $lang2 =~ s/^.* -> //g;
   $lang2 =~ s/[^a-zA-Z_0-9]/_/g;
   $lang3 = $lang;
   $lang3 =~ s/ -> .*$//g;
   print "   if (!strcmp(locale, \"$lang3\")) return gv_nls_${lang2}_dat;\n";
}
print "   return 0;\n}\n";
