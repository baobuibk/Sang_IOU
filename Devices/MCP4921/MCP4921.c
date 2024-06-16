/*
 * MCP4921.c
 *
 * Created: 5/19/2024 8:26:31 AM
 *  Author: Admin
 */ 
// Assert the SS line
#include  "MCP4921.h"
#include  <util/delay.h>
typedef	struct _mcp4921_config_t {
	union {
		uint16_t value;
		struct 
		{
			uint8_t	byte0;
			uint8_t	byte1;			
		} bytefield;
		
		struct __attribute__((__packed__)) {
			uint16_t data : 12;
			uint16_t not_shutdown : 1;
			uint16_t ga : 1;
			uint16_t buf : 1;
			uint16_t not_write : 1;
		} bitfield;
	};
}mcp4921_config_t;

//static mcp4921_config_t		s_mcp4921_config[4];


void MCP4291_DeassertSS(uint8_t CS_line);

// Deassert the SS line
void MCP4291_AssertSS(uint8_t CS_line);
void MCP4291_assert_LDAC(void);
void MCP4291_deassert_LDAC(void);

void	MCP4921_init(void)
{
		MCP4921_SS0_DDR |= 1 << MCP4921_SS0_PIN;
		MCP4921_SS1_DDR |= 1 << MCP4921_SS1_PIN;
		MCP4921_SS2_DDR |= 1 << MCP4921_SS2_PIN;
		MCP4921_SS3_DDR |= 1 << MCP4921_SS3_PIN;
		MCP4921_LDAC_DDR |= 1 << MCP4921_LDAC_PIN;
		SPI_Init();
}

// Assert the SS line
void MCP4291_AssertSS(uint8_t CS_line)
{
	switch (CS_line)
	{
		case 0:
			MCP4921_SS0_PORT &= ~(1 << MCP4921_SS0_PIN);
			break;
		case 1:
			MCP4921_SS1_PORT &= ~(1 << MCP4921_SS1_PIN);
			break;
		case 2:
			MCP4921_SS2_PORT &= ~(1 << MCP4921_SS2_PIN);
			break;
		case 3:
			MCP4921_SS3_PORT &= ~(1 << MCP4921_SS3_PIN);
			break;
	}

}

// Deassert the SS line
void MCP4291_DeassertSS(uint8_t CS_line)
{
	switch (CS_line)
	{
		case 0:
			MCP4921_SS0_PORT |= (1 << MCP4921_SS0_PIN);
			break;
		case 1:
			MCP4921_SS1_PORT |= (1 << MCP4921_SS1_PIN);
			break;
		case 2:
			MCP4921_SS2_PORT |= (1 << MCP4921_SS2_PIN);
			break;
		case 3:
			MCP4921_SS3_PORT |= (1 << MCP4921_SS3_PIN);
			break;
	}
}

void MCP4291_assert_LDAC(void)
{
	MCP4921_LDAC_PORT &= ~(1 << MCP4921_LDAC_PIN);
}
void MCP4291_deassert_LDAC(void)
{
	MCP4921_LDAC_PORT |= (1 << MCP4921_LDAC_PIN);
}
/*******************************************************************************
  * @name   MCP4291_set_output
  * @brief  Function to set the output voltage of the DAC
  * @param  Val: 12bit
  * @param	shutdown:	turn off output
  * @param	gain_ena = 0 mean gain = 1; gain_ena = 1 mean gain = 2.
  * @param	buf_ena = 1: output buffered.
  * @param	DAC_number: from 0-3
  * @retval status: 0 PASS, >0: error
  *****************************************************************************/
uint8_t	MCP4291_set_output(uint16_t	Val, uint8_t shutdown, uint8_t gain_ena, uint8_t buf_ena, uint8_t DAC_num)
{
	volatile mcp4921_config_t		s_mcp4921_config[4];
	if (DAC_num >3)	return 1;
	
	MCP4291_AssertSS(DAC_num);
	s_mcp4921_config[DAC_num].bitfield.data = Val ;
	if (shutdown)
		s_mcp4921_config[DAC_num].bitfield.not_shutdown = 0;
		else s_mcp4921_config[DAC_num].bitfield.not_shutdown = 1;
	if (gain_ena)	s_mcp4921_config[DAC_num].bitfield.ga = 0;
	else s_mcp4921_config[DAC_num].bitfield.ga = 1;
	s_mcp4921_config[DAC_num].bitfield.buf = buf_ena;
	s_mcp4921_config[DAC_num].bitfield.not_write = 0;
	
	SPI_SendByte(s_mcp4921_config[DAC_num].bytefield.byte1);

	SPI_SendByte(s_mcp4921_config[DAC_num].bytefield.byte0);

	MCP4291_DeassertSS(DAC_num);
	MCP4291_assert_LDAC();
	MCP4291_deassert_LDAC();
	return	0;
}