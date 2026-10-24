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
#define DRM_MTK_LAYERING_RULE		0x05
#define DRM_MTK_CRTC_GETFENCE		0x06
#define DRM_MTK_WAIT_REPAINT		0x07
#define DRM_MTK_GET_DISPLAY_CAPS	0x08
#define DRM_MTK_SET_DDP_MODE		0x09
#define DRM_MTK_GET_SESSION_INFO	0x0A
#define DRM_MTK_SEC_HND_TO_GEM_HND	0x0B
#define DRM_MTK_GET_MASTER_INFO		0x0C
#define DRM_MTK_CRTC_GETSFFENCE		0x0D
#define DRM_MTK_MML_GEM_SUBMIT		0x0E
#define DRM_MTK_SET_MSYNC_PARAMS	0x0F
#define DRM_MTK_GET_MSYNC_PARAMS	0x10
#define DRM_MTK_FACTORY_LCM_AUTO_TEST	0x11
#define DRM_MTK_DRM_SET_LEASE_INFO	0x12
#define DRM_MTK_DRM_GET_LEASE_INFO	0x13
#define DRM_MTK_CRTC_FENCE_REL		0x14
#define DRM_MTK_GET_MODE_EXT_INFO	0x15
#define DRM_MTK_HWVSYNC_ON		0x16
#define DRM_MTK_DUMMY_CMD_ON		0x17

#define DRM_MTK_GET_LCM_INDEX		0x2C
#define DRM_MTK_SUPPORT_COLOR_TRANSFORM	0x2D
#define DRM_MTK_DEBUG_LOG		0x3E

#define DRM_MTK_GET_CHIST		0x46
#define DRM_MTK_GET_CHIST_CAPS		0x47
#define DRM_MTK_SET_CHIST_CONFIG	0x48

#define DRM_MTK_GET_PQ_CAPS		0x54
#define DRM_MTK_SET_PQ_CAPS		0x55

#define DRM_MTK_ODDMR_LOAD_PARAM	0x57
#define DRM_MTK_ODDMR_CTL		0x58
#define DRM_MTK_GET_PANELS_INFO		0x5A
#define DRM_MTK_KICK_IDLE		0x5B
#define DRM_MTK_PQ_FRAME_CONFIG		0x5C
#define DRM_MTK_ESD_STAT_CHK		0x5D
#define DRM_MTK_MML_CTRL		0x5F

#define GET_PANELS_STR_LEN		64
#define MAX_CRTC_CNT			10
#define MTK_PANEL_NUM			5
#define LYE_CRTC			4
#define DISP_COLOR_TM_MAX		4
#define MTK_DRM_DISP_CHIST_CHANNEL_COUNT 4

#define MTK_SESSION_MODE(id)		(((id) >> 24) & 0xff)
#define MTK_SESSION_TYPE(id)		(((id) >> 16) & 0xff)
#define MTK_SESSION_DEV(id)		((id) & 0xff)
#define MAKE_MTK_SESSION(type, dev)	((unsigned int)((type) << 16 | (dev)))

enum MTK_SESSION_TYPE {
	MTK_SESSION_PRIMARY = 1,
	MTK_SESSION_EXTERNAL = 2,
	MTK_SESSION_MEMORY = 3,
	MTK_SESSION_MAX = 4,
};

enum MTK_DRM_DISP_FEATURE {
	DRM_DISP_FEATURE_HRT = 0x00000001,
	DRM_DISP_FEATURE_DISP_SELF_REFRESH = 0x00000002,
	DRM_DISP_FEATURE_RPO = 0x00000004,
	DRM_DISP_FEATURE_FORCE_DISABLE_AOD = 0x00000008,
	DRM_DISP_FEATURE_OUTPUT_ROTATED = 0x00000010,
	DRM_DISP_FEATURE_THREE_SESSION = 0x00000020,
	DRM_DISP_FEATURE_FBDC = 0x00000040,
	DRM_DISP_FEATURE_SF_PRESENT_FENCE = 0x00000080,
	DRM_DISP_FEATURE_PQ_34_COLOR_MATRIX = 0x00000100,
	DRM_DISP_FEATURE_MSYNC2_0 = 0x00000200,
	DRM_DISP_FEATURE_MML_PRIMARY = 0x00000400,
	DRM_DISP_FEATURE_VIRUTAL_DISPLAY = 0x00000800,
	DRM_DISP_FEATURE_IOMMU = 0x00001000,
	DRM_DISP_FEATURE_OVL_BW_MONITOR = 0x00002000,
	DRM_DISP_FEATURE_GPU_CACHE = 0x00004000,
	DRM_DISP_FEATURE_SPHRT = 0x00008000,
	DRM_DISP_FEATURE_PARTIAL_UPDATE = 0x00010000,
};

enum MTK_LAYERING_CAPS {
	MTK_LAYERING_OVL_ONLY = 0x00000001,
	MTK_MDP_RSZ_LAYER = 0x00000002,
	MTK_DISP_RSZ_LAYER = 0x00000004,
	MTK_MDP_ROT_LAYER = 0x00000008,
	MTK_MDP_HDR_LAYER = 0x00000010,
	MTK_NO_FBDC = 0x00000020,
	MTK_CLIENT_CLEAR_LAYER = 0x00000040,
	MTK_DISP_CLIENT_CLEAR_LAYER = 0x00000080,
};

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

struct drm_mtk_fence {
	__u32 crtc_id;
	__s32 fence_fd;
	__u32 fence_idx;
};

struct drm_mtk_sec_gem_hnd {
	__u32 sec_hnd;
	__u32 gem_hnd;
};

struct msync_level_table {
	unsigned int level_id;
	unsigned int level_fps;
	unsigned int max_fps;
	unsigned int min_fps;
};

struct msync_parameter_table {
	unsigned int msync_max_fps;
	unsigned int msync_min_fps;
	unsigned int msync_level_num;
	struct msync_level_table *level_tb;
};

struct DISP_COLOR_TRANSFORM {
	int matrix[DISP_COLOR_TM_MAX][DISP_COLOR_TM_MAX];
};

struct drm_mtk_mode_ext_info {
	unsigned int crtc_id;
	unsigned int mode_num;
	unsigned int *total_offset;
	unsigned int *real_te_duration;
	void **switch_mode_delay;
	unsigned int idle_fps;
};

struct drm_mtk_layer_config {
	__u32 ovl_id;
	__u32 src_fmt;
	int dataspace;
	__u32 dst_offset_x, dst_offset_y;
	__u32 dst_width, dst_height;
	int ext_sel_layer;
	__u32 src_offset_x, src_offset_y;
	__u32 src_width, src_height;
	__u32 layer_caps;
	__u32 clip;
	__u64 buffer_alloc_id;
	__u8 compress;
	__u8 secure;
};

struct wb_frame_info {
	__u32 fmt;
	__u32 src_width, src_height;
	__u32 dst_width, dst_height;
};

struct drm_mtk_layering_info {
	struct drm_mtk_layer_config *input_config[LYE_CRTC];
	int disp_mode[LYE_CRTC];
	int disp_mode_idx[LYE_CRTC];
	int layer_num[LYE_CRTC];
	int gles_head[LYE_CRTC];
	int gles_tail[LYE_CRTC];
	__u32 disp_caps[LYE_CRTC];
	__u32 frame_idx[LYE_CRTC];
	int hrt_num;
	__u32 disp_idx;
	__u32 disp_list;
	int res_idx;
	__u32 hrt_weight;
	__u32 hrt_idx;
	void *mml_cfg[LYE_CRTC];
	struct wb_frame_info wb_cfg[LYE_CRTC];
	__u32 exec_reserved_time;
};

struct mtk_drm_disp_caps_info {
	unsigned int hw_ver;
	unsigned int disp_feature_flag;
	int lcm_degree;
	unsigned int rsz_in_max[2];
	int lcm_color_mode;
	unsigned int max_luminance;
	unsigned int average_luminance;
	unsigned int min_luminance;
	unsigned int color_format;
	unsigned int max_bin;
	unsigned int max_channel;
	unsigned int msync_level_num;
};

struct drm_mtk_channel_hist {
	unsigned int *hist_bin;
};

struct drm_mtk_chist_info {
	unsigned int present_fence;
	unsigned int device_id;
	int caller;
	unsigned int get_channel_count;
	struct drm_mtk_channel_hist channel_hist[MTK_DRM_DISP_CHIST_CHANNEL_COUNT];
	unsigned int lcm_width;
	unsigned int lcm_height;
};

struct drm_mtk_channel_config {
	_Bool enabled;
	int color_format;
	unsigned int bin_count;
	unsigned int channel_id;
	unsigned int blk_width;
	unsigned int blk_height;
	unsigned int roi_start_x;
	unsigned int roi_start_y;
	unsigned int roi_end_x;
	unsigned int roi_end_y;
};

struct drm_mtk_chist_caps {
	unsigned int device_id;
	unsigned int support_color;
	unsigned int lcm_width;
	unsigned int lcm_height;
	struct drm_mtk_channel_config chist_config[MTK_DRM_DISP_CHIST_CHANNEL_COUNT];
};

struct drm_mtk_chist_config {
	unsigned int device_id;
	unsigned int lcm_color_mode;
	unsigned int config_channel_count;
	int caller;
	struct drm_mtk_channel_config chist_config[MTK_DRM_DISP_CHIST_CHANNEL_COUNT];
};

struct drm_mtk_ccorr_caps {
	unsigned int ccorr_bit;
	unsigned int ccorr_number;
	unsigned int ccorr_linear;
};

struct mtk_drm_pq_caps_info {
	struct drm_mtk_ccorr_caps ccorr_caps;
};

#define DRM_IOCTL_MTK_SESSION_CREATE	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_SESSION_CREATE, struct drm_mtk_session)
#define DRM_IOCTL_MTK_SESSION_DESTROY	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_SESSION_DESTROY, struct drm_mtk_session)
#define DRM_IOCTL_MTK_LAYERING_RULE	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_LAYERING_RULE, struct drm_mtk_layering_info)
#define DRM_IOCTL_MTK_CRTC_GETFENCE	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_CRTC_GETFENCE, struct drm_mtk_fence)
#define DRM_IOCTL_MTK_CRTC_FENCE_REL	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_CRTC_FENCE_REL, struct drm_mtk_fence)
#define DRM_IOCTL_MTK_CRTC_GETSFFENCE	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_CRTC_GETSFFENCE, struct drm_mtk_fence)
#define DRM_IOCTL_MTK_WAIT_REPAINT	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_WAIT_REPAINT, unsigned int)
#define DRM_IOCTL_MTK_GET_DISPLAY_CAPS	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_GET_DISPLAY_CAPS, struct mtk_drm_disp_caps_info)
#define DRM_IOCTL_MTK_SET_DDP_MODE	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_SET_DDP_MODE, unsigned int)
#define DRM_IOCTL_MTK_GET_SESSION_INFO	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_GET_SESSION_INFO, struct drm_mtk_session_info)
#define DRM_IOCTL_MTK_SEC_HND_TO_GEM_HND DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_SEC_HND_TO_GEM_HND, struct drm_mtk_sec_gem_hnd)
#define DRM_IOCTL_MTK_GET_MASTER_INFO	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_GET_MASTER_INFO, int)
#define DRM_IOCTL_MTK_SET_MSYNC_PARAMS	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_SET_MSYNC_PARAMS, struct msync_parameter_table)
#define DRM_IOCTL_MTK_GET_MSYNC_PARAMS	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_GET_MSYNC_PARAMS, struct msync_parameter_table)
#define DRM_IOCTL_MTK_GET_MODE_EXT_INFO	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_GET_MODE_EXT_INFO, struct drm_mtk_mode_ext_info)
#define DRM_IOCTL_MTK_HWVSYNC_ON	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_HWVSYNC_ON, unsigned int)
#define DRM_IOCTL_MTK_GET_LCM_INDEX	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_GET_LCM_INDEX, unsigned int)
#define DRM_IOCTL_MTK_SUPPORT_COLOR_TRANSFORM DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_SUPPORT_COLOR_TRANSFORM, struct DISP_COLOR_TRANSFORM)
#define DRM_IOCTL_MTK_DEBUG_LOG		DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_DEBUG_LOG, int)
#define DRM_IOCTL_MTK_GET_CHIST		DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_GET_CHIST, struct drm_mtk_chist_info)
#define DRM_IOCTL_MTK_GET_CHIST_CAPS	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_GET_CHIST_CAPS, struct drm_mtk_chist_caps)
#define DRM_IOCTL_MTK_SET_CHIST_CONFIG	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_SET_CHIST_CONFIG, struct drm_mtk_chist_config)
#define DRM_IOCTL_MTK_GET_PQ_CAPS	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_GET_PQ_CAPS, struct mtk_drm_pq_caps_info)
#define DRM_IOCTL_MTK_SET_PQ_CAPS	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_SET_PQ_CAPS, struct mtk_drm_pq_caps_info)
#define DRM_IOCTL_MTK_GET_PANELS_INFO	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_GET_PANELS_INFO, struct mtk_drm_panels_info)
#define DRM_IOCTL_MTK_KICK_IDLE		DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_KICK_IDLE, unsigned int)

#endif /* _UAPI_MEDIATEK_DRM_H */
