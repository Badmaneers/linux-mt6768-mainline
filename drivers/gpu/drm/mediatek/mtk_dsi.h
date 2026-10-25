/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2015 MediaTek Inc.
 * Copyright (C) 2026 Badmaneers <dukebraham24@gmail.com>
 */

#ifndef _MTK_DSI_H_
#define _MTK_DSI_H_

#include <drm/drm_connector.h>
#include <linux/types.h>

struct mtk_drm_conn_caps {
	unsigned int lcm_degree;
	int lcm_color_mode;
};

#define MTK_CONNECTOR_MAX_MODES 30

struct mtk_drm_connector_caps {
	struct mtk_drm_conn_caps conn_caps;
	unsigned int width_after_pq[MTK_CONNECTOR_MAX_MODES];
	unsigned int height_after_pq[MTK_CONNECTOR_MAX_MODES];
};

enum mtk_connector_prop_id {
	CONNECTOR_PROP_CAPS_BLOB_ID,
	CONNECTOR_PROP_CSC_BL,
	CONNECTOR_PROP_PANEL_NITS,
	CONNECTOR_PROP_CUSTOM_MAX,
};

struct mtk_connector_state {
	struct drm_connector_state base;
	u64 prop_val[CONNECTOR_PROP_CUSTOM_MAX];
};

#define to_mtk_connector_state(s) \
	container_of((s), struct mtk_connector_state, base)

#endif /* _MTK_DSI_H_ */
