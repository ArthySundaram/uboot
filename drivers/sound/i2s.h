/*
 * Copyright (C) 2012 Samsung Electronics
 * R. Chandrasekar <rcsekar@samsung.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __UBOOT_SAMSUNG_I2S_H
#define __UBOOT_SAMSUNG_I2S_H

/*
 * DAI hardware audio formats.
 *
 * Describes the physical PCM data formating and clocking. Add new formats
 * to the end.
 */
#define SND_SOC_DAIFMT_I2S		1 /* I2S mode */
#define SND_SOC_DAIFMT_RIGHT_J		2 /* Right Justified mode */
#define SND_SOC_DAIFMT_LEFT_J		3 /* Left Justified mode */
#define SND_SOC_DAIFMT_DSP_A		4 /* L data MSB after FRM LRC */
#define SND_SOC_DAIFMT_DSP_B		5 /* L data MSB during FRM LRC */
#define SND_SOC_DAIFMT_AC97		6 /* AC97 */
#define SND_SOC_DAIFMT_PDM		7 /* Pulse density modulation */

/* left and right justified also known as MSB and LSB respectively */
#define SND_SOC_DAIFMT_MSB		SND_SOC_DAIFMT_LEFT_J
#define SND_SOC_DAIFMT_LSB		SND_SOC_DAIFMT_RIGHT_J

/*
 * DAI hardware signal inversions.
 *
 * Specifies whether the DAI can also support inverted clocks for the specified
 * format.
 */
#define SND_SOC_DAIFMT_NB_NF	(1 << 8) /* normal bit clock + frame */
#define SND_SOC_DAIFMT_NB_IF	(2 << 8) /* normal BCLK + inv FRM */
#define SND_SOC_DAIFMT_IB_NF	(3 << 8) /* invert BCLK + nor FRM */
#define SND_SOC_DAIFMT_IB_IF	(4 << 8) /* invert BCLK + FRM */

/*
 * DAI hardware clock masters.
 *
 * This is wrt the codec, the inverse is true for the interface
 * i.e. if the codec is clk and FRM master then the interface is
 * clk and frame slave.
 */
#define SND_SOC_DAIFMT_CBM_CFM	(1 << 12) /* codec clk & FRM master */
#define SND_SOC_DAIFMT_CBS_CFM	(2 << 12) /* codec clk slave & FRM master */
#define SND_SOC_DAIFMT_CBM_CFS	(3 << 12) /* codec clk master & frame slave */
#define SND_SOC_DAIFMT_CBS_CFS	(4 << 12) /* codec clk & FRM slave */

#define SND_SOC_DAIFMT_FORMAT_MASK	0x000f
#define SND_SOC_DAIFMT_CLOCK_MASK	0x00f0
#define SND_SOC_DAIFMT_INV_MASK		0x0f00
#define SND_SOC_DAIFMT_MASTER_MASK	0xf000

/*
 * Master Clock Directions
 */
#define SND_SOC_CLOCK_IN		0
#define SND_SOC_CLOCK_OUT		1

/* I2S Tx Control */
#define I2S_TX_ON	1
#define I2S_TX_OFF	0

/* I2s Registers */
struct exynos5_i2s {
	unsigned int	con;		/* base + 0 , Control register */
	unsigned int	mod;		/* Mode register */
	unsigned int	fic;		/* FIFO control register */
	unsigned int	psr;		/* Reserved */
	unsigned int	txd;		/* Transmit data register */
	unsigned int	rxd;		/* Receive Data Register */
};

/* This structure stores the i2s related information */
struct i2stx_info {
	unsigned int rfs;		/* LR clock frame size */
	unsigned int bfs;		/* Bit slock frame size */
	unsigned int audio_pll_clk;	/* Audio pll frequency in Hz */
	unsigned int samplingrate;	/* sampling rate */
	unsigned int bitspersample;	/* bits per sample */
	unsigned int channels;		/* audio channels */
	unsigned int base_address;	/* I2S Register Base */
};

/*
 * Sends the given data through i2s tx
 *
 * @param pi2s_tx	pointer of i2s transmitter parameter structure.
 * @param *data		address of the data buffer
 * @param data_size	array size of the int buffer (total size / size of int)
 *
 * @return		int value 0 for success, -1 in case of error
 */
int i2s_transfer_tx_data(struct i2stx_info *pi2s_tx, unsigned *data,
				unsigned long data_size);

/*
 * Initialise i2s transmiter
 *
 * @param pi2s_tx	pointer of i2s transmitter parameter structure.
 *
 * @return		int value 0 for success, -1 in case of error
 */
int i2s_tx_init(struct i2stx_info *pi2s_tx);

#endif /* __I2S_H__ */
