
#ifndef _PD_H_
#define _PD_H_

typedef enum discharge_type_e {
	discharge_internal = 0,			// 内部放电
	discharge_surface,				// 表明(沿面)放电
	discharge_corona,				// 电晕放电
	discharge_floating_potential,	// 悬浮电位放电
	discharge_interference,			// 干扰

	discharge_type_num
} discharge_type_t;

#endif

