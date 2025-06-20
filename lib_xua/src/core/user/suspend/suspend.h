// Copyright 2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef _SUSPEND_H_
#define _SUSPEND_H_

/**
 * @brief   User code shut down the xcore during USB suspend
 *
 * It is called from EP0 on the XUD_TILE.
 * This function is called during suspend and should not return until the power setting is complete.
 */
void XUA_UserSuspendPowerDown();

/**
 * @brief   User code power up the xcore at USB resume
 *
 * It is called from EP0 on the XUD_TILE.
 * This function is called during resume and should not return until the power setting is complete.
 */
void XUA_UserSuspendPowerUp();

#endif
