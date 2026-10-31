// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2018-2019 MediaTek Inc.
 * Copyright (c) 2026 Badmaneers <dukebraham24@gmail.com>
 *
 * MediaTek MT6768 GPU MFGCFG clock controller driver.
 * Provides clock gates for MFG (mfgcfg_bg3d), clock provider for syscon@13000000.
 */

#include <linux/clk-provider.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <dt-bindings/clock/mediatek,mt6768-clock.h>

#include "clk-mtk.h"
#include "clk-gate.h"

static const struct mtk_gate_regs mfgcfg_cg_regs = {
	.set_ofs = 0x4,
	.clr_ofs = 0x8,
	.sta_ofs = 0x0,
};

#define GATE_MFGCFG(_id, _name, _parent, _shift)			\
	GATE_MTK_FLAGS(_id, _name, _parent, &mfgcfg_cg_regs, _shift,	\
		       &mtk_clk_gate_ops_setclr, CLK_SET_RATE_PARENT)

static const struct mtk_gate mfgcfg_clks[] = {
	GATE_MFGCFG(CLK_MFGCFG_BG3D, "mfgcfg_bg3d", "mfg_sel", 0),
};

static const struct mtk_clk_desc mfgcfg_desc = {
	.clks = mfgcfg_clks,
	.num_clks = ARRAY_SIZE(mfgcfg_clks),
	.need_runtime_pm = true,
};

static const struct of_device_id of_match_clk_mt6768_mfgcfg[] = {
	{
		.compatible = "mediatek,mt6768-mfgcfg",
		.data = &mfgcfg_desc,
	}, {
		.compatible = "mediatek,mfgcfg",
		.data = &mfgcfg_desc,
	}, {
		/* sentinel */
	}
};
MODULE_DEVICE_TABLE(of, of_match_clk_mt6768_mfgcfg);

static struct platform_driver clk_mt6768_mfgcfg_drv = {
	.probe = mtk_clk_simple_probe,
	.remove = mtk_clk_simple_remove,
	.driver = {
		.name = "clk-mt6768-mfgcfg",
		.of_match_table = of_match_clk_mt6768_mfgcfg,
	},
};
module_platform_driver(clk_mt6768_mfgcfg_drv);

MODULE_DESCRIPTION("MediaTek MT6768 GPU mfgcfg clock driver");
MODULE_LICENSE("GPL");
