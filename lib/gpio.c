/*
 * gpio.h:
 *      Copyright (c) 2015 David Mandala <david.mandala@linaro.org
 *
 ***********************************************************************
 * This file is part of 96BoardsGPIO:
 * https://
 *
 * This library (96BoardsGPIO) is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of the
 * License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 *
 ***********************************************************************
 */
#include <stdlib.h>

#include <libsoc_board.h>
#include <libsoc_gpio.h>


typedef struct _gpio_list{
	gpio *g;
	struct _gpio_list *next;
} gpio_list;

static gpio_list *gpios = NULL;
static board_config *config = NULL;

__attribute__((destructor)) static void _cleanup()
{
	gpio_list *tmp, *ptr = gpios;
	while (ptr) {
		tmp = ptr;
		libsoc_gpio_free(ptr->g);
		ptr = ptr->next;
		free(tmp);
	}
	if (config)
		libsoc_board_free(config);
}


unsigned int gpio_id(const char *pin_name)
{
	if (!config)
		config = libsoc_board_init();
	return libsoc_board_gpio_id(config, pin_name);
}

int gpio_open(unsigned int gpio_id, const char *direction)
{
	int rc = -1;
	gpio_list *ptr;
	gpio *g = libsoc_gpio_request(gpio_id, LS_SHARED);
	if (!g)
		return rc;
	if (!strcmp(direction, "in"))
		rc = libsoc_gpio_set_direction(g, INPUT);
	else
		rc = libsoc_gpio_set_direction(g, OUTPUT);

	ptr = calloc(sizeof(gpio_list), 1);
	ptr->g = g;
	ptr->next = gpios;
	gpios = ptr;
	return rc;
}

int digitalRead(unsigned int gpio_id)
{
	gpio_list *ptr = gpios;
	while(ptr) {
		if (ptr->g->gpio == gpio_id) {
			return libsoc_gpio_wait_interrupt(ptr->g, -1);
		}
		ptr = ptr->next;
	}
	return -1;
}

int digitalWrite(unsigned int gpio_id, unsigned int value)
{
	gpio_list *ptr = gpios;
	while(ptr) {
		if (ptr->g->gpio == gpio_id) {
			return libsoc_gpio_set_level(ptr->g, value);
		}
		ptr = ptr->next;
	}
	return -1;
}
