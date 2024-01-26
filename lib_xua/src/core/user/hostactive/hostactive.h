// Copyright 2013-2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/**
 * @brief   User host active code
 *
 * This function can be used to perform user defined actions based on host present/not-present events.
 * This function is called on a change in state.
 *
 * \param active  Indicates if the host is active or not. 1 for active, else 0
 */
void UserHostActive(int active);
