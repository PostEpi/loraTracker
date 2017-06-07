#include "common.h"
#include "menulib.h"

extern UART_HandleTypeDef UartHandle;


uint8_t readByte(void)
{
    uint8_t key;
    HAL_UART_Receive(&UartHandle, &key, 1, RX_TIMEOUT);

	return key;
}

uint8_t WaitKey(void)
{
	uint8_t Key;

	while ((Key = readByte())==(uint8_t)OEM_DEBUG_READ_NODATA)
	{
		;
	}

	printf("%c\r\n", Key);

	if(Key >= 'a')
		Key -= 0x20;

	return Key;
}


uint32_t GetHexnum32(void)
{
	uint8_t InChar  = 0;
	uint32_t Result = 0, cCount = 0;

	while(InChar != '\r' && InChar != '\n' || cCount == 0)
	{
		while((InChar = (unsigned char)readByte()) == (uint8_t)OEM_DEBUG_READ_NODATA)
		{
			;
		}

		if(cCount != 8)
		{
			if(InChar >= '0' && InChar <= '9')
			{
				printf("%c", InChar);
				Result = (Result<<4) + (InChar - '0');
				cCount++;
			}
			else if(InChar >= 'a' && InChar <= 'f')
			{
				printf("%c", InChar);
				Result = (Result<<4) + (InChar-'a' + 0x0a);
				cCount++;
			}
			else if(InChar >= 'A' && InChar <= 'F')
			{
				printf("%c", InChar);
				Result = (Result<<4) + (InChar-'A' + 0x0A);
				cCount++;
			}
		}

		if(cCount!=0)
		{
			if(InChar == 0x08)		// backspace '\b'
			{
				Result >>= 4;
				cCount--;
				printf("%c", InChar);
				printf(" ");
				printf("%c", InChar);
			}
		}
	}

	return Result;
}

//------------------------------------------------------------------------------
//  @func : GetDecnum
//
uint32_t GetDecnum(void)
{
	uint8_t  InChar=0;
	uint32_t Result=0, cCount=0;

	while(InChar!='\r' && InChar!='\n' || cCount==0)
	{
		while((InChar = (unsigned char)readByte())==(uint8_t)OEM_DEBUG_READ_NODATA);

		if(Result < 429496728)		// (0xFFFFFFFF = 4294967295 -9)/10
		{
			if(InChar >= '0' && InChar <= '9')
			{
				printf("%c", InChar);
				Result = Result * 10 + (InChar - '0');
				cCount++;
			}
		}

		if(Result >= 0)
		{
			if(InChar == '\b')		// backspace
			{
				Result /= 10;
				cCount--;
				printf("%c", InChar);
				printf(" ");
				printf("%c", InChar);
			}
		}
	}

	return Result;
}

//------------------------------------------------------------------------------
//  @func : GetString
//
void GetString(
		uint8_t * string,
		uint32_t  len
		)
{
	uint8_t * strptr = string;
	uint8_t   InChar=0;

	while(InChar != '\r' && InChar != '\n')
	{

		while((InChar = readByte()) == (uint8_t)OEM_DEBUG_READ_NODATA)
		{
			;
		}

		if(InChar == '\b')
		{
			if( (int)strptr < (int)string )
			{
				printf("%c", InChar);
				printf(" ");
				printf("%c", InChar);
				string--;
				len++;
			}
		}
		else if(InChar != '\r' && InChar != '\n' )
		{
			if(len <= 1)
				continue;

			*string++ = InChar;
			len--;
			printf("%c", InChar);
		}
	}

	*string = '\0';
	printf("\n");
}

//------------------------------------------------------------------------------
//  @func : GetIntNum
//
int32_t GetIntNum(void)
{
	uint8_t    str[30];
	uint8_t  * string = str;
	uint32_t   base   = 10;
	bool minus  = false;
	int32_t   result = 0;
	uint32_t   lastIndex, i;

	GetString(string, 30);

	if(string[0] == '-')
	{
		minus = true;
		string++;
	}

	if(string[0] == '0' && (string[1]== 'x' || string[1]=='X'))
	{
		base    = 0x10;
		string += 2;
	}

	lastIndex = strlen((uint8_t*)string) - 1;

	if(lastIndex < 0)
		return -1;

	if(string[lastIndex]=='h' || string[lastIndex]=='H' )
	{
		base = 0x10;
		string[lastIndex] = 0;
		lastIndex--;
	}

	if(base==10)
	{
		result = atoi((uint8_t*)string);
	}
	else
	{
		for(i=0; i <= lastIndex; i++)
		{
			if(string[i] >= 'A' && string[i] <= 'F' )
				result = (result<<4) + string[i] - 'A' + 0x0A;

			else if(string[i] >= 'a' && string[i] <= 'f' )
				result = (result<<4) + string[i] - 'a' + 0x0a;

			else if(string[i] >= '0' && string[i] <= '9' )
				result = (result<<4) + string[i] - '0' + 0x00;
		}
	}

	return (minus ? (-1*result) : result);
}
