#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bbox.h"
#include "debug.h"

static	bool		bFlagRead,					// flag used by the parser, when a valid sentence has begun
					bFlagDataReady;				// valid BBox fix and data available, user can call reader functions
static	char		words[20][15],				//	hold parsed words for one given BBox sentence
					szChecksum[15];				//	hold the received checksum for one given BBox sentence
	
// will be set to true for characters between $ and * only
static	bool		bFlagComputedCks ;			// used to compute checksum and indicate valid checksum interval (between $ and before checksum in a given sentence)
static	int			nChecksum ;					// numeric checksum, computed for a given sentence
static	int			received_checksum ;		    // used to parse received checksum
	
// word cutting variables
static	int			nWordIdx ,					// the current word in a sentence
					nPrevIdx,					// last character index where we did a cut
					nNowIdx ;					// current character index
	
// globals to store parser results
static	char		res_cManufacture;			// manufacture
static	char		res_cEvent;					// Event

/*
 * returns base-16 value of chars '0'-'9' and 'A'-'F';
 * does not trap invalid chars!
 */	
static int digit2dec(char digit) {
	if (digit >= 65) 
		return digit - 55;
	else 
		return digit - 48;
}

static bool parsedata() 
{
   	int received_cks = 16*digit2dec(szChecksum[0]) + digit2dec(szChecksum[1]);
	//uart1.Send("seq: [cc:%X][words:%d][rc:%s:%d]\r\n", m_nChecksum,m_nWordIdx, tmp_szChecksum, received_cks);
	// check checksum, and return if invalid!
	if (nChecksum != received_cks) {
		//m_bFlagDataReady = false;
		DEBUG(ZONE_ERROR, ("Error : BBox checksum \r\n"))
		return false;
	}

	if (strstr(words[0], "$BERDP") == 0) {
		// parse time
		res_cManufacture = digit2dec(words[1][0]) * 10 + digit2dec(words[1][1]);
		res_cEvent = digit2dec(words[2][0]) * 10 + digit2dec(words[2][1]);

		// data ready
		bFlagDataReady = true;

        return true;
	}

    return false;

}

bool parsebbox(char *pdata, int psize)
{
    char *pd = pdata;
    int bufsize = psize;
    char c;

    if(pdata == NULL || psize <= 0) 
    {
        return false;
    }

    if (*pd == '$') {
        bFlagRead = true;
		// init parser vars
		bFlagComputedCks = false;
		nChecksum = 0;

		received_checksum = 0;
		// word cutting variables
		nWordIdx = 0; nPrevIdx = 0; nNowIdx = 0;

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
                if (bFlagComputedCks && nWordIdx >= 3) bFlagComputedCks = false;
                if (bFlagComputedCks) nChecksum ^= c;
                if (c == '$') bFlagComputedCks = true;
                // received m_nChecksum
                if (nWordIdx >= 3)  {
                    szChecksum[received_checksum] = c;
                    received_checksum++;
                }

                // build a word
                words[nWordIdx][nNowIdx - nPrevIdx] = c;
                if (c == ',') {
                    words[nWordIdx][nNowIdx - nPrevIdx] = 0;
                    nWordIdx++;
                    nPrevIdx = nNowIdx;
                }
                else nNowIdx++;
            }	
        } while( bufsize-- > 0);			
	}

    return false;
}

bool isbboxready()
{
    return bFlagDataReady;
}

char getbboxManufacture()
{
    return res_cManufacture;
}

char getbboxEvent()
{   
    return res_cEvent;
}
