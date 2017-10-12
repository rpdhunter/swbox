/********************************************************************
	created:	2016/06/15
	created:	15:6:2016   11:19
	filename: 	D:\rd\src_code\rtu\src_bin\setting_value.c
	file path:	D:\rd\src_code\rtu\src_bin
	file base:	setting_value
	file ext:	c
	author:		
	
	purpose:	setting values
*********************************************************************/
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "setting_table.h"
#include "../rdb/rdb.h"
#include "../rdb/comm_head.h"

unsigned int * prot_setting_mem_base = NULL;
unsigned int prot_setting_mem_size = 0;
unsigned int prot_setting_mem_offset = 0;

static setting_param_table_t * setting_param_tables;

static int init_setting_param_memory ();
static int init_device_parameter (char * param_file);
static int init_setting_value (char * setting_file);

static int init_setting_param_memory ()
{
	int i;

	/* 
	init memory space of tables 
	prot_setting_mem_base第一部分为setting_param_tables，其为一个10行的向量组，每个向量大小不一。
	*/
	SDRAM_MALLOC (setting_param_tables, setting_param_table_t, num_of_setting_and_param_table,
		prot_setting_mem_base, prot_setting_mem_size, prot_setting_mem_offset);
	if (setting_param_tables == NULL) {
		printf ("init_setting_param_mem malloc fail\n");
		return -1;
	}

	/* init everything of tables */
	/*初始化矩阵第1行p_device_param */
	setting_param_tables [p_device_param].entry_num = num_of_device_paramter;
	strcpy (setting_param_tables [p_device_param].desc, CONV_STR (p_device_param));
	/*初始化矩阵第2行s_voltage_setting*/
	setting_param_tables [s_voltage_setting].entry_num = num_of_voltage_setting;
	strcpy (setting_param_tables [s_voltage_setting].desc, CONV_STR (s_voltage_setting));
	/*初始化矩阵第3行s_current_setting*/
	setting_param_tables [s_current_setting].entry_num = num_of_current_setting;
	strcpy (setting_param_tables [s_current_setting].desc, CONV_STR (s_current_setting));
	/*初始化矩阵第4行s_protect_setting*/
	setting_param_tables [s_protect_setting].entry_num = num_of_protection_setting;
	strcpy (setting_param_tables [s_protect_setting].desc, CONV_STR (s_protect_setting));
	/*初始化矩阵第5行s_running_alarm_setting*/
	setting_param_tables [s_running_alarm_setting].entry_num = num_of_running_alarm_setting;
	strcpy (setting_param_tables [s_running_alarm_setting].desc, CONV_STR (s_running_alarm_setting));
	/*初始化矩阵第6行s_yx_setting*/
	setting_param_tables [s_yx_setting].entry_num = num_of_yx_setting;
	strcpy (setting_param_tables [s_yx_setting].desc, CONV_STR (s_yx_setting));
	/*初始化矩阵第7行s_yk_setting*/
	setting_param_tables [s_yk_setting].entry_num = num_of_yk_setting;
	strcpy (setting_param_tables [s_yk_setting].desc, CONV_STR (s_yk_setting));
	/*初始化矩阵第8行s_ac_sam_adj_setting*/
	setting_param_tables [s_ac_sam_adj_setting].entry_num = num_of_ac_sample_adjust_setting;
	strcpy (setting_param_tables [s_ac_sam_adj_setting].desc, CONV_STR (s_ac_sam_adj_setting));
	/*初始化矩阵第9行s_ac_sam_base_setting*/
	setting_param_tables [s_ac_sam_base_setting].entry_num = num_of_ac_sample_base_setting;
	strcpy (setting_param_tables [s_ac_sam_base_setting].desc, CONV_STR (s_ac_sam_base_setting));
	/*初始化矩阵第10行s_dc_sam_adj_setting*/
	setting_param_tables [s_dc_sam_adj_setting].entry_num = num_of_dc_sample_adjust_setting;
	strcpy (setting_param_tables [s_dc_sam_adj_setting].desc, CONV_STR (s_dc_sam_adj_setting));

	/* init memory space for every table */
	for (i = 0; i < num_of_setting_and_param_table; i++) {
		SDRAM_MALLOC (setting_param_tables [i].setting_param_entries, 
			setting_param_entry_t, 
			setting_param_tables [i].entry_num, 
			prot_setting_mem_base, prot_setting_mem_size, prot_setting_mem_offset);
		if (setting_param_tables [i].setting_param_entries == NULL) {
			printf ("init_setting_param_mem malloc fail\n");
			return -1;
		}
		memset (setting_param_tables [i].setting_param_entries, 0, 
			sizeof (setting_param_entry_t) * setting_param_tables [i].entry_num);
	}
	
	return 0;
}

static int init_device_parameter (char * param_file)
{
	setting_param_entry_t * pe;

	/* init values */
	pe = setting_param_tables [p_device_param].setting_param_entries;

	/* only for test */
#if 1
	pe [device_address].val_type = S_P_TYPE_UINT;
	pe [device_address].value.i_val = 10;
	strcpy (pe [device_address].desc, CONV_STR(device_address));

	pe [ip_addr_1].val_type = S_P_TYPE_UINT;
	pe [ip_addr_1].value.u_val = 0xc0a80048;
	strcpy (pe [ip_addr_1].desc, CONV_STR(ip_addr_1));
#endif

	return 0;
}

static int init_setting_value (char * setting_file)
{
	setting_param_entry_t * pe;

	/* init values */
	pe = setting_param_tables [s_voltage_setting].setting_param_entries;

	/* only for test */
#if 1
	pe [pt_pri_vol].val_type = S_P_TYPE_INT;
	pe [pt_pri_vol].value.i_val = 110000;
	strcpy (pe [pt_pri_vol].desc, CONV_STR(pt_pri_vol));

	pe [pt_sec_vol].val_type = S_P_TYPE_FLOAT;
	pe [pt_sec_vol].value.f_val = 100.0;
	strcpy (pe [pt_sec_vol].desc, CONV_STR(pt_sec_vol));

	pe [vol_upper_limit].val_type = S_P_TYPE_INT;
	pe [vol_upper_limit].value.i_val = 125000;
	strcpy (pe [vol_upper_limit].desc, CONV_STR(vol_upper_limit));
#endif
	
	return 0;
}

int get_setting_param (
	int table_index, 
	int item_index, 
	setting_param_entry_t * p_mem
	)
{
	setting_param_entry_t * ps;
	
	if (p_mem == NULL) {
		return -1;
	}

	if (table_index < 0 || table_index > num_of_setting_and_param_table) {
		return -1;
	}

	if (item_index == -1) {
		/* get a whole setting table */
		memcpy (p_mem, setting_param_tables [table_index].setting_param_entries, 
			sizeof (setting_param_entry_t) * setting_param_tables [table_index].entry_num);
	}
	else if (item_index >= 0 && item_index < setting_param_tables [table_index].entry_num) {
		ps = setting_param_tables [table_index].setting_param_entries;
		memcpy (p_mem, &ps [item_index], sizeof (setting_param_entry_t));
	}
	else {
		printf ("get_setting_param: talbe index %d, item index %d error\n", table_index, item_index);
		return -1;
	}

	return 0;
}

/* set settings or parameters in memory */
int set_setting_param (
	int app_id,					/* who set settings or params */
	int table_index, 
	int item_index, 
	setting_param_entry_t * p_mem
	)
{
	setting_param_entry_t * ps;
	time_t tt;
	struct tm * ltm;
	
	if (p_mem == NULL) {
		return -1;
	}

	if (table_index < 0 || table_index > num_of_setting_and_param_table) {
		return -1;
	}

	if (item_index == -1) {
		/* set a whole setting table */
		memcpy (setting_param_tables [table_index].setting_param_entries, p_mem,
			sizeof (setting_param_entry_t) * setting_param_tables [table_index].entry_num);
	}
	else if (item_index >= 0 && item_index < setting_param_tables [table_index].entry_num) {
		ps = setting_param_tables [table_index].setting_param_entries;
		memcpy (&ps [item_index], p_mem, sizeof (setting_param_entry_t));
	}
	else {
		printf ("set_setting_param: talbe index %d, item index %d error\n", table_index, item_index);
		return -1;
	}

	/* settings or parameters change event should be recorded */
	tt = time (NULL);
	ltm = localtime (&tt);
	_DPRINTF ("App %d at %04d-%02d-%02d, %02d-%02d-%02d, set setting or parameter table %d, item %d\n", 
		app_id,
		ltm->tm_year + 1900,
		ltm->tm_mon + 1,
		ltm->tm_mday,
		ltm->tm_hour,
		ltm->tm_min,
		ltm->tm_sec,
		table_index, item_index);

	return 0;
}

/* save settings and parameters in flash */
int save_setting_param (int app_id)
{
	time_t tt;
	struct tm * ltm;

	/* settings or parameters saving event should be recorded */
	tt = time (NULL);
	ltm = localtime (&tt);
	_DPRINTF ("App %d at %04d-%02d-%02d, %02d-%02d-%02d, save setting or parameter\n", 
		app_id,
		ltm->tm_year + 1900,
		ltm->tm_mon + 1,
		ltm->tm_mday,
		ltm->tm_hour,
		ltm->tm_min,
		ltm->tm_sec);
	
	return 0;
}

int setting_param_show ()
{
	int i, j;
	setting_param_table_t * ps;
	setting_param_entry_t * pe;

	printf ("  Setting and Parameter Tables:\n");
	for (i = 0; i < num_of_setting_and_param_table; i++) {
		ps = &setting_param_tables [i];
		printf ("    %s table:\n", ps->desc);
		for (j = 0; j < ps->entry_num; j++) {
			pe = &ps->setting_param_entries [j];
			if (pe->val_type == S_P_TYPE_UINT) {
				printf ("      %04d, value 0x%08x, desc %s\n", j, pe->value.u_val, pe->desc);
			}
			else if (pe->val_type == S_P_TYPE_INT) {
				printf ("      %04d, value %010d, desc %s\n", j, pe->value.i_val, pe->desc);
			}
			else {
				printf ("      %04d, value %10.2f, desc %s\n", j, pe->value.f_val, pe->desc);
			}
		}
	}

	return 0;
}

int init_device_setting_param ()
{
	/*
	prot_setting_mem第一部分为setting_param_tables，其为一个10行的向量组，每个向量大小不一。
	初始化了setting_param_tables
	*/
	init_setting_param_memory ();
	/*
	实例化了setting_param_tables的p_device_param = 0（仅供测试）
	*/
	init_device_parameter (NULL);
	/*
	实例化了setting_param_tables的s_voltage_setting,（仅供测试）
	*/
	init_setting_value (NULL);
	
	return 0;
}

