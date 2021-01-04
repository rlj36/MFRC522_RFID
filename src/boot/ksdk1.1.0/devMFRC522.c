#include <stdint.h>

#include "fsl_spi_master_driver.h"
#include "fsl_port_hal.h"

#include "SEGGER_RTT.h"
#include "gpio_pins.h"
#include "warp.h"
#include "devMFRC522.h"

volatile uint8_t	inBuffer[32];
volatile uint8_t	payloadBytes[32];
extern volatile WarpSPIDeviceState	deviceMFRC522State;
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

static int
writeCommand(uint8_t deviceRegister, uint8_t payload)
{
	spi_status_t status;

	payloadBytes[0] = deviceRegister;
	payloadBytes[1] = payload;
	status = SPI_DRV_MasterTransferBlocking(0	/* master instance */,
					NULL		/* spi_master_user_config_t */,
					(const uint8_t * restrict)&payloadBytes,
					(uint8_t * restrict)&inBuffer[0],
					2		/* transfer size */,
					1000		/* timeout in microseconds (unlike I2C which is ms) */);
	return kWarpStatusOK;
}


WarpStatus
readCommand(uint8_t deviceRegister)
{
	return writeCommand(deviceRegister /*command == read register*/, 0x00 /*writeValue*/);
}




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

	enableSPIpins();

	/*
	 *	Override Warp firmware's use of these pins.
	 *
	 *	Reconfigure to use as GPIO.
	 */
	PORT_HAL_SetMuxMode(PORTB_BASE, 10u, kPortMuxAsGpio);
	PORT_HAL_SetMuxMode(PORTA_BASE, 5u, kPortMuxAsGpio);


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

//	writeCommand(0xxxxxx0 , )

	SEGGER_RTT_printf(0, "\n%d", readCommand(0xEE));
	return 0;
}
