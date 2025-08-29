/**
 ******************************************************************************
 * @file           : OLED_driver.c
 * @author         : Haoyi Chen
 * @date           : 2025-08-22
 * @brief          : OLED hardware driver port implementation for OLED_UI_Core
 ******************************************************************************
 * @details
 * OLED_UI_Core Library Hardware Abstraction Layer - OLED Driver Port Implementation
 * 
 * This file contains the hardware-level OLED driver port implementation for
 * integrating the OLED_UI_Core library with bare-metal STM32F407VGT6 applications.
 * 
 * Original library: OLED_UI_Core (HAL-based, SPI communication)
 * Port target: STM32F407VGT6 bare-metal (register-based, I2C communication)
 * 
 * Key port implementations:
 * - I2C communication functions replacing original SPI
 * - Multiple OLED controller initialization sequences
 * - Framebuffer management and display updates
 * - Hardware abstraction maintaining library compatibility
 * 
 * Hardware configuration:
 * - I2C1: PB6 (SCL), PB7 (SDA) at 400kHz
 * - Reset: Not connected (software initialization only)
 * - I2C Address: 0x3C (SSD1309_I2C_ADDR_DEFAULT)
 * - Supported controllers: SSD1309
 * 
 * Library integration features:
 * - Framebuffer-based rendering (1024 bytes)
 * - Partial screen updates for efficiency
 * - Compatible interface with original OLED_UI_Core
 * - Register-based hardware abstraction
 ******************************************************************************
 */
#include "OLED_driver.h"
#include "ssd1309.h"



/**
 * @brief OLED display buffer - framebuffer in RAM
 * @details 8 pages × 128 columns = 1024 bytes total
 * Each page represents 8 vertical pixels (1 byte per column)
 */
uint8_t OLED_DisplayBuf[SSD1309_PAGES][SSD1309_WIDTH];

/**
 * @brief Current color mode setting
 * @details true = normal mode, false = inverted mode
 */
bool OLED_ColorMode = true;

/**
 * @brief SSD1309 instance for OLED UI Core integration
 */
static SSD1309_InitTypeDef oled_ssd1309 = {
    .I2Cx = OLED_I2C_PERIPHERAL,
    .DevAddress = OLED_I2C_ADDR,
    .Contrast = 128,           /* Default contrast */
    .InvertDisplay = 0         /* Normal display */
};

/**
 * @brief Write command to OLED via I2C using SSD1309 driver
 * @param data Command byte to send
 */
void OLED_Write_CMD(uint8_t data)
{
    ssd1309_send_command(&oled_ssd1309, data);
}

/**
 * @brief Write single data byte to OLED via I2C using SSD1309 driver
 * @param data Data byte to send
 */
void OLED_Write_DATA(uint8_t data)
{
    uint8_t processed_data = OLED_ColorMode ? data : ~data;  /* Apply color mode */
    ssd1309_send_data(&oled_ssd1309, &processed_data, 1);
}

/**
 * @brief Write data array to OLED via I2C using SSD1309 driver
 * @param Data Data buffer pointer
 * @param Count Number of bytes to send
 */
void OLED_WriteDataArr(uint8_t *Data, uint8_t Count)
{
    static uint8_t buffer[SSD1309_WIDTH];  /* Buffer for processed data */
    
    if (Count > SSD1309_WIDTH) Count = SSD1309_WIDTH;  /* Safety limit */
    
    /* Apply color mode transformation */
    for (uint8_t i = 0; i < Count; i++)
    {
        buffer[i] = OLED_ColorMode ? Data[i] : ~Data[i];
    }
    
    ssd1309_send_data(&oled_ssd1309, buffer, Count);
}

/**
 * @brief Toggle display color inversion using SSD1309 driver
 * @param i Color inversion mode: 0=normal display, 1=inverted display
 */
void OLED_ColorTurn(uint8_t i)
{
	/* Use SSD1309 driver invert function */
	ssd1309_invert_display(&oled_ssd1309, i);
}

/**
 * @brief Control screen display orientation
 * @param i Display orientation: 0=normal display, 1=180° rotated
 * @note Screen will flicker briefly during orientation change
 */
void OLED_DisplayTurn(uint8_t i)
{
	/* Turn off display to prevent flicker during orientation change */
	ssd1309_display_on_off(&oled_ssd1309, 0); /* Display OFF */
	if (i == 0)
	{
		OLED_Write_CMD(SSD1309_COMSCANDEC); /* Normal display */
		OLED_Write_CMD(SSD1309_SEGREMAP_REVERSE);
	}
	if (i == 1)
	{
		OLED_Write_CMD(SSD1309_COMSCANINC); /* 180° rotated display */
		OLED_Write_CMD(SSD1309_SEGREMAP);
	}
	ssd1309_display_on_off(&oled_ssd1309, 1); /* Display ON */
}

/**
 * @brief Turn OLED display ON using SSD1309 driver
 * @details Enables charge pump and activates display
 */
void OLED_DisPlay_On(void)
{
	/* Use SSD1309 driver display on/off function */
	ssd1309_display_on_off(&oled_ssd1309, 1);
}

/**
 * @brief Turn OLED display OFF using SSD1309 driver
 * @details Disables charge pump and deactivates display
 */
void OLED_DisPlay_Off(void)
{
	/* Use SSD1309 driver display on/off function */
	ssd1309_display_on_off(&oled_ssd1309, 0);
}
/**
 * @brief Set OLED display cursor position using SSD1309 driver
 * @param Page Page number (0-7)
 * @param X Column position (0-127)
 * @note OLED Y-axis can only be written in 8-bit groups (1 page = 8 Y coordinates)
 */
void OLED_SetCursor(uint8_t Page, uint8_t X)
{
    
	/* Use SSD1309 driver cursor setting function */
	ssd1309_set_cursor(&oled_ssd1309, X, Page);
}

/**
 * @brief Update entire display from framebuffer
 * @details Transfers all 8 pages (1024 bytes) from OLED_DisplayBuf to display
 */
void OLED_Update(void)
{
	uint8_t j;
	/* Iterate through each page */
	for (j = 0; j < SSD1309_PAGES; j++)
	{
		/* Set cursor to first column of each page */
		OLED_SetCursor(j, 0);
		/* Write entire row data from display buffer to OLED hardware */
		OLED_WriteDataArr(OLED_DisplayBuf[j], SSD1309_WIDTH);
	}
}

/**
 * @brief Update specific area of display from framebuffer
 * @param X Left coordinate of update area (0-127)
 * @param Y Top coordinate of update area (0-63)
 * @param Width Width of update area (0-128)
 * @param Height Height of update area (0-64)
 * @note This function updates at least the specified region
 * @note If Y region includes partial pages, entire pages will be updated
 * @note All display functions only modify the framebuffer; call update functions
 *       to actually transfer data to hardware
 */
void OLED_UpdateArea(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height)
{
	uint8_t j;

	/* Parameter validation to ensure area doesn't exceed screen bounds */
	if (X > SSD1309_WIDTH - 1)
	{
		return;
	}
	if (Y > SSD1309_HEIGHT - 1)
	{
		return;
	}
	if (X + Width > SSD1309_WIDTH)
	{
		Width = SSD1309_WIDTH - X;
	}
	if (Y + Height > SSD1309_HEIGHT)
	{
		Height = SSD1309_HEIGHT - Y;
	}

	/* Iterate through pages affected by the specified area */
	/* Calculate pages with ceiling division: (Y + Height - 1) / 8 + 1 */
	for (j = Y / SSD1309_PAGES; j < (Y + Height - 1) / SSD1309_PAGES + 1; j++)
	{
		/* Set cursor to specified column of affected page */
		OLED_SetCursor(j, X);
		/* Write Width bytes from display buffer to OLED hardware */
		OLED_WriteDataArr(&OLED_DisplayBuf[j][X], Width);
	}
}

/**
 * @brief Clear display buffer (external function)
 * @details This function is implemented in the OLED graphics library
 */
extern void OLED_Clear(void);

/**
 * @brief Initialize OLED display hardware
 * @details Configures I2C communication, GPIO pins, and display controller
 * @note Hardware configuration:
 *       - I2C1: PB8 (SCL), PB9 (SDA) at 400kHz
 *       - Reset: PB5 (active low)
 *       - I2C Address: 0x3C
 */
void OLED_Init(void)
{
	/* Initialize I2C peripheral and GPIO pins */
	I2C_InitTypeDef i2c_config;
	i2c_config.ClockSpeed = I2C_CLOCKSPEED_400KHZ;  /* 400kHz I2C speed - safe and reliable */
	i2c_config.DutyCycle = I2C_DUTYCYCLE_2;
	
	/* Configure I2C GPIO pins: PB6=SCL, PB7=SDA */
	i2c_gpio_init(OLED_I2C_PERIPHERAL, GPIOB, I2C1_SCL_PIN_PB6, I2C1_SDA_PIN_PB7);
	i2c_init(OLED_I2C_PERIPHERAL, &i2c_config);
	
	/* Initialize SSD1309 configuration */
	oled_ssd1309.I2Cx = OLED_I2C_PERIPHERAL;
	oled_ssd1309.DevAddress = OLED_I2C_ADDR;
	oled_ssd1309.Contrast = 128;
	oled_ssd1309.InvertDisplay = 0;
	
#ifdef OLED_USE_RST_PIN
	/* Initialize Reset GPIO pin */
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;  /* Enable GPIOB clock */
	gpio_init(OLED_RST_PORT, OLED_RST_PIN, GPIO_MODE_OUTPUT, 
			  GPIO_OTYPE_PP, GPIO_SPEED_MED, GPIO_NOPULL);

	/* Hardware reset sequence - hold reset low then release */
	OLED_RES_Clr();  /* Assert reset (active low) */
	systick_delay_ms(1);  /* 1ms delay using systick */
	OLED_RES_Set();  /* Release reset */
#else
	/* No hardware reset - add software delay for power-on stabilization */
	systick_delay_ms(2);  /* 2ms delay using systick for stabilization */
#endif




	/* Use SSD1309 driver initialization - comprehensive init sequence */
	ssd1309_init(&oled_ssd1309);
	OLED_Brightness(-1); /* Initialize brightness setting (-1 equals 0) */
	OLED_Clear();
	ssd1309_display_on_off(&oled_ssd1309, 1); /* Display ON using SSD1309 driver */
	systick_delay_ms(1); /* 1ms delay using systick */
}

/**
 * @brief Set OLED display brightness/contrast
 * @param Brightness Brightness level (0-255)
 * @note Effects may vary between different display controllers
 * @note Avoid extreme values (too high or too low)
 * @note Function uses caching to avoid redundant I2C transactions
 */
void OLED_Brightness(int16_t Brightness)
{

	/* Check if brightness setting changed, send command only if changed */
	static int16_t Last_Brightness;
	if (Brightness == Last_Brightness)
	{
		return;
	}
	else
	{
		Last_Brightness = Brightness;
	}

	if (Brightness > SSD1309_MAX_BRIGHTNESS)
	{
		Brightness = SSD1309_MAX_BRIGHTNESS;
	}
	if (Brightness < 0)
	{
		Brightness = 0;
	}
	/* Use SSD1309 driver contrast function */
	ssd1309_set_contrast(&oled_ssd1309, (uint8_t)Brightness);
}

/**
 * @brief Set OLED display color mode
 * @param colormode Color mode: true=normal mode, false=inverted mode
 * @note This affects both framebuffer processing and hardware inversion
 * @note Function uses caching to avoid redundant operations
 */
void OLED_SetColorMode(bool colormode)
{
	OLED_ColorMode = colormode;

	/* Check if display mode changed, send command only if changed */
	static bool Last_OLED_ColorMode;
	if (OLED_ColorMode == Last_OLED_ColorMode)
	{
		return;
	}
	else
	{
		Last_OLED_ColorMode = OLED_ColorMode;
	}

	if (OLED_ColorMode)
		OLED_ColorTurn(0);
	else
		OLED_ColorTurn(1);
}
