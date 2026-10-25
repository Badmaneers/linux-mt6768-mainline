/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2015 MediaTek Inc.
 */

#ifndef MTK_CRTC_H
#define MTK_CRTC_H

#include <drm/drm_crtc.h>
#include "mtk_ddp_comp.h"
#include "mtk_drm_drv.h"
#include "mtk_plane.h"

#define MTK_MAX_BPC	10
#define MTK_MIN_BPC	3

#define MTK_DRM_DUMP_POINT_NUM 4

enum mtk_crtc_ability {
	ABILITY_FBDC			= BIT(0),
	ABILITY_EXT_LAYER		= BIT(1),
	ABILITY_IDLEMGR			= BIT(2),
	ABILITY_ESD_CHECK		= BIT(3),
	ABILITY_PQ			= BIT(4),
	ABILITY_RSZ			= BIT(5),
	ABILITY_MML			= BIT(6),
	ABILITY_WCG			= BIT(7),
	ABILITY_PRE_TE			= BIT(8),
	ABILITY_BW_MONITOR		= BIT(9),
	ABILITY_CWB			= BIT(10),
	ABILITY_MSYNC20			= BIT(11),
	ABILITY_DUAL_DISCRETE		= BIT(12),
	ABILITY_PARTIAL_UPDATE		= BIT(13),
	ABILITY_PARTIAL_UPDATE_BISO	= BIT(14),
	ABILITY_STASH_CMD		= BIT(15),
};

struct mtk_drm_wb_caps {
	bool support;
	bool rsz;
	bool rsz_crop;
	unsigned int rsz_out_min_w;
	unsigned int rsz_out_min_h;
	unsigned int rsz_out_max_w;
	unsigned int rsz_out_max_h;
};

struct mtk_drm_crtc_caps {
	struct mtk_drm_wb_caps wb_caps[MTK_DRM_DUMP_POINT_NUM];
	unsigned int crtc_ability;
	unsigned int ovl_csc_bit_number;
	unsigned int rpo_support_num;
};

enum mtk_crtc_prop_id {
	CRTC_PROP_OVERLAP_LAYER_NUM,
	CRTC_PROP_LYE_IDX,
	CRTC_PROP_PRES_FENCE_IDX,
	CRTC_PROP_SF_PRES_FENCE_IDX,
	CRTC_PROP_DOZE_ACTIVE,
	CRTC_PROP_OUTPUT_ENABLE,
	CRTC_PROP_OUTPUT_BUFF_IDX,
	CRTC_PROP_OUTPUT_X,
	CRTC_PROP_OUTPUT_Y,
	CRTC_PROP_OUTPUT_WIDTH,
	CRTC_PROP_OUTPUT_HEIGHT,
	CRTC_PROP_OUTPUT_DST_WIDTH,
	CRTC_PROP_OUTPUT_DST_HEIGHT,
	CRTC_PROP_OUTPUT_FB_ID,
	CRTC_PROP_INTF_BUFF_IDX,
	CRTC_PROP_DISP_MODE_IDX,
	CRTC_PROP_HBM_ENABLE,
	CRTC_PROP_COLOR_TRANSFORM,
	CRTC_PROP_USER_SCEN,
	CRTC_PROP_HDR_ENABLE,
	CRTC_PROP_MSYNC2_0_ENABLE,
	CRTC_PROP_MSYNC2_0_EPT,
	CRTC_PROP_OVL_DSI_SEQ,
	CRTC_PROP_OUTPUT_SCENARIO,
	CRTC_PROP_CAPS_BLOB_ID,
	CRTC_PROP_AOSP_CCORR_LINEAR,
	CRTC_PROP_PARTIAL_UPDATE_ENABLE,
	CRTC_PROP_BL_SYNC_GAMMA_GAIN,
	CRTC_PROP_DYNAMIC_WCG_OFF,
	CRTC_PROP_WCG_BY_COLOR_MODE,
	CRTC_PROP_AUTO_MODE,
	CRTC_PROP_AUTO_FAKE_FRAME,
	CRTC_PROP_AUTO_MIN_FPS,
	CRTC_PROP_HW_BRIGHTNESS,
	CRTC_PROP_BRIGHTNESS_NEED_SYNC,
	CRTC_PROP_STYLUS,
	CRTC_PROP_CUSTOM_MAX,
};

struct mtk_crtc_state {
	struct drm_crtc_state		base;

	bool				pending_config;
	unsigned int			pending_width;
	unsigned int			pending_height;
	unsigned int			pending_vrefresh;
	u64				prop_val[CRTC_PROP_CUSTOM_MAX];
};

static inline struct mtk_crtc_state *to_mtk_crtc_state(struct drm_crtc_state *s)
{
	return container_of(s, struct mtk_crtc_state, base);
}

void mtk_crtc_commit(struct drm_crtc *crtc);
int mtk_crtc_create(struct drm_device *drm_dev, const unsigned int *path,
		    unsigned int path_len, int priv_data_index,
		    const struct mtk_drm_route *conn_routes,
		    unsigned int num_conn_routes);
int mtk_crtc_plane_check(struct drm_crtc *crtc, struct drm_plane *plane,
			 struct mtk_plane_state *state);
void mtk_crtc_plane_disable(struct drm_crtc *crtc, struct drm_plane *plane);
void mtk_crtc_async_update(struct drm_crtc *crtc, struct drm_plane *plane,
			   struct drm_atomic_commit *plane_state);
struct device *mtk_crtc_dma_dev_get(struct drm_crtc *crtc);
struct sync_timeline *mtk_crtc_get_present_timeline(struct drm_crtc *crtc);
atomic_t *mtk_crtc_get_present_fence_idx(struct drm_crtc *crtc);
void mtk_crtc_set_hwvsync_en(struct drm_crtc *crtc, bool en);
void mtk_crtc_release_present_fence(struct drm_crtc *crtc);

#endif /* MTK_CRTC_H */
