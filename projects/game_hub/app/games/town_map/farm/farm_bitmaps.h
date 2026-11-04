/**
 ******************************************************************************
 * @file    farm_bitmaps.h
 * @brief   Farm bitmaps module
 *
 * @module  Game Hub - Games
 ******************************************************************************
 */

#ifndef __FARM_BITMAPS_H__
#define __FARM_BITMAPS_H__

/*==================================================================================================
*                                          INCLUDE FILES
==================================================================================================*/
#include <stdint.h>

#include "bitmap_viewer_bitmaps.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Editable FARM bitmap sheet.
 *
 * Encoding: row-major, MSB first. '#' in the ASCII notes means pixel ON,
 * '.' means pixel OFF. stride = (width + 7) / 8 bytes for every row.
 * Change width/height together with the byte array when replacing an icon.
 */
#define FARM_BITMAP(width_value, height_value, data_value)                                         \
    {width_value, height_value, (width_value + 7u) / 8u, data_value}

/* WATERING CAN, 12x8. ASCII role: watering-required marker and toolbar tool. */

/*==================================================================================================
*                                         LOCAL VARIABLES
==================================================================================================*/
static const uint8_t farm_watering_can_bits[] = {
    0x07, 0x00, 0x28, 0xB0, 0x6F, 0xD0, 0xFF, 0x90, 0x3F, 0x90, 0x1F, 0xE0, 0x0F, 0x80, 0x1F, 0xC0,
};
static const bitmap_viewer_entry_t farm_watering_can_bitmap =
    FARM_BITMAP(12u, 8u, farm_watering_can_bits);

/* HOE, 12x10. ASCII role: clear an immature plot or harvest a mature crop. */
static const uint8_t farm_hoe_bits[] = {
    0x01, 0x80, 0x03, 0x40, 0x07, 0xA0, 0x05, 0xD0, 0x0C, 0xF0,
    0x18, 0x60, 0x30, 0x00, 0x60, 0x00, 0xC0, 0x00, 0xC0, 0x00,
};
static const bitmap_viewer_entry_t farm_hoe_bitmap = FARM_BITMAP(12u, 10u, farm_hoe_bits);

/* SEED BAG, 9x10. ASCII role: plant one of the three random crop types. */
static const uint8_t farm_seed_bag_bits[] = {
    0x7F, 0x00, 0x3E, 0x00, 0x00, 0x00, 0x2A, 0x00, 0x7F, 0x00,
    0xF7, 0x80, 0xDD, 0x80, 0xEB, 0x80, 0xF7, 0x80, 0x7F, 0x00,
};
static const bitmap_viewer_entry_t farm_seed_bag_bitmap = FARM_BITMAP(9u, 10u, farm_seed_bag_bits);

/* EXIT, 9x9. ASCII role: fourth toolbar item, returns to Town. */
static const uint8_t farm_exit_bits[] = {
    0xF7, 0x80, 0x97, 0x80, 0x97, 0x80, 0xF7, 0x80, 0x00,
    0x00, 0xF7, 0x80, 0xF7, 0x80, 0xF7, 0x80, 0xF7, 0x80,
};
static const bitmap_viewer_entry_t farm_exit_bitmap = FARM_BITMAP(9u, 9u, farm_exit_bits);

/* COIN, 7x7: shared currency icon shown in FARM and Town Map.
 * .#####.
 * ##...##
 * #.#.#.#
 * #..##.#
 * #.#.#.#
 * ##...##
 * .#####.
 */
static const uint8_t farm_coin_bits[] = {
    0x7C, 0xC6, 0xAA, 0x9A, 0xAA, 0xC6, 0x7C,
};
static const bitmap_viewer_entry_t farm_coin_bitmap = FARM_BITMAP(7u, 7u, farm_coin_bits);

/* SUN 8x8 and MOON 5x8: exactly one is shown beside the farm clock. */
static const uint8_t farm_sun_bits[] = {0x24, 0x00, 0x99, 0x24, 0x24, 0x99, 0x00, 0x24};
static const uint8_t farm_moon_bits[] = {0x18, 0x30, 0x60, 0xE0, 0xE0, 0x60, 0x30, 0x18};
static const bitmap_viewer_entry_t farm_sun_bitmap = FARM_BITMAP(8u, 8u, farm_sun_bits);
static const bitmap_viewer_entry_t farm_moon_bitmap = FARM_BITMAP(5u, 8u, farm_moon_bits);

/* PLOT, 15x15. ASCII role: the common soil tile and the cancel-X tile base. */
static const uint8_t farm_plot_bits[] = {
    0xFF, 0xFE, 0x92, 0x4A, 0x80, 0x02, 0xC0, 0x06, 0x80, 0x02, 0x80, 0x02, 0xC0, 0x06, 0x80,
    0x02, 0x80, 0x02, 0x80, 0x06, 0xC0, 0x02, 0x80, 0x02, 0x80, 0x06, 0xC9, 0x22, 0xFF, 0xFE,
};
static const bitmap_viewer_entry_t farm_plot_bitmap = FARM_BITMAP(15u, 15u, farm_plot_bits);

/* SEED/SPROUT, 9x7: shown immediately after planting, before first watering. */
static const uint8_t farm_sprout_bits[] = {
    0xDB, 0x00, 0x00, 0x00, 0x6D, 0x80, 0x00, 0x00, 0xDB, 0x00, 0x00, 0x00, 0x6D, 0x80,
};
static const bitmap_viewer_entry_t farm_sprout_bitmap = FARM_BITMAP(9u, 7u, farm_sprout_bits);

/* SOFT SOIL, 9x8: appears after the first watering. */
static const uint8_t farm_soft_soil_bits[] = {
    0x63, 0x00, 0x94, 0x80, 0x00, 0x00, 0x63, 0x00, 0x94, 0x80, 0x00, 0x00, 0x63, 0x00, 0x94, 0x80,
};
static const bitmap_viewer_entry_t farm_soft_soil_bitmap = FARM_BITMAP(9u, 8u, farm_soft_soil_bits);

/* TWO-LEAF SEEDLING, 5x6: grows after the soft-soil stage.
 * ASCII preview:
 * #...#
 * .#.#.
 * ..#..
 * ..#..
 * .###.
 * ..#..
 */
static const uint8_t farm_two_leaves_bits[] = {
    0x88, 0x50, 0x20, 0x20, 0x70, 0x20,
};
static const bitmap_viewer_entry_t farm_two_leaves_bitmap =
    FARM_BITMAP(5u, 6u, farm_two_leaves_bits);

/* FLOWERING SEEDLING, 7x6.
 * ASCII preview:
 * .#...#.
 * #.#.#.#
 * .#.#.#.
 * ...#...
 * ...#...
 * ..###..
 */
static const uint8_t farm_flower_bits[] = {0x44, 0xAA, 0x54, 0x10, 0x10, 0x38};
static const bitmap_viewer_entry_t farm_flower_bitmap = FARM_BITMAP(7u, 6u, farm_flower_bits);

/* MATURE CROP 1, WHEAT, 9x15. */
static const uint8_t farm_wheat_bits[] = {
    0x08, 0x00, 0x08, 0x00, 0x36, 0x00, 0x1C, 0x00, 0x28, 0x00, 0x36, 0x00, 0x1C, 0x00, 0x2A,
    0x00, 0x36, 0x00, 0x1C, 0x00, 0x41, 0x00, 0x2A, 0x00, 0x08, 0x00, 0x49, 0x00, 0xAA, 0x80,
};
static const bitmap_viewer_entry_t farm_wheat_bitmap = FARM_BITMAP(9u, 15u, farm_wheat_bits);

/* MATURE CROP 2, PUMPKIN, 9x10. */
static const uint8_t farm_pumpkin_bits[] = {
    0x20, 0x00, 0x52, 0x00, 0x2C, 0x00, 0x08, 0x00, 0x3E, 0x00,
    0x55, 0x00, 0xAA, 0x80, 0xAA, 0x80, 0x55, 0x00, 0x3E, 0x00,
};
static const bitmap_viewer_entry_t farm_pumpkin_bitmap = FARM_BITMAP(9u, 10u, farm_pumpkin_bits);

/* MATURE CROP 3, TOMATO, 9x14. */
static const uint8_t farm_tomato_bits[] = {
    0x08, 0x00, 0x14, 0x00, 0x0B, 0x00, 0x13, 0x00, 0x28, 0x00, 0x68, 0x00, 0x6A, 0x00,
    0x05, 0x80, 0x09, 0x80, 0x28, 0x00, 0xD0, 0x00, 0xC8, 0x00, 0x0C, 0x00, 0x29, 0x00,
};
static const bitmap_viewer_entry_t farm_tomato_bitmap = FARM_BITMAP(9u, 14u, farm_tomato_bits);

#undef FARM_BITMAP

#ifdef __cplusplus
}
#endif

#endif /* __FARM_BITMAPS_H__ */
