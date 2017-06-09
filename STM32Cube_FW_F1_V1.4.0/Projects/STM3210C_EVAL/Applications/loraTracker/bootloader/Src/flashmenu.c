/**
  ******************************************************************************
  * @file    FLASH/HalfPage/main.c 
  * @author  MCD Application Team
  * @version V1.1.1
  * @date    13-April-2012
  * @brief   Main program body.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */
#include "common.h"
#include "flashmenu.h"
#include "menulib.h"

extern UART_HandleTypeDef UartHandle;

/* Private typedef -----------------------------------------------------------*/
typedef enum { FAILED = 0,
               PASSED = !FAILED } TestStatus;

#if !defined(EF_FLASH_START_ADDRESS) && !defined(FE_FLASH_END_ADDRESS)
#error "Please select first the FLASH Start and End addresses in main.c file..."
#endif

#define FLASH_START_ADDR EF_FLASH_START_ADDRESS
#define FLASH_END_ADDR EF_FLASH_END_ADDRESS

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
__IO uint32_t FLASHStatus = FLASHIF_OK;
__IO TestStatus MemoryProgramStatus = PASSED;
uint32_t NbrOfPage = 0, j = 0, Address = 0;
uint32_t Data[32] = {
    0x11223344, 0x11223344, 0x11223344, 0x11223344, 0x11223344, 0x11223344, 0x11223344, 0x11223344,
    0x11223344, 0x11223344, 0x11223344, 0x11223344, 0x11223344, 0x11223344, 0x11223344, 0x11223344,
    0x11223344, 0x11223344, 0x11223344, 0x11223344, 0x11223344, 0x11223344, 0x11223344, 0x11223344,
    0x11223344, 0x11223344, 0x11223344, 0x11223344, 0x11223344, 0x11223344, 0x11223344, 0x11223344,
};
uint32_t Data_chunk[8] = {
    0x11223344, 0x11223344, 0x11223344, 0x11223344, 0x11223344, 0x11223344, 0x11223344, 0x11223344,
};
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

static void flash_auto_test()
{
    printf("  memory test is starting !!!  \r\n\n");

    /* Unlock the Program memory */
    HAL_FLASH_Unlock();

    /* Clear all FLASH flags */
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);

    /* Unlock the Program memory */
    HAL_FLASH_Lock();

    Address = FLASH_START_ADDR;
    NbrOfPage = (FLASH_END_ADDR - Address) / FLASH_PAGE_SIZE;

    /* Erase the FLASH Program memory pages */
    for (j = 0; j < NbrOfPage; j++)
    {
        printf("  page %d\r\n", j);
        FLASHStatus = FLASH_If_GPS_Erase_Page(Address + (FLASH_PAGE_SIZE * j));

        if (FLASHStatus != FLASHIF_OK)
        {
            printf("  FLASH_If_GPS_Erase_Page is failed !!!  \r\n\n");
            MemoryProgramStatus = FAILED;
        }
        else
        {
            /* Unlock the Program memory */
            HAL_FLASH_Unlock();
            __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
            /* Unlock the Program memory */
            HAL_FLASH_Lock();
        }
    }

    Address = FLASH_START_ADDR;

    /* Write the FLASH Program memory using HalfPage operation */
    while (Address <= FLASH_END_ADDR)
    {
        printf("  0x%x \r\n", Address);
        FLASHStatus = FLASH_If_Write(Address, Data, 128 / 4);

        if (FLASHStatus == FLASHIF_OK)
        {
            Address = Address + 128;
        }
        else
        {
            /* Unlock the Program memory */
            HAL_FLASH_Unlock();
            
            __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
            
            /* Unlock the Program memory */
            HAL_FLASH_Lock();
            if(Address >= FLASH_END_ADDR)
                break;
        }
    }

    Address = FLASH_START_ADDR;

    /* Check the written data */
    while (Address < FLASH_END_ADDR)
    {
        if (*(__IO uint32_t *)Address != Data[0])
        {
            printf(" failed offset 0x%x = 0x%x\r\n", Address, *(__IO uint32_t *)Address);
            MemoryProgramStatus = FAILED;
        }
        Address = Address + 4;
    }

    if (MemoryProgramStatus != FAILED)
    {
        /* OK */
        /* Turn on LD1 */
        printf("  memory test is successfull \r\n\n");
    }
    else
    {
        /* KO */
        /* Turn on LD2 */
        printf("  memory test is failed !!!  \r\n\n");
    }
}

static void flash_write_data()
{
}

void Flash_Menu(void)
{

    uint8_t key = 0;
    uint32_t num = 0;
    __IO  uint32_t flashaddress = FLASH_START_ADDR;

    while (1)
    {

        printf("\r\n=================== Flash Menu ===========================\r\n\n");
        printf("  Flash auto test for GPS store  ----------------------- 1\r\n\n");
        printf("  Erase the flash of GPS         ----------------------- 2\r\n\n");
        printf("  Erase a page on the flash      ----------------------- 3\r\n\n");
        printf("  Read 4bytes from a page        ----------------------- 4\r\n\n");
        printf("  Write 4bytes to a page         ----------------------- 5\r\n\n");
        printf("  Write some data to a page      ----------------------- 6\r\n\n");
        printf("  Back to main menu              ----------------------- 7\r\n\n");
        printf("==========================================================\r\n\n");

        /* Clean the input path */
        __HAL_UART_FLUSH_DRREGISTER(&UartHandle);

        /* Receive key */
        key = WaitKey();

        switch (key)
        {
        case '1':
            flash_auto_test();
            break;
        case '2':
            Address = FLASH_START_ADDR;
            NbrOfPage = (FLASH_END_ADDR - Address) / FLASH_PAGE_SIZE;

            printf(" Erase between 0x%x and 0x%x\r\n", Address, FLASH_END_ADDR);

            /* Erase the FLASH Program memory pages */
            for (j = 0; j < NbrOfPage; j++)
            {
                FLASHStatus = FLASH_If_GPS_Erase_Page(Address + (FLASH_PAGE_SIZE * j));

                if (FLASHStatus != FLASHIF_OK)
                {
                    printf("  the erase process is failed...\r\n");
                }
                else
                {
                    /* Unlock the Program memory */
                    HAL_FLASH_Unlock();
                    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
                    /* Unlock the Program memory */
                    HAL_FLASH_Lock();
                }
            }
            break;
        case '3':
            printf("  which page do you want to erase(0~57)?");
            num = GetDecnum();
            if(num > 57)
            {
                printf("\r\n the (%d) is larger than 57\r\n", num);
                break;
            }
            Address = FLASH_START_ADDR + (FLASH_PAGE_SIZE * num);

            printf("\r\n address offset = 0x%x\r\n", Address);

            flashaddress = Address;
            FLASHStatus = FLASH_If_GPS_Erase_Page(Address);
            break;
        case '4':
            //Address = flashaddress;
            printf("  which address do you want to read?");
            Address = GetHexnum32();
            Address = Address & ~(3);
            printf("\r\n offset 0x%x = 0x%x\r\n", Address, *(__IO uint32_t *)Address);
            break;
        case '5':    
            printf("  which address do you want to write?");
            Address = GetHexnum32();
            Address = Address & ~(3);
            printf("\r\n  Trying to write 4bytes to 0x%x...\r\n", Address);
            if (FLASH_If_Write(Address, Data_chunk, 1) == FLASHIF_OK)
            {
                //flashaddress += 4;
            }
            else 
            {
                printf("  the write is failed\r\n");
            }
            break;
        case '6':
            //printf("  which address do you want to write 32bytes?(the address set to 32bytes aligned)");
            printf("  which address do you want to write 32bytes?");
            Address = GetHexnum32();
            flashaddress = Address & 0x807fff0;
            printf("  Write 32bytes to 0x%x...\r\n", flashaddress);
            if (FLASH_If_Write(flashaddress, Data_chunk, 32 / 4) == FLASHIF_OK)
            {
                flashaddress += 32;
            }
            else
            {
                printf("  Write error. you should erase a page %d before doing it.\r\n", (Address - FLASH_START_ADDR) / FLASH_PAGE_SIZE );
            }

            while(Address < flashaddress) 
            {
                if (*(__IO uint32_t *)Address != Data[0])
                {
                    printf(" failed offset 0x%x = 0x%x\r\n", Address, *(__IO uint32_t *)Address);
                    MemoryProgramStatus = FAILED;
                }
                Address = Address + 4;
            }

            break;
        case '7':
            // back to main menu
            return;
        default:
            printf("Invalid Number ! ==> The number should be either 1, 2, 3, 4, 5, 6 or 7\r");
            break;
        }
    }
}