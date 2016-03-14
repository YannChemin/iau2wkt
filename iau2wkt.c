#include<stdio.h>
#include<stdlib.h>
#include<string.h>

/*------------------------------------------------------------------------------
 *  Name: create_IAU2000_WKT_v2.py
 *  Author: Trent Hare
 *  Original: Jan 2006
 *  Last modified Feb 2016 to:
 * ---- update to report IAU Mean from reports, 
 * ---- Asteroids and IAU reported Comets,
 * ---- and two new projections (Mollweide and Robinson)
 *
 * Description: This Python script creates a IAU2000/2009 WKT projection strings for WMS services
 * 
 * License: Public Domain
 *
 * INPUT: (naifcodes_radii_m_wAsteroids_IAU2000.csv or naifcodes_radii_m_wAsteroids_IAU2009.csv)
 *
 * Example file format:
 * Naif_id,Body,IAU2000_Mean,IAU2000_Semimajor,IAU2000_Axisb,IAU2000_Semiminor
 * 199,Mercury,2439700.00,2439700.00,2439700.00,2439700.00
 * 299,Venus,6051800.00,6051800.00,6051800.00,6051800.00
 * 399,Earth,6371000.00,6378140.00,6378140.00,6356750.00
 * 301,Moon,1737400.00,1737400.00,1737400.00,1737400.00
 * 499,Mars,3389500.00,3396190.00,3396190.00,3376200.00
 * 401,Phobos,11100.00,13400.00,11200.00,9200.00
 * 402,Deimos,6200.00,7500.00,6100.00,5200.00
 * ...
 * OUTPUT:
 *   Example: WMS#,GEOGCS["Mars 2000",DATUM["D_Mars_2000",SPHEROID["Mars_2000_IAU_IAG",3396190,169.8944472236118]],PRIMEM["Reference_Meridian",0],UNIT["Decimal_Degree",0.0174532925199433]]
 *------------------------------------------------------------------------------
*/

void usage(){
	printf("usage:\n\tiau2wkt naifcodes_radii_m_wAsteroids_IAU2000.csv outputFileName.wtk\n");
}

const char* getfield(char* line, int num)
{
    const char* tok;
    for (tok = strtok(line, ",");
            tok && *tok;
            tok = strtok(NULL, ",\n"))
    {
        if (!--num)
            return tok;
    }
    return NULL;
}

int main(int argc, char * argv[])
{
	if(argc < 2) usage();
	FILE * stream;
	stream = fopen(argv[1],"r");
		
	/*grab year from file name*/
	int theYear = 0;
	char * pch;
	pch = strtok(argv[1], "U.");
		
	int count=0;
	while (pch != NULL)
	{
		if(count==1) theYear=atoi(pch);
		pch = strtok(NULL, "U.");
		count++;
	}
	/*printf("theYear = %d\n",theYear);*/
	/*Check if the Year has been found from filename*/
	FILE *f1;
	f1 = fopen(argv[2], "w") ;
	
	if ( theYear ){
		/*Open output file for writing*/
		fprintf(f1, "# IAU%i WKT Codes\n", theYear);
	} else {
		printf("Can't parse the year from filename: %s \n",argv[2]);	
		usage();
		exit(EXIT_FAILURE);
	}
	/*Read input file line by line*/
	char line[1024];

	int theNaifNum = 0;
	char * theTarget = NULL;
	char * theMeanChar = NULL;
	float theMean = 0.0, flattening = 0.0;
	float theA = 0.0, theB = 0.0, theC = 0.0;
	/*Start parsing line by line*/
	while (fgets(line, 1024, stream))
	{
		/*Need to replace original line in tmp each time used*/
		char* tmp = strdup(line);
		/*printf("\nline is %s",line);*/
		theNaifNum = atoi(getfield(tmp,1));
		/*printf("NAIF: %d\n",theNaifNum);*/
		tmp = strdup(line);
		if(getfield(tmp,3) != NULL){
			tmp = strdup(line);
			theMeanChar = (char *) getfield(tmp,3);
		} else {
			theMeanChar = NULL;
		}	
		/*printf("MeanChar: %s\n",theMeanChar);*/
		tmp = strdup(line);
		if (theNaifNum != 0 && theMeanChar)/*#is it a number?*/
		{ 
			theTarget = (char *) getfield(tmp,2);
			/*printf("Target: %s\t",theTarget);*/
			tmp = strdup(line);
			/*Check that third token in line is not empty*/
			if (atof(getfield(tmp,3)) != 0)/*Then Radii values exist in input table*/
			{ 
				tmp = strdup(line);
				theMean = atof(getfield(tmp,3));
				tmp = strdup(line);
				theA = atof(getfield(tmp,4));
				tmp = strdup(line);
				theB = atof(getfield(tmp,5));
				tmp = strdup(line);
				theC = atof(getfield(tmp,6));
				/*printf("Mean: %f\t",theMean);*/
				/*printf("A: %f\t",theA);*/
				/*printf("B: %f\t",theB);*/
				/*printf("C: %f\n",theC);*/
				/*#Check to see if the Mean should be used, for triaxial bodies*/
				if ((theA != theB) && (theA != theC))
				{
					theA = theMean;
					theC = theMean;
				}
				flattening = ((theA - theC) / theA);
				if (flattening < 0.0000000001 
				&& flattening > -0.0000000001
				&& flattening != 0.0)
				{
					/*Inverse flattening if too small*/
					flattening = 1.0 / flattening;
				}
				/*Even = Areocentric*/
				/*Odd = Areographic*/

				/*GEOIDS*/
				fprintf(f1, "# IAU%d WKT Codes for %s\n", theYear, theTarget);
	
				int gisCode = theNaifNum * 100;

					fprintf(f1,"# GEOGCS %s Areocentric\n", theTarget);
				fprintf(f1, "%d,GEOGCS[\"%s %d\",DATUM[\"D_%s_%d\",SPHEROID[\"%s_%d_IAU_IAG\",%f,%f]],PRIMEM[\"Reference_Meridian\",0],UNIT[\"Decimal_Degree\",0.0174532925199433],AUTHORITY[\"IAU%d\",\"%d\"]]\n",gisCode,theTarget,theYear,theTarget,theYear,theTarget,theYear,theA,flattening,theYear,gisCode);
				gisCode = theNaifNum * 100 + 1;

					fprintf(f1,"# GEOGCS %s Areographic\n", theTarget);
				fprintf(f1, "%d,GEOGCS[\"%s %d\",DATUM[\"D_%s_%d\",SPHEROID[\"%s_%d_IAU_IAG\",%f,%f]],PRIMEM[\"Reference_Meridian\",0],UNIT[\"Decimal_Degree\",0.0174532925199433],AUTHORITY[\"IAU%d\",\"%d\"]]\n",gisCode,theTarget,theYear,theTarget,theYear,theTarget,theYear,theA,flattening,theYear,gisCode);
	
				/*#Static Projections*/
				gisCode = theNaifNum * 100 + 10; /*# Equirectangular, ocentric, clon=0*/

					fprintf(f1,"# Equirectangular %s Areocentric, clon=0\n", theTarget);
				fprintf(f1,"%d,PROJCS[\"%s_Equidistant_Cylindrical\",GEOGCS[\"%s %d\",DATUM[\"D_%s_%d\",SPHEROID[\"%s_%d_IAU_IAG\",%f,%f]],PRIMEM[\"Reference_Meridian\",0],UNIT[\"Decimal_Degree\",0.0174532925199433]],PROJECTION[\"Equirectangular\"],PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",0],PARAMETER[\"Standard_Parallel_1\",0],UNIT[\"Meter\",1],AUTHORITY[\"IAU%d\",\"%d\"]]\n",gisCode,theTarget,theTarget,theYear,theTarget,theYear,theTarget,theYear,theA,flattening,theYear,gisCode);
				gisCode = theNaifNum * 100 + 11; /*# Equirectangular, ographic, clon=0*/

					fprintf(f1,"# Equirectangular %s Areographic, clon=0\n", theTarget);
				fprintf(f1, "%d,PROJCS[\"%s_Equidistant_Cylindrical\",GEOGCS[\"%s %d\",DATUM[\"D_%s_%d\",SPHEROID[\"%s_%d_IAU_IAG\",%f,%f]],PRIMEM[\"Reference_Meridian\",0],UNIT[\"Decimal_Degree\",0.0174532925199433]],PROJECTION[\"Equirectangular\"],PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",0],PARAMETER[\"Standard_Parallel_1\",0],UNIT[\"Meter\",1],AUTHORITY[\"IAU%d\",\"%d\"]]\n",gisCode,theTarget,theTarget,theYear,theTarget,theYear,theTarget,theYear,theA,flattening,theYear,gisCode);
				gisCode = theNaifNum * 100 + 12; /*# Equirectangular, ocentric, clon=180*/

					fprintf(f1,"# Equirectangular %s Areocentric, clon=180\n", theTarget);
				fprintf(f1, "%d,PROJCS[\"%s_Equidistant_Cylindrical\",GEOGCS[\"%s %d\",DATUM[\"D_%s_%d\",SPHEROID[\"%s_%d_IAU_IAG\",%f,%f]],PRIMEM[\"Reference_Meridian\",0],UNIT[\"Decimal_Degree\",0.0174532925199433]],PROJECTION[\"Equirectangular\"],PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",180],PARAMETER[\"Standard_Parallel_1\",0],UNIT[\"Meter\",1],AUTHORITY[\"IAU%d\",\"%d\"]]\n",gisCode,theTarget,theTarget,theYear,theTarget,theYear,theTarget,theYear,theA,flattening,theYear,gisCode);
				gisCode = theNaifNum * 100 + 13; /*# Equirectangular, ographic, clon=180*/

					fprintf(f1,"# Equirectangular %s Areographic, clon=180\n", theTarget);
				fprintf(f1, "%d,PROJCS[\"%s_Equidistant_Cylindrical\",GEOGCS[\"%s %d\",DATUM[\"D_%s_%d\",SPHEROID[\"%s_%d_IAU_IAG\",%f,%f]],PRIMEM[\"Reference_Meridian\",0],UNIT[\"Decimal_Degree\",0.0174532925199433]],PROJECTION[\"Equirectangular\"],PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",180],PARAMETER[\"Standard_Parallel_1\",0],UNIT[\"Meter\",1],AUTHORITY[\"IAU%d\",\"%d\"]]\n",gisCode,theTarget,theTarget,theYear,theTarget,theYear,theTarget,theYear,theA,flattening,theYear,gisCode);
	
				gisCode = theNaifNum * 100 + 14; /*# Sinusoidal, ocentric, clon=0*/

					fprintf(f1,"# Sinusoidal %s Areocentric, clon=0\n", theTarget);
				fprintf(f1, "%d,PROJCS[\"%s_Sinusoidal\",GEOGCS[\"%s %d\",DATUM[\"D_%s_%d\",SPHEROID[\"%s_%d_IAU_IAG\",%f,%f]],PRIMEM[\"Reference_Meridian\",0],UNIT[\"Decimal_Degree\",0.0174532925199433]],PROJECTION[\"Sinusoidal\"],PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",0],UNIT[\"Meter\",1],AUTHORITY[\"IAU%d\",\"%d\"]]\n",gisCode,theTarget,theTarget,theYear,theTarget,theYear,theTarget,theYear,theA,flattening,theYear,gisCode);
				gisCode = theNaifNum * 100 + 15; /*# Sinusoidal, ographic, clon=0*/

					fprintf(f1,"# Sinusoidal %s Areographic, clon=0\n", theTarget);
				fprintf(f1, "%d,PROJCS[\"%s_Sinusoidal\",GEOGCS[\"%s %d\",DATUM[\"D_%s_%d\",SPHEROID[\"%s_%d_IAU_IAG\",%f,%f]],PRIMEM[\"Reference_Meridian\",0],UNIT[\"Decimal_Degree\",0.0174532925199433]],PROJECTION[\"Sinusoidal\"],PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",0],UNIT[\"Meter\",1],AUTHORITY[\"IAU%d\",\"%d\"]]\n",gisCode,theTarget,theTarget,theYear,theTarget,theYear,theTarget,theYear,theA,flattening,theYear,gisCode);
				gisCode = theNaifNum * 100 + 16; /*# Sinusoidal, ocentric, clon=180*/

					fprintf(f1,"# Sinusoidal %s Areocentric, clon=180\n", theTarget);
				fprintf(f1, "%d,PROJCS[\"%s_Sinusoidal\",GEOGCS[\"%s %d\",DATUM[\"D_%s_%d\",SPHEROID[\"%s_%d_IAU_IAG\",%f,%f]],PRIMEM[\"Reference_Meridian\",0],UNIT[\"Decimal_Degree\",0.0174532925199433]],PROJECTION[\"Sinusoidal\"],PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",180],UNIT[\"Meter\",1],AUTHORITY[\"IAU%d\",\"%d\"]]\n",gisCode,theTarget,theTarget,theYear,theTarget,theYear,theTarget,theYear,theA,flattening,theYear,gisCode);
				gisCode = theNaifNum * 100 + 17; /*# Sinusoidal, ographic, clon=180*/

					fprintf(f1,"# Sinusoidal %s Areographic, clon=180\n", theTarget);
				fprintf(f1, "%d,PROJCS[\"%s_Sinusoidal\",GEOGCS[\"%s %d\",DATUM[\"D_%s_%d\",SPHEROID[\"%s_%d_IAU_IAG\",%f,%f]],PRIMEM[\"Reference_Meridian\",0],UNIT[\"Decimal_Degree\",0.0174532925199433]],PROJECTION[\"Sinusoidal\"],PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",180],UNIT[\"Meter\",1],AUTHORITY[\"IAU%d\",\"%d\"]]\n",gisCode,theTarget,theTarget,theYear,theTarget,theYear,theTarget,theYear,theA,flattening,theYear,gisCode);
	
				gisCode = theNaifNum * 100 + 18; /*# North Polar, ocentric, clon=0*/

					fprintf(f1,"# North Polar %s Areocentric, clon=0\n", theTarget);
				fprintf(f1, "%d,PROJCS[\"%s_North_Pole_Stereographic\",GEOGCS[\"%s %d\",DATUM[\"D_%s_%d\",SPHEROID[\"%s_%d_IAU_IAG\",%f,%f]],PRIMEM[\"Reference_Meridian\",0],UNIT[\"Decimal_Degree\",0.0174532925199433]],PROJECTION[\"Stereographic\"],PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",0],PARAMETER[\"Scale_Factor\",1],PARAMETER[\"Latitude_Of_Origin\",90],UNIT[\"Meter\",1],AUTHORITY[\"IAU%d\",\"%d\"]]\n",gisCode,theTarget,theTarget,theYear,theTarget,theYear,theTarget,theYear,theA,flattening,theYear,gisCode);
				gisCode = theNaifNum * 100 + 19; /*# North Polar, ographic, clon=0*/

					fprintf(f1,"# North Polar %s Areographic, clon=0\n", theTarget);
				fprintf(f1, "%d,PROJCS[\"%s_North_Pole_Stereographic\",GEOGCS[\"%s %d\",DATUM[\"D_%s_%d\",SPHEROID[\"%s_%d_IAU_IAG\",%f,%f]],PRIMEM[\"Reference_Meridian\",0],UNIT[\"Decimal_Degree\",0.0174532925199433]],PROJECTION[\"Stereographic\"],PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",0],PARAMETER[\"Scale_Factor\",1],PARAMETER[\"Latitude_Of_Origin\",90],UNIT[\"Meter\",1],AUTHORITY[\"IAU%d\",\"%d\"]]\n",gisCode,theTarget,theTarget,theYear,theTarget,theYear,theTarget,theYear,theA,flattening,theYear,gisCode);
				gisCode = theNaifNum * 100 + 20; /*# South Polar, ocentric, clon=0*/

					fprintf(f1,"# South Polar %s Areocentric, clon=0\n", theTarget);
				fprintf(f1, "%d,PROJCS[\"%s_South_Pole_Stereographic\",GEOGCS[\"%s %d\",DATUM[\"D_%s_%d\",SPHEROID[\"%s_%d_IAU_IAG\",%f,%f]],PRIMEM[\"Reference_Meridian\",0],UNIT[\"Decimal_Degree\",0.0174532925199433]],PROJECTION[\"Stereographic\"],PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",0],PARAMETER[\"Scale_Factor\",1],PARAMETER[\"Latitude_Of_Origin\",-90],UNIT[\"Meter\",1],AUTHORITY[\"IAU%d\",\"%d\"]]\n",gisCode,theTarget,theTarget,theYear,theTarget,theYear,theTarget,theYear,theA,flattening,theYear,gisCode);
				gisCode = theNaifNum * 100 + 21; /*# South Polar, ographic, clon=0*/

					fprintf(f1,"# South Polar %s Areographic, clon=0\n", theTarget);
				fprintf(f1, "%d,PROJCS[\"%s_South_Pole_Stereographic\",GEOGCS[\"%s %d\",DATUM[\"D_%s_%d\",SPHEROID[\"%s_%d_IAU_IAG\",%f,%f]],PRIMEM[\"Reference_Meridian\",0],UNIT[\"Decimal_Degree\",0.0174532925199433]],PROJECTION[\"Stereographic\"],PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",0],PARAMETER[\"Scale_Factor\",1],PARAMETER[\"Latitude_Of_Origin\",-90],UNIT[\"Meter\",1],AUTHORITY[\"IAU%d\",\"%d\"]]\n",gisCode,theTarget,theTarget,theYear,theTarget,theYear,theTarget,theYear,theA,flattening,theYear,gisCode);
	
				gisCode = theNaifNum * 100 + 22; /*# Mollweide, ocentric, clon=0*/

					fprintf(f1,"# Mollweide %s Areocentric, clon=0\n", theTarget);
				fprintf(f1, "%d,PROJCS[\"%s_Mollweide\",GEOGCS[\"%s %d\",DATUM[\"D_%s_%d\",SPHEROID[\"%s_%d_IAU_IAG\",%f,%f]],PRIMEM[\"Reference_Meridian\",0],UNIT[\"Decimal_Degree\",0.0174532925199433]],PROJECTION[\"Mollweide\"],PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",0],UNIT[\"Meter\",1],AUTHORITY[\"IAU%d\",\"%d\"]]\n",gisCode,theTarget,theTarget,theYear,theTarget,theYear,theTarget,theYear,theA,flattening,theYear,gisCode);
				gisCode = theNaifNum * 100 + 23; /*# Mollweide, ographic, clon=0*/

					fprintf(f1,"# Mollweide %s Areographic, clon=0\n", theTarget);
				fprintf(f1, "%d,PROJCS[\"%s_Mollweide\",GEOGCS[\"%s %d\",DATUM[\"D_%s_%d\",SPHEROID[\"%s_%d_IAU_IAG\",%f,%f]],PRIMEM[\"Reference_Meridian\",0],UNIT[\"Decimal_Degree\",0.0174532925199433]],PROJECTION[\"Mollweide\"],PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",0],UNIT[\"Meter\",1],AUTHORITY[\"IAU%d\",\"%d\"]]\n",gisCode,theTarget,theTarget,theYear,theTarget,theYear,theTarget,theYear,theA,flattening,theYear,gisCode);
				gisCode = theNaifNum * 100 + 24; /*# Mollweide, ocentric, clon=180*/

					fprintf(f1,"# Mollweide %s Areocentric, clon=180\n", theTarget);
				fprintf(f1, "%d,PROJCS[\"%s_Mollweide\",GEOGCS[\"%s %d\",DATUM[\"D_%s_%d\",SPHEROID[\"%s_%d_IAU_IAG\",%f,%f]],PRIMEM[\"Reference_Meridian\",0],UNIT[\"Decimal_Degree\",0.0174532925199433]],PROJECTION[\"Mollweide\"],PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",180],UNIT[\"Meter\",1],AUTHORITY[\"IAU%d\",\"%d\"]]\n",gisCode,theTarget,theTarget,theYear,theTarget,theYear,theTarget,theYear,theA,flattening,theYear,gisCode);
				gisCode = theNaifNum * 100 + 25; /*# Mollweide, ocentric, clon=180*/

					fprintf(f1,"# Mollweide %s Areographic, clon=180\n", theTarget);
				fprintf(f1, "%d,PROJCS[\"%s_Mollweide\",GEOGCS[\"%s %d\",DATUM[\"D_%s_%d\",SPHEROID[\"%s_%d_IAU_IAG\",%f,%f]],PRIMEM[\"Reference_Meridian\",0],UNIT[\"Decimal_Degree\",0.0174532925199433]],PROJECTION[\"Mollweide\"],PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",180],UNIT[\"Meter\",1],AUTHORITY[\"IAU%d\",\"%d\"]]\n",gisCode,theTarget,theTarget,theYear,theTarget,theYear,theTarget,theYear,theA,flattening,theYear,gisCode);
	
				gisCode = theNaifNum * 100 + 26; /*# Robinson, ocentric, clon=0*/

					fprintf(f1,"# Robinson %s Areocentric, clon=0\n", theTarget);
				fprintf(f1, "%d,PROJCS[\"%s_Robinson\",GEOGCS[\"%s %d\",DATUM[\"D_%s_%d\",SPHEROID[\"%s_%d_IAU_IAG\",%f,%f]],PRIMEM[\"Reference_Meridian\",0],UNIT[\"Decimal_Degree\",0.0174532925199433]],PROJECTION[\"Robinson\"],PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",0],UNIT[\"Meter\",1],AUTHORITY[\"IAU%d\",\"%d\"]]\n",gisCode,theTarget,theTarget,theYear,theTarget,theYear,theTarget,theYear,theA,flattening,theYear,gisCode);
				gisCode = theNaifNum * 100 + 27; /*# Robinson, ographic, clon=0*/

					fprintf(f1,"# Robinson %s Areographic, clon=0\n", theTarget);
				fprintf(f1, "%d,PROJCS[\"%s_Robinson\",GEOGCS[\"%s %d\",DATUM[\"D_%s_%d\",SPHEROID[\"%s_%d_IAU_IAG\",%f,%f]],PRIMEM[\"Reference_Meridian\",0],UNIT[\"Decimal_Degree\",0.0174532925199433]],PROJECTION[\"Robinson\"],PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",0],UNIT[\"Meter\",1],AUTHORITY[\"IAU%d\",\"%d\"]]\n",gisCode,theTarget,theTarget,theYear,theTarget,theYear,theTarget,theYear,theA,flattening,theYear,gisCode);
				gisCode = theNaifNum * 100 + 28; /*# Robinson, ocentric, clon=180*/

					fprintf(f1,"# Robinson %s Areocentric, clon=180\n", theTarget);
				fprintf(f1, "%d,PROJCS[\"%s_Robinson\",GEOGCS[\"%s %d\",DATUM[\"D_%s_%d\",SPHEROID[\"%s_%d_IAU_IAG\",%f,%f]],PRIMEM[\"Reference_Meridian\",0],UNIT[\"Decimal_Degree\",0.0174532925199433]],PROJECTION[\"Robinson\"],PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",180],UNIT[\"Meter\",1],AUTHORITY[\"IAU%d\",\"%d\"]]\n",gisCode,theTarget,theTarget,theYear,theTarget,theYear,theTarget,theYear,theA,flattening,theYear,gisCode);
				gisCode = theNaifNum * 100 + 29; /*# Robinson, ographic, clon=180*/

					fprintf(f1,"# Robinson %s Areographic, clon=180\n", theTarget);
				fprintf(f1, "%d,PROJCS[\"%s_Robinson\",GEOGCS[\"%s %d\",DATUM[\"D_%s_%d\",SPHEROID[\"%s_%d_IAU_IAG\",%f,%f]],PRIMEM[\"Reference_Meridian\",0],UNIT[\"Decimal_Degree\",0.0174532925199433]],PROJECTION[\"Robinson\"],PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",180],UNIT[\"Meter\",1],AUTHORITY[\"IAU%d\",\"%d\"]]\n",gisCode,theTarget,theTarget,theYear,theTarget,theYear,theTarget,theYear,theA,flattening,theYear,gisCode);
	
				/*#AUTO Projections*/
				gisCode = theNaifNum * 100 + 60; /*# Sinusoidal, ocentric*/

					fprintf(f1,"# Sinusoidal AUTO %s Areocentric\n", theTarget);
				fprintf(f1, "%d,PROJCS[\"%s_Sinusoidal_AUTO\",GEOGCS[\"%s %d\",DATUM[\"D_%s_%d\",SPHEROID[\"%s_%d_IAU_IAG\",%f,%f]],PRIMEM[\"Reference_Meridian\",0],UNIT[\"Decimal_Degree\",0.0174532925199433]],PROJECTION[\"Sinusoidal\"],PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",0],UNIT[\"Meter\",1],AUTHORITY[\"IAU%d\",\"%d\"]]\n",gisCode,theTarget,theTarget,theYear,theTarget,theYear,theTarget,theYear,theA,flattening,theYear,gisCode);
				gisCode = theNaifNum * 100 + 61; /*# Sinusoidal, ographic*/

					fprintf(f1,"# Sinusoidal AUTO %s Areographic\n", theTarget);
				fprintf(f1, "%d,PROJCS[\"%s_Sinusoidal_AUTO\",GEOGCS[\"%s %d\",DATUM[\"D_%s_%d\",SPHEROID[\"%s_%d_IAU_IAG\",%f,%f]],PRIMEM[\"Reference_Meridian\",0],UNIT[\"Decimal_Degree\",0.0174532925199433]],PROJECTION[\"Sinusoidal\"],PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",0],UNIT[\"Meter\",1],AUTHORITY[\"IAU%d\",\"%d\"]]\n",gisCode,theTarget,theTarget,theYear,theTarget,theYear,theTarget,theYear,theA,flattening,theYear,gisCode);
	
				gisCode = theNaifNum * 100 + 62; /*# Stereographic, ocentric, clon=0*/

					fprintf(f1,"# Stereographic AUTO %s Areocentric, clon=0\n", theTarget);
				fprintf(f1, "%d,PROJCS[\"%s_Stereographic_AUTO\",GEOGCS[\"%s %d\",DATUM[\"D_%s_%d\",SPHEROID[\"%s_%d_IAU_IAG\",%f,%f]],PRIMEM[\"Reference_Meridian\",0],UNIT[\"Decimal_Degree\",0.0174532925199433]],PROJECTION[\"Stereographic\"],PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",0],PARAMETER[\"Scale_Factor\",1],PARAMETER[\"Latitude_Of_Origin\",0],UNIT[\"Meter\",1],AUTHORITY[\"IAU%d\",\"%d\"]]\n",gisCode,theTarget,theTarget,theYear,theTarget,theYear,theTarget,theYear,theA,flattening,theYear,gisCode);
				gisCode = theNaifNum * 100 + 63; /*# Stereographic, ographic, clon=0*/

					fprintf(f1,"# Stereographic AUTO %s Areographic, clon=0\n", theTarget);
				fprintf(f1, "%d,PROJCS[\"%s_Stereographic_AUTO\",GEOGCS[\"%s %d\",DATUM[\"D_%s_%d\",SPHEROID[\"%s_%d_IAU_IAG\",%f,%f]],PRIMEM[\"Reference_Meridian\",0],UNIT[\"Decimal_Degree\",0.0174532925199433]],PROJECTION[\"Stereographic\"],PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",0],PARAMETER[\"Scale_Factor\",1],PARAMETER[\"Latitude_Of_Origin\",0],UNIT[\"Meter\",1],AUTHORITY[\"IAU%d\",\"%d\"]]\n",gisCode,theTarget,theTarget,theYear,theTarget,theYear,theTarget,theYear,theA,flattening,theYear,gisCode);
	
				gisCode = theNaifNum * 100 + 64; /*# Transverse Mercator, ocentric*/

					fprintf(f1,"# Transverse Mercator AUTO %s Areocentric\n", theTarget);
				fprintf(f1, "%d,PROJCS[\"%s_Transverse_Mercator_AUTO\",GEOGCS[\"%s %d\",DATUM[\"D_%s_%d\",SPHEROID[\"%s_%d_IAU_IAG\",%f,%f]],PRIMEM[\"Reference_Meridian\",0],UNIT[\"Decimal_Degree\",0.0174532925199433]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",0],PARAMETER[\"Scale_Factor\",0.9996],PARAMETER[\"Latitude_Of_Origin\",0],UNIT[\"Meter\",1],AUTHORITY[\"IAU%d\",\"%d\"]]\n",gisCode,theTarget,theTarget,theYear,theTarget,theYear,theTarget,theYear,theA,flattening,theYear,gisCode);
				gisCode = theNaifNum * 100 + 65; /*# Transverse Mercator, ographic*/

					fprintf(f1,"# Transverse Mercator AUTO %s Areographic\n", theTarget);
				fprintf(f1, "%d,PROJCS[\"%s_Transverse_Mercator_AUTO\",GEOGCS[\"%s %d\",DATUM[\"D_%s_%d\",SPHEROID[\"%s_%d_IAU_IAG\",%f,%f]],PRIMEM[\"Reference_Meridian\",0],UNIT[\"Decimal_Degree\",0.0174532925199433]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",0],PARAMETER[\"Scale_Factor\",0.9996],PARAMETER[\"Latitude_Of_Origin\",0],UNIT[\"Meter\",1],AUTHORITY[\"IAU%d\",\"%d\"]]\n",gisCode,theTarget,theTarget,theYear,theTarget,theYear,theTarget,theYear,theA,flattening,theYear,gisCode);
	
				gisCode = theNaifNum * 100 + 66; /*# Orthographic, ocentric*/

					fprintf(f1,"# Orthographic AUTO %s Areocentric\n", theTarget);
				fprintf(f1, "%d,PROJCS[\"%s_Orthographic_AUTO\",GEOGCS[\"%s %d\",DATUM[\"D_%s_%d\",SPHEROID[\"%s_%d_IAU_IAG\",%f,%f]],PRIMEM[\"Reference_Meridian\",0],UNIT[\"Decimal_Degree\",0.0174532925199433]],PROJECTION[\"Orthographic\"],PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Longitude_Of_Center\",0.0],PARAMETER[\"Latitude_Of_Center\",90.0],UNIT[\"Meter\",1],AUTHORITY[\"IAU%d\",\"%d\"]]\n",gisCode,theTarget,theTarget,theYear,theTarget,theYear,theTarget,theYear,theA,flattening,theYear,gisCode);
				gisCode = theNaifNum * 100 + 67; /*# Orthographic, ographic*/

					fprintf(f1,"# Orthographic AUTO %s Areographic\n", theTarget);
				fprintf(f1, "%d,PROJCS[\"%s_Orthographic_AUTO\",GEOGCS[\"%s %d\",DATUM[\"D_%s_%d\",SPHEROID[\"%s_%d_IAU_IAG\",%f,%f]],PRIMEM[\"Reference_Meridian\",0],UNIT[\"Decimal_Degree\",0.0174532925199433]],PROJECTION[\"Orthographic\"],PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Longitude_Of_Center\",0.0],PARAMETER[\"Latitude_Of_Center\",90.0],UNIT[\"Meter\",1],AUTHORITY[\"IAU%d\",\"%d\"]]\n",gisCode,theTarget,theTarget,theYear,theTarget,theYear,theTarget,theYear,theA,flattening,theYear,gisCode);
	
				gisCode = theNaifNum * 100 + 68; /*# Equidistant_Cylindrical, ocentric*/

					fprintf(f1,"# Equidistant Cylindrical AUTO %s Areocentric\n", theTarget);
				fprintf(f1, "%d,PROJCS[\"%s_Equidistant_Cylindrical_AUTO\",GEOGCS[\"%s %d\",DATUM[\"D_%s_%d\",SPHEROID[\"%s_%d_IAU_IAG\",%f,%f]],PRIMEM[\"Reference_Meridian\",0],UNIT[\"Decimal_Degree\",0.0174532925199433]],PROJECTION[\"Equirectangular\"],PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",0],PARAMETER[\"Standard_Parallel_1\",0],UNIT[\"Meter\",1],AUTHORITY[\"IAU%d\",\"%d\"]]\n",gisCode,theTarget,theTarget,theYear,theTarget,theYear,theTarget,theYear,theA,flattening,theYear,gisCode);
				gisCode = theNaifNum * 100 + 69; /*# Equidistant_Cylindrical, ographic*/

					fprintf(f1,"# Equidistant Cylindrical AUTO %s Areographic\n", theTarget);
				fprintf(f1, "%d,PROJCS[\"%s_Equidistant_Cylindrical_AUTO\",GEOGCS[\"%s %d\",DATUM[\"D_%s_%d\",SPHEROID[\"%s_%d_IAU_IAG\",%f,%f]],PRIMEM[\"Reference_Meridian\",0],UNIT[\"Decimal_Degree\",0.0174532925199433]],PROJECTION[\"Equirectangular\"],PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",0],PARAMETER[\"Standard_Parallel_1\",0],UNIT[\"Meter\",1],AUTHORITY[\"IAU%d\",\"%d\"]]\n",gisCode,theTarget,theTarget,theYear,theTarget,theYear,theTarget,theYear,theA,flattening,theYear,gisCode);
	
				gisCode = theNaifNum * 100 + 70; /*# Lambert_Conformal_Conic, ocentric*/

					fprintf(f1,"# Lambert Conformal Conic AUTO %s Areocentric\n", theTarget);
				fprintf(f1, "%d,PROJCS[\"%s_Lambert_Conformal_Conic_AUTO\",GEOGCS[\"%s %d\",DATUM[\"D_%s_%d\",SPHEROID[\"%s_%d_IAU_IAG\",%f,%f]],PRIMEM[\"Reference_Meridian\",0],UNIT[\"Decimal_Degree\",0.0174532925199433]],PROJECTION[\"Lambert_Conformal_Conic_2SP\"],PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",0],PARAMETER[\"Standard_Parallel_1\",-20],PARAMETER[\"Standard_Parallel_2\",20],PARAMETER[\"Latitude_Of_Origin\",0],UNIT[\"Meter\",1],AUTHORITY[\"IAU%d\",\"%d\"]]\n",gisCode,theTarget,theTarget,theYear,theTarget,theYear,theTarget,theYear,theA,flattening,theYear,gisCode);
				gisCode = theNaifNum * 100 + 71; /*# Lambert_Conformal_Conic, ographic*/

					fprintf(f1,"# Lambert Conformal Conic AUTO %s Areographic\n", theTarget);
				fprintf(f1, "%d,PROJCS[\"%s_Lambert_Conformal_Conic_AUTO\",GEOGCS[\"%s %d\",DATUM[\"D_%s_%d\",SPHEROID[\"%s_%d_IAU_IAG\",%f,%f]],PRIMEM[\"Reference_Meridian\",0],UNIT[\"Decimal_Degree\",0.0174532925199433]],PROJECTION[\"Lambert_Conformal_Conic_2SP\"],PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",0],PARAMETER[\"Standard_Parallel_1\",-20],PARAMETER[\"Standard_Parallel_2\",20],PARAMETER[\"Latitude_Of_Origin\",0],UNIT[\"Meter\",1],AUTHORITY[\"IAU%d\",\"%d\"]]\n",gisCode,theTarget,theTarget,theYear,theTarget,theYear,theTarget,theYear,theA,flattening,theYear,gisCode);
	
				gisCode = theNaifNum * 100 + 72; /*# Lambert_Azimuthal_Equal_Area, ocentric*/

					fprintf(f1,"# Lambert Azimuthal Equal Area AUTO %s Areocentric\n", theTarget);
				fprintf(f1, "%d,PROJCS[\"%s_Lambert_Azimuthal_Equal_Area_AUTO\",GEOGCS[\"%s %d\",DATUM[\"D_%s_%d\",SPHEROID[\"%s_%d_IAU_IAG\",%f,%f]],PRIMEM[\"Reference_Meridian\",0],UNIT[\"Decimal_Degree\",0.0174532925199433]],PROJECTION[\"Lambert_Azimuthal_Equal_Area\"],PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",0],PARAMETER[\"Latitude_Of_Origin\",90],UNIT[\"Meter\",1],AUTHORITY[\"IAU%d\",\"%d\"]]\n",gisCode,theTarget,theTarget,theYear,theTarget,theYear,theTarget,theYear,theA,flattening,theYear,gisCode);
				gisCode = theNaifNum * 100 + 73; /*# Lambert_Azimuthal_Equal_Area, ographic*/

					fprintf(f1,"# Lambert Azimuthal Equal Area AUTO %s Areographic\n", theTarget);
				fprintf(f1, "%d,PROJCS[\"%s_Lambert_Azimuthal_Equal_Area_AUTO\",GEOGCS[\"%s %d\",DATUM[\"D_%s_%d\",SPHEROID[\"%s_%d_IAU_IAG\",%f,%f]],PRIMEM[\"Reference_Meridian\",0],UNIT[\"Decimal_Degree\",0.0174532925199433]],PROJECTION[\"Lambert_Azimuthal_Equal_Area\"],PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",0],PARAMETER[\"Latitude_Of_Origin\",90],UNIT[\"Meter\",1],AUTHORITY[\"IAU%d\",\"%d\"]]\n",gisCode,theTarget,theTarget,theYear,theTarget,theYear,theTarget,theYear,theA,flattening,theYear,gisCode);
	
				gisCode = theNaifNum * 100 + 74; /*# Mercator, ocentric*/

					fprintf(f1,"# Mercator AUTO %s Areocentric\n", theTarget);
				fprintf(f1, "%d,PROJCS[\"%s_Mercator_AUTO\",GEOGCS[\"%s %d\",DATUM[\"D_%s_%d\",SPHEROID[\"%s_%d_IAU_IAG\",%f,%f]],PRIMEM[\"Reference_Meridian\",0],UNIT[\"Decimal_Degree\",0.0174532925199433]],PROJECTION[\"Mercator_1SP\"],PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",0],PARAMETER[\"Standard_Parallel_1\",0],UNIT[\"Meter\",1],AUTHORITY[\"IAU%d\",\"%d\"]]\n",gisCode,theTarget,theTarget,theYear,theTarget,theYear,theTarget,theYear,theA,flattening,theYear,gisCode);
				gisCode = theNaifNum * 100 + 75; /*# Mercator, ographic*/

					fprintf(f1,"# Mercator AUTO %s Areographic\n", theTarget);
				fprintf(f1, "%d,PROJCS[\"%s_Mercator_AUTO\",GEOGCS[\"%s %d\",DATUM[\"D_%s_%d\",SPHEROID[\"%s_%d_IAU_IAG\",%f,%f]],PRIMEM[\"Reference_Meridian\",0],UNIT[\"Decimal_Degree\",0.0174532925199433]],PROJECTION[\"Mercator_1SP\"],PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",0],PARAMETER[\"Standard_Parallel_1\",0],UNIT[\"Meter\",1],AUTHORITY[\"IAU%d\",\"%d\"]]\n",gisCode,theTarget,theTarget,theYear,theTarget,theYear,theTarget,theYear,theA,flattening,theYear,gisCode);
	
				gisCode = theNaifNum * 100 + 76; /*# Albers, ocentric*/

					fprintf(f1,"# Albers AUTO %s Areocentric\n", theTarget);
				fprintf(f1, "%d,PROJCS[\"%s_Albers_AUTO\",GEOGCS[\"%s %d\",DATUM[\"D_%s_%d\",SPHEROID[\"%s_%d_IAU_IAG\",%f,%f]],PRIMEM[\"Reference_Meridian\",0],UNIT[\"Decimal_Degree\",0.0174532925199433]],PROJECTION[\"Albers_Conic_Equal_Area\"],PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",0.0],PARAMETER[\"Standard_Parallel_1\",60.0],PARAMETER[\"Standard_Parallel_2\",20.0],PARAMETER[\"Latitude_Of_Origin\",40.0],UNIT[\"Meter\",1],AUTHORITY[\"IAU%d\",\"%d\"]]\n",gisCode,theTarget,theTarget,theYear,theTarget,theYear,theTarget,theYear,theA,flattening,theYear,gisCode);
				gisCode = theNaifNum * 100 + 77; /*# Albers, ographic*/

					fprintf(f1,"# Albers AUTO %s Areographic\n", theTarget);
				fprintf(f1, "%d,PROJCS[\"%s_Albers_AUTO\",GEOGCS[\"%s %d\",DATUM[\"D_%s_%d\",SPHEROID[\"%s_%d_IAU_IAG\",%f,%f]],PRIMEM[\"Reference_Meridian\",0],UNIT[\"Decimal_Degree\",0.0174532925199433]],PROJECTION[\"Albers_Conic_Equal_Area\"],PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",0.0],PARAMETER[\"Standard_Parallel_1\",60.0],PARAMETER[\"Standard_Parallel_2\",20.0],PARAMETER[\"Latitude_Of_Origin\",40.0],UNIT[\"Meter\",1],AUTHORITY[\"IAU%d\",\"%d\"]]\n",gisCode,theTarget,theTarget,theYear,theTarget,theYear,theTarget,theYear,theA,flattening,theYear,gisCode);
	
				gisCode = theNaifNum * 100 + 78; /*# Oblique Cylindrical Equal Area, ocentric*/

					fprintf(f1,"# Oblique Cylindrical Equal Area AUTO %s Areocentric\n", theTarget);
				fprintf(f1, "%d,PROJCS[\"%s_Oblique_Cylindrical_Equal_Area_AUTO\",GEOGCS[\"%s %d\",DATUM[\"D_%s_%d\",SPHEROID[\"%s_%d_IAU_IAG\",%f,%f]],PRIMEM[\"Reference_Meridian\",0],UNIT[\"Decimal_Degree\",0.0174532925199433]],PROJECTION[\"Oblique_Cylindrical_Equal_Area\"],PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",0.0],PARAMETER[\"Standard_Parallel_1\",0.0],UNIT[\"Meter\",1],AUTHORITY[\"IAU%d\",\"%d\"]]\n",gisCode,theTarget,theTarget,theYear,theTarget,theYear,theTarget,theYear,theA,flattening,theYear,gisCode);
				gisCode = theNaifNum * 100 + 79; /*# Oblique Cylindrical Equal Area, ographic*/

					fprintf(f1,"# Oblique Cylindrical Equal Area AUTO %s Areographic\n", theTarget);
				fprintf(f1, "%d,PROJCS[\"%s_Oblique_Cylindrical_Equal_Area_AUTO\",GEOGCS[\"%s %d\",DATUM[\"D_%s_%d\",SPHEROID[\"%s_%d_IAU_IAG\",%f,%f]],PRIMEM[\"Reference_Meridian\",0],UNIT[\"Decimal_Degree\",0.0174532925199433]],PROJECTION[\"Oblique_Cylindrical_Equal_Area\"],PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",0.0],PARAMETER[\"Standard_Parallel_1\",0.0],UNIT[\"Meter\",1],AUTHORITY[\"IAU%d\",\"%d\"]]\n",gisCode,theTarget,theTarget,theYear,theTarget,theYear,theTarget,theYear,theA,flattening,theYear,gisCode);
	
				gisCode = theNaifNum * 100 + 80; /*# Mollweide, ocentric*/

					fprintf(f1,"# Mollweide AUTO %s Areocentric\n", theTarget);
				fprintf(f1, "%d,PROJCS[\"%s_Mollweide_AUTO\",GEOGCS[\"%s %d\",DATUM[\"D_%s_%d\",SPHEROID[\"%s_%d_IAU_IAG\",%f,%f]],PRIMEM[\"Reference_Meridian\",0],UNIT[\"Decimal_Degree\",0.0174532925199433]],PROJECTION[\"Mollweide\"],PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",0],UNIT[\"Meter\",1],AUTHORITY[\"IAU%d\",\"%d\"]]\n",gisCode,theTarget,theTarget,theYear,theTarget,theYear,theTarget,theYear,theA,flattening,theYear,gisCode);
				gisCode = theNaifNum * 100 + 81; /*# Mollweide, ographic*/

					fprintf(f1,"# Mollweide AUTO %s Areographic\n", theTarget);
				fprintf(f1, "%d,PROJCS[\"%s_Mollweide_AUTO\",GEOGCS[\"%s %d\",DATUM[\"D_%s_%d\",SPHEROID[\"%s_%d_IAU_IAG\",%f,%f]],PRIMEM[\"Reference_Meridian\",0],UNIT[\"Decimal_Degree\",0.0174532925199433]],PROJECTION[\"Mollweide\"],PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",0],UNIT[\"Meter\",1],AUTHORITY[\"IAU%d\",\"%d\"]]\n",gisCode,theTarget,theTarget,theYear,theTarget,theYear,theTarget,theYear,theA,flattening,theYear,gisCode);
	
				gisCode = theNaifNum * 100 + 82; /*# Robinson, ocentric*/

					fprintf(f1,"# Robinson AUTO %s Areocentric\n", theTarget);
				fprintf(f1, "%d,PROJCS[\"%s_Robinson_AUTO\",GEOGCS[\"%s %d\",DATUM[\"D_%s_%d\",SPHEROID[\"%s_%d_IAU_IAG\",%f,%f]],PRIMEM[\"Reference_Meridian\",0],UNIT[\"Decimal_Degree\",0.0174532925199433]],PROJECTION[\"Robinson\"],PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",0],UNIT[\"Meter\",1],AUTHORITY[\"IAU%d\",\"%d\"]]\n",gisCode,theTarget,theTarget,theYear,theTarget,theYear,theTarget,theYear,theA,flattening,theYear,gisCode);
				gisCode = theNaifNum * 100 + 83; /*# Robinson, ographic*/

					fprintf(f1,"# Robinson AUTO %s Areographic\n", theTarget);
				fprintf(f1, "%d,PROJCS[\"%s_Robinson_AUTO\",GEOGCS[\"%s %d\",DATUM[\"D_%s_%d\",SPHEROID[\"%s_%d_IAU_IAG\",%f,%f]],PRIMEM[\"Reference_Meridian\",0],UNIT[\"Decimal_Degree\",0.0174532925199433]],PROJECTION[\"Robinson\"],PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",0],UNIT[\"Meter\",1],AUTHORITY[\"IAU%d\",\"%d\"]]\n",gisCode,theTarget,theTarget,theYear,theTarget,theYear,theTarget,theYear,theA,flattening,theYear,gisCode);
			}
		}
		free(tmp);
	}
	fclose(stream);
	fclose(f1);
	return(EXIT_SUCCESS);
}
