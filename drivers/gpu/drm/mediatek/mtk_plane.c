// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2015 MediaTek Inc.
 * Author: CK Hu <ck.hu@mediatek.com>
 */

#include <drm/drm_atomic.h>
#include <drm/drm_atomic_helper.h>
#include <drm/drm_atomic_uapi.h>
#include <drm/drm_blend.h>
#include <drm/drm_fourcc.h>
#include <drm/drm_framebuffer.h>
#include <drm/drm_gem_atomic_helper.h>
#include <drm/drm_gem_dma_helper.h>
#include <drm/drm_print.h>
#include <linux/align.h>

#include "mtk_crtc.h"
#include "mtk_ddp_comp.h"
#include "mtk_drm_drv.h"
#include "mtk_plane.h"

static const u64 modifiers[] = {
	DRM_FORMAT_MOD_LINEAR,
	DRM_FORMAT_MOD_INVALID,
};

static void mtk_plane_reset(struct drm_plane *plane)
{
	struct mtk_plane_state *state;

	if (plane->state) {
		__drm_atomic_helper_plane_destroy_state(plane->state);

		state = to_mtk_plane_state(plane->state);
		memset(state, 0, sizeof(*state));
	} else {
		state = kzalloc_obj(*state);
		if (!state)
			return;
	}

	__drm_atomic_helper_plane_reset(plane, &state->base);

	state->base.plane = plane;
	state->pending.format = DRM_FORMAT_RGB565;
	state->pending.modifier = DRM_FORMAT_MOD_LINEAR;
}

static struct drm_plane_state *mtk_plane_duplicate_state(struct drm_plane *plane)
{
	struct mtk_plane_state *old_state = to_mtk_plane_state(plane->state);
	struct mtk_plane_state *state;

	state = kmalloc_obj(*state);
	if (!state)
		return NULL;

	__drm_atomic_helper_plane_duplicate_state(plane, &state->base);

	WARN_ON(state->base.plane != plane);

	state->pending = old_state->pending;
	if (old_state)
		memcpy(state->prop_val, old_state->prop_val, sizeof(state->prop_val));
	else
		memset(state->prop_val, 0, sizeof(state->prop_val));

	return &state->base;
}

static bool mtk_plane_format_mod_supported(struct drm_plane *plane,
					   uint32_t format,
					   uint64_t modifier)
{
	return modifier == DRM_FORMAT_MOD_LINEAR;
}

static void mtk_plane_destroy_state(struct drm_plane *plane,
				    struct drm_plane_state *state)
{
	__drm_atomic_helper_plane_destroy_state(state);
	kfree(to_mtk_plane_state(state));
}

static int mtk_plane_atomic_async_check(struct drm_plane *plane,
					struct drm_atomic_commit *state, bool flip)
{
	struct drm_plane_state *new_plane_state = drm_atomic_get_new_plane_state(state,
										 plane);
	struct drm_crtc_state *crtc_state;
	int ret;

	if (plane != new_plane_state->crtc->cursor)
		return -EINVAL;

	if (!plane->state)
		return -EINVAL;

	if (!plane->state->fb)
		return -EINVAL;

	ret = mtk_crtc_plane_check(new_plane_state->crtc, plane,
				   to_mtk_plane_state(new_plane_state));
	if (ret)
		return ret;

	crtc_state = drm_atomic_get_new_crtc_state(state,
						   new_plane_state->crtc);

	return drm_atomic_helper_check_plane_state(plane->state, crtc_state,
						   DRM_PLANE_NO_SCALING,
						   DRM_PLANE_NO_SCALING,
						   true, true);
}

static void mtk_plane_update_new_state(struct drm_plane_state *new_state,
				       struct mtk_plane_state *mtk_plane_state)
{
	struct drm_framebuffer *fb = new_state->fb;
	struct drm_gem_dma_object *dma_obj;
	struct drm_gem_object *gem;
	unsigned int pitch, format;
	u64 modifier;
	dma_addr_t addr;
	dma_addr_t hdr_addr = 0;
	unsigned int hdr_pitch = 0;
	int offset;

	gem = fb->obj[0];
	if (gem) {
		dma_obj = to_drm_gem_dma_obj(gem);
		addr = dma_obj->dma_addr;
	} else {
		addr = 0;
	}
	pitch = fb->pitches[0];
	format = fb->format->format;
	modifier = fb->modifier;

	if (gem && modifier == DRM_FORMAT_MOD_LINEAR) {
		/*
		 * Using dma_addr_t variable to calculate with multiplier of different types,
		 * for example: addr += (new_state->src.x1 >> 16) * fb->format->cpp[0];
		 * may cause coverity issue with unintentional overflow.
		 */
		offset = (new_state->src.x1 >> 16) * fb->format->cpp[0];
		addr += offset;
		offset = (new_state->src.y1 >> 16) * pitch;
		addr += offset;
	} else if (gem) {
		int width_in_blocks = ALIGN(fb->width, AFBC_DATA_BLOCK_WIDTH)
				      / AFBC_DATA_BLOCK_WIDTH;
		int height_in_blocks = ALIGN(fb->height, AFBC_DATA_BLOCK_HEIGHT)
				       / AFBC_DATA_BLOCK_HEIGHT;
		int x_offset_in_blocks = (new_state->src.x1 >> 16) / AFBC_DATA_BLOCK_WIDTH;
		int y_offset_in_blocks = (new_state->src.y1 >> 16) / AFBC_DATA_BLOCK_HEIGHT;
		int hdr_size, hdr_offset;

		hdr_pitch = width_in_blocks * AFBC_HEADER_BLOCK_SIZE;
		pitch = width_in_blocks * AFBC_DATA_BLOCK_WIDTH *
			AFBC_DATA_BLOCK_HEIGHT * fb->format->cpp[0];

		hdr_size = ALIGN(hdr_pitch * height_in_blocks, AFBC_HEADER_ALIGNMENT);
		hdr_offset = hdr_pitch * y_offset_in_blocks +
			AFBC_HEADER_BLOCK_SIZE * x_offset_in_blocks;

		/*
		 * Using dma_addr_t variable to calculate with multiplier of different types,
		 * for example: addr += hdr_pitch * y_offset_in_blocks;
		 * may cause coverity issue with unintentional overflow.
		 */
		hdr_addr = addr + hdr_offset;

		/* The data plane is offset by 1 additional block. */
		offset = pitch * y_offset_in_blocks +
			 AFBC_DATA_BLOCK_WIDTH * AFBC_DATA_BLOCK_HEIGHT *
			 fb->format->cpp[0] * (x_offset_in_blocks + 1);

		/*
		 * Using dma_addr_t variable to calculate with multiplier of different types,
		 * for example: addr += pitch * y_offset_in_blocks;
		 * may cause coverity issue with unintentional overflow.
		 */
		addr = addr + hdr_size + offset;
	}

	mtk_plane_state->pending.enable = true;
	mtk_plane_state->pending.pitch = pitch;
	mtk_plane_state->pending.hdr_pitch = hdr_pitch;
	mtk_plane_state->pending.format = format;
	mtk_plane_state->pending.modifier = modifier;
	mtk_plane_state->pending.addr = addr;
	mtk_plane_state->pending.hdr_addr = hdr_addr;
	mtk_plane_state->pending.x = new_state->dst.x1;
	mtk_plane_state->pending.y = new_state->dst.y1;
	mtk_plane_state->pending.width = drm_rect_width(&new_state->dst);
	mtk_plane_state->pending.height = drm_rect_height(&new_state->dst);
	mtk_plane_state->pending.rotation = new_state->rotation;
	mtk_plane_state->pending.color_encoding = new_state->color_encoding;
}

static void mtk_plane_atomic_async_update(struct drm_plane *plane,
					  struct drm_atomic_commit *state)
{
	struct drm_plane_state *new_state = drm_atomic_get_new_plane_state(state,
									   plane);
	struct mtk_plane_state *new_plane_state = to_mtk_plane_state(plane->state);

	plane->state->crtc_x = new_state->crtc_x;
	plane->state->crtc_y = new_state->crtc_y;
	plane->state->crtc_h = new_state->crtc_h;
	plane->state->crtc_w = new_state->crtc_w;
	plane->state->src_x = new_state->src_x;
	plane->state->src_y = new_state->src_y;
	plane->state->src_h = new_state->src_h;
	plane->state->src_w = new_state->src_w;
	plane->state->dst.x1 = new_state->dst.x1;
	plane->state->dst.y1 = new_state->dst.y1;

	mtk_plane_update_new_state(new_state, new_plane_state);
	swap(plane->state->fb, new_state->fb);
	wmb(); /* Make sure the above parameters are set before update */
	new_plane_state->pending.async_dirty = true;
	mtk_crtc_async_update(new_state->crtc, plane, state);
}

struct mtk_plane_prop_desc {
	const char *name;
	u64 min;
	u64 max;
	u64 default_val;
};

static const struct mtk_plane_prop_desc mtk_plane_props[PLANE_PROP_CUSTOM_MAX] = {
	[PLANE_PROP_NEXT_BUFF_IDX] = { "NEXT_BUFF_IDX", 0, U64_MAX, 0 },
	[PLANE_PROP_LYE_BLOB_IDX] = { "LYE_BLOB_IDX", 0, U64_MAX, 0 },
	[PLANE_PROP_DATASPACE] = { "DATASPACE", 0, U64_MAX, 0 },
	[PLANE_PROP_VPITCH] = { "VPITCH", 0, U64_MAX, 0 },
	[PLANE_PROP_COMPRESS] = { "COMPRESS", 0, U64_MAX, 0 },
	[PLANE_PROP_PLANE_ALPHA] = { "PLANE_PROP_PLANE_ALPHA", 0, U64_MAX, 0 },
	[PLANE_PROP_ALPHA_CON] = { "PLANE_PROP_ALPHA_CON", 0, U64_MAX, 0 },
	[PLANE_PROP_DIM_COLOR] = { "DIM_COLOR", 0, U64_MAX, 0 },
	[PLANE_PROP_IS_MML] = { "IS_MML", 0, U64_MAX, 0 },
	[PLANE_PROP_MML_SUBMIT] = { "MML_SUBMIT", 0, U64_MAX, 0 },
	[PLANE_PROP_BUFFER_ALLOC_ID] = { "BUFFER_ALLOC_ID", 0, U64_MAX, 0 },
	[PLANE_PROP_OVL_CSC_SET_BRIGHTNESS] = { "OVL_CSC_SET_BRIGHTNESS", 0, U64_MAX, 0 },
	[PLANE_PROP_OVL_CSC_SET_COLORTRANSFORM] = { "OVL_CSC_SET_COLORTRANSFORM", 0, U64_MAX, 0 },
	[PLANE_PROP_DIRTY_ROI_NUM] = { "DIRTY_ROI_NUM", 0, U64_MAX, 0 },
	[PLANE_PROP_DIRTY_ROI_X] = { "DIRTY_ROI_X", 0, U64_MAX, 0 },
	[PLANE_PROP_DIRTY_ROI_Y] = { "DIRTY_ROI_Y", 0, U64_MAX, 0 },
	[PLANE_PROP_DIRTY_ROI_W] = { "DIRTY_ROI_W", 0, U64_MAX, 0 },
	[PLANE_PROP_DIRTY_ROI_H] = { "DIRTY_ROI_H", 0, U64_MAX, 0 },
	[PLANE_PROP_MODE] = { "MODE", 0, U64_MAX, 0 },
};

static struct drm_property *mtk_plane_prop_ptrs[PLANE_PROP_CUSTOM_MAX];

static int mtk_plane_atomic_set_property(struct drm_plane *plane,
					 struct drm_plane_state *state,
					 struct drm_property *property,
					 uint64_t val)
{
	struct mtk_plane_state *plane_state = to_mtk_plane_state(state);
	int i;

	for (i = 0; i < PLANE_PROP_CUSTOM_MAX; i++) {
		if (mtk_plane_prop_ptrs[i] == property) {
			plane_state->prop_val[i] = val;
			return 0;
		}
	}

	return -EINVAL;
}

static int mtk_plane_atomic_get_property(struct drm_plane *plane,
					 const struct drm_plane_state *state,
					 struct drm_property *property,
					 uint64_t *val)
{
	const struct mtk_plane_state *plane_state =
		container_of(state, const struct mtk_plane_state, base);
	int i;

	for (i = 0; i < PLANE_PROP_CUSTOM_MAX; i++) {
		if (mtk_plane_prop_ptrs[i] == property) {
			*val = plane_state->prop_val[i];
			return 0;
		}
	}

	return -EINVAL;
}

static const struct drm_plane_funcs mtk_plane_funcs = {
	.update_plane = drm_atomic_helper_update_plane,
	.disable_plane = drm_atomic_helper_disable_plane,
	.destroy = drm_plane_cleanup,
	.reset = mtk_plane_reset,
	.atomic_duplicate_state = mtk_plane_duplicate_state,
	.atomic_destroy_state = mtk_plane_destroy_state,
	.format_mod_supported = mtk_plane_format_mod_supported,
	.atomic_set_property = mtk_plane_atomic_set_property,
	.atomic_get_property = mtk_plane_atomic_get_property,
};

static int mtk_plane_atomic_check(struct drm_plane *plane,
				  struct drm_atomic_commit *state)
{
	struct drm_plane_state *new_plane_state = drm_atomic_get_new_plane_state(state,
										 plane);
	struct drm_framebuffer *fb = new_plane_state->fb;
	struct drm_crtc_state *crtc_state;
	int ret;

	if (!fb)
		return 0;

	if (fb->format->format == DRM_FORMAT_C8)
		return 0;

	if (WARN_ON(!new_plane_state->crtc))
		return 0;

	ret = mtk_crtc_plane_check(new_plane_state->crtc, plane,
				   to_mtk_plane_state(new_plane_state));
	if (ret)
		return ret;

	crtc_state = drm_atomic_get_crtc_state(state,
					       new_plane_state->crtc);
	if (IS_ERR(crtc_state))
		return PTR_ERR(crtc_state);

	return drm_atomic_helper_check_plane_state(new_plane_state,
						   crtc_state,
						   DRM_PLANE_NO_SCALING,
						   DRM_PLANE_NO_SCALING,
						   true, true);
}

static void mtk_plane_atomic_disable(struct drm_plane *plane,
				     struct drm_atomic_commit *state)
{
	struct drm_plane_state *new_state = drm_atomic_get_new_plane_state(state,
									   plane);
	struct mtk_plane_state *mtk_plane_state = to_mtk_plane_state(new_state);
	struct drm_plane_state *old_state = drm_atomic_get_old_plane_state(state,
									   plane);

	mtk_plane_state->pending.enable = false;
	wmb(); /* Make sure the above parameter is set before update */
	mtk_plane_state->pending.dirty = true;

	if (old_state && old_state->crtc)
		mtk_crtc_plane_disable(old_state->crtc, plane);
}

static void mtk_plane_atomic_update(struct drm_plane *plane,
				    struct drm_atomic_commit *state)
{
	struct drm_plane_state *new_state = drm_atomic_get_new_plane_state(state,
									   plane);
	struct mtk_plane_state *mtk_plane_state = to_mtk_plane_state(new_state);

	if (!new_state->crtc || WARN_ON(!new_state->fb))
		return;

	if (!new_state->visible) {
		mtk_plane_atomic_disable(plane, state);
		return;
	}

	mtk_plane_update_new_state(new_state, mtk_plane_state);
	wmb(); /* Make sure the above parameters are set before update */
	mtk_plane_state->pending.dirty = true;
}

static const struct drm_plane_helper_funcs mtk_plane_helper_funcs = {
	.atomic_check = mtk_plane_atomic_check,
	.atomic_update = mtk_plane_atomic_update,
	.atomic_disable = mtk_plane_atomic_disable,
	.atomic_async_update = mtk_plane_atomic_async_update,
	.atomic_async_check = mtk_plane_atomic_async_check,
};

int mtk_plane_init(struct drm_device *dev, struct drm_plane *plane,
		   unsigned long possible_crtcs, enum drm_plane_type type,
		   unsigned int supported_rotations, const u32 blend_modes,
		   const u32 *formats, size_t num_formats,
		   bool supports_afbc, unsigned int plane_idx)
{
	int err;

	if (!formats || !num_formats) {
		DRM_ERROR("no formats for plane\n");
		return -EINVAL;
	}

	err = drm_universal_plane_init(dev, plane, possible_crtcs,
				       &mtk_plane_funcs, formats,
				       num_formats,
				       supports_afbc ? modifiers : NULL,
				       type, NULL);
	if (err) {
		DRM_ERROR("failed to initialize plane\n");
		return err;
	}

	/*
	 * The hardware does not support repositioning planes by muxing: their
	 * Z-position is infact fixed and the only way to change the actual
	 * order is to swap the contents of the entire register set of one
	 * overlay with another, which may be more expensive than desired.
	 *
	 * With no repositioning, the caller of this function guarantees that
	 * the plane_idx is correct. This means that, for example, the PRIMARY
	 * plane fed to this function will always have plane_idx zero.
	 */
	err = drm_plane_create_zpos_immutable_property(plane, plane_idx);
	if (err) {
		DRM_ERROR("Failed to create zpos property for plane %u\n", plane_idx);
		return err;
	}

	if (supported_rotations) {
		err = drm_plane_create_rotation_property(plane,
							 DRM_MODE_ROTATE_0,
							 supported_rotations);
		if (err)
			DRM_INFO("Create rotation property failed\n");
	}

	err = drm_plane_create_alpha_property(plane);
	if (err)
		DRM_ERROR("failed to create property: alpha\n");

	if (blend_modes) {
		err = drm_plane_create_blend_mode_property(plane, blend_modes);
		if (err)
			DRM_ERROR("failed to create property: blend_mode\n");
	}

	{
		int i;

		for (i = 0; i < PLANE_PROP_CUSTOM_MAX; i++) {
			if (!mtk_plane_prop_ptrs[i]) {
				mtk_plane_prop_ptrs[i] = drm_property_create_range(dev,
					DRM_MODE_PROP_ATOMIC,
					mtk_plane_props[i].name,
					mtk_plane_props[i].min,
					mtk_plane_props[i].max);
			}
			if (mtk_plane_prop_ptrs[i]) {
				drm_object_attach_property(&plane->base,
					mtk_plane_prop_ptrs[i],
					mtk_plane_props[i].default_val);
			}
		}
	}

	drm_plane_helper_add(plane, &mtk_plane_helper_funcs);

	return 0;
}
