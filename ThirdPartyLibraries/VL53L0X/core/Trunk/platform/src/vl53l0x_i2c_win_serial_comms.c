/*
 * COPYRIGHT (C) STMicroelectronics 2015. All rights reserved.
 *
 * This software is the confidential and proprietary information of
 * STMicroelectronics ("Confidential Information").  You shall not
 * disclose such Confidential Information and shall use it only in
 * accordance with the terms of the license agreement you entered into
 * with STMicroelectronics
 *
 * Programming Golden Rule: Keep it Simple!
 *
 */

#include "vl53l0x_i2c_platform.h"
#include "vl53l0x_def.h"
#include "vl53l0x_types.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "vl53l0x_platform_log.h"

#ifdef VL53L0X_LOG_ENABLE
#define trace_print(level, ...) trace_print_module_function(TRACE_MODULE_PLATFORM, level, TRACE_FUNCTION_NONE, ##__VA_ARGS__)
#define trace_i2c(...) trace_print_module_function(TRACE_MODULE_NONE, TRACE_LEVEL_NONE, TRACE_FUNCTION_I2C, ##__VA_ARGS__)
#endif

char  debug_string[VL53L0X_MAX_STRING_LENGTH_PLT];

uint8_t cached_page = 0;

#define MIN_COMMS_VERSION_MAJOR     1
#define MIN_COMMS_VERSION_MINOR     8
#define MIN_COMMS_VERSION_BUILD     1
#define MIN_COMMS_VERSION_REVISION  0


#define MAX_STR_SIZE 255
#define MAX_MSG_SIZE 100
#define MAX_DEVICES 4
#define STATUS_OK              0x00
#define STATUS_FAIL            0x01

static unsigned char _dataBytes[MAX_MSG_SIZE];

bool_t _check_min_version(void)
{
    return 0;
}

int VL53L0X_i2c_init(char *comPortStr, unsigned int baudRate) // mja
{
    return;
}

int32_t VL53L0X_comms_close(void)
{
    return;
}

int32_t VL53L0X_write_multi(uint8_t address, uint8_t reg, uint8_t *pdata, int32_t count)
{
    int32_t status = STATUS_OK;

    unsigned int retries = 3;
    uint8_t *pWriteData    = pdata;
    uint8_t writeDataCount = count;
    uint8_t writeReg       = reg;

    /* For multi writes, the serial comms dll requires multiples 4 bytes or
     * anything less than 4 bytes. So if an irregular size is required, the
     * message is broken up into two writes.
     */
    if((count > 4) && (count % 4 != 0))
    {
        writeDataCount = 4*(count/4);
        status = VL53L0X_write_multi(address, writeReg, pWriteData, writeDataCount);

        writeReg = reg + writeDataCount;
        pWriteData += writeDataCount;
        writeDataCount = count - writeDataCount;
    }
}

int32_t VL53L0X_read_multi(uint8_t address, uint8_t index, uint8_t *pdata, int32_t count)
{
    int32_t status = STATUS_OK;
    int32_t readDataCount = count;

    unsigned int retries = 3;

    {
        /* The serial comms interface requires multiples of 4 bytes so we
         * must apply padding if required.
         */
        if((count % 4) != 0)
        {
            readDataCount = (4*(count/4)) + 4;
        }

        if(readDataCount > MAX_MSG_SIZE)
        {
            status = STATUS_FAIL;
        }

    }

    return status;
}


int32_t VL53L0X_write_byte(uint8_t address, uint8_t index, uint8_t data)
{
    int32_t status = STATUS_OK;
    const int32_t cbyte_count = 1;

    status = VL53L0X_write_multi(address, index, &data, cbyte_count);

    return status;

}


int32_t VL53L0X_write_word(uint8_t address, uint8_t index, uint16_t data)
{
    int32_t status = STATUS_OK;

    uint8_t  buffer[BYTES_PER_WORD];

    // Split 16-bit word into MS and LS uint8_t
    buffer[0] = (uint8_t)(data >> 8);
    buffer[1] = (uint8_t)(data &  0x00FF);

    if(index%2 == 1)
    {
        status = VL53L0X_write_multi(address, index, &buffer[0], 1);
        status = VL53L0X_write_multi(address, index + 1, &buffer[1], 1);
        // serial comms cannot handle word writes to non 2-byte aligned registers.
    }
    else
    {
        status = VL53L0X_write_multi(address, index, buffer, BYTES_PER_WORD);
    }

    return status;

}


int32_t VL53L0X_write_dword(uint8_t address, uint8_t index, uint32_t data)
{
    int32_t status = STATUS_OK;
    uint8_t  buffer[BYTES_PER_DWORD];

    // Split 32-bit word into MS ... LS bytes
    buffer[0] = (uint8_t) (data >> 24);
    buffer[1] = (uint8_t)((data &  0x00FF0000) >> 16);
    buffer[2] = (uint8_t)((data &  0x0000FF00) >> 8);
    buffer[3] = (uint8_t) (data &  0x000000FF);

    status = VL53L0X_write_multi(address, index, buffer, BYTES_PER_DWORD);

    return status;
}


/*
/******************************************************************************
 * @function VL53L0X_read_byte
 *
 * @brief read an I2C register
 *
 * This function will read the selected register
 *
 * @param[in]   nRegister        register to read
 * @param[in]   pnData           pointer to data which is read
 *
 * @return      
 *
 *****************************************************************************/
/*
int32_t VL53L0X_read_byte(uint8_t nRegister, uint8_t index, uint16_t *pData)
{
  int32_t     status = STATUS_OK;
  uint8_t     nLength = 1;
  I2CXFRCTL   tXfrCtl; 
  I2CERR      eI2cErr;  

  // set up for read
  tXfrCtl.nDevAddr = VL53L0X_DEV_ADDR;
  tXfrCtl.tAddress.anValue[ LE_U32_LSB_IDX ] = nRegister;
  tXfrCtl.nAddrLen = 1;
  tXfrCtl.pnData = pData;
  tXfrCtl.wDataLen = nLength;
  tXfrCtl.uTimeout = 500;
  
  eI2cErr = I2c_Read( VL53L0X_I2C_ENUM, &tXfrCtl );

  // set the error
  if( eI2cErr == I2C_ERR_NONE ) {
	  status = STATUS_OK;
  }
	
  return status;
}
*/

int32_t VL53L0X_read_word(uint8_t address, uint8_t index, uint16_t *pdata)
{
    int32_t  status = STATUS_OK;
	uint8_t  buffer[BYTES_PER_WORD];

    status = VL53L0X_read_multi(address, index, buffer, BYTES_PER_WORD);
	*pdata = ((uint16_t)buffer[0]<<8) + (uint16_t)buffer[1];

    return status;

}

int32_t VL53L0X_read_dword(uint8_t address, uint8_t index, uint32_t *pdata)
{
    int32_t status = STATUS_OK;
	uint8_t  buffer[BYTES_PER_DWORD];

    status = VL53L0X_read_multi(address, index, buffer, BYTES_PER_DWORD);
    *pdata = ((uint32_t)buffer[0]<<24) + ((uint32_t)buffer[1]<<16) + ((uint32_t)buffer[2]<<8) + (uint32_t)buffer[3];

    return status;

}


int32_t VL53L0X_platform_wait_us(int32_t wait_us)
{
    int32_t status = STATUS_OK;
    float wait_ms = (float)wait_us/1000.0f;

//    WaitForSingleObject(hEvent, (int)(wait_ms + 0.5f));

    return status;

}


int32_t VL53L0X_wait_ms(int32_t wait_ms)
{
    int32_t status = STATUS_OK;

  //  WaitForSingleObject(hEvent, wait_ms);

    return status;

}


int32_t VL53L0X_set_gpio(uint8_t level)
{
    int32_t status = STATUS_OK;
    //status = VL53L0X_set_gpio_sv(level);

    return status;
}


int32_t VL53L0X_get_gpio(uint8_t *plevel)
{
    int32_t status = STATUS_OK;

    return status;
}


int32_t VL53L0X_release_gpio(void)
{
    int32_t status = STATUS_OK;

    return status;
}

int32_t VL53L0X_cycle_power(void)
{
    int32_t status = STATUS_OK;

	return status;
}


int32_t VL53L0X_get_timer_frequency(int32_t *ptimer_freq_hz)
{
       *ptimer_freq_hz = 0;
       return STATUS_FAIL;
}


int32_t VL53L0X_get_timer_value(int32_t *ptimer_count)
{
       *ptimer_count = 0;
       return STATUS_FAIL;
}
