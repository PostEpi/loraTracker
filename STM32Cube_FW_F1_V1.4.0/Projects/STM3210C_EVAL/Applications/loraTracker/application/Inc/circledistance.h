#ifndef _PROJ_DEF_H
#define _PROJ_DEF_H

//
//  Common Util Function & Define
// 

#define _EPSLN	  1.0e-10

#ifndef _PIE_DEFINE
#define _PIE_DEFINE
	#define _RADIAN_PER_DEGREE	0.0174532925199432957692369076848833
	#define _DEGREE_PER_RADIAN	57.2957795130823208767981548170142
	#define _PIE 				3.14159265358979323846264338327950288419716939937510
	#define _HALF_PIE			1.5707963267948966192313216916395
	#define _TWO_PIE			6.283185307179586476925286766558
#endif

inline double I_GPN2RAD( double gpn ) { return gpn * _RADIAN_PER_DEGREE; }
inline double I_RAD2GPN( double rad ) { return rad * _DEGREE_PER_RADIAN; }

#define BESSEL_AXIS		(6377397.155)
#define BESSEL_FLAT     (1.0/299.1528128)
#define BESSEL_INVFLAT	299.1528128
#define BESSEL_SQUARED  (2.*BESSEL_FLAT-BESSEL_FLAT*BESSEL_FLAT)

#define WGS84_AXIS      (6378137.)
#define WGS84_FLAT      (1./298.257223563)
#define WGS84_INVFLAT	298.257223563
#define WGS84_SQUARED   (2.*WGS84_FLAT-WGS84_FLAT*WGS84_FLAT)

#define GRS80_AXIS      (6378137.0)
#define GRS80_FLAT      (1./298.257222101)
#define GRS80_INVFLAT	298.257222101
#define GRS80_SQUARED   (2.*GRS80_FLAT-GRS80_FLAT*GRS80_FLAT)


#define _MAX_VAL 4
#define _MAXLONG 2147483647.
#define _DBLLONG 4.61168601e18

#define sign( x ) ( (x)<0 ? -1:1 )


double GetDistance(double dLat1, double dLon1, double dLat2, double dLon2);

double degreesToRadians(double degrees);
double distanceInKmBetweenEarthCoordinates(double lat1, double lon1, double lat2, double lon2);


#endif