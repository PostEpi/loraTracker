#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bbox.h"
#include "utilities.h"
#include "debug.h"



static	bool		bFlagRead,					// flag used by the parser, when a valid sentence has begun
					bFlagDataReady;				// valid BBox fix and data available, user can call reader functions
static	char		words[MAX_SIZE_OF_BBOX_ROW][MAX_SIZE_OF_BBOX_COLUME+1],		//	hold parsed words for one given BBox sentence
					szChecksum[15];				//	hold the received checksum for one given BBox sentence
static  char		usermsg[BBOX_MESSAGE_SIZE];	//  hold the user message

// will be set to true for characters between $ and * only
static	bool		bFlagComputedCks ;			// used to compute checksum and indicate valid checksum interval (between $ and before checksum in a given sentence)
static	int			nChecksum ;					// numeric checksum, computed for a given sentence
static	bool		bFlagReceivedCks ;			// after getting  * we start cuttings the received checksum
static	int			received_checksum ;		    // used to parse received checksum
static 	bool 		bUserMessageCks ;			// we start cutting the receivced user message.	
// word cutting variables
static	int			nWordIdx ,					// the current word in a sentence
					nPrevIdx,					// last character index where we did a cut
					nNowIdx ;					// current character index

#define 			RES_DATA_FIELD_COUNT		7	
// globals to store parser results
static 	char		res_cServiceCode;			// service code
static	char		res_cManufacture;			// manufacture
static	char		res_cEvent;					// Event
static 	double 		res_EventInfo;				// Event Information
static  float      	res_Battery;                // battery status
static  char		res_Temperature;			// temperature
static 	char    	res_sSerialNum[16];			// blox box serial number

static  int			res_DataFieldMaxSize[RES_DATA_FIELD_COUNT] = {3, 3, 3, 16, 16, 3, 16}; // service, manufacture, event, event info, battery, temperatrue. black box serial
#if 0
/*
 * returns base-16 value of chars '0'-'9' and 'A'-'F';
 * does not trap invalid chars!
 */	
int digit2dec(char digit) {
        if (digit >= 97)
                return digit - 87;
	else if (digit >= 65) 
		return digit - 55;
	else 
		return digit - 48;
}


/* returns base-10 value of zero-terminated string
 * that contains only chars '+','-','0'-'9','.';
 * does not trap invalid strings! 
 */
float string2float(char* s) {
	long  integer_part = 0;
	float decimal_part = 0.0;
	float decimal_pivot = 0.1;
	bool isdecimal = false, isnegative = false;
	
	char c;
	while ( ( c = *s++) )  { 
		// skip special/sign chars
		if (c == '-') { isnegative = true; continue; }
		if (c == '+') continue;
		if (c == '.') { isdecimal = true; continue; }
		
		if (!isdecimal) {
			integer_part = (10 * integer_part) + (c - 48);
		}
		else {
			decimal_part += decimal_pivot * (float)(c - 48);
			decimal_pivot /= 10.0;
		}
	}
	// add integer part
	decimal_part += (float)integer_part;
	
	// check negative
	if (isnegative)  decimal_part = - decimal_part;

	return decimal_part;
}
#endif

static bool IsdataValided()
{
	for(int i = 0 ; i < RES_DATA_FIELD_COUNT ; i++)
	{
		if(strlen(words[i+1]) > res_DataFieldMaxSize[i])
		{
			DEBUG(ZONE_ERROR, ("IsdataValided : Error : argument overflow @@@ \r\n"));

			return false;
		}
	}

	return true;
}


static bool parsedata() 
{
   	int received_cks = 16*digit2dec(szChecksum[0]) + digit2dec(szChecksum[1]);
	//uart1.Send("seq: [cc:%X][words:%d][rc:%s:%d]\r\n", m_nChecksum,m_nWordIdx, tmp_szChecksum, received_cks);
	// check checksum, and return if invalid!
	if (nChecksum != received_cks) {
		bFlagDataReady = false;
		DEBUG(ZONE_ERROR, ("parsedata : Error : BBox checksum \r\n"));
		return false;
	}

#if 0
	if (strstr(words[0], BBOX_STX_BERDP) != NULL) {
		// parse time
		// res_cManufacture = digit2dec(words[1][0]) * 10 + digit2dec(words[1][1]);
		// res_cEvent = digit2dec(words[2][0]) * 10 + digit2dec(words[2][1]);
        // res_sBattery = (short)(string2float(words[3]) * 5) ;
		// // data ready
		// bFlagDataReady = true;


		bFlagDataReady = true;

        return true;
	}
#endif

	if (strstr(words[0], BBOX_STR_BURDP) != NULL) {
		if(IsdataValided() == false)
		{
			// data overflow.... 
			return false;
		}

		int value, size;
		if(Str2Int( words[1], &value) == 0)
		{
			DEBUG(ZONE_ERROR, ("servie code = %d\r\n", value));
			return false;
		}
		res_cServiceCode = value;

		if(Str2Int( words[2], &value) == 0)
		{
			DEBUG(ZONE_ERROR, ("manufacture code = %d\r\n", value));
			return false;
		}
		res_cManufacture = value;

		if(Str2Int( words[3], &value) == 0)
		{
			DEBUG(ZONE_ERROR, ("event code = %d\r\n", value));
			return false;
		}
		res_cEvent = value;

		// checking data integrity for event additinal information .
		size = strlen(words[4]);
		for(value = 0; value < size; value++)
		{
            char c = words[4][value];
			if(ISVALIDDEC(c) == false)
			{
				if (c == '-') continue;
				if (c == '+') continue;
				if (c == '.') continue;
				DEBUG(ZONE_ERROR, ("event additinal code = %d\r\n", c));
				return false; 
			}
		}
		sscanf(words[4], "%f", &res_EventInfo);		// event additinal information

		// checking data integrity for battery level
		size = strlen(words[5]);
		for(value = 0; value < size; value++)
		{
            char c = words[5][value];
			if(ISVALIDDEC(c) == false)
			{
				if (c == '-') continue;
				if (c == '+') continue;
				if (c == '.') continue;
				DEBUG(ZONE_ERROR, ("battery = %d\r\n", c));
				return false; 
			}
		}
		res_Battery 	= string2float(words[5]);  	// battery level.
		
		if(Str2Int( words[6], &value) == 0)
		{
			DEBUG(ZONE_ERROR, ("temperature = %d\r\n", value));
			return false;
		}
		res_Temperature = value;
		
		memcpy(res_sSerialNum, words[7], MAX_SIZE_OF_BBOX_COLUME);	

		// data ready
		bFlagDataReady = true;

        return true;
	}
	else if (strstr(words[0], BBOX_STX_BDRDP) != NULL) {
		// data ready
		bFlagDataReady = true;

        return true;
	}
    
    return false;

}

bool parsebbox(const char *pdata, int psize)
{
    const char *pd = pdata;
    int bufsize = psize;
    char c;

    if(pdata == NULL || psize == 0 || psize > BBOX_MESSAGE_SIZE) 
    {
        return false;
    }

    if (*pd == '$') {
        bFlagRead = true;
		// init parser vars
		bFlagComputedCks = false;
		nChecksum = 0;

		// after getting  * we start cuttings the received m_nChecksum
		bFlagReceivedCks = false;
		received_checksum = 0;
		// word cutting variables
		nWordIdx = 0; nPrevIdx = 0; nNowIdx = 0;

		// to cut the user message
		bUserMessageCks = false;

		bFlagDataReady = false;

        do {
            c = *pd++;
			

            // check ending
            if (c == '\r' || c== '\n') {
                // catch last ending item too
                words[nWordIdx][nNowIdx - nPrevIdx] = 0;
                nWordIdx++;
                // cut received m_nChecksum
                szChecksum[received_checksum] = 0;
                // sentence complete, read done
                bFlagRead = false;
                // parse
                return parsedata();
            } else {
                // computed m_nChecksum logic: count all chars between $ and * exclusively
                if (bFlagComputedCks && c == '*') bFlagComputedCks = false;
                if (bFlagComputedCks) nChecksum ^= c;
                if (c == '$') bFlagComputedCks = true;
                
				// received m_nChecksum
				if(bFlagReceivedCks) {
                    szChecksum[received_checksum] = c;
                    received_checksum++;
                }
				
				if (c == '*') bFlagReceivedCks = true;

				if(bUserMessageCks == true && bFlagReceivedCks == false) 
				{
					usermsg[nNowIdx] = c;
				}
				else if(bUserMessageCks == true)
				{
					bUserMessageCks = false;
					usermsg[nNowIdx-1] = '\0';
				}

                // build a word
                words[nWordIdx][nNowIdx - nPrevIdx] = c;
                if (c == ',') {
					bUserMessageCks = true;
                    words[nWordIdx][nNowIdx - nPrevIdx] = 0;
                    nWordIdx++;
                    nPrevIdx = nNowIdx;
                }
                else nNowIdx++;
            }

			if(nWordIdx >= MAX_SIZE_OF_BBOX_ROW || ((nNowIdx - nPrevIdx) > MAX_SIZE_OF_BBOX_COLUME))
			{
				DEBUG(ZONE_ERROR, ("parsebbox : Error : Message is invalied \r\n"));
				return false;
			}	
        } while( bufsize-- > 0);			
	}

    return false;
}

bool isbboxready()
{
    return bFlagDataReady;
}

int getUserMessage(char *buf, int size)
{
	int max = 0;
	if(buf != NULL && size > 0)
	{
		max = 22; // iot GPS specification 
		if(max > size) max = size;
		memcpy(buf, usermsg, max); 
	}

	return max;
}


char getServiceCode()
{
	return res_cServiceCode;
}

char getbboxManufacture()
{
    return res_cManufacture;
}

char getbboxEvent()
{   
    return res_cEvent;
}

double getbboxEventInfo()
{   
    return res_EventInfo;
}

float getbboxBattery()
{   
    return res_Battery;
}

char getbboxTemperature() 
{
    return res_Temperature;
}

const char* getbboxSerial()
{
	return res_sSerialNum;
}