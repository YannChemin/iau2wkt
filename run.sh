#!/bin/bash
make clean
make
rm -f outputIAU200*.csv
./iau2wkt naifcodes_radii_m_wAsteroids_IAU2000.csv outputIAU2000.csv IAU2000
./iau2wkt naifcodes_radii_m_wAsteroids_IAU2009.csv outputIAU2009.csv IAU2009


#for line in $(tail -n +2 outputIAU2000.csv | cut -d ',' -f2-); do gdalsrsinfo $line -o proj4 ; done

#for line in $(tail -n +2 outputIAU2000.csv); do echo "<" >> IAU2000 ; echo $($line | cut -d ',' -f1) >> IAU2000 ; echo ">" >> IAU2000 ; gdalsrsinfo $(echo $line | cut -d ',' -f2-) -o proj4 ; done
