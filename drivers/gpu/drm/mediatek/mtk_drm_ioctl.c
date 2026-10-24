// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2024 MediaTek Inc.
 * Copyright (C) 2026 Badmaneers <dukebraham24@gmail.com>
 */

#include <linux/uaccess.h>
#include <drm/drm_atomic.h>
#include <drm/drm_connector.h>
#include <drm/drm_crtc.h>
#include <drm/drm_device.h>
#include <drm/drm_file.h>
#include <drm/drm_modes.h>

#include "mtk_drm_ioctl.h"

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

	memset(info, 0, sizeof(*info));
	info->session_id = 1;
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

	session->session_id = 1;
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

const struct drm_ioctl_desc mtk_ioctls[MTK_NUM_IOCTLS] = {
	DRM_IOCTL_DEF_DRV(MTK_SESSION_CREATE, mtk_drm_ioctl_session_create, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_SESSION_DESTROY, mtk_drm_ioctl_session_destroy, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_SET_DDP_MODE, mtk_drm_ioctl_set_ddp_mode, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_GET_SESSION_INFO, mtk_drm_ioctl_get_session_info, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_HWVSYNC_ON, mtk_drm_ioctl_hwvsync_on, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_GET_PANELS_INFO, mtk_drm_ioctl_get_all_connector_panel_info, DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF_DRV(MTK_KICK_IDLE, mtk_drm_ioctl_kick_idle, DRM_RENDER_ALLOW),
};
