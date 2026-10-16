/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2021 MediaTek Inc.
 */

#ifndef _MTK_HEAP_H
#define _MTK_HEAP_H

#include <linux/dma-buf.h>

static inline long mtk_dma_buf_set_name(struct dma_buf *dmabuf, const char *buf)
{
	return 0;
}

#endif /* _MTK_HEAP_H */
