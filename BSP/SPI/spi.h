// spi.h
//
// SPI master routines were pulled from the Atmel ATMega168 datasheet.

#ifndef _SPI_H
#define _SPI_H

#include <inttypes.h>

// SPI Definitions


#define SPI_SPCR	SPCR
#define SPI_SPDR	SPDR
#define SPI_SPSR	SPSR
#define SPI_SPIF	SPIF
#define SPI_SPE		SPE
#define SPI_MSTR	MSTR
#define SPI_SPR0	SPR0
#define SPI_SPR1	SPR1


// Loop until any current SPI transmissions have completed
#define spi_wait()	while (!(SPSR & (1 << SPIF)));

// Initialize the SPI subsystem
void SPI_Init(void);

// Transfer a byte of data
uint8_t SPI_SendByte( uint8_t data );

// Read a byte of data
uint8_t SPI_ReadByte( void );



#endif // _SPI_CLARKDAWG
