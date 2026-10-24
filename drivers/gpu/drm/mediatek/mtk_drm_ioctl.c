// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2024 MediaTek Inc.
 * Copyright (C) 2026 Badmaneers <dukebraham24@gmail.com>
 */

#include <linux/uaccess.h>
#include <linux/dma-fence.h>
#include <linux/sync_file.h>
#include <linux/file.h>
#include <linux/wait.h>
#include <linux/vmalloc.h>
#include <linux/atomic.h>
#include <drm/drm_atomic.h>
#include <drm/drm_connector.h>
#include <drm/drm_crtc.h>
#include <drm/drm_device.h>
#include <drm/drm_file.h>
#include <drm/drm_modes.h>
#include <drm/drm_auth.h>

#include "mtk_drm_ioctl.h"

static atomic_t g_fence_seq = ATOMIC_INIT(0);
static DECLARE_WAIT_QUEUE_HEAD(g_repaint_wq);
static int g_repaint_condition;

int mtk_drm_ioctl_get_all_connector_panel_info(struct drm_device *dev, void *data,
					       struct drm_file *file_priv)
{
	struct mtk_drm_panels_info *panel_ctx = data;
	struct drm_connector *connector;
	struct drm_connector_list_iter conn_iter;
	struct drm_crtc *crtc;
	int conn_cnt = 0;
	int default_conn_id = 0;
	int first_crtc_id = 0;
	bool check_only;

	if (!dev || !panel_ctx)
		return -EINVAL;

	/* Find first CRTC ID */
	drm_for_each_crtc(crtc, dev) {
		first_crtc_id = crtc->base.id;
		break;
	}

	/* Count valid connectors and pick default */
	drm_connector_list_iter_begin(dev, &conn_iter);
	drm_for_each_connector_iter(connector, &conn_iter) {
		if (connector->connector_type == DRM_MODE_CONNECTOR_DSI ||
		    connector->connector_type == DRM_MODE_CONNECTOR_eDP ||
		    connector->connector_type == DRM_MODE_CONNECTOR_DPI) {
			if (!default_conn_id)
				default_conn_id = connector->base.id;
			conn_cnt++;
		}
	}
	drm_connector_list_iter_end(&conn_iter);

	if (conn_cnt == 0) {
		drm_connector_list_iter_begin(dev, &conn_iter);
		drm_for_each_connector_iter(connector, &conn_iter) {
			default_conn_id = connector->base.id;
			conn_cnt = 1;
			break;
		}
		drm_connector_list_iter_end(&conn_iter);
	}

	check_only = (panel_ctx->connector_cnt == -1);

	if (check_only) {
		panel_ctx->connector_cnt = conn_cnt;
		panel_ctx->default_connector_id = default_conn_id;
		return 0;
	}

	/* Populate array data for userspace */
	if (panel_ctx->connector_cnt > 0) {
		int i = 0;
		int limit = min_t(int, panel_ctx->connector_cnt, conn_cnt);

		drm_connector_list_iter_begin(dev, &conn_iter);
		drm_for_each_connector_iter(connector, &conn_iter) {
			if (i >= limit)
				break;

			/* connector_obj_id */
			if (panel_ctx->connector_obj_id) {
				unsigned int id = connector->base.id;

				if (copy_to_user(&panel_ctx->connector_obj_id[i], &id, sizeof(id))) {
					drm_connector_list_iter_end(&conn_iter);
					return -EFAULT;
				}
			}

			/* possible_crtc */
			if (panel_ctx->possible_crtc) {
				unsigned int __user *u_crtc_arr;
				unsigned int crtcs[MAX_CRTC_CNT] = {0};

				crtcs[0] = first_crtc_id;
				if (copy_from_user(&u_crtc_arr, &panel_ctx->possible_crtc[i], sizeof(u_crtc_arr))) {
					drm_connector_list_iter_end(&conn_iter);
					return -EFAULT;
				}
				if (u_crtc_arr && copy_to_user(u_crtc_arr, crtcs, sizeof(crtcs))) {
					drm_connector_list_iter_end(&conn_iter);
					return -EFAULT;
				}
			}

			/* panel_name */
			if (panel_ctx->panel_name) {
				char __user *u_name_buf;
				char name[GET_PANELS_STR_LEN] = "even-boe-nt36525b";

				if (copy_from_user(&u_name_buf, &panel_ctx->panel_name[i], sizeof(u_name_buf))) {
					drm_connector_list_iter_end(&conn_iter);
					return -EFAULT;
				}
				if (u_name_buf && copy_to_user(u_name_buf, name, sizeof(name))) {
					drm_connector_list_iter_end(&conn_iter);
					return -EFAULT;
				}
			}

			/* dsi_mode: 1 for video mode, 0 for cmd mode */
			if (panel_ctx->dsi_mode) {
				unsigned int mode_val = 1;

				if (copy_to_user(&panel_ctx->dsi_mode[i], &mode_val, sizeof(mode_val))) {
					drm_connector_list_iter_end(&conn_iter);
					return -EFAULT;
				}
			}

			i++;
		}
		drm_connector_list_iter_end(&conn_iter);
	}

	return 0;
}

int mtk_drm_ioctl_get_session_info(struct drm_device *dev, void *data,
				   struct drm_file *file_priv)
{
	struct drm_mtk_session_info *info = data;
	struct drm_connector *connector;
	struct drm_connector_list_iter conn_iter;
	struct drm_display_mode *mode;

	if (!dev || !info)
		return -EINVAL;

	info->vsyncFPS = 60;
	info->physical_width = 720;
	info->physical_height = 1600;
	info->physicalWidthUm = 68000;
	info->physicalHeightUm = 152000;

	drm_connector_list_iter_begin(dev, &conn_iter);
	drm_for_each_connector_iter(connector, &conn_iter) {
		list_for_each_entry(mode, &connector->modes, head) {
			if (mode->hdisplay && mode->vdisplay) {
				info->physical_width = mode->hdisplay;
				info->physical_height = mode->vdisplay;
				info->vsyncFPS = drm_mode_vrefresh(mode);
				if (connector->display_info.width_mm)
					info->physicalWidthUm = connector->display_info.width_mm * 1000;
				if (connector->display_info.height_mm)
					info->physicalHeightUm = connector->display_info.height_mm * 1000;
				break;
			}
		}
		break;
	}
	drm_connector_list_iter_end(&conn_iter);

	return 0;
}

int mtk_drm_ioctl_session_create(struct drm_device *dev, void *data,
				 struct drm_file *file_priv)
{
	struct drm_mtk_session *session = data;

	if (!session)
		return -EINVAL;

	session->session_id = MAKE_MTK_SESSION(session->type, session->device_id);
	return 0;
}

int mtk_drm_ioctl_session_destroy(struct drm_device *dev, void *data,
				  struct drm_file *file_priv)
{
	return 0;
}

int mtk_drm_ioctl_set_ddp_mode(struct drm_device *dev, void *data,
			       struct drm_file *file_priv)
{
	return 0;
}

int mtk_drm_ioctl_hwvsync_on(struct drm_device *dev, void *data,
			     struct drm_file *file_priv)
{
	return 0;
}

int mtk_drm_ioctl_kick_idle(struct drm_device *dev, void *data,
			    struct drm_file *file_priv)
{
	return 0;
}

int mtk_drm_ioctl_get_display_caps(struct drm_device *dev, void *data,
				   struct drm_file *file_priv)
{
	struct mtk_drm_disp_caps_info *caps_info = data;

	if (!caps_info)
		return -EINVAL;

	memset(caps_info, 0, sizeof(*caps_info));
	caps_info->hw_ver = 0x6768; /* MT6768 */
	caps_info->disp_feature_flag = DRM_DISP_FEATURE_HRT |
				       DRM_DISP_FEATURE_DISP_SELF_REFRESH |
				       DRM_DISP_FEATURE_FBDC;
	caps_info->lcm_color_mode = 0; /* NATIVE */
	return 0;
}

int mtk_drm_ioctl_layering_rule(struct drm_device *dev, void *data,
				struct drm_file *file_priv)
{
	struct drm_mtk_layering_info *info = data;
	static unsigned int s_hrt_idx;
	int i;

	if (!info)
		return -EINVAL;

	info->hrt_num = 0; /* HRT level 0 (valid) */
	info->hrt_weight = 400;
	info->hrt_idx = ++s_hrt_idx;

	for (i = 0; i < LYE_CRTC; i++) {
		info->gles_head[i] = -1;
		info->gles_tail[i] = -1;
		info->disp_caps[i] = 0;

		if (info->layer_num[i] > 0 && info->input_config[i]) {
			struct drm_mtk_layer_config *cfg;
			int l;
			size_t sz = sizeof(struct drm_mtk_layer_config) * info->layer_num[i];

			cfg = vmalloc(sz);
			if (!cfg)
				continue;

			if (copy_from_user(cfg, info->input_config[i], sz) == 0) {
				if (info->layer_num[i] > 4) {
					info->gles_head[i] = 3;
					info->gles_tail[i] = info->layer_num[i] - 1;
				}
				for (l = 0; l < info->layer_num[i]; l++) {
					if (info->layer_num[i] > 4 && l >= 3) {
						cfg[l].ovl_id = -1;
						cfg[l].layer_caps = 0;
					} else {
						cfg[l].ovl_id = l;
						cfg[l].layer_caps |= MTK_LAYERING_OVL_ONLY;
					}
				}
				if (copy_to_user(info->input_config[i], cfg, sz)) {
					vfree(cfg);
					return -EFAULT;
				}
			}
			vfree(cfg);
		}
	}

	return 0;
}

int mtk_drm_ioctl_crtc_getfence(struct drm_device *dev, void *data,
				struct drm_file *file_priv)
{
	struct drm_mtk_fence *args = data;
	int fd;

	if (!args)
		return -EINVAL;

	fd = get_unused_fd_flags(O_CLOEXEC);
	if (fd >= 0) {
		struct dma_fence *fence = dma_fence_get_stub();
		struct sync_file *sync_file = sync_file_create(fence);

		dma_fence_put(fence);
		if (sync_file) {
			fd_install(fd, sync_file->file);
			args->fence_fd = fd;
		} else {
			put_unused_fd(fd);
			args->fence_fd = -1;
		}
	} else {
		args->fence_fd = -1;
	}

	args->fence_idx = atomic_inc_return(&g_fence_seq);
	return 0;
}

int mtk_drm_ioctl_crtc_fence_release(struct drm_device *dev, void *data,
				     struct drm_file *file_priv)
{
	return 0;
}

int mtk_drm_ioctl_crtc_get_sf_fence(struct drm_device *dev, void *data,
				    struct drm_file *file_priv)
{
	return mtk_drm_ioctl_crtc_getfence(dev, data, file_priv);
}

void drm_trigger_repaint(enum DRM_REPAINT_TYPE type, struct drm_device *drm_dev)
{
	g_repaint_condition = 1;
	wake_up_interruptible(&g_repaint_wq);
}

int mtk_drm_ioctl_wait_repaint(struct drm_device *dev, void *data,
			       struct drm_file *file_priv)
{
	unsigned int *type = data;
	int ret;

	if (type)
		*type = 0;

	ret = wait_event_interruptible(g_repaint_wq, g_repaint_condition != 0);
	g_repaint_condition = 0;

	return (ret == -ERESTARTSYS) ? -ERESTARTSYS : 0;
}

int mtk_drm_ioctl_sec_hnd_to_gem_hnd(struct drm_device *dev, void *data,
				     struct drm_file *file_priv)
{
	struct drm_mtk_sec_gem_hnd *hnd = data;

	if (hnd)
		hnd->gem_hnd = hnd->sec_hnd;
	return 0;
}

int mtk_drm_ioctl_get_master_info(struct drm_device *dev, void *data,
				  struct drm_file *file_priv)
{
	int *is_master = data;

	if (is_master)
		*is_master = drm_is_current_master(file_priv);
	return 0;
}

int mtk_drm_ioctl_set_msync_params(struct drm_device *dev, void *data,
				   struct drm_file *file_priv)
{
	return 0;
}

int mtk_drm_ioctl_get_msync_params(struct drm_device *dev, void *data,
				   struct drm_file *file_priv)
{
	return 0;
}

int mtk_drm_ioctl_get_mode_ext_info(struct drm_device *dev, void *data,
				    struct drm_file *file_priv)
{
	return 0;
}

int mtk_drm_ioctl_get_lcm_index(struct drm_device *dev, void *data,
				struct drm_file *file_priv)
{
	unsigned int *idx = data;

	if (idx)
		*idx = 0;
	return 0;
}

int mtk_drm_ioctl_support_color_transform(struct drm_device *dev, void *data,
					  struct drm_file *file_priv)
{
	return 0;
}

int mtk_drm_ioctl_debug_log(struct drm_device *dev, void *data,
			    struct drm_file *file_priv)
{
	return 0;
}

int mtk_drm_ioctl_get_chist(struct drm_device *dev, void *data,
			    struct drm_file *file_priv)
{
	return 0;
}

int mtk_drm_ioctl_get_chist_caps(struct drm_device *dev, void *data,
				 struct drm_file *file_priv)
{
	return 0;
}

int mtk_drm_ioctl_set_chist_config(struct drm_device *dev, void *data,
				   struct drm_file *file_priv)
{
	return 0;
}

int mtk_drm_ioctl_get_pq_caps(struct drm_device *dev, void *data,
			      struct drm_file *file_priv)
{
	return 0;
}

int mtk_drm_ioctl_set_pq_caps(struct drm_device *dev, void *data,
			      struct drm_file *file_priv)
{
	return 0;
}

const struct drm_ioctl_desc mtk_ioctls[MTK_NUM_IOCTLS] = {
	DRM_IOCTL_DEF_DRV(MTK_SESSION_CREATE, mtk_drm_ioctl_session_create, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_SESSION_DESTROY, mtk_drm_ioctl_session_destroy, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_LAYERING_RULE, mtk_drm_ioctl_layering_rule, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_CRTC_GETFENCE, mtk_drm_ioctl_crtc_getfence, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_CRTC_FENCE_REL, mtk_drm_ioctl_crtc_fence_release, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_CRTC_GETSFFENCE, mtk_drm_ioctl_crtc_get_sf_fence, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_WAIT_REPAINT, mtk_drm_ioctl_wait_repaint, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_GET_DISPLAY_CAPS, mtk_drm_ioctl_get_display_caps, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_SET_DDP_MODE, mtk_drm_ioctl_set_ddp_mode, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_GET_SESSION_INFO, mtk_drm_ioctl_get_session_info, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_SEC_HND_TO_GEM_HND, mtk_drm_ioctl_sec_hnd_to_gem_hnd, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_GET_MASTER_INFO, mtk_drm_ioctl_get_master_info, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_SET_MSYNC_PARAMS, mtk_drm_ioctl_set_msync_params, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_GET_MSYNC_PARAMS, mtk_drm_ioctl_get_msync_params, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_GET_MODE_EXT_INFO, mtk_drm_ioctl_get_mode_ext_info, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_HWVSYNC_ON, mtk_drm_ioctl_hwvsync_on, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_GET_LCM_INDEX, mtk_drm_ioctl_get_lcm_index, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_SUPPORT_COLOR_TRANSFORM, mtk_drm_ioctl_support_color_transform, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_DEBUG_LOG, mtk_drm_ioctl_debug_log, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_GET_CHIST, mtk_drm_ioctl_get_chist, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_GET_CHIST_CAPS, mtk_drm_ioctl_get_chist_caps, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_SET_CHIST_CONFIG, mtk_drm_ioctl_set_chist_config, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_GET_PQ_CAPS, mtk_drm_ioctl_get_pq_caps, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_SET_PQ_CAPS, mtk_drm_ioctl_set_pq_caps, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_GET_PANELS_INFO, mtk_drm_ioctl_get_all_connector_panel_info, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_KICK_IDLE, mtk_drm_ioctl_kick_idle, DRM_RENDER_ALLOW),
};
