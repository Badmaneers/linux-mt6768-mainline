/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2015 MediaTek Inc.
 * Author: CK Hu <ck.hu@mediatek.com>
 */

#ifndef _MTK_PLANE_H_
#define _MTK_PLANE_H_

#include <drm/drm_crtc.h>
#include <linux/types.h>

#define AFBC_DATA_BLOCK_WIDTH 32
#define AFBC_DATA_BLOCK_HEIGHT 8
#define AFBC_HEADER_BLOCK_SIZE 16
#define AFBC_HEADER_ALIGNMENT 1024

struct mtk_plane_pending_state {
	bool				config;
	bool				enable;
	dma_addr_t			addr;
	dma_addr_t			hdr_addr;
	unsigned int			pitch;
	unsigned int			hdr_pitch;
	unsigned int			format;
	unsigned long long		modifier;
	unsigned int			x;
	unsigned int			y;
	unsigned int			width;
	unsigned int			height;
	unsigned int			rotation;
	bool				dirty;
	bool				async_dirty;
	bool				async_config;
	enum drm_color_encoding		color_encoding;
};

enum mtk_plane_custom_prop {
	PLANE_PROP_NEXT_BUFF_IDX,
	PLANE_PROP_LYE_BLOB_IDX,
	PLANE_PROP_DATASPACE,
	PLANE_PROP_VPITCH,
	PLANE_PROP_COMPRESS,
	PLANE_PROP_PLANE_ALPHA,
	PLANE_PROP_ALPHA_CON,
	PLANE_PROP_DIM_COLOR,
	PLANE_PROP_IS_MML,
	PLANE_PROP_MML_SUBMIT,
	PLANE_PROP_BUFFER_ALLOC_ID,
	PLANE_PROP_OVL_CSC_SET_BRIGHTNESS,
	PLANE_PROP_OVL_CSC_SET_COLORTRANSFORM,
	PLANE_PROP_DIRTY_ROI_NUM,
	PLANE_PROP_DIRTY_ROI_X,
	PLANE_PROP_DIRTY_ROI_Y,
	PLANE_PROP_DIRTY_ROI_W,
	PLANE_PROP_DIRTY_ROI_H,
	PLANE_PROP_MODE,
	PLANE_PROP_CUSTOM_MAX,
};

struct mtk_plane_state {
	struct drm_plane_state		base;
	struct mtk_plane_pending_state	pending;
	u64				prop_val[PLANE_PROP_CUSTOM_MAX];
};

static inline struct mtk_plane_state *
to_mtk_plane_state(struct drm_plane_state *state)
{
	return container_of(state, struct mtk_plane_state, base);
}

int mtk_plane_init(struct drm_device *dev, struct drm_plane *plane,
		   unsigned long possible_crtcs, enum drm_plane_type type,
		   unsigned int supported_rotations, const u32 blend_modes,
		   const u32 *formats, size_t num_formats,
		   bool supports_afbc, unsigned int plane_idx);
#endif
