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
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/atomic.h>
#include <linux/dma-buf.h>
#include <drm/drm_gem.h>
#include <drm/drm_atomic.h>
#include <drm/drm_connector.h>
#include <drm/drm_crtc.h>
#include <drm/drm_device.h>
#include <drm/drm_file.h>
#include <drm/drm_modes.h>
#include <drm/drm_auth.h>
#include <drm/drm_print.h>

#include "mtk_crtc.h"
#include "mtk_sync.h"
#include "mtk_drm_ioctl.h"

struct repaint_job_t {
	struct list_head link;
	unsigned int type;
};

static struct {
	wait_queue_head_t wq;
	struct list_head job_queue;
	struct list_head job_pool;
	spinlock_t lock;
	bool initialized;
} g_repaint_data = {
	.job_queue = LIST_HEAD_INIT(g_repaint_data.job_queue),
	.job_pool = LIST_HEAD_INIT(g_repaint_data.job_pool),
};

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

#define MTK_MAX_LAYERS 8
static struct sync_timeline *g_layer_timelines[MTK_MAX_LAYERS];
static atomic_t g_layer_fence_idx[MTK_MAX_LAYERS];
static DEFINE_MUTEX(g_gem_submit_lock);

int mtk_drm_ioctl_gem_submit(struct drm_device *dev, void *data,
			     struct drm_file *file_priv)
{
	struct drm_mtk_gem_submit *args = data;
	struct fence_data fence;
	unsigned int layer_id;
	int ret;

	if (!args)
		return -EINVAL;

	if (args->type == MTK_SUBMIT_OUT_FENCE)
		layer_id = 4;
	else
		layer_id = min_t(unsigned int, args->layer_id, MTK_MAX_LAYERS - 1);

	if (args->layer_en) {
		mutex_lock(&g_gem_submit_lock);
		if (!g_layer_timelines[layer_id]) {
			char name[32];
			snprintf(name, sizeof(name), "layer_%u", layer_id);
			g_layer_timelines[layer_id] = mtk_sync_timeline_create(name);
			atomic_set(&g_layer_fence_idx[layer_id], 0);
		}

		if (!g_layer_timelines[layer_id]) {
			mutex_unlock(&g_gem_submit_lock);
			args->fence_fd = -1;
			args->index = 0;
			return -ENOMEM;
		}

		/* Signal the previous buffer release fence for this layer */
		if (g_layer_timelines[layer_id]->value < atomic_read(&g_layer_fence_idx[layer_id])) {
			int inc = atomic_read(&g_layer_fence_idx[layer_id]) - g_layer_timelines[layer_id]->value;
			mtk_sync_timeline_inc(g_layer_timelines[layer_id], inc, ktime_get());
		}

		/* Create a new release fence for the current buffer */
		fence.fence = -1;
		fence.value = atomic_inc_return(&g_layer_fence_idx[layer_id]);
		snprintf(fence.name, sizeof(fence.name), "rel_fence_%u_%u", layer_id, fence.value);

		ret = mtk_sync_fence_create(g_layer_timelines[layer_id], &fence);
		mutex_unlock(&g_gem_submit_lock);

		if (ret == 0) {
			args->fence_fd = fence.fence;
			args->index = fence.value;
		} else {
			args->fence_fd = -1;
			args->index = 0;
		}
	} else {
		args->fence_fd = -1;
		args->index = 0;
	}

	return 0;
}

void mtk_drm_release_all_layer_fences(void)
{
	int i;

	mutex_lock(&g_gem_submit_lock);
	for (i = 0; i < MTK_MAX_LAYERS; i++) {
		if (g_layer_timelines[i]) {
			int target = atomic_read(&g_layer_fence_idx[i]);
			int inc = target - g_layer_timelines[i]->value;

			if (inc > 0)
				mtk_sync_timeline_inc(g_layer_timelines[i], inc, ktime_get());
		}
	}
	mutex_unlock(&g_gem_submit_lock);
}
EXPORT_SYMBOL(mtk_drm_release_all_layer_fences);

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
	mtk_drm_release_all_layer_fences();
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
	unsigned int *crtc_id = data;
	struct drm_crtc *crtc;

	if (!dev || !crtc_id)
		return -EINVAL;

	crtc = drm_crtc_find(dev, file_priv, *crtc_id);
	if (!crtc) {
		drm_for_each_crtc(crtc, dev)
			break;
	}
	if (!crtc)
		return -ENOENT;

	mtk_crtc_set_hwvsync_en(crtc, true);
	return 0;
}

int mtk_drm_ioctl_kick_idle(struct drm_device *dev, void *data,
			    struct drm_file *file_priv)
{
	unsigned int *crtc_id = data;
	struct drm_crtc *crtc;

	if (!dev || !crtc_id)
		return -EINVAL;

	crtc = drm_crtc_find(dev, file_priv, *crtc_id);
	if (!crtc) {
		drm_for_each_crtc(crtc, dev)
			break;
	}
	if (!crtc)
		return -ENOENT;

	drm_trigger_repaint(DRM_REPAINT_FOR_IDLE, dev);
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
	caps_info->disp_feature_flag = DRM_DISP_FEATURE_DISP_SELF_REFRESH |
				       DRM_DISP_FEATURE_HRT |
				       DRM_DISP_FEATURE_FBDC |
				       DRM_DISP_FEATURE_VIRUTAL_DISPLAY |
				       DRM_DISP_FEATURE_IOMMU |
				       DRM_DISP_FEATURE_SF_PRESENT_FENCE;
	caps_info->lcm_degree = 0;
	caps_info->lcm_color_mode = 0; /* NATIVE */
	caps_info->min_luminance = 0;
	caps_info->average_luminance = 200;
	caps_info->max_luminance = 450;
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
	struct drm_crtc *crtc;
	struct sync_timeline *timeline;
	atomic_t *fence_idx_ptr;
	struct fence_data fence;
	int ret;

	if (!dev || !args)
		return -EINVAL;

	crtc = drm_crtc_find(dev, file_priv, args->crtc_id);
	if (!crtc) {
		drm_for_each_crtc(crtc, dev)
			break;
	}
	if (!crtc)
		return -ENOENT;

	timeline = mtk_crtc_get_present_timeline(crtc);
	fence_idx_ptr = mtk_crtc_get_present_fence_idx(crtc);
	if (!timeline || !fence_idx_ptr)
		return -EFAULT;

	fence.fence = -1;
	fence.value = atomic_inc_return(fence_idx_ptr);
	snprintf(fence.name, sizeof(fence.name), "pres_fence_%d", fence.value);

	ret = mtk_sync_fence_create(timeline, &fence);
	if (ret) {
		drm_err(dev, "create Fence Object failed: %d\n", ret);
		return -EFAULT;
	}

	args->fence_fd = fence.fence;
	args->fence_idx = fence.value;
	return 0;
}

int mtk_drm_ioctl_crtc_fence_release(struct drm_device *dev, void *data,
				     struct drm_file *file_priv)
{
	struct drm_mtk_fence *args = data;
	struct drm_crtc *crtc;

	if (!dev || !args)
		return -EINVAL;

	crtc = drm_crtc_find(dev, file_priv, args->crtc_id);
	if (!crtc) {
		drm_for_each_crtc(crtc, dev)
			break;
	}
	if (crtc)
		mtk_crtc_release_present_fence(crtc);

	return 0;
}

int mtk_drm_ioctl_crtc_get_sf_fence(struct drm_device *dev, void *data,
				    struct drm_file *file_priv)
{
	return mtk_drm_ioctl_crtc_getfence(dev, data, file_priv);
}

void drm_trigger_repaint(enum DRM_REPAINT_TYPE type, struct drm_device *drm_dev)
{
	struct repaint_job_t *repaint_job;
	unsigned long flags;

	if (type > DRM_WAIT_FOR_REPAINT && type < DRM_REPAINT_TYPE_NUM) {
		if (!g_repaint_data.initialized) {
			init_waitqueue_head(&g_repaint_data.wq);
			spin_lock_init(&g_repaint_data.lock);
			INIT_LIST_HEAD(&g_repaint_data.job_queue);
			INIT_LIST_HEAD(&g_repaint_data.job_pool);
			g_repaint_data.initialized = true;
		}

		spin_lock_irqsave(&g_repaint_data.lock, flags);
		if (!list_empty(&g_repaint_data.job_pool)) {
			repaint_job = list_first_entry(&g_repaint_data.job_pool,
						       struct repaint_job_t, link);
			list_del_init(&repaint_job->link);
		} else {
			repaint_job = kzalloc(sizeof(*repaint_job), GFP_ATOMIC);
			if (!repaint_job) {
				spin_unlock_irqrestore(&g_repaint_data.lock, flags);
				return;
			}
			INIT_LIST_HEAD(&repaint_job->link);
		}

		repaint_job->type = (unsigned int)type;
		list_add_tail(&repaint_job->link, &g_repaint_data.job_queue);
		spin_unlock_irqrestore(&g_repaint_data.lock, flags);

		wake_up_interruptible(&g_repaint_data.wq);
	}
}

int mtk_drm_ioctl_wait_repaint(struct drm_device *dev, void *data,
			       struct drm_file *file_priv)
{
	unsigned int *type = data;
	struct repaint_job_t *repaint_job;
	unsigned long flags;
	int ret;

	if (!type)
		return -EINVAL;

	if (!g_repaint_data.initialized) {
		init_waitqueue_head(&g_repaint_data.wq);
		spin_lock_init(&g_repaint_data.lock);
		INIT_LIST_HEAD(&g_repaint_data.job_queue);
		INIT_LIST_HEAD(&g_repaint_data.job_pool);
		g_repaint_data.initialized = true;
	}

	ret = wait_event_interruptible(g_repaint_data.wq,
				       !list_empty(&g_repaint_data.job_queue));
	if (ret >= 0) {
		spin_lock_irqsave(&g_repaint_data.lock, flags);
		if (!list_empty(&g_repaint_data.job_queue)) {
			repaint_job = list_first_entry(&g_repaint_data.job_queue,
						       struct repaint_job_t, link);
			*type = repaint_job->type;
			list_del_init(&repaint_job->link);
			list_add_tail(&repaint_job->link, &g_repaint_data.job_pool);
		} else {
			*type = DRM_WAIT_FOR_REPAINT;
		}
		spin_unlock_irqrestore(&g_repaint_data.lock, flags);
	} else {
		*type = DRM_WAIT_FOR_REPAINT;
	}

	return 0;
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
	struct msync_parameter_table *config = data;

	if (!config)
		return -EFAULT;

	config->msync_max_fps = 60;
	config->msync_min_fps = 60;
	config->msync_level_num = 0;
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
	struct DISP_COLOR_TRANSFORM *color_transform = data;
	int i;

	if (!color_transform)
		return -EFAULT;

	for (i = 0; i < 3; i++) {
		if (color_transform->matrix[i][3] != 0)
			return -EFAULT;
	}

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
	struct drm_mtk_chist_caps *caps_info = data;
	int i;

	if (!caps_info)
		return -EINVAL;

	caps_info->lcm_width = 720;
	caps_info->lcm_height = 1600;
	caps_info->support_color = 0;
	for (i = 0; i < MTK_DRM_DISP_CHIST_CHANNEL_COUNT; i++) {
		caps_info->chist_config[i].channel_id = i;
		caps_info->chist_config[i].bin_count = 256;
	}
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
	struct mtk_drm_pq_caps_info *pq_info = data;

	if (!pq_info)
		return -EINVAL;

	pq_info->ccorr_caps.ccorr_bit = 12;
	pq_info->ccorr_caps.ccorr_number = 1;
	pq_info->ccorr_caps.ccorr_linear = 1;
	return 0;
}

int mtk_drm_ioctl_set_pq_caps(struct drm_device *dev, void *data,
			      struct drm_file *file_priv)
{
	return 0;
}

#define COLOR_BASE_ADDR 0x1400f000
#define GAMMA_BASE_ADDR 0x14012000
#define AAL_BASE_ADDR   0x14011000
#define CCORR_BASE_ADDR 0x14010000

static int disp_pq_proxy_virtual_sw_read(struct drm_crtc *crtc, void *data)
{
	struct DISP_READ_REG *rParams = data;
	unsigned int ret = 0;
	unsigned int reg_id = rParams->reg;

	switch (reg_id) {
	case SWREG_COLOR_BASE_ADDRESS:
		ret = COLOR_BASE_ADDR;
		break;
	case SWREG_GAMMA_BASE_ADDRESS:
		ret = GAMMA_BASE_ADDR;
		break;
	case SWREG_AAL_BASE_ADDRESS:
		ret = AAL_BASE_ADDR;
		break;
	case SWREG_CCORR_BASE_ADDRESS:
		ret = CCORR_BASE_ADDR;
		break;
	case SWREG_DISP_TDSHP_BASE_ADDRESS:
		ret = 0;
		break;
	case SWREG_COLOR_MODE:
		ret = 1;
		break;
	case SWREG_TDSHP_TUNING_MODE:
		ret = 0;
		break;
	case SWREG_MIRAVISION_VERSION:
		ret = 0x00030007;
		break;
	case SWREG_SW_VERSION_VIDEO_DC:
		ret = 1;
		break;
	case SWREG_SW_VERSION_AAL:
		ret = 1;
		break;
	default:
		ret = 0;
		break;
	}

	rParams->val = ret;
	return 0;
}

int mtk_drm_ioctl_pq_proxy(struct drm_device *dev, void *data,
			   struct drm_file *file_priv)
{
	struct drm_crtc *crtc;
	struct mtk_drm_pq_proxy_ctl *params = data;
	unsigned int pq_type, cmd;
	char stack_kdata[128];
	char *kdata = stack_kdata;
	int ret = 0;

	if (!params || !params->size || !params->data)
		return -EINVAL;

	crtc = drm_crtc_find(dev, file_priv, params->crtc_id);
	if (!crtc) {
		crtc = list_first_entry_or_null(&dev->mode_config.crtc_list,
						struct drm_crtc, head);
		if (!crtc)
			return -EINVAL;
	}

	if (params->size > sizeof(stack_kdata)) {
		kdata = kmalloc(params->size, GFP_KERNEL);
		if (!kdata)
			return -ENOMEM;
	}

	if (copy_from_user(kdata, (void __user *)params->data, params->size)) {
		if (kdata != stack_kdata)
			kfree(kdata);
		return -EFAULT;
	}

	pq_type = params->cmd >> 16;
	cmd = params->cmd & 0xffff;

	switch (pq_type) {
	case MTK_DISP_PQ_CCORR:
		switch (cmd) {
		case PQ_CCORR_GET_PQ_CAPS: {
			struct mtk_drm_pq_caps_info *pq_info = (struct mtk_drm_pq_caps_info *)kdata;

			if (params->size >= sizeof(struct mtk_drm_pq_caps_info)) {
				pq_info->ccorr_caps.ccorr_bit = 12;
				pq_info->ccorr_caps.ccorr_number = 1;
				pq_info->ccorr_caps.ccorr_linear = 1;
			}
			ret = 0;
			break;
		}
		case PQ_CCORR_SUPPORT_COLOR_MATRIX:
			ret = mtk_drm_ioctl_support_color_transform(dev, kdata, file_priv);
			break;
		case PQ_CCORR_EVENTCTL:
		case PQ_CCORR_SET_CCORR:
		case PQ_CCORR_AIBLD_CV_MODE:
		case PQ_CCORR_GET_IRQ:
		default:
			ret = 0;
			break;
		}
		break;

	case MTK_DISP_PQ_AAL:
		switch (cmd) {
		case PQ_AAL_GET_SIZE: {
			struct {
				unsigned int width;
				unsigned int height;
			} *size_info = (void *)kdata;

			if (params->size >= sizeof(*size_info)) {
				size_info->width = 720;
				size_info->height = 1600;
			}
			ret = 0;
			break;
		}
		default:
			ret = 0;
			break;
		}
		break;

	case MTK_DISP_PQ_C3D:
		switch (cmd) {
		case PQ_C3D_GET_BIN_NUM:
			if (params->size >= sizeof(int))
				*(int *)kdata = 17;
			ret = 0;
			break;
		default:
			ret = 0;
			break;
		}
		break;

	case MTK_DISP_PQ_COLOR:
		switch (cmd) {
		case PQ_COLOR_READ_SW_REG:
			if (params->size >= sizeof(struct DISP_READ_REG))
				ret = disp_pq_proxy_virtual_sw_read(crtc, kdata);
			break;
		default:
			ret = 0;
			break;
		}
		break;

	case MTK_DISP_PQ_GAMMA:
		ret = 0;
		break;

	case MTK_DISP_PQ_TDSHP:
		switch (cmd) {
		case PQ_TDSHP_GET_SIZE: {
			struct {
				unsigned int width;
				unsigned int height;
			} *size_info = (void *)kdata;

			if (params->size >= sizeof(*size_info)) {
				size_info->width = 720;
				size_info->height = 1600;
			}
			ret = 0;
			break;
		}
		default:
			ret = 0;
			break;
		}
		break;

	case MTK_DISP_PQ_DMR:
	case MTK_DISP_PQ_DBI:
		switch (cmd) {
		case PQ_DBI_GET_HW_ID:
			if (params->size >= sizeof(unsigned int))
				*(unsigned int *)kdata = 0x6768;
			ret = 0;
			break;
		case PQ_DBI_GET_WIDTH:
			if (params->size >= sizeof(unsigned int))
				*(unsigned int *)kdata = 720;
			ret = 0;
			break;
		case PQ_DBI_GET_HEIGHT:
			if (params->size >= sizeof(unsigned int))
				*(unsigned int *)kdata = 1600;
			ret = 0;
			break;
		case PQ_DBI_GET_DBV:
			if (params->size >= sizeof(unsigned int))
				*(unsigned int *)kdata = 2047;
			ret = 0;
			break;
		case PQ_DBI_GET_FPS:
			if (params->size >= sizeof(unsigned int))
				*(unsigned int *)kdata = 60;
			ret = 0;
			break;
		default:
			ret = 0;
			break;
		}
		break;

	case MTK_DISP_VIRTUAL_TYPE:
		switch (cmd) {
		case PQ_VIRTUAL_GET_MASTER_INFO:
			if (params->size >= sizeof(int))
				*(int *)kdata = drm_is_current_master(file_priv);
			ret = 0;
			break;
		case PQ_VIRTUAL_WAIT_CRTC_READY:
			if (params->size >= sizeof(int))
				*(int *)kdata = 1;
			ret = 0;
			break;
		case PQ_COLOR_READ_SW_REG:
			if (params->size >= sizeof(struct DISP_READ_REG))
				ret = disp_pq_proxy_virtual_sw_read(crtc, kdata);
			break;
		case PQ_VIRTUAL_SET_PROPERTY:
		case PQ_VIRTUAL_CHECK_TRIGGER:
		case PQ_VIRTUAL_RELAY_ENGINES:
		case PQ_VIRTUAL_GET_PIXEL_TYPE_BY_FENCE:
		case PQ_COLOR_WRITE_SW_REG:
		default:
			ret = 0;
			break;
		}
		break;

	default:
		ret = 0;
		break;
	}

	if (cmd > PQ_GET_CMD_START) {
		if (copy_to_user((void __user *)params->data, kdata, params->size))
			ret = -EFAULT;
	}

	if (kdata != stack_kdata)
		kfree(kdata);

	return ret;
}

int mtk_drm_ioctl_pq_frame_config(struct drm_device *dev, void *data,
				  struct drm_file *file_priv)
{
	return 0;
}

int mtk_drm_ioctl_gem_create(struct drm_device *dev, void *data,
			     struct drm_file *file_priv)
{
	return 0;
}

int mtk_drm_ioctl_gem_map_offset(struct drm_device *dev, void *data,
				 struct drm_file *file_priv)
{
	struct drm_mtk_gem_map_off *args = data;

	if (!args)
		return -EINVAL;

	return drm_gem_dumb_map_offset(file_priv, dev, args->handle, &args->offset);
}

int mtk_drm_ioctl_mml_gem_submit(struct drm_device *dev, void *data,
				 struct drm_file *file_priv)
{
	return -EINVAL;
}

int mtk_drm_ioctl_fm_lcm_auto_test(struct drm_device *dev, void *data,
				   struct drm_file *file_priv)
{
	int *result = data;

	if (result)
		*result = 0;
	return 0;
}

static struct file *g_lease_file;
static DEFINE_MUTEX(g_lease_mutex);

int mtk_drm_ioctl_set_lease_info(struct drm_device *dev, void *data,
				 struct drm_file *file_priv)
{
	int *fd = data;

	if (!fd)
		return -EINVAL;

	mutex_lock(&g_lease_mutex);
	g_lease_file = fget(*fd);
	if (!g_lease_file) {
		mutex_unlock(&g_lease_mutex);
		return -EBADF;
	}
	mutex_unlock(&g_lease_mutex);
	return 0;
}

int mtk_drm_ioctl_get_lease_info(struct drm_device *dev, void *data,
				 struct drm_file *file_priv)
{
	int *ret_fd = data;
	int fd;

	if (!ret_fd)
		return -EINVAL;

	mutex_lock(&g_lease_mutex);
	if (!g_lease_file) {
		mutex_unlock(&g_lease_mutex);
		return -ENOENT;
	}

	fd = get_unused_fd_flags(O_CLOEXEC);
	if (fd < 0) {
		mutex_unlock(&g_lease_mutex);
		return fd;
	}
	fd_install(fd, g_lease_file);
	get_file(g_lease_file);
	*ret_fd = fd;
	mutex_unlock(&g_lease_mutex);
	return 0;
}

int mtk_drm_ioctl_dummy_cmd_on(struct drm_device *dev, void *data,
			       struct drm_file *file_priv)
{
	return 0;
}

int mtk_drm_ioctl_set_ovl_layer(struct drm_device *dev, void *data,
				struct drm_file *file_priv)
{
	return 0;
}

struct mtk_se_dma_map {
	struct list_head list;
	int fd;
	struct dma_buf *dmabuf;
	struct dma_buf_attachment *attach;
	struct sg_table *sgt;
};

static LIST_HEAD(g_dma_map_list);
static DEFINE_MUTEX(g_dma_map_lock);

int mtk_drm_ioctl_map_dma_buf(struct drm_device *dev, void *data,
			      struct drm_file *file_priv)
{
	struct mtk_drm_dma_buf *dma_map = data;
	struct mtk_se_dma_map *map_entry;
	dma_addr_t mva;

	if (!dma_map || dma_map->fd <= 0)
		return -EINVAL;

	map_entry = kmalloc(sizeof(*map_entry), GFP_KERNEL);
	if (!map_entry)
		return -ENOMEM;

	map_entry->fd = dma_map->fd;
	map_entry->dmabuf = dma_buf_get(dma_map->fd);
	if (IS_ERR(map_entry->dmabuf)) {
		kfree(map_entry);
		return PTR_ERR(map_entry->dmabuf);
	}

	map_entry->attach = dma_buf_attach(map_entry->dmabuf, dev->dev);
	if (IS_ERR(map_entry->attach)) {
		dma_buf_put(map_entry->dmabuf);
		kfree(map_entry);
		return PTR_ERR(map_entry->attach);
	}

	map_entry->sgt = dma_buf_map_attachment(map_entry->attach, DMA_BIDIRECTIONAL);
	if (IS_ERR(map_entry->sgt)) {
		dma_buf_detach(map_entry->dmabuf, map_entry->attach);
		dma_buf_put(map_entry->dmabuf);
		kfree(map_entry);
		return PTR_ERR(map_entry->sgt);
	}

	mva = sg_dma_address(map_entry->sgt->sgl);
	dma_map->mva = mva;

	mutex_lock(&g_dma_map_lock);
	list_add_tail(&map_entry->list, &g_dma_map_list);
	mutex_unlock(&g_dma_map_lock);

	return 0;
}

int mtk_drm_ioctl_unmap_dma_buf(struct drm_device *dev, void *data,
				struct drm_file *file_priv)
{
	struct mtk_se_dma_map *map_entry, *next;
	int *fd = data;

	if (!fd)
		return -EINVAL;

	mutex_lock(&g_dma_map_lock);
	list_for_each_entry_safe(map_entry, next, &g_dma_map_list, list) {
		if (*fd == map_entry->fd) {
			list_del_init(&map_entry->list);
			dma_buf_unmap_attachment(map_entry->attach, map_entry->sgt,
						 DMA_BIDIRECTIONAL);
			dma_buf_detach(map_entry->dmabuf, map_entry->attach);
			dma_buf_put(map_entry->dmabuf);
			kfree(map_entry);
			break;
		}
	}
	mutex_unlock(&g_dma_map_lock);
	return 0;
}

int mtk_drm_ioctl_oddmr_load_param(struct drm_device *dev, void *data,
				   struct drm_file *file_priv)
{
	return 0;
}

int mtk_drm_ioctl_oddmr_ctl(struct drm_device *dev, void *data,
			    struct drm_file *file_priv)
{
	struct mtk_drm_oddmr_ctl *param = data;

	if (!param)
		return -EFAULT;

	switch (param->cmd) {
	case MTK_DRM_ODDMR_OD_INIT:
	case MTK_DRM_ODDMR_OD_ENABLE:
	case MTK_DRM_ODDMR_OD_DISABLE:
	case MTK_DRM_ODDMR_DMR_INIT:
	case MTK_DRM_ODDMR_DMR_ENABLE:
	case MTK_DRM_ODDMR_DMR_DISABLE:
	case MTK_DRM_ODDMR_LOAD_PARAM:
	case MTK_DRM_ODDMR_OD_READ_SW_REG:
	case MTK_DRM_ODDMR_OD_WRITE_SW_REG:
	case MTK_DRM_ODDMR_OD_USER_GAIN:
	case MTK_DRM_ODDMR_REMAP_ENABLE:
	case MTK_DRM_ODDMR_REMAP_DISABLE:
	case MTK_DRM_ODDMR_REMAP_TARGET:
	case MTK_DRM_ODDMR_DBI_NORMAL_COUNTING_CTL:
	case MTK_DRM_ODDMR_DBI_IRDROP_ENABLE:
	case MTK_DRM_ODDMR_DBI_SET_GAIN_RATIO:
	case MTK_DRM_ODDMR_DMR_BINSET_INIT:
	case MTK_DRM_ODDMR_DMR_BINSET_CHG:
	case MTK_DRM_ODDMR_REG_TUNING_INIT:
	case MTK_DRM_ODDMR_REG_TUNING_ENABLE:
		return 0;
	default:
		return -EINVAL;
	}
}

int mtk_drm_ioctl_esd_stat_chk(struct drm_device *dev, void *data,
			       struct drm_file *file_priv)
{
	return 0;
}

int mtk_drm_ioctl_mml_ctrl(struct drm_device *dev, void *data,
			   struct drm_file *file_priv)
{
	struct mtk_drm_mml_ctrl *mml_ctrl = data;

	if (!mml_ctrl)
		return -EINVAL;

	switch (mml_ctrl->func) {
	case mtk_drm_mml_func_get_caps:
		mml_ctrl->caps.mode_caps = MTK_MML_DISP_MDP_LAYER;
		mml_ctrl->caps.hw_caps = 0;
		return 0;
	case mtk_drm_mml_func_query_hw_support:
		mml_ctrl->query.support = false;
		return 0;
	default:
		return -EINVAL;
	}
}

const struct drm_ioctl_desc mtk_ioctls[MTK_NUM_IOCTLS] = {
	DRM_IOCTL_DEF_DRV(MTK_GEM_CREATE, mtk_drm_ioctl_gem_create, DRM_AUTH | DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_GEM_MAP_OFFSET, mtk_drm_ioctl_gem_map_offset, DRM_AUTH | DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_GEM_SUBMIT, mtk_drm_ioctl_gem_submit, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_SESSION_CREATE, mtk_drm_ioctl_session_create, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_SESSION_DESTROY, mtk_drm_ioctl_session_destroy, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_LAYERING_RULE, mtk_drm_ioctl_layering_rule, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_CRTC_GETFENCE, mtk_drm_ioctl_crtc_getfence, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_WAIT_REPAINT, mtk_drm_ioctl_wait_repaint, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_GET_DISPLAY_CAPS, mtk_drm_ioctl_get_display_caps, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_SET_DDP_MODE, mtk_drm_ioctl_set_ddp_mode, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_GET_SESSION_INFO, mtk_drm_ioctl_get_session_info, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_SEC_HND_TO_GEM_HND, mtk_drm_ioctl_sec_hnd_to_gem_hnd, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_GET_MASTER_INFO, mtk_drm_ioctl_get_master_info, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_CRTC_GETSFFENCE, mtk_drm_ioctl_crtc_get_sf_fence, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_MML_GEM_SUBMIT, mtk_drm_ioctl_mml_gem_submit, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_SET_MSYNC_PARAMS, mtk_drm_ioctl_set_msync_params, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_GET_MSYNC_PARAMS, mtk_drm_ioctl_get_msync_params, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_FACTORY_LCM_AUTO_TEST, mtk_drm_ioctl_fm_lcm_auto_test, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_DRM_SET_LEASE_INFO, mtk_drm_ioctl_set_lease_info, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_DRM_GET_LEASE_INFO, mtk_drm_ioctl_get_lease_info, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_CRTC_FENCE_REL, mtk_drm_ioctl_crtc_fence_release, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_GET_MODE_EXT_INFO, mtk_drm_ioctl_get_mode_ext_info, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_HWVSYNC_ON, mtk_drm_ioctl_hwvsync_on, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_DUMMY_CMD_ON, mtk_drm_ioctl_dummy_cmd_on, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_GET_LCM_INDEX, mtk_drm_ioctl_get_lcm_index, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_SUPPORT_COLOR_TRANSFORM, mtk_drm_ioctl_support_color_transform, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_PQ_PROXY_IOCTL, mtk_drm_ioctl_pq_proxy, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_DEBUG_LOG, mtk_drm_ioctl_debug_log, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_GET_CHIST, mtk_drm_ioctl_get_chist, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_GET_CHIST_CAPS, mtk_drm_ioctl_get_chist_caps, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_SET_CHIST_CONFIG, mtk_drm_ioctl_set_chist_config, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_SET_OVL_LAYER, mtk_drm_ioctl_set_ovl_layer, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_MAP_DMA_BUF, mtk_drm_ioctl_map_dma_buf, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_UNMAP_DMA_BUF, mtk_drm_ioctl_unmap_dma_buf, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_GET_PQ_CAPS, mtk_drm_ioctl_get_pq_caps, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_SET_PQ_CAPS, mtk_drm_ioctl_set_pq_caps, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_ODDMR_LOAD_PARAM, mtk_drm_ioctl_oddmr_load_param, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_ODDMR_CTL, mtk_drm_ioctl_oddmr_ctl, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_GET_PANELS_INFO, mtk_drm_ioctl_get_all_connector_panel_info, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_KICK_IDLE, mtk_drm_ioctl_kick_idle, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_PQ_FRAME_CONFIG, mtk_drm_ioctl_pq_frame_config, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_ESD_STAT_CHK, mtk_drm_ioctl_esd_stat_chk, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_MML_CTRL, mtk_drm_ioctl_mml_ctrl, DRM_RENDER_ALLOW),
};
