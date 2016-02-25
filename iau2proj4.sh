#!/bin/bash
#IAU2000
#Collect Descriptions
tail -n +3 outputIAU2000.csv | grep -v Oblique | grep -e \# | sed 's:.*IAU2000.*$::' | sed '/^\s*$/d' > tempIAU2000comments.csv
#create a comment-less version
tail -n +3 outputIAU2000.csv | grep -v Oblique | grep -v \# | sed '/^\s*$/d' > temp0.csv
#remove the initial column to reveal the WKT only
cat temp0.csv | cut -d ',' -f2- > temp.csv
#Apply line by line gdalsrsinfo -o proj4
sed 's/^/gdalsrsinfo -o proj4 /' temp.csv > temp1.csv
#Remove Comment lines
sed 's:.*#.*$::g' temp1.csv > temp3.csv
#Delete Oblique_Cylindrical_equal_area lines (GDALSRSINFO bug)
sed 's:.*Oblique.*$::g' temp3.csv > temp4.csv
#Remove empty lines
sed '/^\s*$/d' temp4.csv > temp5.csv
#Generate a IAU code generation code before the conversion code
sed 's/\(.*\)IAU2000","\(.*\)"]]/<\2> /' temp5.csv > temp2.csv
#run the conversion
bash temp5.csv > error.proj4
#Remove single quotes
sed "s/'//g" error.proj4 > temp.proj4
#Paste both files together
paste temp2.csv temp.proj4 > out2000.csv
#Add comments 
paste tempIAU2000comments.csv out2000.csv > outfull.csv
#Finally, add newline before "<"
sed "s/\t</\n</" outfull.csv > iau2000.csv

#IAU2009
#Collect Descriptions
tail -n +3 outputIAU2009.csv | grep -v Oblique | grep -e \# | sed 's:.*IAU2009.*$::' | sed '/^\s*$/d' > tempIAU2009comments.csv
#create a comment-less version
tail -n +3 outputIAU2009.csv | grep -v Oblique | grep -v \# | sed '/^\s*$/d' > temp0.csv
#remove the initial column to reveal the WKT only
cat temp0.csv | cut -d ',' -f2- > temp.csv
#Apply line by line gdalsrsinfo -o proj4
sed 's/^/gdalsrsinfo -o proj4 /' temp.csv > temp1.csv
#Remove Comment lines
sed 's:.*#.*$::g' temp1.csv > temp3.csv
#Delete Oblique_Cylindrical_equal_area lines (GDALSRSINFO bug)
sed 's:.*Oblique.*$::g' temp3.csv > temp4.csv
#Remove empty lines
sed '/^\s*$/d' temp4.csv > temp5.csv
#Generate a IAU code generation code before the conversion code
sed 's/\(.*\)IAU2009","\(.*\)"]]/<\2> /' temp5.csv > temp2.csv
#run the conversion
bash temp5.csv > error.proj4
#Remove single quotes
sed "s/'//g" error.proj4 > temp.proj4
#Paste both files together
paste temp2.csv temp.proj4 > out2009.csv
#Add comments 
paste tempIAU2009comments.csv out2009.csv > outfull.csv
#Finally, add newline before "<"
sed "s/\t</\n</" outfull.csv > iau2009.csv

