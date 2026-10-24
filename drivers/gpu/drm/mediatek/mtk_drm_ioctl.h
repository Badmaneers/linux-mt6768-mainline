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

int mtk_drm_ioctl_get_all_connector_panel_info(struct drm_device *dev, void *data,
					       struct drm_file *file_priv);
int mtk_drm_ioctl_get_session_info(struct drm_device *dev, void *data,
				   struct drm_file *file_priv);
int mtk_drm_ioctl_session_create(struct drm_device *dev, void *data,
				 struct drm_file *file_priv);
int mtk_drm_ioctl_session_destroy(struct drm_device *dev, void *data,
				  struct drm_file *file_priv);
int mtk_drm_ioctl_set_ddp_mode(struct drm_device *dev, void *data,
			       struct drm_file *file_priv);
int mtk_drm_ioctl_hwvsync_on(struct drm_device *dev, void *data,
			     struct drm_file *file_priv);

#define MTK_NUM_IOCTLS 0x60

extern const struct drm_ioctl_desc mtk_ioctls[MTK_NUM_IOCTLS];

#endif /* _MTK_DRM_IOCTL_H */
