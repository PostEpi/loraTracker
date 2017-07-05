/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Helper functions implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "utilities.h"

/*!
 * Redefinition of rand() and srand() standard C functions.
 * These functions are redefined in order to get the same behavior across
 * different compiler toolchains implementations.
 */
// Standard random functions redefinition start
#define RAND_LOCAL_MAX 2147483647L

static uint32_t next = 1;

int32_t rand1( void )
{
    return ( ( next = next * 1103515245L + 12345L ) % RAND_LOCAL_MAX );
}

void srand1( uint32_t seed )
{
    next = seed;
}
// Standard random functions redefinition end

int32_t randr( int32_t min, int32_t max )
{
    return ( int32_t )rand1( ) % ( max - min + 1 ) + min;
}

void memcpy1( uint8_t *dst, const uint8_t *src, uint16_t size )
{
    while( size-- )
    {
        *dst++ = *src++;
    }
}

void memcpyr( uint8_t *dst, const uint8_t *src, uint16_t size )
{
    dst = dst + ( size - 1 );
    while( size-- )
    {
        *dst-- = *src++;
    }
}

void memset1( uint8_t *dst, uint8_t value, uint16_t size )
{
  while( size-- )
  {
    *dst++ = value;
  }
}

int8_t Nibble2HexChar( uint8_t a )
{
  if( a < 10 )
  {
    return '0' + a;
  }
  else if( a < 16 )
  {
    return 'A' + ( a - 10 );
  }
  else
  {
    return '?';
  }
}

/**
  * @brief  Convert an Integer to a string
  * @param  p_str: The string output pointer
  * @param  intnum: The integer to be converted
  * @retval None
  */
void Int2Str(uint8_t *p_str, uint32_t intnum)
{
  uint32_t i, divider = 1000000000, pos = 0, status = 0;

  for (i = 0; i < 10; i++)
  {
    p_str[pos++] = (intnum / divider) + 48;

    intnum = intnum % divider;
    divider /= 10;
    if ((p_str[pos-1] == '0') & (status == 0))
    {
      pos = 0;
    }
    else
    {
      status++;
    }
  }
}

/**
  * @brief  Convert a string to an integer
  * @param  p_inputstr: The string to be converted
  * @param  p_intnum: The integer value
  * @retval 1: Correct
  *         0: Error
  */
uint32_t Str2Int(uint8_t *p_inputstr, uint32_t *p_intnum)
{
  uint32_t i = 0, res = 0;
  uint32_t val = 0;

  if ((p_inputstr[0] == '0') && ((p_inputstr[1] == 'x') || (p_inputstr[1] == 'X')))
  {
    i = 2;
    while ( ( i < 11 ) && ( p_inputstr[i] != '\0' ) )
    {
      if (ISVALIDHEX(p_inputstr[i]))
      {
        val = (val << 4) + CONVERTHEX(p_inputstr[i]);
      }
      else
      {
        /* Return 0, Invalid input */
        res = 0;
        break;
      }
      i++;
    }

    /* valid result */
    if (p_inputstr[i] == '\0')
    {
      *p_intnum = val;
      res = 1;
    }
  }
  else /* max 10-digit decimal input */
  {
    while ( ( i < 11 ) && ( res != 1 ) )
    {
      if (p_inputstr[i] == '\0')
      {
        *p_intnum = val;
        /* return 1 */
        res = 1;
      }
      else if (((p_inputstr[i] == 'k') || (p_inputstr[i] == 'K')) && (i > 0))
      {
        val = val << 10;
        *p_intnum = val;
        res = 1;
      }
      else if (((p_inputstr[i] == 'm') || (p_inputstr[i] == 'M')) && (i > 0))
      {
        val = val << 20;
        *p_intnum = val;
        res = 1;
      }
      else if (ISVALIDDEC(p_inputstr[i]))
      {
        val = val * 10 + CONVERTDEC(p_inputstr[i]);
      }
      else
      {
        /* return 0, Invalid input */
        res = 0;
        break;
      }
      i++;
    }
  }

  return res;
}

void itohex(char *buf, int size, int value)
{
    int i = 0, p = 0;
    char num[10];
    
    sprintf(num, "%x", value);
    
    while(size--) 
    {
        buf[p] = digit2dec(num[i++]) * 16;
        buf[p] += digit2dec(num[i++]);
        p++;
    }
}

void ftohex(char *buf, int size, float value)
{
    int i = 0, p = 0;
    char num[20];
    
    sprintf(num, "%f", value);
    
    while(size--) 
    {
        buf[p] = digit2dec(num[i++]) * 16;
        buf[p] += digit2dec(num[i++]);
        p++;
    }
}

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

