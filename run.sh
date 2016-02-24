#!/bin/bash
make clean
make
rm -f outputIAU200*.csv
./iau2wkt naifcodes_radii_m_wAsteroids_IAU2000.csv outputIAU2000.csv
./iau2wkt naifcodes_radii_m_wAsteroids_IAU2009.csv outputIAU2009.csv

