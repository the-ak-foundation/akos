/**
 ******************************************************************************
 * @file    bitmap_viewer_bitmaps.h
 * @brief   Bitmap viewer bitmaps module
 *
 * @module  Game Hub - Games
 ******************************************************************************
 */

#ifndef __BITMAP_VIEWER_BITMAPS_H__
#define __BITMAP_VIEWER_BITMAPS_H__

/*==================================================================================================
*                                          INCLUDE FILES
==================================================================================================*/
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*==================================================================================================
*                                  STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/

typedef struct {
    uint8_t width;
    uint8_t height;
    uint8_t stride;
    const uint8_t *data;
} bitmap_viewer_entry_t;

typedef struct {
    bitmap_viewer_entry_t bitmap;
    uint8_t x;
    uint8_t y;
} bitmap_viewer_placement_t;

extern const bitmap_viewer_entry_t bitmap_viewer_catalog[];
extern const size_t bitmap_viewer_catalog_count;
extern const bitmap_viewer_placement_t bitmap_viewer_farm_layout[];
extern const size_t bitmap_viewer_farm_layout_count;

#ifdef __cplusplus
}
#endif

#endif /* __BITMAP_VIEWER_BITMAPS_H__ */
