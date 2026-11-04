/**
 ******************************************************************************
 * @file    bike_racer_bitmap.h
 * @brief   Bike racer bitmap module
 *
 * @module  Game Hub - Games
 ******************************************************************************
 */

#ifndef __BIKE_RACER_BITMAP_H__
#define __BIKE_RACER_BITMAP_H__

/*==================================================================================================
*                                          INCLUDE FILES
==================================================================================================*/
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*==================================================================================================
*                                        DEFINES AND MACROS
==================================================================================================*/

#define BIKE_RACER_BIKE_WIDTH  (16u)
#define BIKE_RACER_BIKE_HEIGHT (16u)

extern const uint8_t bike_racer_bike[];
extern const uint8_t bike_racer_bike_mask[];
extern const uint8_t bike_racer_sine[];

#ifdef __cplusplus
}
#endif

#endif /* __BIKE_RACER_BITMAP_H__ */
