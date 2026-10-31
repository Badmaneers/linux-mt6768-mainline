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
#define DRM_MTK_PQ_PROXY_IOCTL		0x37
#define DRM_MTK_DEBUG_LOG		0x3E

#define DRM_MTK_GET_CHIST		0x46
#define DRM_MTK_GET_CHIST_CAPS		0x47
#define DRM_MTK_SET_CHIST_CONFIG	0x48

#define DRM_MTK_SET_OVL_LAYER		0x4A
#define DRM_MTK_MAP_DMA_BUF		0x4B
#define DRM_MTK_UNMAP_DMA_BUF		0x4C

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
	MTK_MML_DISP_MDP_LAYER = 0x00002000,
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

enum DRM_REPAINT_TYPE {
	DRM_WAIT_FOR_REPAINT,
	DRM_REPAINT_FOR_ANTI_LATENCY,
	DRM_REPAINT_FOR_SWITCH_DECOUPLE,
	DRM_REPAINT_FOR_SWITCH_DECOUPLE_MIRROR,
	DRM_REPAINT_FOR_IDLE,
	DRM_REPAINT_FOR_MML,
	DRM_REPAINT_TYPE_NUM,
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

struct mtk_drm_pq_config_ctl {
	__u32 crtc_id;
	__u8 check_trigger;
	__u8 len;
	void *data;
};

struct mtk_drm_pq_param {
	__u32 cmd;
	__u32 size;
	void *data;
};

struct mtk_drm_pq_proxy_ctl {
	__u32 crtc_id;
	__u32 cmd;
	__u32 size;
	__u32 extra_size;
	void *data;
	void *extra_data;
};

enum mtk_pq_module_type {
	MTK_DISP_PQ_COLOR,
	MTK_DISP_PQ_DITHER,
	MTK_DISP_PQ_CCORR,
	MTK_DISP_PQ_AAL,
	MTK_DISP_PQ_GAMMA,
	MTK_DISP_PQ_CHIST,
	MTK_DISP_PQ_C3D,
	MTK_DISP_PQ_TDSHP,
	MTK_DISP_PQ_DMR,
	MTK_DISP_PQ_DBI,
	MTK_DISP_VIRTUAL_TYPE,
	MTK_DISP_PQ_TYPE_MAX = 65535,
};

enum mtk_pq_frame_cfg_cmd {
	PQ_CMD_INVALID,
	PQ_AAL_EVENTCTL,
	PQ_AAL_INIT_REG,
	PQ_AAL_SET_ESS20_SPECT_PARAM,
	PQ_AAL_SET_PARAM,
	PQ_AAL_INIT_DRE30,
	PQ_AAL_CLARITY_SET_REG,
	PQ_GAMMA_SET_GAMMALUT = 100,
	PQ_GAMMA_SET_12BIT_GAMMALUT,
	PQ_GAMMA_BYPASS_GAMMA,
	PQ_GAMMA_DISABLE_MUL_EN,
	PQ_CHIST_CONFIG = 200,
	PQ_COLOR_MUTEX_CONTROL = 300,
	PQ_COLOR_BYPASS,
	PQ_COLOR_SET_PQINDEX,
	PQ_COLOR_SET_PQPARAM,
	PQ_COLOR_WRITE_REG,
	PQ_COLOR_WRITE_SW_REG,
	PQ_COLOR_SET_COLOR_REG,
	PQ_COLOR_SET_WINDOW,
	PQ_COLOR_DRECOLOR_SET_PARAM,
	PQ_CCORR_EVENTCTL = 400,
	PQ_CCORR_SUPPORT_COLOR_MATRIX,
	PQ_CCORR_SET_CCORR,
	PQ_CCORR_AIBLD_CV_MODE,
	PQ_CCORR_SET_PQ_CAPS,
	PQ_C3D_EVENTCTL = 500,
	PQ_C3D_BYPASS,
	PQ_C3D_SET_LUT,
	PQ_TDSHP_SET_REG = 600,
	PQ_DITHER_SET_DITHER_PARAM = 700,
	PQ_DBI_LOAD_PARAM = 800,
	PQ_DBI_LOAD_TB,
	PQ_DBI_ENABLE,
	PQ_DBI_DISABLE,
	PQ_DBI_REMAP_TARGET,
	PQ_DBI_REMAP_CHG,
	PQ_DBI_REMAP_ENABLE,
	PQ_DBI_REMAP_DISABLE,
	PQ_DBI_SET_GAIN_RATIO,
	PQ_DBI_LOAD_SCP_PARAM,
	PQ_DBI_GET_SCP_LIFECYCLE,
	PQ_VIRTUAL_SET_PROPERTY = 900,
	PQ_VIRTUAL_CHECK_TRIGGER,
	PQ_VIRTUAL_RELAY_ENGINES,
	PQ_GET_CMD_START = 60000,
	PQ_AAL_GET_HIST,
	PQ_AAL_GET_SIZE,
	PQ_AAL_SET_TRIGGER_STATE,
	PQ_AAL_GET_BASE_VOLTAGE,
	PQ_CHIST_GET = 60100,
	PQ_COLOR_READ_REG = 60200,
	PQ_COLOR_READ_SW_REG,
	PQ_CCORR_GET_IRQ = 60300,
	PQ_CCORR_GET_PQ_CAPS,
	PQ_C3D_GET_IRQ = 60400,
	PQ_C3D_GET_IRQ_STATUS,
	PQ_C3D_GET_BIN_NUM,
	PQ_TDSHP_GET_SIZE = 60500,
	PQ_DMR_INIT = 60600,
	PQ_DMR_ENABLE,
	PQ_DMR_DISABLE,
	PQ_DBI_GET_HW_ID,
	PQ_DBI_GET_WIDTH,
	PQ_DBI_GET_HEIGHT,
	PQ_DBI_GET_DBV,
	PQ_DBI_GET_FPS,
	PQ_DBI_GET_SCP,
	PQ_DMR_BINSET_INIT,
	PQ_DMR_BINSET_CHG,
	PQ_ODDMR_REG_TUNING_INIT,
	PQ_ODDMR_REG_TUNING_ENABLE,
	PQ_VIRTUAL_GET_MASTER_INFO = 60700,
	PQ_VIRTUAL_GET_IRQ,
	PQ_VIRTUAL_WAIT_CRTC_READY,
	PQ_VIRTUAL_GET_PIXEL_TYPE_BY_FENCE,
	PQ_CMD_MAX = 65535,
};

struct drm_mtk_gem_create {
	__u64 size;
	__u32 flags;
	__u32 handle;
};

struct drm_mtk_gem_map_off {
	__u32 handle;
	__u32 pad;
	__u64 offset;
};

struct mml_submit {
	_Bool update;
};

enum MTK_PANEL_ID {
	MTK_PANEL_DSI0,
	MTK_PANEL_EDP,
	MTK_PANEL_DP0,
	MTK_PANEL_DP1,
	MTK_PANEL_DSI1_0,
	MTK_PANEL_DSI1_1,
	MTK_PANEL_DSI2,
};

enum MTK_FB_USER_TYPE {
	MTK_USER_NORMAL,
	MTK_USER_AVM,
};

struct mtk_drm_layer_info {
	enum MTK_PANEL_ID panel_id;
	unsigned int layer_id;
	unsigned int layer_en;
	unsigned int src_x;
	unsigned int src_y;
	unsigned int src_w;
	unsigned int src_h;
	unsigned int src_pitch;
	unsigned int crop_w;
	unsigned int crop_h;
	unsigned int tgt_x;
	unsigned int tgt_y;
	unsigned int tgt_w;
	unsigned int tgt_h;
	unsigned int src_format;
	unsigned long long pts;

	void *src_vaddr;
	__u64 src_mvaddr;
	int dma_fd;
	_Bool compress;
	enum MTK_FB_USER_TYPE user_type;
};

struct mtk_drm_dma_buf {
	__s32 fd;
	__u64 mva;
};

enum MTK_DRM_ODDMR_CTL_CMD {
	MTK_DRM_ODDMR_OD_INIT = 0,
	MTK_DRM_ODDMR_OD_ENABLE = 1,
	MTK_DRM_ODDMR_OD_DISABLE = 2,
	MTK_DRM_ODDMR_DMR_INIT = 3,
	MTK_DRM_ODDMR_DMR_ENABLE = 4,
	MTK_DRM_ODDMR_DMR_DISABLE = 5,
	MTK_DRM_ODDMR_LOAD_PARAM = 6,
	MTK_DRM_ODDMR_OD_READ_SW_REG = 7,
	MTK_DRM_ODDMR_OD_WRITE_SW_REG = 8,
	MTK_DRM_ODDMR_OD_USER_GAIN = 9,
	MTK_DRM_ODDMR_REMAP_ENABLE = 10,
	MTK_DRM_ODDMR_REMAP_DISABLE = 11,
	MTK_DRM_ODDMR_REMAP_TARGET = 12,
	MTK_DRM_ODDMR_DBI_NORMAL_COUNTING_CTL = 13,
	MTK_DRM_ODDMR_DBI_IRDROP_ENABLE = 14,
	MTK_DRM_ODDMR_DBI_SET_GAIN_RATIO = 15,
	MTK_DRM_ODDMR_DMR_BINSET_INIT = 16,
	MTK_DRM_ODDMR_DMR_BINSET_CHG = 17,
	MTK_DRM_ODDMR_REG_TUNING_INIT = 19,
	MTK_DRM_ODDMR_REG_TUNING_ENABLE = 20,
};

struct mtk_drm_oddmr_ctl {
	enum MTK_DRM_ODDMR_CTL_CMD cmd;
	__u32 size;
	__u8 *data;
};

struct mtk_drm_oddmr_param {
	__u32 head_id;
	__u32 size;
	__u8 *data;
	__u32 checksum;
};

struct mtk_drm_mml_caps_info {
	__u32 mode_caps;
	__u32 hw_caps;
};

struct mtk_drm_mml_query_hw_support {
	void *info;
	_Bool support;
};

enum mtk_drm_mml_func {
	mtk_drm_mml_func_unknown,
	mtk_drm_mml_func_get_caps,
	mtk_drm_mml_func_query_hw_support,
};

struct mtk_drm_mml_ctrl {
	__u32 func;
	union {
		struct mtk_drm_mml_caps_info caps;
		struct mtk_drm_mml_query_hw_support query;
	};
};

struct DISP_WRITE_REG {
	unsigned int reg;
	unsigned int val;
	unsigned int mask;
};

struct DISP_READ_REG {
	unsigned int reg;
	unsigned int val;
	unsigned int mask;
};

#define DISP_COLOR_SWREG_START          (0xFFFF0000)
#define DISP_COLOR_SWREG_COLOR_BASE     (DISP_COLOR_SWREG_START)
#define DISP_COLOR_SWREG_TDSHP_BASE     (DISP_COLOR_SWREG_COLOR_BASE + 0x1000)
#define DISP_COLOR_SWREG_PQDC_BASE      (DISP_COLOR_SWREG_TDSHP_BASE + 0x1000)
#define DISP_COLOR_SWREG_PQDS_BASE      (DISP_COLOR_SWREG_PQDC_BASE + 0x1000)
#define DISP_COLOR_SWREG_MDP_COLOR_BASE (DISP_COLOR_SWREG_PQDS_BASE + 0x1000)
#define DISP_COLOR_SWREG_END            (DISP_COLOR_SWREG_MDP_COLOR_BASE + 0x1000)

#define SWREG_COLOR_BASE_ADDRESS        (DISP_COLOR_SWREG_COLOR_BASE + 0x0000)
#define SWREG_GAMMA_BASE_ADDRESS        (DISP_COLOR_SWREG_COLOR_BASE + 0x0001)
#define SWREG_TDSHP_BASE_ADDRESS        (DISP_COLOR_SWREG_COLOR_BASE + 0x0002)
#define SWREG_AAL_BASE_ADDRESS          (DISP_COLOR_SWREG_COLOR_BASE + 0x0003)
#define SWREG_MIRAVISION_VERSION        (DISP_COLOR_SWREG_COLOR_BASE + 0x0004)
#define SWREG_SW_VERSION_VIDEO_DC       (DISP_COLOR_SWREG_COLOR_BASE + 0x0005)
#define SWREG_SW_VERSION_AAL            (DISP_COLOR_SWREG_COLOR_BASE + 0x0006)
#define SWREG_CCORR_BASE_ADDRESS        (DISP_COLOR_SWREG_COLOR_BASE + 0x0007)
#define SWREG_MDP_COLOR_BASE_ADDRESS    (DISP_COLOR_SWREG_COLOR_BASE + 0x0008)
#define SWREG_COLOR_MODE                (DISP_COLOR_SWREG_COLOR_BASE + 0x0009)
#define SWREG_RSZ_BASE_ADDRESS          (DISP_COLOR_SWREG_COLOR_BASE + 0x000A)
#define SWREG_MDP_RDMA_BASE_ADDRESS     (DISP_COLOR_SWREG_COLOR_BASE + 0x000B)
#define SWREG_MDP_AAL_BASE_ADDRESS      (DISP_COLOR_SWREG_COLOR_BASE + 0x000C)
#define SWREG_MDP_HDR_BASE_ADDRESS      (DISP_COLOR_SWREG_COLOR_BASE + 0x000D)
#define SWREG_MML_COLOR_BASE_ADDRESS    (DISP_COLOR_SWREG_COLOR_BASE + 0x000E)
#define SWREG_MML_TDSHP_BASE_ADDRESS    (DISP_COLOR_SWREG_COLOR_BASE + 0x000F)
#define SWREG_DISP_TDSHP_BASE_ADDRESS   (DISP_COLOR_SWREG_COLOR_BASE + 0x0010)
#define SWREG_MML_AAL_BASE_ADDRESS      (DISP_COLOR_SWREG_COLOR_BASE + 0x0011)
#define SWREG_MML_HDR_BASE_ADDRESS      (DISP_COLOR_SWREG_COLOR_BASE + 0x0012)
#define SWREG_TDSHP_TUNING_MODE         (DISP_COLOR_SWREG_TDSHP_BASE + 0x0000)

#define MTK_SUBMIT_NO_IMPLICIT   0x0 /* disable implicit sync */
#define MTK_SUBMIT_IN_FENCE      0x1 /* enable input fence */
#define MTK_SUBMIT_OUT_FENCE     0x2 /* enable output fence */

struct drm_mtk_gem_submit {
	__u32 type;
	/* session */
	__u32 session_id;
	/* layer */
	__u32 layer_id;
	__u32 layer_en;
	/* buffer */
	__u32 fb_id;
	/* output */
	__u32 index;
	__s32 fence_fd;
	__u32 interface_index;
	__s32 interface_fence_fd;
	__s32 ion_fd;
};

#define DRM_IOCTL_MTK_GEM_CREATE	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_GEM_CREATE, struct drm_mtk_gem_create)
#define DRM_IOCTL_MTK_GEM_MAP_OFFSET	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_GEM_MAP_OFFSET, struct drm_mtk_gem_map_off)
#define DRM_IOCTL_MTK_GEM_SUBMIT	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_GEM_SUBMIT, struct drm_mtk_gem_submit)
#define DRM_IOCTL_MTK_SESSION_CREATE	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_SESSION_CREATE, struct drm_mtk_session)
#define DRM_IOCTL_MTK_SESSION_DESTROY	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_SESSION_DESTROY, struct drm_mtk_session)
#define DRM_IOCTL_MTK_LAYERING_RULE	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_LAYERING_RULE, struct drm_mtk_layering_info)
#define DRM_IOCTL_MTK_CRTC_GETFENCE	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_CRTC_GETFENCE, struct drm_mtk_fence)
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
#define DRM_IOCTL_MTK_CRTC_GETSFFENCE	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_CRTC_GETSFFENCE, struct drm_mtk_fence)
#define DRM_IOCTL_MTK_MML_GEM_SUBMIT	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_MML_GEM_SUBMIT, struct mml_submit)
#define DRM_IOCTL_MTK_SET_MSYNC_PARAMS	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_SET_MSYNC_PARAMS, struct msync_parameter_table)
#define DRM_IOCTL_MTK_GET_MSYNC_PARAMS	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_GET_MSYNC_PARAMS, struct msync_parameter_table)
#define DRM_IOCTL_MTK_FACTORY_LCM_AUTO_TEST	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_FACTORY_LCM_AUTO_TEST, int)
#define DRM_IOCTL_MTK_DRM_SET_LEASE_INFO	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_DRM_SET_LEASE_INFO, int)
#define DRM_IOCTL_MTK_DRM_GET_LEASE_INFO	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_DRM_GET_LEASE_INFO, int)
#define DRM_IOCTL_MTK_CRTC_FENCE_REL	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_CRTC_FENCE_REL, struct drm_mtk_fence)
#define DRM_IOCTL_MTK_GET_MODE_EXT_INFO	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_GET_MODE_EXT_INFO, struct drm_mtk_mode_ext_info)
#define DRM_IOCTL_MTK_HWVSYNC_ON	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_HWVSYNC_ON, unsigned int)
#define DRM_IOCTL_MTK_DUMMY_CMD_ON	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_DUMMY_CMD_ON, unsigned int)
#define DRM_IOCTL_MTK_GET_LCM_INDEX	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_GET_LCM_INDEX, unsigned int)
#define DRM_IOCTL_MTK_SUPPORT_COLOR_TRANSFORM DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_SUPPORT_COLOR_TRANSFORM, struct DISP_COLOR_TRANSFORM)
#define DRM_IOCTL_MTK_PQ_PROXY_IOCTL	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_PQ_PROXY_IOCTL, struct mtk_drm_pq_proxy_ctl)
#define DRM_IOCTL_MTK_DEBUG_LOG		DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_DEBUG_LOG, int)
#define DRM_IOCTL_MTK_GET_CHIST		DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_GET_CHIST, struct drm_mtk_chist_info)
#define DRM_IOCTL_MTK_GET_CHIST_CAPS	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_GET_CHIST_CAPS, struct drm_mtk_chist_caps)
#define DRM_IOCTL_MTK_SET_CHIST_CONFIG	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_SET_CHIST_CONFIG, struct drm_mtk_chist_config)
#define DRM_IOCTL_MTK_SET_OVL_LAYER	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_SET_OVL_LAYER, struct mtk_drm_layer_info)
#define DRM_IOCTL_MTK_MAP_DMA_BUF	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_MAP_DMA_BUF, struct mtk_drm_dma_buf)
#define DRM_IOCTL_MTK_UNMAP_DMA_BUF	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_UNMAP_DMA_BUF, int)
#define DRM_IOCTL_MTK_GET_PQ_CAPS	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_GET_PQ_CAPS, struct mtk_drm_pq_caps_info)
#define DRM_IOCTL_MTK_SET_PQ_CAPS	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_SET_PQ_CAPS, struct mtk_drm_pq_caps_info)
#define DRM_IOCTL_MTK_ODDMR_LOAD_PARAM	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_ODDMR_LOAD_PARAM, struct mtk_drm_oddmr_param)
#define DRM_IOCTL_MTK_ODDMR_CTL		DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_ODDMR_CTL, struct mtk_drm_oddmr_ctl)
#define DRM_IOCTL_MTK_GET_PANELS_INFO	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_GET_PANELS_INFO, struct mtk_drm_panels_info)
#define DRM_IOCTL_MTK_KICK_IDLE		DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_KICK_IDLE, unsigned int)
#define DRM_IOCTL_MTK_PQ_FRAME_CONFIG	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_PQ_FRAME_CONFIG, struct mtk_drm_pq_config_ctl)
#define DRM_IOCTL_MTK_ESD_STAT_CHK	DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_ESD_STAT_CHK, unsigned int)
#define DRM_IOCTL_MTK_MML_CTRL		DRM_IOWR(DRM_COMMAND_BASE + \
		DRM_MTK_MML_CTRL, struct mtk_drm_mml_ctrl)

#endif /* _UAPI_MEDIATEK_DRM_H */
