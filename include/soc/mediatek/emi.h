/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

#ifndef __EMI_H__
#define __EMI_H__

static inline unsigned int mtk_emicen_get_rk_cnt(void)
{
	return 1;
}

static inline unsigned long long mtk_emicen_get_rk_size(unsigned int rk_id)
{
	return 0;
}

#endif /* __EMI_H__ */
