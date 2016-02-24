#!/bin/bash
make clean
make
rm -f outputIAU200*.csv
./iau2wkt naifcodes_radii_m_wAsteroids_IAU2000.csv outputIAU2000.csv
./iau2wkt naifcodes_radii_m_wAsteroids_IAU2009.csv outputIAU2009.csv


#for line in $(tail -n +2 outputIAU2000.csv | cut -d ',' -f2-); do gdalsrsinfo $line -o proj4 ; done

