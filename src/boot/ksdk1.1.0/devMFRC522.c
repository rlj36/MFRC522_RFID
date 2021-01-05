#include <stdint.h>

#include "fsl_spi_master_driver.h"
#include "fsl_port_hal.h"
#include "fsl_misc_utilities.h"
#include "fsl_device_registers.h"
#include "fsl_rtc_driver.h"
#include "fsl_clock_manager.h"
#include "fsl_power_manager.h"
#include "fsl_mcglite_hal.h"

#include "SEGGER_RTT.h"
#include "gpio_pins.h"
#include "warp.h"
#include "devMFRC522.h"


volatile uint8_t	inBuffer[32];
volatile uint8_t	payloadBytes[32];
volatile WarpSPIDeviceState	deviceMFRC522State;
extern volatile uint32_t		gWarpSpiTimeoutMicroseconds;
extern volatile uint32_t		gWarpSPIBaudRateKbps;


/*
 *	Override Warp firmware's use of these pins and define new aliases.
 */
enum
{
	kMFRC522PinRST		= GPIO_MAKE_PIN(HW_GPIOB, 10),
	kMFRC522PinMISO		= GPIO_MAKE_PIN(HW_GPIOB, 11),
	kMFRC522PinMOSI		= GPIO_MAKE_PIN(HW_GPIOA, 7),
	kMFRC522PinSCK		= GPIO_MAKE_PIN(HW_GPIOB, 0),
	kMFRC522PinSDA		= GPIO_MAKE_PIN(HW_GPIOA, 5),
};




//command: 0x00 for write, 0x01 for read
WarpStatus
writeCommand(uint8_t command, uint8_t deviceRegister, uint8_t writeValue,int numberOfBytes)
{
	spi_status_t status;
	uint8_t addressByte;

	if (command == 0x00)
	{
		 addressByte = (deviceRegister<<1);
	}

	if (command == 0x01)
	{
		 addressByte = ((deviceRegister<<1) | 0x80);
	}

	deviceMFRC522State.spiSourceBuffer[0] = addressByte;
	deviceMFRC522State.spiSourceBuffer[1] = writeValue;

	deviceMFRC522State.spiSinkBuffer[0] = 0x00;
	deviceMFRC522State.spiSinkBuffer[1] = 0x00;
	deviceMFRC522State.spiSinkBuffer[2] = 0x00;

	SEGGER_RTT_printf(0, "\r\t Address Byte: 0x%x \n", addressByte);
	SEGGER_RTT_printf(0, "\r\t SPI Source Buffer: 0x%x \n", deviceMFRC522State.spiSourceBuffer);
	SEGGER_RTT_printf(0, "\r\t SPI Source Buffer: 0x%x \n", deviceMFRC522State.spiSinkBuffer);


	status = SPI_DRV_MasterTransferBlocking(0	/* master instance */,
					NULL		/* spi_master_user_config_t */,
					(const uint8_t * restrict)deviceMFRC522State.spiSourceBuffer,
					(uint8_t * restrict)deviceMFRC522State.spiSinkBuffer,
					numberOfBytes		/* transfer size */,
					1000		/* timeout in microseconds (unlike I2C which is ms) */);
	return kWarpStatusOK;
}


//readCommand(uint8_t deviceRegister)
//{
//	return writeCommand(deviceRegister /*command == read register*/, 0x00 /*writeValue*/);
//}




int
devMFRC522init(void)
{
	/*
	 *	Override Warp firmware's use of these pins.
	 *
	 *	Re-configure SPI to be on PTA8 and PTA9 for MOSI, MISO and SCK respectively.
	 */
	PORT_HAL_SetMuxMode(PORTA_BASE, 7u, kPortMuxAlt3);
	PORT_HAL_SetMuxMode(PORTB_BASE, 11u, kPortMuxAlt3);
	PORT_HAL_SetMuxMode(PORTB_BASE, 0u, kPortMuxAlt3);
	PORT_HAL_SetMuxMode(PORTA_BASE, 5u, kPortMuxAlt3);

	enableSPIpins();

	/*
	 *	Override Warp firmware's use of these pins.
	 *
	 *	Reconfigure to use as GPIO.
	 */
	PORT_HAL_SetMuxMode(PORTB_BASE, 10u, kPortMuxAsGpio);


	/*
	 *	RST high->low->high.
	 */
	GPIO_DRV_SetPinOutput(kMFRC522PinRST);
	OSA_TimeDelay(100);
	GPIO_DRV_ClearPinOutput(kMFRC522PinRST);
	OSA_TimeDelay(100);
	GPIO_DRV_SetPinOutput(kMFRC522PinRST);
	OSA_TimeDelay(100);


	SEGGER_RTT_WriteString(0, "\r\n\tMFRC test...\n");

//	SEGGER_RTT_printf(0, "\n%d", writeCommand(0xEE, 0xFF));
	writeCommand(0x01, 0x07, 0x00, 0x05);
  SEGGER_RTT_printf(0, "\r\t Bytes Received: 0x%x 0x%x 0x%x \n", deviceMFRC522State.spiSinkBuffer[0], deviceMFRC522State.spiSinkBuffer[1], deviceMFRC522State.spiSinkBuffer[2]);
	return 0;
}
