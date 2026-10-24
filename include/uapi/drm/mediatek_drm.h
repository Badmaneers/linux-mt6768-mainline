/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/*
 * Copyright (c) 2019-2024 MediaTek Inc.
 * Copyright (C) 2026 Badmaneers <dukebraham24@gmail.com>
 */

#ifndef _UAPI_MEDIATEK_DRM_H
#define _UAPI_MEDIATEK_DRM_H

#include <drm/drm.h>

#define DRM_MTK_GEM_CREATE		0x00
#define DRM_MTK_GEM_MAP_OFFSET		0x01
#define DRM_MTK_GEM_SUBMIT		0x02
#define DRM_MTK_SESSION_CREATE		0x03
#define DRM_MTK_SESSION_DESTROY		0x04
#define DRM_MTK_SET_DDP_MODE		0x09
#define DRM_MTK_GET_SESSION_INFO	0x0A
#define DRM_MTK_HWVSYNC_ON		0x16
#define DRM_MTK_GET_PANELS_INFO		0x5A
#define DRM_MTK_KICK_IDLE		0x5B

#define GET_PANELS_STR_LEN		64
#define MAX_CRTC_CNT			10
#define MTK_PANEL_NUM			5

struct drm_mtk_session {
	__u32 type;
	__u32 device_id;
	__u32 mode;
	__u32 session_id;
};

struct drm_mtk_session_info {
	unsigned int session_id;
	unsigned int vsyncFPS;
	unsigned int physicalWidthUm;
	unsigned int physicalHeightUm;
	unsigned int physical_width;
	unsigned int physical_height;
	unsigned int crop_width[MTK_PANEL_NUM];
	unsigned int crop_height[MTK_PANEL_NUM];
	int rotate;
};

struct mtk_drm_panels_info {
	int connector_cnt;
	int default_connector_id;
	unsigned int *connector_obj_id;
	unsigned int **possible_crtc;
	char **panel_name;
	unsigned int *panel_id;
	unsigned int *dsi_mode;
};

#define DRM_IOCTL_MTK_SESSION_CREATE	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_SESSION_CREATE, struct drm_mtk_session)
#define DRM_IOCTL_MTK_SESSION_DESTROY	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_SESSION_DESTROY, struct drm_mtk_session)
#define DRM_IOCTL_MTK_SET_DDP_MODE	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_SET_DDP_MODE, unsigned int)
#define DRM_IOCTL_MTK_GET_SESSION_INFO	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_GET_SESSION_INFO, struct drm_mtk_session_info)
#define DRM_IOCTL_MTK_HWVSYNC_ON	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_HWVSYNC_ON, unsigned int)
#define DRM_IOCTL_MTK_GET_PANELS_INFO	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_GET_PANELS_INFO, struct mtk_drm_panels_info)
#define DRM_IOCTL_MTK_KICK_IDLE		DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_KICK_IDLE, unsigned int)

#endif /* _UAPI_MEDIATEK_DRM_H */
