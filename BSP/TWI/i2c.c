/* ------------------------------------------------------------------
 * I2C Library for AVR Devices
 * ------------------------------------------------------------------ */

#define F_CPU 800000UL
#include "i2c.h"
#include "IOU_board.h"
/**
 * Initialize I2C Interface
 */
void i2c_init ( void )
{
    TWCR = 1 << TWEN ;
    TWBR = (F_CPU / I2C_CONFIG_F_SCL - 16 ) / 2;
}

/**
 * Initialize I2C Interface with pullups
 */
void i2c_init_with_pullups ( void )
{
    I2C_CONFIG_DDR &= ~( (1 <<  I2C_CONFIG_SDA ) | (1 <<  I2C_CONFIG_SCL ) );
    I2C_CONFIG_PORT |=  ( 1 << I2C_CONFIG_SDA ) | ( 1 << I2C_CONFIG_SCL );
    i2c_init (  );
}

/**
 * Uninitialize I2C Interface
 */
void i2c_uninit ( void )
{
    TWCR &= ~ (1 <<  TWEN );
}

/**
 * Wait until I2C Interface is ready
 */
static void i2c_wait ( void )
{
    while ( !( TWCR &  (1 << TWINT ) ) );
}

/**
 * Send I2C Start Condition
 */
int8_t i2c_start ( void )
{
    TWCR =  (1 << TWEN ) | (1 << TWINT ) |  (1 << TWSTA );
    i2c_wait (  );
    return ( TWSR & TW_STATUS_MASK ) != TW_START;
}

/**
 * Send I2C Repeated-Start Condition
 */
int8_t i2c_restart ( void )
{
    TWCR =  (1 << TWEN ) |  (1 << TWINT ) |  (1 << TWSTA );
    i2c_wait (  );
    return ( TWSR & TW_STATUS_MASK ) != TW_REP_START;
}

/**
 * Send I2C Stop Condition
 */
int8_t i2c_stop ( void )
{
    TWCR =  ( 1 << TWEN ) |  (1 << TWINT ) |  (1 << TWSTO );
    while ( TWCR &  (1 << TWSTO ) );
    return 0;
}

/**
 * Select I2C Slave Address
 */
int8_t i2c_addr ( uint8_t addr )
{
    TWDR = addr;
    TWCR =  (1 << TWEN ) |  (1 << TWINT );
    i2c_wait (  );
    return ( TWSR & TW_STATUS_MASK ) != ( ( addr & 1 ) ? TW_MR_SLA_ACK : TW_MT_SLA_ACK );
}

/**
 * Send data byte to Slave Device
 */
int8_t i2c_tx_byte ( uint8_t byte )
{
    TWDR = byte;
    TWCR =  ( 1 << TWEN ) |  (1 << TWINT );
    i2c_wait (  );
    return ( TWSR & TW_STATUS_MASK ) != TW_MT_DATA_ACK;
}

/**
 * Send data bytes to Slave Device
 */
int8_t i2c_tx_data ( const uint8_t * data, size_t len )
{
    size_t i;

    for ( i = 0; i < len; i++ )
    {
        if ( i2c_tx_byte ( data[i] ) )
        {
            return 1;
        }
    }

    return 0;
}

/**
 * Receive data byte from Slave Device
 */
int8_t i2c_rx_byte ( uint8_t * byte )
{
    TWCR =  (1 << TWEN ) |  (1 << TWINT ) |  (1 << TWEA );
    i2c_wait (  );
    *byte = TWDR;
    return ( TWSR & TW_STATUS_MASK ) != TW_MR_DATA_ACK;
}

/**
 * Receive last data byte from Slave Device
 */
int8_t i2c_rx_last ( uint8_t * byte )
{
    TWCR =  (1 << TWEN ) |  (1 << TWINT );
    i2c_wait (  );
    *byte = TWDR;
    return ( TWSR & TW_STATUS_MASK ) != TW_MR_DATA_NACK;
}

/**
 * Receive data bytes from Slave Device
 */
int8_t i2c_rx_data ( uint8_t * data, size_t len )
{
    size_t i;

    for ( i = 0; i + 1 < len; i++ )
    {
        if ( i2c_rx_byte ( data + i ) )
        {
            return 1;
        }
    }

    if ( len )
    {
        if ( i2c_rx_last ( data + i ) )
        {
            return 1;
        }
    }

    return 0;
}
