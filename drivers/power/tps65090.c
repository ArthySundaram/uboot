/*
 * Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

#include <common.h>
#include <i2c.h>
#include <tps65090.h>

/* TPS65090 register addresses */
enum {
	REG_CG_CTRL0 = 4,
	REG_CG_STATUS1 = 0xa,
	REG_FET1_CTRL = 0x0f,
	REG_FET2_CTRL,
	REG_FET3_CTRL,
	REG_FET4_CTRL,
	REG_FET5_CTRL,
	REG_FET6_CTRL,
	REG_FET7_CTRL,
};

enum {
	CG_CTRL0_ENC_MASK	= 0x01,

	MAX_FET_NUM	= 7,
	MAX_CTRL_READ_TRIES = 5,

	/* TPS65090 FET_CTRL register values */
	FET_CTRL_TOFET		= 1 << 7,  /* Timeout, startup, overload */
	FET_CTRL_PGFET		= 1 << 4,  /* Power good for FET status */
	FET_CTRL_WAIT		= 3 << 2,  /* Overcurrent timeout max */
	FET_CTRL_ADENFET	= 1 << 1,  /* Enable output auto discharge */
	FET_CTRL_ENFET		= 1 << 0,  /* Enable FET */
};

static struct {
	int bus;
	int addr;
	int old_bus;
} config;

/**
 * Write a value to a register
 *
 * @param	chip_addr	i2c slave addr for max77686
 * @param	reg_addr	register address to write
 * @param	value		value to be written
 * @return	0 on success, non-0 on failure
 */
static int tps65090_i2c_write(unsigned int reg_addr, unsigned char value)
{
	int ret;

	ret = i2c_write(config.addr, reg_addr, 1, &value, 1);
	debug("%s: reg=%#x, value=%#x, ret=%d\n", __func__, reg_addr, value,
	      ret);
	return ret;
}

/**
 * Read a value from a register
 *
 * @param	chip_addr	i2c addr for max77686
 * @param	reg_addr	register address to read
 * @param	value		address to store the value to be read
 * @return	0 on success, non-0 on failure
 */
static int tps65090_i2c_read(unsigned int reg_addr, unsigned char *value)
{
	int ret;

	debug("%s: reg=%#x, ", __func__, reg_addr);
	ret = i2c_read(config.addr, reg_addr, 1, value, 1);
	if (ret)
		debug("fail, ret=%d\n", ret);
	else
		debug("value=%#x, ret=%d\n", *value, ret);
	return ret;
}

static int tps65090_select(void)
{
	int ret;

	config.old_bus = i2c_get_bus_num();
	if (config.old_bus != config.bus) {
		debug("%s: Select bus %d\n", __func__, config.bus);
		ret = i2c_set_bus_num(config.bus);
		if (ret) {
			debug("%s: Cannot select TPS65090, err %d\n",
			      __func__, ret);
			return -1;
		}
	}

	return 0;
}

static int tps65090_deselect(void)
{
	int ret;

	if (config.old_bus != i2c_get_bus_num()) {
		ret = i2c_set_bus_num(config.old_bus);
		debug("%s: Select bus %d\n", __func__, config.old_bus);
		if (ret) {
			debug("%s: Cannot restore i2c bus, err %d\n",
			      __func__, ret);
			return -1;
		}
	}
	config.old_bus = -1;
	return 0;
}

/**
 * Checks for a valid FET number
 *
 * @param fet_id	FET number to check
 * @return 0 if ok, -1 if FET value is out of range
 */
static int tps65090_check_fet(unsigned int fet_id)
{
	if (fet_id == 0 || fet_id > MAX_FET_NUM) {
		debug("parameter fet_id is out of range, %u not in 1 ~ %u\n",
				fet_id, MAX_FET_NUM);
		return -1;
	}

	return 0;
}

int tps65090_fet_enable(unsigned int fet_id)
{
	unsigned char reg;
	int ret, i;

	if (tps65090_check_fet(fet_id))
		return -1;
	if (tps65090_select())
		return -1;

	ret = tps65090_i2c_write(REG_FET1_CTRL + fet_id - 1,
			FET_CTRL_WAIT | FET_CTRL_ADENFET | FET_CTRL_ENFET);
	if (ret)
		goto out;

	for (i = 0; i < MAX_CTRL_READ_TRIES; i++) {
		ret = tps65090_i2c_read(REG_FET1_CTRL + fet_id - 1, &reg);
		if (ret)
			goto out;

		if (!(reg & FET_CTRL_TOFET))
			break;

		mdelay(1);
	}
	if (!(reg & FET_CTRL_PGFET)) {
		debug("still no power after enable FET%d\n", fet_id);

		/*
		 * Unfortunately, there are some conditions where the power
		 * good bit will be 0, but the fet still comes up. One such
		 * case occurs with the lcd backlight. We'll just return 0 here
		 * and assume that the fet will eventually come up.
		 */
		ret = 0;
	}
out:
	tps65090_deselect();

	return ret;
}

int tps65090_fet_disable(unsigned int fet_id)
{
	unsigned char reg;
	int ret;

	if (tps65090_check_fet(fet_id))
		return -1;
	if (tps65090_select())
		return -1;
	ret = tps65090_i2c_write(REG_FET1_CTRL + fet_id - 1,
			FET_CTRL_ADENFET);
	if (!ret) {
		ret = tps65090_i2c_read(REG_FET1_CTRL + fet_id - 1,
				&reg);
	}
	tps65090_deselect();
	if (ret)
		return ret;

	if (reg & FET_CTRL_PGFET) {
		debug("still power good after disable FET%d\n", fet_id);
		return -2;
	}

	return ret;
}

int tps65090_fet_is_enabled(unsigned int fet_id)
{
	unsigned char reg;
	int ret;

	if (tps65090_check_fet(fet_id))
		return -1;
	if (tps65090_select())
		return -1;
	ret = tps65090_i2c_read(REG_FET1_CTRL + fet_id - 1,
			&reg);
	tps65090_deselect();
	if (ret) {
		debug("fail to read FET%u_CTRL register over I2C", fet_id);
		return -2;
	}

	return reg & FET_CTRL_ENFET;
}

int tps65090_get_charging(void)
{
	unsigned char val;
	int ret;

	if (tps65090_select())
		return -1;
	ret = tps65090_i2c_read(REG_CG_CTRL0, &val);
	tps65090_deselect();
	if (ret)
		return ret;
	return val & CG_CTRL0_ENC_MASK ? 1 : 0;
}

int tps65090_set_charge_enable(int enable)
{
	unsigned char val;
	int ret;

	if (tps65090_select())
		return -1;
	ret = tps65090_i2c_read(REG_CG_CTRL0, &val);
	if (!ret) {
		if (enable)
			val |= CG_CTRL0_ENC_MASK;
		else
			val &= ~CG_CTRL0_ENC_MASK;
		ret = tps65090_i2c_write(REG_CG_CTRL0, val);
	}
	tps65090_deselect();
	if (ret) {
		debug("%s: Failed to read/write register\n", __func__);
		return ret;
	}
	return 0;
}

int tps65090_get_status(void)
{
	unsigned char val;
	int ret;

	if (tps65090_select())
		return -1;
	ret = tps65090_i2c_read(REG_CG_STATUS1, &val);
	tps65090_deselect();
	if (ret)
		return ret;
	return val;
}

int tps65090_init(void)
{
	int ret;

	/* TODO(sjg): Move to fdt */
	config.old_bus = -1;
	config.bus = CONFIG_TPS65090_I2C_BUS;
	config.addr = TPS65090_I2C_ADDR;

	if (tps65090_select())
		return -1;

	/* Probe the chip */
	ret = i2c_probe(config.addr);
	if (ret)
		debug("%s: failed to probe TPS65090 over I2C, returned %d\n",
		      __func__, ret);

	return ret;
}
