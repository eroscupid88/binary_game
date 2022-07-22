/*
 * pointer.h
 *
 *  Created on: Mar 6, 2021
 *      Author: Dillon
 */
#include "address_map_nios2.h"
#ifndef POINTER_H_
#define POINTER_H_

#define LEDR_pointer 		(*(volatile int*) LEDR_BASE)
#define LEDG_pointer 		(*(volatile int*) LEDG_BASE)
#define SW_pointer 			(*(volatile int*) SW_BASE)
#define KEY_pointer 		(*(volatile int*) KEY_BASE)
#define HEX3_HEX0_pointer 	(*(volatile int*) HEX3_HEX0_BASE)
#define HEX7_HEX4_pointer 	(*(volatile int*) HEX7_HEX4_BASE)

#endif /* POINTER_H_ */
