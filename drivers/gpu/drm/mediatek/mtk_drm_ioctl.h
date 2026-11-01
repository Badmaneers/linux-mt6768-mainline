/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2024 MediaTek Inc.
 * Copyright (C) 2026 Badmaneers <dukebraham24@gmail.com>
 */

#ifndef _MTK_DRM_IOCTL_H
#define _MTK_DRM_IOCTL_H

#include <drm/drm_device.h>
#include <drm/drm_file.h>
#include <drm/drm_ioctl.h>
#include <uapi/drm/mediatek_drm.h>

int mtk_drm_ioctl_gem_submit(struct drm_device *dev, void *data,
			     struct drm_file *file_priv);
int mtk_drm_ioctl_session_create(struct drm_device *dev, void *data,
				 struct drm_file *file_priv);
int mtk_drm_ioctl_session_destroy(struct drm_device *dev, void *data,
				  struct drm_file *file_priv);
int mtk_drm_ioctl_layering_rule(struct drm_device *dev, void *data,
				struct drm_file *file_priv);
int mtk_drm_ioctl_crtc_getfence(struct drm_device *dev, void *data,
				struct drm_file *file_priv);
int mtk_drm_ioctl_crtc_fence_release(struct drm_device *dev, void *data,
				     struct drm_file *file_priv);
int mtk_drm_ioctl_crtc_get_sf_fence(struct drm_device *dev, void *data,
				    struct drm_file *file_priv);
int mtk_drm_ioctl_wait_repaint(struct drm_device *dev, void *data,
			       struct drm_file *file_priv);
int mtk_drm_ioctl_get_display_caps(struct drm_device *dev, void *data,
				   struct drm_file *file_priv);
int mtk_drm_ioctl_set_ddp_mode(struct drm_device *dev, void *data,
			       struct drm_file *file_priv);
int mtk_drm_ioctl_get_session_info(struct drm_device *dev, void *data,
				   struct drm_file *file_priv);
int mtk_drm_ioctl_sec_hnd_to_gem_hnd(struct drm_device *dev, void *data,
				     struct drm_file *file_priv);
int mtk_drm_ioctl_get_master_info(struct drm_device *dev, void *data,
				  struct drm_file *file_priv);
int mtk_drm_ioctl_set_msync_params(struct drm_device *dev, void *data,
				   struct drm_file *file_priv);
int mtk_drm_ioctl_get_msync_params(struct drm_device *dev, void *data,
				   struct drm_file *file_priv);
int mtk_drm_ioctl_get_mode_ext_info(struct drm_device *dev, void *data,
				    struct drm_file *file_priv);
int mtk_drm_ioctl_hwvsync_on(struct drm_device *dev, void *data,
			     struct drm_file *file_priv);
int mtk_drm_ioctl_get_lcm_index(struct drm_device *dev, void *data,
				struct drm_file *file_priv);
int mtk_drm_ioctl_support_color_transform(struct drm_device *dev, void *data,
					  struct drm_file *file_priv);
int mtk_drm_ioctl_debug_log(struct drm_device *dev, void *data,
			    struct drm_file *file_priv);
int mtk_drm_ioctl_get_chist(struct drm_device *dev, void *data,
			    struct drm_file *file_priv);
int mtk_drm_ioctl_get_chist_caps(struct drm_device *dev, void *data,
				 struct drm_file *file_priv);
int mtk_drm_ioctl_set_chist_config(struct drm_device *dev, void *data,
				   struct drm_file *file_priv);
int mtk_drm_ioctl_get_pq_caps(struct drm_device *dev, void *data,
			      struct drm_file *file_priv);
int mtk_drm_ioctl_set_pq_caps(struct drm_device *dev, void *data,
			      struct drm_file *file_priv);
int mtk_drm_ioctl_get_all_connector_panel_info(struct drm_device *dev, void *data,
					       struct drm_file *file_priv);
int mtk_drm_ioctl_kick_idle(struct drm_device *dev, void *data,
			    struct drm_file *file_priv);
int mtk_drm_ioctl_pq_proxy(struct drm_device *dev, void *data,
			   struct drm_file *file_priv);
int mtk_drm_ioctl_pq_frame_config(struct drm_device *dev, void *data,
				  struct drm_file *file_priv);
int mtk_drm_ioctl_gem_create(struct drm_device *dev, void *data,
			     struct drm_file *file_priv);
int mtk_drm_ioctl_gem_map_offset(struct drm_device *dev, void *data,
				 struct drm_file *file_priv);
int mtk_drm_ioctl_mml_gem_submit(struct drm_device *dev, void *data,
				 struct drm_file *file_priv);
int mtk_drm_ioctl_fm_lcm_auto_test(struct drm_device *dev, void *data,
				   struct drm_file *file_priv);
int mtk_drm_ioctl_set_lease_info(struct drm_device *dev, void *data,
				 struct drm_file *file_priv);
int mtk_drm_ioctl_get_lease_info(struct drm_device *dev, void *data,
				 struct drm_file *file_priv);
int mtk_drm_ioctl_dummy_cmd_on(struct drm_device *dev, void *data,
			       struct drm_file *file_priv);
int mtk_drm_ioctl_set_ovl_layer(struct drm_device *dev, void *data,
				struct drm_file *file_priv);
int mtk_drm_ioctl_map_dma_buf(struct drm_device *dev, void *data,
			      struct drm_file *file_priv);
int mtk_drm_ioctl_unmap_dma_buf(struct drm_device *dev, void *data,
				struct drm_file *file_priv);
int mtk_drm_ioctl_oddmr_load_param(struct drm_device *dev, void *data,
				   struct drm_file *file_priv);
int mtk_drm_ioctl_oddmr_ctl(struct drm_device *dev, void *data,
			    struct drm_file *file_priv);
int mtk_drm_ioctl_esd_stat_chk(struct drm_device *dev, void *data,
			       struct drm_file *file_priv);
int mtk_drm_ioctl_mml_ctrl(struct drm_device *dev, void *data,
			   struct drm_file *file_priv);
void drm_trigger_repaint(enum DRM_REPAINT_TYPE type, struct drm_device *drm_dev);
void mtk_drm_release_all_layer_fences(void);

#define MTK_NUM_IOCTLS 0x60

extern const struct drm_ioctl_desc mtk_ioctls[MTK_NUM_IOCTLS];

#endif /* _MTK_DRM_IOCTL_H */
