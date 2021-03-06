/*
 * Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

/* PCBEEP interface for Chrome OS verified boot */

#ifndef CHROMEOS_PCBEEP_H_
#define CHROMEOS_PCBEEP_H_

/* Beep control */

/*
 * Start the beep
 *
 * @param frequency	The frequency of the beep in Hz
 */
void enable_beep(uint32_t frequency);

/*
 * Stop the beep
 */
void disable_beep(void);

#endif /* CHROMEOS_PCBEEP_H_ */
