/*
 * ringled.c
 *
 * Created: 5/28/2024 4:24:02 PM
 *  Author: HTSANG
 */ 

#include "SK6812.h"
#include "IOU_board.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>

rgbw_color status_rgbw_color;

void ringled_init(void)
{
	ringled_clear();
	ringled_show(colorMap, LED_COUNT);
}

void __attribute__((noinline)) ringled_show(const rgbw_color *colors, uint8_t pixels)
{
	RINGLED_PORT &= ~(1 << RINGLED_DQ);
	RINGLED_DDR  |= (1 << RINGLED_DQ);
	
	cli();   // Disable interrupts temporarily because we don't want our pulse timing to be messed up.
	while (pixels--)
	{
		uint8_t portValue = RINGLED_PORT;
		// Send a color to the LED strip.
		// The assembly below also increments the 'colors' pointer,
		// it will be pointing to the next color at the end of this loop.
		asm volatile (
		"ld __tmp_reg__, %a0+\n"
		"ld __tmp_reg__, %a0\n"
		"rcall send_led_strip_byte%=\n"  // Send green component.
		"ld __tmp_reg__, -%a0\n"
		"rcall send_led_strip_byte%=\n"  // Send red component.
		"ld __tmp_reg__, %a0+\n"
		"ld __tmp_reg__, %a0+\n"
		"ld __tmp_reg__, %a0+\n"
		"rcall send_led_strip_byte%=\n"  // Send blue component.
		"ld __tmp_reg__, %a0+\n"
		"rcall send_led_strip_byte%=\n"  // Send white component.
		"rjmp led_strip_asm_end%=\n"     // Jump past the assembly subroutines.
		// send_led_strip_byte subroutine:  Sends a byte to the LED strip.
		"send_led_strip_byte%=:\n"
		"rcall send_led_strip_bit%=\n"  // Send most-significant bit (bit 7).
		"rcall send_led_strip_bit%=\n"
		"rcall send_led_strip_bit%=\n"
		"rcall send_led_strip_bit%=\n"
		"rcall send_led_strip_bit%=\n"
		"rcall send_led_strip_bit%=\n"
		"rcall send_led_strip_bit%=\n"
		"rcall send_led_strip_bit%=\n"  // Send least-significant bit (bit 0).
		"ret\n"
		// send_led_strip_bit subroutine:  Sends single bit to the LED strip by driving the data line
		// high for some time.  The amount of time the line is high depends on whether the bit is 0 or 1,
		// but this function always takes the same time (2 us).
		"send_led_strip_bit%=:\n"
		"rol __tmp_reg__\n"                      // Rotate left through carry.
		"sts %2, %4\n"                           // Drive the line high.
		// If the bit to send is 0, drive the line low now.
		"brcs .+4\n" "sts %2, %3\n"
		"nop\n" "nop\n"
		"brcc .+4\n" "sts %2, %3\n"
		"ret\n"
		"led_strip_asm_end%=: "
		: "=b" (colors)
		: "0" (colors),       // %a0 points to the next color to display
		"" (&RINGLED_PORT),   // %2 is the port register (e.g. PORTH)
		"r" ((uint8_t)(portValue & ~(1 << RINGLED_DQ))),  // %3
		"r" ((uint8_t)(portValue | (1 << RINGLED_DQ)))    // %4
		);
	}
	sei();          // Re-enable interrupts now that we are done.
}


void ringled_clear(void)
{
	for (uint8_t i = 0; i < LED_COUNT; i++)
	colorMap[i] = (rgbw_color){0, 0, 0, 0};
}

void ringled_set_RGBW(uint8_t red, uint8_t green, uint8_t blue, uint8_t white)
{
	status_rgbw_color = (rgbw_color){red, green, blue, white};
	for (uint8_t i = 0; i < LED_COUNT; i++)
		colorMap[i] = (rgbw_color){red, green, blue, white};
	ringled_show(colorMap, LED_COUNT);
}

rgbw_color ringled_get_RGBW(void)
{
	return status_rgbw_color;
}