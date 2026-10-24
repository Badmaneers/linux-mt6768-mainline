// SPDX-License-Identifier: GPL-2.0-only
/*
 * Ilitek ILI9882N and ILI7807S panel drivers for Realme even family
 * (Realme C25, C25s, Narzo 50A / MT6768 platform)
 */

#include <linux/backlight.h>
#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/regulator/consumer.h>

#include <video/mipi_display.h>

#include <drm/drm_connector.h>
#include <drm/drm_crtc.h>
#include <drm/drm_mipi_dsi.h>
#include <drm/drm_modes.h>
#include <drm/drm_panel.h>

struct ilitek_even_desc {
	const struct drm_display_mode *modes;
	unsigned int num_modes;
	unsigned int width_mm;
	unsigned int height_mm;
	unsigned int bpc;
	unsigned int lanes;
	enum mipi_dsi_pixel_format format;
	unsigned long mode_flags;
	int (*init)(struct mipi_dsi_multi_context *ctx);
};

struct ilitek_even_panel {
	struct drm_panel panel;
	struct mipi_dsi_device *dsi;
	const struct ilitek_even_desc *desc;
	struct backlight_device *backlight;
	struct regulator *vddio;
	struct regulator *vsp;
	struct regulator *vsn;
	struct gpio_desc *reset_gpio;
};

static int truly_ili9882n_init(struct mipi_dsi_multi_context *ctx)
{
	mipi_dsi_dcs_write_seq_multi(ctx, 0xff, 0x98, 0x82, 0x01);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x00, 0x47);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x01, 0x32);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x02, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x03, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x04, 0x04);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x05, 0x32);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x06, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x07, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x08, 0x85);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x09, 0x04);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x0a, 0x72);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x0b, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x0c, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x0d, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x0e, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x0f, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x28, 0x48);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x29, 0x88);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x2a, 0x48);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x2b, 0x88);

	mipi_dsi_dcs_write_seq_multi(ctx, 0x31, 0x0c);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x32, 0x02);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x33, 0x02);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x34, 0x23);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x35, 0x02);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x36, 0x08);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x37, 0x0a);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x38, 0x06);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x39, 0x06);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x3a, 0x10);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x3b, 0x10);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x3c, 0x12);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x3d, 0x12);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x3e, 0x14);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x3f, 0x14);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x40, 0x16);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x41, 0x16);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x42, 0x07);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x43, 0x07);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x44, 0x07);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x45, 0x07);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x46, 0x07);

	mipi_dsi_dcs_write_seq_multi(ctx, 0x47, 0x0d);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x48, 0x02);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x49, 0x02);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x4a, 0x23);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x4b, 0x02);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x4c, 0x09);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x4d, 0x0b);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x4e, 0x06);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x4f, 0x06);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x50, 0x11);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x51, 0x11);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x52, 0x13);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x53, 0x13);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x54, 0x15);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x55, 0x15);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x56, 0x17);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x57, 0x17);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x58, 0x07);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x59, 0x07);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x5a, 0x07);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x5b, 0x07);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x5c, 0x07);

	mipi_dsi_dcs_write_seq_multi(ctx, 0x61, 0x0c);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x62, 0x02);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x63, 0x02);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x64, 0x23);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x65, 0x02);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x66, 0x08);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x67, 0x0a);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x68, 0x06);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x69, 0x06);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x6a, 0x10);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x6b, 0x10);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x6c, 0x12);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x6d, 0x12);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x6e, 0x14);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x6f, 0x14);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x70, 0x16);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x71, 0x16);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x72, 0x07);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x73, 0x07);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x74, 0x07);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x75, 0x07);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x76, 0x07);

	mipi_dsi_dcs_write_seq_multi(ctx, 0x77, 0x0d);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x78, 0x02);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x79, 0x02);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x7a, 0x23);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x7b, 0x02);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x7c, 0x09);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x7d, 0x0b);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x7e, 0x06);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x7f, 0x06);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x80, 0x11);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x81, 0x11);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x82, 0x13);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x83, 0x13);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x84, 0x15);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x85, 0x15);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x86, 0x17);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x87, 0x17);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x88, 0x07);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x89, 0x07);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x8a, 0x07);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x8b, 0x07);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x8c, 0x07);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xb0, 0x33);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xb1, 0x33);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xb2, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xd0, 0x01);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xd1, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xe2, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xe6, 0x22);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xe7, 0x54);

	mipi_dsi_dcs_write_seq_multi(ctx, 0xff, 0x98, 0x82, 0x02);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xf1, 0x1c);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x4b, 0x5a);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x50, 0xca);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x51, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x06, 0x8f);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x0b, 0xa0);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x0c, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x0d, 0x14);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x0e, 0xe6);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x4e, 0x11);

	mipi_dsi_dcs_write_seq_multi(ctx, 0xff, 0x98, 0x82, 0x05);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x03, 0x01);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x04, 0x2c);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x58, 0x61);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x63, 0x8d);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x64, 0x8d);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x68, 0xa1);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x69, 0xa7);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x6a, 0x79);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x6b, 0x6b);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x85, 0x37);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x46, 0x00);

	mipi_dsi_dcs_write_seq_multi(ctx, 0xff, 0x98, 0x82, 0x06);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xd9, 0x10);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xc0, 0x40);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xc1, 0x16);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x06, 0xa4);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xd6, 0x55);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xdd, 0x61);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xdc, 0x68);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x80, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x08, 0x00);

	mipi_dsi_dcs_write_seq_multi(ctx, 0xff, 0x98, 0x82, 0x08);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xe0, 0x00, 0x24, 0x76, 0xab, 0xeb, 0x55, 0x20, 0x49,
				     0x79, 0xa0, 0xa9, 0xde, 0x11, 0x3d, 0x68, 0xea, 0x94, 0xc6,
				     0xe6, 0x0d, 0xff, 0x2c, 0x55, 0x85, 0xb5, 0x03, 0xec);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xe1, 0x00, 0x24, 0x76, 0xab, 0xeb, 0x55, 0x20, 0x49,
				     0x79, 0xa0, 0xa9, 0xde, 0x11, 0x3d, 0x68, 0xea, 0x94, 0xc6,
				     0xe6, 0x0d, 0xff, 0x2c, 0x55, 0x85, 0xb5, 0x03, 0xec);

	mipi_dsi_dcs_write_seq_multi(ctx, 0xff, 0x98, 0x82, 0x0b);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x9a, 0x44);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x9b, 0x81);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x9c, 0x03);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x9d, 0x03);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x9e, 0x70);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x9f, 0x70);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xab, 0xe0);

	mipi_dsi_dcs_write_seq_multi(ctx, 0xff, 0x98, 0x82, 0x0e);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x11, 0x10);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x13, 0x10);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x00, 0xa0);

	mipi_dsi_dcs_write_seq_multi(ctx, 0xff, 0x98, 0x82, 0x03);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x81, 0x14);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x82, 0x15);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x83, 0x30);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x84, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x88, 0xcc);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x89, 0xe5);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x8a, 0xed);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x8b, 0xef);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xb3, 0xe5);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xac, 0xfa);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xad, 0xe0);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x8c, 0x9b);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x8d, 0xa9);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x8e, 0xad);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x8f, 0xaf);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x90, 0xb2);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x91, 0xbc);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x92, 0xbd);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x93, 0xba);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x94, 0xca);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x95, 0xd5);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xb4, 0xd9);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xae, 0xd3);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x96, 0x70);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x97, 0x89);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x98, 0x99);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x99, 0x9f);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x9a, 0xa3);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x9b, 0xa6);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x9c, 0xa9);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x9d, 0xad);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x9e, 0xc2);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x9f, 0xcc);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xb5, 0xd0);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xa0, 0xc0);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xa1, 0x7e);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xa2, 0x7e);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xa7, 0xe5);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xa8, 0xe5);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xb6, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xb7, 0xd0);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xb8, 0xd0);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xb1, 0x66);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xb2, 0x66);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x86, 0x6c);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x87, 0x6c);

	mipi_dsi_dcs_write_seq_multi(ctx, 0xff, 0x98, 0x82, 0x03);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xac, 0xfa);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xff, 0x98, 0x82, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x55, 0x01);

	mipi_dsi_dcs_write_seq_multi(ctx, 0xff, 0x98, 0x82, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x51, 0x00, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x53, 0x24);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x5e, 0x00, 0x50);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x35, 0x00); /* TE */

	mipi_dsi_dcs_exit_sleep_mode_multi(ctx);
	mipi_dsi_msleep(ctx, 120);

	mipi_dsi_dcs_set_display_on_multi(ctx);
	mipi_dsi_msleep(ctx, 20);

	return ctx->accum_err;
}

static int hlt_ili7807s_init(struct mipi_dsi_multi_context *ctx)
{
	mipi_dsi_dcs_write_seq_multi(ctx, 0xff, 0x78, 0x07, 0x03);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x83, 0x20);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x84, 0x01);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x86, 0x3f);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x87, 0x3f);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xaf, 0x18);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x88, 0xe3);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x89, 0xea);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x8a, 0xeb);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x8b, 0xf6);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xb3, 0xf9);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xac, 0xff);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xad, 0xff);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x8c, 0xc0);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x8d, 0xc2);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x8e, 0xc6);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x8f, 0xc8);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x90, 0xd1);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x91, 0xd6);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x92, 0xd8);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x93, 0xd1);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x94, 0xe8);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x95, 0xf6);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xb4, 0xe8);

	mipi_dsi_dcs_write_seq_multi(ctx, 0xff, 0x78, 0x07, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x51, 0x00, 0x00);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x53, 0x2c);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x55, 0x01);

	mipi_dsi_dcs_exit_sleep_mode_multi(ctx);
	mipi_dsi_msleep(ctx, 120);

	mipi_dsi_dcs_write_seq_multi(ctx, 0xff, 0x78, 0x07, 0x0e);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xb0, 0x01);

	mipi_dsi_dcs_write_seq_multi(ctx, 0xff, 0x78, 0x07, 0x06);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xdd, 0x17);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x08, 0x20);

	mipi_dsi_dcs_write_seq_multi(ctx, 0xff, 0x78, 0x07, 0x05);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x7b, 0x88);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x7c, 0x79);

	mipi_dsi_dcs_write_seq_multi(ctx, 0xff, 0x78, 0x07, 0x08);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xe0, 0x00, 0x00, 0x1b, 0x45, 0x00, 0x84, 0xb1, 0xd5,
				     0x15, 0x0d, 0x38, 0x7a, 0x25, 0xaa, 0xf3, 0x2f, 0x2a,
				     0x68, 0xa8, 0xd1, 0x3f, 0x04, 0x28, 0x52, 0x3f, 0x6f,
				     0x91, 0xbd, 0x0f, 0xd8, 0xd8);
	mipi_dsi_dcs_write_seq_multi(ctx, 0xe1, 0x00, 0x00, 0x1b, 0x45, 0x00, 0x84, 0xb1, 0xd5,
				     0x15, 0x0d, 0x38, 0x7a, 0x25, 0xaa, 0xf3, 0x2f, 0x2a,
				     0x68, 0xa8, 0xd1, 0x3f, 0x04, 0x28, 0x52, 0x3f, 0x6f,
				     0x91, 0xbd, 0x0f, 0xd8, 0xd8);

	mipi_dsi_dcs_write_seq_multi(ctx, 0xff, 0x78, 0x07, 0x00);
	mipi_dsi_dcs_set_display_on_multi(ctx);
	mipi_dsi_msleep(ctx, 20);
	mipi_dsi_dcs_write_seq_multi(ctx, 0x35, 0x00); /* TE */

	return ctx->accum_err;
}

static const struct drm_display_mode truly_ili9882n_modes[] = {
	{
		.clock = 87066,
		.hdisplay = 720,
		.hsync_start = 747,
		.hsync_end = 755,
		.htotal = 781,
		.vdisplay = 1600,
		.vsync_start = 1840,
		.vsync_end = 1842,
		.vtotal = 1858,
		.width_mm = 68,
		.height_mm = 151,
		.type = DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED,
	},
};

static const struct ilitek_even_desc truly_ili9882n_desc = {
	.modes = truly_ili9882n_modes,
	.num_modes = ARRAY_SIZE(truly_ili9882n_modes),
	.width_mm = 68,
	.height_mm = 151,
	.bpc = 8,
	.lanes = 3,
	.format = MIPI_DSI_FMT_RGB888,
	.mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_SYNC_PULSE | MIPI_DSI_MODE_LPM,
	.init = truly_ili9882n_init,
};

static const struct drm_display_mode hlt_ili7807s_modes[] = {
	{
		.clock = 86724,
		.hdisplay = 720,
		.hsync_start = 800,
		.hsync_end = 812,
		.htotal = 876,
		.vdisplay = 1600,
		.vsync_start = 1632,
		.vsync_end = 1634,
		.vtotal = 1650,
		.width_mm = 68,
		.height_mm = 151,
		.type = DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED,
	},
};

static const struct ilitek_even_desc hlt_ili7807s_desc = {
	.modes = hlt_ili7807s_modes,
	.num_modes = ARRAY_SIZE(hlt_ili7807s_modes),
	.width_mm = 68,
	.height_mm = 151,
	.bpc = 8,
	.lanes = 3,
	.format = MIPI_DSI_FMT_RGB888,
	.mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_SYNC_PULSE | MIPI_DSI_MODE_LPM,
	.init = hlt_ili7807s_init,
};

static int ilitek_even_bl_update_status(struct backlight_device *bl)
{
	struct mipi_dsi_device *dsi = bl_get_data(bl);
	u16 brightness = backlight_get_brightness(bl);
	int ret;

	dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;
	ret = mipi_dsi_dcs_set_display_brightness_large(dsi, brightness);
	dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	return ret;
}

static int ilitek_even_bl_get_brightness(struct backlight_device *bl)
{
	struct mipi_dsi_device *dsi = bl_get_data(bl);
	u16 brightness;
	int ret;

	dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;
	ret = mipi_dsi_dcs_get_display_brightness_large(dsi, &brightness);
	dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	if (ret < 0)
		return ret;

	return brightness;
}

static const struct backlight_ops ilitek_even_bl_ops = {
	.update_status = ilitek_even_bl_update_status,
	.get_brightness = ilitek_even_bl_get_brightness,
};

static struct backlight_device *ilitek_even_create_backlight(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	const struct backlight_properties props = {
		.type = BACKLIGHT_RAW,
		.brightness = 512,
		.max_brightness = 4095,
		.scale = BACKLIGHT_SCALE_NON_LINEAR,
	};

	return devm_backlight_device_register(dev, dev_name(dev), dev, dsi,
					      &ilitek_even_bl_ops, &props);
}

static int ilitek_even_disable(struct drm_panel *panel)
{
	struct ilitek_even_panel *p = container_of(panel, struct ilitek_even_panel, panel);
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = p->dsi };

	mipi_dsi_dcs_set_display_off_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 20);
	mipi_dsi_dcs_enter_sleep_mode_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 120);

	return dsi_ctx.accum_err;
}

static int ilitek_even_unprepare(struct drm_panel *panel)
{
	struct ilitek_even_panel *p = container_of(panel, struct ilitek_even_panel, panel);

	if (p->reset_gpio)
		gpiod_set_value_cansleep(p->reset_gpio, 1);

	if (p->vsn) {
		regulator_disable(p->vsn);
		msleep(5);
	}

	if (p->vsp)
		regulator_disable(p->vsp);

	if (p->vddio)
		regulator_disable(p->vddio);

	return 0;
}

static int ilitek_even_prepare(struct drm_panel *panel)
{
	struct ilitek_even_panel *p = container_of(panel, struct ilitek_even_panel, panel);
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = p->dsi };
	int ret;

	if (p->vddio) {
		ret = regulator_enable(p->vddio);
		if (ret) {
			dev_err(panel->dev, "failed to enable vddio regulator: %d\n", ret);
			return ret;
		}
	}

	if (p->vsp) {
		ret = regulator_enable(p->vsp);
		if (ret) {
			dev_err(panel->dev, "failed to enable vsp regulator: %d\n", ret);
			goto err_disable_vddio;
		}
		msleep(5);
	}

	if (p->vsn) {
		ret = regulator_enable(p->vsn);
		if (ret) {
			dev_err(panel->dev, "failed to enable vsn regulator: %d\n", ret);
			goto err_disable_vsp;
		}
		msleep(10);
	}

	if (p->reset_gpio) {
		gpiod_set_value_cansleep(p->reset_gpio, 1);
		msleep(10);
		gpiod_set_value_cansleep(p->reset_gpio, 0);
		msleep(10);
	}

	ret = p->desc->init(&dsi_ctx);
	if (ret < 0) {
		dev_err(panel->dev, "failed to initialize panel: %d\n", ret);
		goto err_disable_vsn;
	}

	return 0;

err_disable_vsn:
	if (p->vsn)
		regulator_disable(p->vsn);
err_disable_vsp:
	if (p->vsp)
		regulator_disable(p->vsp);
err_disable_vddio:
	if (p->vddio)
		regulator_disable(p->vddio);
	return ret;
}

static int ilitek_even_get_modes(struct drm_panel *panel,
				 struct drm_connector *connector)
{
	struct ilitek_even_panel *p = container_of(panel, struct ilitek_even_panel, panel);
	int i;

	for (i = 0; i < p->desc->num_modes; i++) {
		const struct drm_display_mode *m = &p->desc->modes[i];
		struct drm_display_mode *mode;

		mode = drm_mode_duplicate(connector->dev, m);
		if (!mode) {
			dev_err(panel->dev, "failed to add mode %ux%u@%u\n",
				m->hdisplay, m->vdisplay, drm_mode_vrefresh(m));
			return -ENOMEM;
		}

		mode->type = DRM_MODE_TYPE_DRIVER;
		if (i == 0)
			mode->type |= DRM_MODE_TYPE_PREFERRED;

		drm_mode_set_name(mode);
		drm_mode_probed_add(connector, mode);
	}

	connector->display_info.width_mm = p->desc->width_mm;
	connector->display_info.height_mm = p->desc->height_mm;
	connector->display_info.bpc = p->desc->bpc;

	return p->desc->num_modes;
}

static const struct drm_panel_funcs ilitek_even_panel_funcs = {
	.disable = ilitek_even_disable,
	.prepare = ilitek_even_prepare,
	.unprepare = ilitek_even_unprepare,
	.get_modes = ilitek_even_get_modes,
};

static int ilitek_even_probe(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct ilitek_even_panel *p;
	int ret;

	p = devm_drm_panel_alloc(dev, struct ilitek_even_panel, panel,
				 &ilitek_even_panel_funcs,
				 DRM_MODE_CONNECTOR_DSI);
	if (IS_ERR(p))
		return PTR_ERR(p);

	p->dsi = dsi;
	mipi_dsi_set_drvdata(dsi, p);

	p->desc = of_device_get_match_data(dev);
	if (!p->desc)
		return -ENODEV;

	p->vddio = devm_regulator_get_optional(dev, "vddio");
	if (IS_ERR(p->vddio)) {
		if (PTR_ERR(p->vddio) != -ENODEV)
			return dev_err_probe(dev, PTR_ERR(p->vddio), "failed to get vddio regulator\n");
		p->vddio = NULL;
	}

	p->vsp = devm_regulator_get_optional(dev, "vsp");
	if (IS_ERR(p->vsp)) {
		if (PTR_ERR(p->vsp) != -ENODEV)
			return dev_err_probe(dev, PTR_ERR(p->vsp), "failed to get vsp regulator\n");
		p->vsp = NULL;
	}

	p->vsn = devm_regulator_get_optional(dev, "vsn");
	if (IS_ERR(p->vsn)) {
		if (PTR_ERR(p->vsn) != -ENODEV)
			return dev_err_probe(dev, PTR_ERR(p->vsn), "failed to get vsn regulator\n");
		p->vsn = NULL;
	}

	p->reset_gpio = devm_gpiod_get_optional(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(p->reset_gpio))
		return dev_err_probe(dev, PTR_ERR(p->reset_gpio), "failed to get reset gpio\n");

	p->panel.prepare_prev_first = true;
	p->panel.backlight = ilitek_even_create_backlight(dsi);
	if (IS_ERR(p->panel.backlight))
		return dev_err_probe(dev, PTR_ERR(p->panel.backlight), "failed to create backlight\n");

	drm_panel_add(&p->panel);

	dsi->lanes = p->desc->lanes;
	dsi->format = p->desc->format;
	dsi->mode_flags = p->desc->mode_flags;

	ret = devm_mipi_dsi_attach(dev, dsi);
	if (ret < 0)
		return dev_err_probe(dev, ret, "failed to attach to DSI host\n");

	return 0;
}

static void ilitek_even_remove(struct mipi_dsi_device *dsi)
{
	struct ilitek_even_panel *p = mipi_dsi_get_drvdata(dsi);

	drm_panel_remove(&p->panel);
}

static const struct of_device_id ilitek_even_of_match[] = {
	{
		.compatible = "realme,even-truly-ili9882n",
		.data = &truly_ili9882n_desc,
	},
	{
		.compatible = "realme,even-hlt-ili7807s",
		.data = &hlt_ili7807s_desc,
	},
	{},
};
MODULE_DEVICE_TABLE(of, ilitek_even_of_match);

static struct mipi_dsi_driver ilitek_even_driver = {
	.probe = ilitek_even_probe,
	.remove = ilitek_even_remove,
	.driver = {
		.name = "panel-ilitek-even",
		.of_match_table = ilitek_even_of_match,
	},
};
module_mipi_dsi_driver(ilitek_even_driver);

MODULE_AUTHOR("Antigravity Port Team");
MODULE_DESCRIPTION("Ilitek ILI9882N/ILI7807S panel driver for Realme even");
MODULE_LICENSE("GPL");
