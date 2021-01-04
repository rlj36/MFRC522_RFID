#include <stdlib.h>

#include "fsl_misc_utilities.h"
#include "fsl_device_registers.h"
#include "fsl_i2c_master_driver.h"
#include "fsl_spi_master_driver.h"
#include "fsl_rtc_driver.h"
#include "fsl_clock_manager.h"
#include "fsl_power_manager.h"
#include "fsl_mcglite_hal.h"
#include "fsl_port_hal.h"

#include "gpio_pins.h"
#include "SEGGER_RTT.h"
#include "warp.h"

extern volatile WarpI2CDeviceState	deviceINA219State;
extern volatile uint32_t		gWarpI2cBaudRateKbps;
extern volatile uint32_t		gWarpI2cTimeoutMilliseconds;
extern volatile uint32_t		gWarpSupplySettlingDelayMilliseconds;

void
initINA219(const uint8_t i2cAddress, WarpI2CDeviceState volatile *  deviceStatePointer)
{
	deviceStatePointer->i2cAddress	= i2cAddress;
	return;
}

WarpStatus
writeSensorRegisterINA219(uint8_t deviceRegister, uint8_t payload, uint8_t payload1, uint16_t menuI2cPullupValue)
{
	uint8_t		payloadByte[2];
	uint8_t		commandByte[1];
	i2c_status_t	status;

	switch (deviceRegister)
	{
		case 0x00: case 0x05:
		{
			/* OK */
			break;
		}

		default:
		{
			return kWarpStatusBadDeviceCommand;
		}
	}

	i2c_device_t slave =
	{
		.address = deviceINA219State.i2cAddress,
		.baudRate_kbps = gWarpI2cBaudRateKbps
	};

	commandByte[0] = deviceRegister;
	payloadByte[0] = payload;
	payloadByte[1] = payload1;
	status = I2C_DRV_MasterSendDataBlocking(
							0 /* I2C instance */,
							&slave,
							commandByte,
							1,
							payloadByte,
							2,
							gWarpI2cTimeoutMilliseconds);
	if (status != kStatus_I2C_Success)
	{
		return kWarpStatusDeviceCommunicationFailed;
	}

	return kWarpStatusOK;
}


/*
configure the INA219 such that it has value 0x019F set in the configuration register
*/
WarpStatus
configureSensorINA219(uint16_t payloadConfiguration, uint16_t menuI2cPullupValue)
{
	WarpStatus	i2cWriteStatus1;

	i2cWriteStatus1 = writeSensorRegisterINA219(kWarpSensorConfigurationRegisterINA219Configuration,
							0x01,
							0x9F,
							menuI2cPullupValue);
	return (i2cWriteStatus1);
}

WarpStatus
readSensorRegisterINA219(uint8_t deviceRegister, int numberOfBytes)
{
	uint8_t		cmdBuf[1] = {0xFF};
	i2c_status_t	status;



	switch (deviceRegister)
	{
		case 0x00: case 0x01: case 0x02: case 0x03:
		case 0x04: case 0x05:/* only when register is 0x00 -> 0x05*/
		{
			/* OK */
			break;
		}

		default:
		{
			return kWarpStatusBadDeviceCommand;
		}
	}


	i2c_device_t slave =
	{
		.address = deviceINA219State.i2cAddress,
		.baudRate_kbps = gWarpI2cBaudRateKbps
	};


	cmdBuf[0] = deviceRegister;

	status = I2C_DRV_MasterReceiveDataBlocking(
							0 /* I2C peripheral instance */,
							&slave,
							cmdBuf,
							1,
							(uint8_t *)deviceINA219State.i2cBuffer,
							numberOfBytes,
							gWarpI2cTimeoutMilliseconds);

	if (status != kStatus_I2C_Success)
	{
		return kWarpStatusDeviceCommunicationFailed;
	}

	return kWarpStatusOK;
}

void
printSensorDataINA219(bool hexModeFlag)
{
	int16_t	bus_voltage_raw;
	WarpStatus	i2cReadStatus;
	WarpStatus  i2cWriteStatus;
	//int16_t	current_raw;

	//i2cconfig = configureSensorINA219(0x5000, 0x20);

	// set the calibration register of the INA219 (register 0x05) to the payload byte 0x5000 (concatenated), which is the calibration value
	i2cWriteStatus = writeSensorRegisterINA219(0x05, 0x50, 0x00, 0x00);

	if (i2cWriteStatus != kWarpStatusOK)
	{
		SEGGER_RTT_printf(0, "Write failed");
	}


  	//int16_t ina219_cal = 4096 (0x1000);


	/*
	 *	From the MMA8451Q datasheet:
	 *
	 *		"A random read access to the LSB registers is not possible.
	 *		Reading the MSB register and then the LSB register in sequence
	 *		ensures that both bytes (LSB and MSB) belong to the same data
	 *		sample, even if a new data sample arrives between reading the
	 *		MSB and the LSB byte."
	 *
	 *	We therefore do 2-byte read transactions, for each of the registers.
	 *	We could also improve things by doing a 6-byte read transaction.
	 */

  i2cReadStatus = readSensorRegisterINA219(kWarpSensorOutputRegisterINA219BusVoltage, 2/* numberOfBytes */);
  uint16_t bus_voltage_rawMSB = deviceINA219State.i2cBuffer[0];
  uint16_t bus_voltage_rawLSB = deviceINA219State.i2cBuffer[1];
  int16_t combined = ((bus_voltage_rawMSB & 0xFF) << 6) | (bus_voltage_rawLSB >> 2);
	bus_voltage_raw = (int16_t)((combined >> 3) * 4);

	i2cReadStatus = readSensorRegisterINA219(kWarpSensorOutputRegisterINA219ShuntVoltage, 2 /* numberOfBytes */);
	uint16_t shunt_MSB = deviceINA219State.i2cBuffer[0];
	uint16_t shunt_LSB = deviceINA219State.i2cBuffer[1];
	int16_t shunt_voltage_raw = ((shunt_MSB & 0xFF) << 6) | (shunt_LSB >> 2);

	i2cReadStatus = readSensorRegisterINA219(kWarpSensorOutputRegisterINA219Current, 2 /* numberOfBytes */);
	uint16_t current_MSB = deviceINA219State.i2cBuffer[0];
	uint16_t current_LSB = deviceINA219State.i2cBuffer[1];
	int16_t current_raw = ((current_MSB & 0xFF) << 6) | (current_LSB >> 2);

	i2cReadStatus = readSensorRegisterINA219(kWarpSensorOutputRegisterINA219Power, 2 /* numberOfBytes */);
	uint16_t power_MSB = deviceINA219State.i2cBuffer[0];
	uint16_t power_LSB = deviceINA219State.i2cBuffer[1];
	int16_t power_raw = ((power_MSB & 0xFF) << 6) | (power_LSB >> 2);


	if (i2cReadStatus != kWarpStatusOK)
	{
		SEGGER_RTT_WriteString(0, " ----,");
	}
	else
	{
		/*
		if (hexModeFlag)
		{
			SEGGER_RTT_printf(0, " 0x%02x 0x%02x,", readSensorRegisterValueMSB, readSensorRegisterValueLSB);
		}
		else
		{
			SEGGER_RTT_printf(0, " %d,", readSensorRegisterValueCombined);
		}
		*/
		SEGGER_RTT_printf(0, "\n%d", current_raw);
	}
}
