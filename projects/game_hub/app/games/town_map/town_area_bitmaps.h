/**
 ******************************************************************************
 * @file    town_area_bitmaps.h
 * @brief   Town area bitmaps module
 *
 * @module  Game Hub - Games
 ******************************************************************************
 */

#ifndef __TOWN_AREA_BITMAPS_H__
#define __TOWN_AREA_BITMAPS_H__

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

#define TOWN_AREA_BITMAP_WIDTH  (96u)
#define TOWN_AREA_BITMAP_HEIGHT (48u)
#define TOWN_AREA_BITMAP_SIZE   (576u)
#define TOWN_FARM_ICON_WIDTH    (32u)
#define TOWN_FARM_ICON_HEIGHT   (32u)
#define TOWN_FARM_ICON_SIZE     (128u)
#define TOWN_POND_ICON_WIDTH    (32u)
#define TOWN_POND_ICON_HEIGHT   (32u)
#define TOWN_POND_ICON_SIZE     (128u)

extern const uint8_t town_farm_bitmap[TOWN_AREA_BITMAP_SIZE];
extern const uint8_t town_fish_pond_bitmap[TOWN_AREA_BITMAP_SIZE];
extern const uint8_t town_farm_entrance_icon[TOWN_FARM_ICON_SIZE];
extern const uint8_t town_pond_entrance_icon[TOWN_POND_ICON_SIZE];

#ifdef __cplusplus
}
#endif

#endif /* __TOWN_AREA_BITMAPS_H__ */
