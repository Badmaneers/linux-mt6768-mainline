// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 MediaTek Inc.
 * Copyright (c) 2026 Badmaneers <dukebraham24@gmail.com>
 *
 * MediaTek MT6768 SCPSYS / MTCMOS power-gate clocks driver.
 * Provides clock handles for MTCMOS domains, enabling power-gate control
 * for GPU MFG (async, mfg, core0, core1) and other hardware subsystems.
 */

#include <linux/clk-provider.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <linux/mfd/syscon.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <linux/slab.h>
#include <dt-bindings/clock/mediatek,mt6768-clock.h>

/* SPM registers (relative to SPM base 0x10006000) */
#define SPM_PWR_STATUS			0x0180
#define SPM_PWR_STATUS_2ND		0x0184
#define SPM_MFG_CORE1_PWR_CON		0x0310
#define SPM_MFG_ASYNC_PWR_CON		0x0334
#define SPM_MFG_PWR_CON			0x0338
#define SPM_MFG_CORE0_PWR_CON		0x034c

/* SPM MTCMOS power control bits */
#define PWR_RST_B			BIT(0)
#define PWR_ISO				BIT(1)
#define PWR_ON				BIT(2)
#define PWR_ON_2ND			BIT(3)
#define PWR_CLK_DIS			BIT(4)

/* SRAM control in PWR_CON */
#define SRAM_PDN			BIT(8)
#define SRAM_PDN_ACK			BIT(12)

/* SPM Power Status Masks */
#define MFG_PWR_STA_MASK		BIT(11)
#define MFG_CORE0_PWR_STA_MASK		BIT(12)
#define MFG_CORE1_PWR_STA_MASK		BIT(13)
#define MFG_ASYNC_PWR_STA_MASK		BIT(14)

/* INFRACFG registers (relative to INFRACFG base 0x10001000) */
#define INFRA_TOPAXI_PROTECTEN		0x0220
#define INFRA_TOPAXI_PROTECTEN_STA1	0x0228
#define INFRA_TOPAXI_PROTECTEN_SET	0x02a0
#define INFRA_TOPAXI_PROTECTEN_CLR	0x02a4

/* MFG Bus Protect Masks */
#define MFG_PROT_STEP1_0_MASK		BIT(25)
#define MFG_PROT_STEP2_0_MASK		(BIT(21) | BIT(22))

struct mt6768_scpsys_clk_data {
	struct device *dev;
	struct regmap *spm;
	struct regmap *infra;
};

struct mt6768_mtcmos_clk {
	struct clk_hw hw;
	struct mt6768_scpsys_clk_data *data;
	const char *name;
	u32 id;
	u32 ctl_offs;
	u32 sta_mask;
	bool has_sram;
	bool has_bus_prot;
};

#define to_mtcmos_clk(_hw) container_of(_hw, struct mt6768_mtcmos_clk, hw)

static int mt6768_mtcmos_enable(struct mt6768_mtcmos_clk *mclk)
{
	struct mt6768_scpsys_clk_data *data = mclk->data;
	struct regmap *spm = data->spm;
	struct regmap *infra = data->infra;
	u32 val;
	int ret;

	if (!spm)
		return 0;

	/* 1. Check if domain is already powered */
	regmap_read(spm, SPM_PWR_STATUS, &val);
	if ((val & mclk->sta_mask) == mclk->sta_mask) {
		/* Already powered: ensure clock and reset are deasserted */
		regmap_update_bits(spm, mclk->ctl_offs, PWR_CLK_DIS | PWR_ISO, 0);
		regmap_update_bits(spm, mclk->ctl_offs, PWR_RST_B, PWR_RST_B);
		if (mclk->has_sram) {
			regmap_update_bits(spm, mclk->ctl_offs, SRAM_PDN, 0);
			regmap_read_poll_timeout_atomic(spm, mclk->ctl_offs, val,
							!(val & SRAM_PDN_ACK), 1, 1000);
		}
		if (mclk->has_bus_prot && infra) {
			regmap_write(infra, INFRA_TOPAXI_PROTECTEN_CLR, MFG_PROT_STEP2_0_MASK);
			regmap_write(infra, INFRA_TOPAXI_PROTECTEN_CLR, MFG_PROT_STEP1_0_MASK);
		}
		return 0;
	}

	/* 2. Assert PWR_ON and PWR_ON_2ND */
	regmap_update_bits(spm, mclk->ctl_offs, PWR_ON | PWR_ON_2ND, PWR_ON | PWR_ON_2ND);

	/* 3. Wait until PWR_STATUS and PWR_STATUS_2ND report domain powered */
	ret = regmap_read_poll_timeout_atomic(spm, SPM_PWR_STATUS, val,
					      (val & mclk->sta_mask) == mclk->sta_mask,
					      1, 5000);
	if (ret)
		dev_warn(data->dev, "%s: timeout waiting for PWR_STATUS (0x%08x)\n",
			 mclk->name, val);

	ret = regmap_read_poll_timeout_atomic(spm, SPM_PWR_STATUS_2ND, val,
					      (val & mclk->sta_mask) == mclk->sta_mask,
					      1, 5000);
	if (ret)
		dev_warn(data->dev, "%s: timeout waiting for PWR_STATUS_2ND (0x%08x)\n",
			 mclk->name, val);

	/* 4. Clear PWR_CLK_DIS */
	regmap_update_bits(spm, mclk->ctl_offs, PWR_CLK_DIS, 0);

	/* 5. Clear PWR_ISO */
	regmap_update_bits(spm, mclk->ctl_offs, PWR_ISO, 0);

	/* 6. Set PWR_RST_B */
	regmap_update_bits(spm, mclk->ctl_offs, PWR_RST_B, PWR_RST_B);

	/* 7. Clear SRAM_PDN if present */
	if (mclk->has_sram) {
		regmap_update_bits(spm, mclk->ctl_offs, SRAM_PDN, 0);
		ret = regmap_read_poll_timeout_atomic(spm, mclk->ctl_offs, val,
						      !(val & SRAM_PDN_ACK), 1, 1000);
		if (ret)
			dev_warn(data->dev, "%s: timeout waiting for SRAM_PDN_ACK clear\n",
				 mclk->name);
	}

	/* 8. Release bus protect if present */
	if (mclk->has_bus_prot && infra) {
		regmap_write(infra, INFRA_TOPAXI_PROTECTEN_CLR, MFG_PROT_STEP2_0_MASK);
		regmap_write(infra, INFRA_TOPAXI_PROTECTEN_CLR, MFG_PROT_STEP1_0_MASK);
	}

	dev_dbg(data->dev, "%s: MTCMOS power on complete\n", mclk->name);
	return 0;
}

static void mt6768_mtcmos_disable(struct mt6768_mtcmos_clk *mclk)
{
	struct mt6768_scpsys_clk_data *data = mclk->data;
	struct regmap *spm = data->spm;
	struct regmap *infra = data->infra;
	u32 val;
	int ret;

	if (!spm)
		return;

	/* 1. Assert bus protect if present */
	if (mclk->has_bus_prot && infra) {
		regmap_write(infra, INFRA_TOPAXI_PROTECTEN_SET, MFG_PROT_STEP1_0_MASK);
		ret = regmap_read_poll_timeout_atomic(infra, INFRA_TOPAXI_PROTECTEN_STA1, val,
						      (val & MFG_PROT_STEP1_0_MASK) == MFG_PROT_STEP1_0_MASK,
						      1, 5000);
		if (ret)
			dev_warn(data->dev, "%s: timeout waiting for bus protect step1 (0x%08x)\n",
				 mclk->name, val);

		regmap_write(infra, INFRA_TOPAXI_PROTECTEN_SET, MFG_PROT_STEP2_0_MASK);
		ret = regmap_read_poll_timeout_atomic(infra, INFRA_TOPAXI_PROTECTEN_STA1, val,
						      (val & MFG_PROT_STEP2_0_MASK) == MFG_PROT_STEP2_0_MASK,
						      1, 5000);
		if (ret)
			dev_warn(data->dev, "%s: timeout waiting for bus protect step2 (0x%08x)\n",
				 mclk->name, val);
	}

	/* 2. Assert SRAM_PDN if present */
	if (mclk->has_sram) {
		regmap_update_bits(spm, mclk->ctl_offs, SRAM_PDN, SRAM_PDN);
		ret = regmap_read_poll_timeout_atomic(spm, mclk->ctl_offs, val,
						      (val & SRAM_PDN_ACK) == SRAM_PDN_ACK,
						      1, 1000);
		if (ret)
			dev_warn(data->dev, "%s: timeout waiting for SRAM_PDN_ACK set\n",
				 mclk->name);
	}

	/* 3. Set PWR_ISO */
	regmap_update_bits(spm, mclk->ctl_offs, PWR_ISO, PWR_ISO);

	/* 4. Set PWR_CLK_DIS */
	regmap_update_bits(spm, mclk->ctl_offs, PWR_CLK_DIS, PWR_CLK_DIS);

	/* 5. Clear PWR_RST_B */
	regmap_update_bits(spm, mclk->ctl_offs, PWR_RST_B, 0);

	/* 6. Clear PWR_ON and PWR_ON_2ND */
	regmap_update_bits(spm, mclk->ctl_offs, PWR_ON | PWR_ON_2ND, 0);

	/* 7. Wait until status cleared */
	ret = regmap_read_poll_timeout_atomic(spm, SPM_PWR_STATUS, val,
					      !(val & mclk->sta_mask), 1, 5000);
	if (ret)
		dev_warn(data->dev, "%s: timeout waiting for SPM_PWR_STATUS clear (0x%08x)\n",
			 mclk->name, val);

	ret = regmap_read_poll_timeout_atomic(spm, SPM_PWR_STATUS_2ND, val,
					      !(val & mclk->sta_mask), 1, 5000);
	if (ret)
		dev_warn(data->dev, "%s: timeout waiting for SPM_PWR_STATUS_2ND clear (0x%08x)\n",
			 mclk->name, val);

	dev_dbg(data->dev, "%s: MTCMOS power off complete\n", mclk->name);
}

static int mt6768_mtcmos_clk_prepare(struct clk_hw *hw)
{
	struct mt6768_mtcmos_clk *mclk = to_mtcmos_clk(hw);
	return mt6768_mtcmos_enable(mclk);
}

static void mt6768_mtcmos_clk_unprepare(struct clk_hw *hw)
{
	struct mt6768_mtcmos_clk *mclk = to_mtcmos_clk(hw);
	mt6768_mtcmos_disable(mclk);
}

static int mt6768_mtcmos_is_enabled(struct clk_hw *hw)
{
	struct mt6768_mtcmos_clk *mclk = to_mtcmos_clk(hw);
	struct mt6768_scpsys_clk_data *data = mclk->data;
	u32 val = 0;

	if (!data->spm)
		return 0;

	if (regmap_read(data->spm, SPM_PWR_STATUS, &val))
		return 0;

	return (val & mclk->sta_mask) == mclk->sta_mask;
}

static const struct clk_ops mt6768_mtcmos_clk_ops = {
	.prepare = mt6768_mtcmos_clk_prepare,
	.unprepare = mt6768_mtcmos_clk_unprepare,
	.is_prepared = mt6768_mtcmos_is_enabled,
};

static const struct clk_ops mt6768_dummy_clk_ops = {
};

struct mtcmos_init_data {
	u32 id;
	const char *name;
	u32 ctl_offs;
	u32 sta_mask;
	bool has_sram;
	bool has_bus_prot;
	bool is_active;
};

static const struct mtcmos_init_data mt6768_mtcmos_clks[] = {
	{
		.id = CLK_SCP_SYS_MD1,
		.name = "scpsys_md1",
		.is_active = false,
	},
	{
		.id = CLK_SCP_SYS_CONN,
		.name = "scpsys_conn",
		.is_active = false,
	},
	{
		.id = CLK_SCP_SYS_DPY,
		.name = "scpsys_dpy",
		.is_active = false,
	},
	{
		.id = CLK_SCP_SYS_DIS,
		.name = "scpsys_dis",
		.is_active = false,
	},
	{
		.id = CLK_SCP_SYS_MFG,
		.name = "scpsys_mfg",
		.ctl_offs = SPM_MFG_PWR_CON,
		.sta_mask = MFG_PWR_STA_MASK,
		.has_sram = true,
		.has_bus_prot = true,
		.is_active = true,
	},
	{
		.id = CLK_SCP_SYS_ISP,
		.name = "scpsys_isp",
		.is_active = false,
	},
	{
		.id = CLK_SCP_SYS_IFR,
		.name = "scpsys_ifr",
		.is_active = false,
	},
	{
		.id = CLK_SCP_SYS_MFG_CORE0,
		.name = "scpsys_mfg_core0",
		.ctl_offs = SPM_MFG_CORE0_PWR_CON,
		.sta_mask = MFG_CORE0_PWR_STA_MASK,
		.has_sram = true,
		.has_bus_prot = false,
		.is_active = true,
	},
	{
		.id = CLK_SCP_SYS_MFG_CORE1,
		.name = "scpsys_mfg_core1",
		.ctl_offs = SPM_MFG_CORE1_PWR_CON,
		.sta_mask = MFG_CORE1_PWR_STA_MASK,
		.has_sram = true,
		.has_bus_prot = false,
		.is_active = true,
	},
	{
		.id = CLK_SCP_SYS_MFG_ASYNC,
		.name = "scpsys_mfg_async",
		.ctl_offs = SPM_MFG_ASYNC_PWR_CON,
		.sta_mask = MFG_ASYNC_PWR_STA_MASK,
		.has_sram = false,
		.has_bus_prot = false,
		.is_active = true,
	},
	{
		.id = CLK_SCP_SYS_CAM,
		.name = "scpsys_cam",
		.is_active = false,
	},
	{
		.id = CLK_SCP_SYS_VENC,
		.name = "scpsys_venc",
		.is_active = false,
	},
	{
		.id = CLK_SCP_SYS_VDEC,
		.name = "scpsys_vdec",
		.is_active = false,
	},
};

static int clk_mt6768_scpsys_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *node = dev->of_node;
	struct mt6768_scpsys_clk_data *data;
	struct clk_hw_onecell_data *clk_data;
	struct regmap *spm_regmap = NULL;
	struct regmap *infra_regmap = NULL;
	int i, ret;

	data = devm_kzalloc(dev, sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	data->dev = dev;

	/* 1. Retrieve SPM regmap: parent node (scpsys simple-mfd) or phandle or compatible */
	if (dev->parent && dev->parent->of_node)
		spm_regmap = syscon_node_to_regmap(dev->parent->of_node);
	if (IS_ERR_OR_NULL(spm_regmap))
		spm_regmap = syscon_node_to_regmap(node);
	if (IS_ERR_OR_NULL(spm_regmap))
		spm_regmap = syscon_regmap_lookup_by_compatible("mediatek,mt6768-scpsys");
	if (IS_ERR_OR_NULL(spm_regmap))
		spm_regmap = syscon_regmap_lookup_by_compatible("mediatek,mt6768-power-controller");
	if (IS_ERR_OR_NULL(spm_regmap)) {
		int err = spm_regmap ? PTR_ERR(spm_regmap) : -ENODEV;
		dev_err(dev, "failed to get SPM regmap: %d\n", err);
		return err;
	}
	data->spm = spm_regmap;

	/* 2. Retrieve INFRACFG regmap: phandle or compatible */
	infra_regmap = syscon_regmap_lookup_by_phandle(node, "mediatek,infracfg");
	if (IS_ERR_OR_NULL(infra_regmap))
		infra_regmap = syscon_regmap_lookup_by_compatible("mediatek,mt6768-infracfg-ao");
	if (IS_ERR_OR_NULL(infra_regmap))
		infra_regmap = syscon_regmap_lookup_by_compatible("mediatek,mt6768-infracfg");
	if (IS_ERR(infra_regmap)) {
		dev_warn(dev, "could not get INFRACFG regmap, bus protect will be skipped\n");
		infra_regmap = NULL;
	}
	data->infra = infra_regmap;

	/* Unconditionally clear MFG INFRACFG bus protection */
	if (data->infra) {
		regmap_write(data->infra, INFRA_TOPAXI_PROTECTEN_CLR, MFG_PROT_STEP2_0_MASK);
		regmap_write(data->infra, INFRA_TOPAXI_PROTECTEN_CLR, MFG_PROT_STEP1_0_MASK);
	}

	/* 3. Allocate clock onecell data */
	clk_data = devm_kzalloc(dev, struct_size(clk_data, hws, CLK_SCP_SYS_NR_CLK), GFP_KERNEL);
	if (!clk_data)
		return -ENOMEM;

	clk_data->num = CLK_SCP_SYS_NR_CLK;

	for (i = 0; i < CLK_SCP_SYS_NR_CLK; i++)
		clk_data->hws[i] = ERR_PTR(-ENOENT);

	/* 4. Register MTCMOS clocks */
	for (i = 0; i < ARRAY_SIZE(mt6768_mtcmos_clks); i++) {
		const struct mtcmos_init_data *init_d = &mt6768_mtcmos_clks[i];
		struct mt6768_mtcmos_clk *mclk;
		struct clk_init_data init = {};

		mclk = devm_kzalloc(dev, sizeof(*mclk), GFP_KERNEL);
		if (!mclk)
			return -ENOMEM;

		mclk->data = data;
		mclk->name = init_d->name;
		mclk->id = init_d->id;
		mclk->ctl_offs = init_d->ctl_offs;
		mclk->sta_mask = init_d->sta_mask;
		mclk->has_sram = init_d->has_sram;
		mclk->has_bus_prot = init_d->has_bus_prot;

		init.name = init_d->name;
		init.flags = CLK_IGNORE_UNUSED;
		if (init_d->is_active)
			init.ops = &mt6768_mtcmos_clk_ops;
		else
			init.ops = &mt6768_dummy_clk_ops;

		mclk->hw.init = &init;

		ret = devm_clk_hw_register(dev, &mclk->hw);
		if (ret) {
			dev_err(dev, "failed to register clk %s: %d\n", init_d->name, ret);
			return ret;
		}

		clk_data->hws[init_d->id] = &mclk->hw;
	}

	ret = devm_of_clk_add_hw_provider(dev, of_clk_hw_onecell_get, clk_data);
	if (ret) {
		dev_err(dev, "failed to add clock provider: %d\n", ret);
		return ret;
	}

	dev_info(dev, "MediaTek MT6768 SCPSYS MTCMOS clock provider registered\n");
	return 0;
}

static const struct of_device_id of_match_clk_mt6768_scpsys[] = {
	{ .compatible = "mediatek,mt6768-scpsys-clk" },
	{ .compatible = "mediatek,scpsys-clk" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, of_match_clk_mt6768_scpsys);

static struct platform_driver clk_mt6768_scpsys_drv = {
	.probe = clk_mt6768_scpsys_probe,
	.driver = {
		.name = "clk-mt6768-scpsys",
		.of_match_table = of_match_clk_mt6768_scpsys,
	},
};
module_platform_driver(clk_mt6768_scpsys_drv);

MODULE_DESCRIPTION("MediaTek MT6768 SCPSYS / MTCMOS power-gate clocks driver");
MODULE_LICENSE("GPL");
