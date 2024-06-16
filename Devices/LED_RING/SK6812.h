/*
 * ringled.h
 *
 * Created: 5/28/2024 4:23:48 PM
 *  Author: HTSANG
 */ 


#ifndef SK6812_H_
#define SK6812_H_

#include <stdint.h>
#include <IOU_board.h>

typedef struct rgb_color
{
	uint8_t red, green, blue, white;
} rgb_color;

#define LED_COUNT 10
rgb_color colorMap[LED_COUNT];

void ringled_show(const rgb_color *colors, uint8_t pixels);
void ringled_clear(void);
void ringled_init(void);
void ringled_set_RGB(uint8_t red, uint8_t green, uint8_t blue, uint8_t white);
rgb_color ringled_get_RGB(void);

#endif /* RINGLED_H_ */