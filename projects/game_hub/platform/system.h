/**
 ******************************************************************************
 * @file    system.h
 * @brief   System module
 *
 * @module  Game Hub - Platform
 ******************************************************************************
 */

#ifndef __GAME_SYSTEM_H__
#define __GAME_SYSTEM_H__

#ifdef __cplusplus
extern "C" {
#endif

/*==================================================================================================
*                                       FUNCTION PROTOTYPES
==================================================================================================*/
void system_init_console(void);
void system_reboot(void);
void SysInitConsole(void);

#ifdef __cplusplus
}
#endif

#endif /* __GAME_SYSTEM_H__ */
