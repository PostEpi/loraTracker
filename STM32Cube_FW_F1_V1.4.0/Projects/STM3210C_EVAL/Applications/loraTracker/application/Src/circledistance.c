#include <stdio.h>
#include <math.h>
#include "circledistance.h"
#include "debug.h"

#define M_PI       3.14159265358979323846

typedef struct _SMPTransferMeractorParam {
	double		fAxis;    // Major Axis
	double		fFlat;    // Flat
	double		fCentralMeridian;
	double		fLatitudeOfOrigin;
	double		fScaleFactor;
	double		fFalseEasting;
	double		fFalseNorthing;
}SMPTransferMeractorParam;

static double AdjustLongitude(double x)
{
	long count = 0;
	for (; ; ) {
		if (fabs(x) <= _PIE)
			break;
		else if (((long)fabs(x / _PIE)) < 2)
			x = x - (sign(x) * _TWO_PIE);
		else if (((long)fabs(x / _TWO_PIE)) < _MAXLONG) {
			x = x - (((long)(x / _TWO_PIE))*_TWO_PIE);
		}
		else if (((long)fabs(x / (_MAXLONG * _TWO_PIE))) < _MAXLONG) {
			x = x - (((long)(x / (_MAXLONG * _TWO_PIE))) * (_TWO_PIE * _MAXLONG));
		}
		else if (((long)fabs(x / (_DBLLONG * _TWO_PIE))) < _MAXLONG) {
			x = x - (((long)(x / (_DBLLONG * _TWO_PIE))) * (_TWO_PIE * _DBLLONG));
		}
		else
			x = x - (sign(x) * _TWO_PIE);
		count++;
		if (count > _MAX_VAL)
			break;
	}

	return(x);
}

static void TM_Forword(double lon_deg, double lat_deg, double* x, double* y)
{
	SMPTransferMeractorParam param = { GRS80_AXIS,  GRS80_FLAT,  127.000000000000, 38.0, 1, 200000, 600000 };

	double a, b, f, recf, fe, nfn, ok;
	double es, ebs;
	double ap, bp, cp, dp, ep, fp, gp, hp, ip;
	double olam, ophi;
	double fScaleFactor;
	double fFalseEasting;
	double fFalseNorthing;

	olam = I_GPN2RAD(param.fCentralMeridian);
	ophi = I_GPN2RAD(param.fLatitudeOfOrigin);
	fScaleFactor = param.fScaleFactor;
	fFalseEasting = param.fFalseEasting;
	fFalseNorthing = param.fFalseNorthing;

	// Internal Params 
	a = param.fAxis;
	f = param.fFlat;
	recf = 1. / f;
	b = a*(recf - 1.) / recf;
	fe = fFalseEasting;
	nfn = fFalseNorthing;
	ok = fScaleFactor;

	es = (a*a - b*b) / (a*a);
	ebs = (a*a - b*b) / (b*b);

	ap = 1 + 3 * es / 4 + 45 * pow(es, 2) / 64 + 175 * pow(es, 3) / 256 + 11025 * pow(es, 4) / 16384 + 43659 * pow(es, 5) / 65536;
	bp = 3 * es / 4 + 15 * pow(es, 2) / 16 + 525 * pow(es, 3) / 512 + 2205 * pow(es, 4) / 2048 + 72765 * pow(es, 5) / 65536;
	cp = 15 * pow(es, 2) / 64 + 105 * pow(es, 3) / 256 + 2205 * pow(es, 4) / 4096 + 10395 * pow(es, 5) / 16384;
	dp = 35 * pow(es, 3) / 512 + 315 * pow(es, 4) / 2048 + 31185 * pow(es, 5) / 131072;
	ep = 315 * pow(es, 4) / 16384 + 3465 * pow(es, 5) / 65536 + 99099 * pow(es, 6) / 1048480 + 4099100 * pow(es, 7) / 29360128 + 348423075 * pow(es, 8) / 1879048192;
	fp = 693 * pow(es, 5) / 131072 + 9009 * pow(es, 6) / 524288 + 4099100 * pow(es, 7) / 117440512 + 26801775 * pow(es, 8) / 469762048;
	gp = 3003 * pow(es, 6) / 2097150 + 315315 * pow(es, 7) / 58720256 + 11486475 * pow(es, 8) / 939524096;
	hp = 45045 * pow(es, 7) / 117440512 + 765765 * pow(es, 8) / 469762048;
	ip = 765765 * pow(es, 8) / 7516192768.0;


	// 계산
	double slam = I_GPN2RAD(lon_deg);
	double sphi = I_GPN2RAD(lat_deg);

	double dlam = AdjustLongitude(slam - olam);

	double tmd = a*(1 - es)*(
		ap*(sphi - ophi) - bp*(sin(2 * sphi) - sin(2 * ophi)) / 2
		+ cp*(sin(4 * sphi) - sin(4 * ophi)) / 4 - dp*(sin(6 * sphi) - sin(6 * ophi)) / 6
		+ ep*(sin(8 * sphi) - sin(8 * ophi)) / 8 - fp*(sin(10 * sphi) - sin(10 * ophi)) / 10
		+ gp*(sin(12 * sphi) - sin(12 * ophi)) / 12 - hp*(sin(14 * sphi) - sin(14 * ophi)) / 14
		+ ip*(sin(16 * sphi) - sin(16 * ophi)) / 16);

	double s = sin(sphi);
	double c = cos(sphi);
	double t = tan(sphi); // sin/cos
	double eta = ebs*c*c;
	double sn = a / sqrt(1 - es*s*s);

	double t1, t2, t3, t4, t5, t6, t7, t8, t9;
	t1 = tmd*ok;
	t2 = sn*s*c*ok / 2.;
	t3 = sn*s*c*c*c*ok*(5. - t*t + 9.*eta + 4.*eta*eta) / 24.;
	t4 = sn*s*c*c*c*c*c*ok*(61. - 58.*t*t + t*t*t*t + 270.*eta - 330.*t*t*eta
		+ 445.*eta*eta + 324.*eta*eta*eta - 680 * eta*eta*t*t + 88.*eta*eta*eta*eta
		- 600.*t*t*eta*eta*eta - 192.*t*t*eta*eta*eta*eta) / 720.;
	t5 = sn*s*c*c*c*c*c*c*c*ok*(1385. - 3111.*t*t + 543.*t*t*t*t - t*t*t*t*t*t) / 40320.;

	*y = nfn + t1 + dlam*dlam*t2 + dlam*dlam*dlam*dlam*t3 + dlam*dlam*dlam*dlam*dlam*dlam*t4
		+ dlam*dlam*dlam*dlam*dlam*dlam*dlam*dlam*t5;

	t6 = sn*c*ok;
	t7 = sn*c*c*c*ok*(1. - t*t + eta) / 6.;
	t8 = sn*c*c*c*c*c*ok*(5. - 18.*t*t + t*t*t*t + 14.*eta - 58.*t*t*eta
		+ 13.*eta*eta + 4.*eta*eta*eta - 64.*eta*eta*t*t - 24.*eta*eta*eta*eta) / 120.;
	t9 = sn*c*c*c*c*c*c*c*ok*(61. - 479.*t*t + 179.*t*t*t*t - t*t*t*t*t*t) / 5040.;

	*x = fe + dlam*t6 + dlam*dlam*dlam*t7 + dlam*dlam*dlam*dlam*dlam*t8 + dlam*dlam*dlam*dlam*dlam*dlam*dlam*t9;
}

double GetDistance(double dLat1, double dLon1, double dLat2, double dLon2)
{
	double dX1, dY1;
	TM_Forword(dLon1, dLat1, &dX1, &dY1);

	double dX2, dY2;
	TM_Forword(dLon2, dLat2, &dX2, &dY2);

	double deltaX = (dX1 - dX2);
	double deltaY = (dY1 - dY2);

	double dist = sqrt((deltaX * deltaX) + (deltaY * deltaY));

	return dist;

}

double degreesToRadians(double degrees) {
  return degrees * M_PI / 180;
}

double distanceInKmBetweenEarthCoordinates(double lat1, double lon1, double lat2, double lon2) {
  double earthRadiusKm = 6371;

  double dLat = degreesToRadians(lat2-lat1);
  double dLon = degreesToRadians(lon2-lon1);

  lat1 = degreesToRadians(lat1);
  lat2 = degreesToRadians(lat2);

  double a = sin(dLat/2) * sin(dLat/2) + sin(dLon/2) * sin(dLon/2) * cos(lat1) * cos(lat2); 
  double c = 2 * atan2(sqrt(a), sqrt(1-a)); 
  return earthRadiusKm * c;
}

void getDistanceEx(double lat1, double lng1, double lat2, double lng2, double *retValue)
{
	double dLat, dLon;  
    double earth_radius = 6373;
	double a, c, d;
    dLat = degreesToRadians(lat2 - lat1);
    dLon = degreesToRadians(lng2 - lng1);
    a = sin(dLat/2) * sin(dLat/2) + cos(degreesToRadians(lat1)) * cos(degreesToRadians(lat2)) * sin(dLon/2) * sin(dLon/2);
    c = 2 * asin(sqrt(a));
    *retValue = earth_radius * c;

	*retValue *= 1000.0;

}
