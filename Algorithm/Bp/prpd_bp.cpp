#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "prpd_bp.h"

c_bp_neural_param prpd_bp_param = {
	N_BUF, O_BUF, C_BUF, 0.05, 0.02, 0.001, 1000000, 0.5
};

// 从文件输入phase, q, n
int prpd_bp_neural_network::prpd_bp_init_points_file (FILE * fp, c_prpd_point points [], int max_num)
{
	int i = 0, c_index = 0, d_index = 0;
	unsigned char cbuf [2];
	int c, data = 0;
	int end_flag = 0;

	if (max_num <= 0 || max_num > MAX_PRPD_NUM_PER_FILE) {
		max_num = MAX_PRPD_NUM_PER_FILE;
	}

	fread (cbuf, sizeof (char), sizeof (cbuf), fp);

	//printf ("head %02x %02x\n", cbuf [0], cbuf [1]);
	
	if (cbuf [0] == 0xff && cbuf [1] == 0xfe) {
		/* unicode小端 */
		//printf ("unicode le\n");
		while ((c = fgetc (fp)) != EOF) {
			cbuf [c_index++] = c;
			if (c_index == 2) {
				c_index = 0;

				if (cbuf [0] == 0x09 || cbuf [0] == 0x0a || cbuf [0] == 0x0d) {
					if (!end_flag) {
						//printf ("data [%d]: %d\n", d_index, data);
						switch (d_index) {
						case 0:
							points [i].phase = data;
							break;
						case 1:
							points [i].q = data;
							break;
						case 2:
							points [i].n = data;
							break;
						default:
							break;
						}
						data = 0;
						
						d_index++;
						if (d_index == 3) {
							d_index = 0;
							//printf ("%d:%d,%d,%d\n", i, points [i].phase, points [i].q, points [i].n);
							i++;
						}
					}
					end_flag = 1;
				}
				else {
					end_flag = 0;
					data = data * 10 + (cbuf [0] - 0x30);
				}
			}
		}
	}
	else if (cbuf [0] == 0xff && cbuf [1] == 0xfe) {
		/* unicode大端 */
		//printf ("unicode be\n");
		while ((c = fgetc (fp)) != EOF) {
			cbuf [c_index++] = c;
			if (c_index == 2) {
				c_index = 0;

				if (cbuf [1] == 0x09 || cbuf [1] == 0x0a || cbuf [1] == 0x0d) {
					if (!end_flag) {
						//printf ("data [%d]: %d\n", d_index, data);
						switch (d_index) {
						case 0:
							points [i].phase = data;
							break;
						case 1:
							points [i].q = data;
							break;
						case 2:
							points [i].n = data;
							break;
						default:
							break;
						}
						data = 0;
						
						d_index++;
						if (d_index == 3) {
							d_index = 0;
							//printf ("%d:%d,%d,%d\n", i, points [i].phase, points [i].q, points [i].n);
							i++;
						}
					}
					end_flag = 1;
				}
				else {
					end_flag = 0;
					data = data * 10 + (cbuf [1] - 0x30);
				}
			}
		}
	}
	else {
		//printf ("asc\n");
		rewind (fp);
		
		if (fscanf (fp, "%d", &points [i].phase) <= 0) {
			return 0;
		}

		do {
			fscanf (fp, "%d", &points [i].q);
			fscanf (fp, "%d", &points [i].n);
			//printf ("%d:%d,%d,%d\n", i, points [i].phase, points [i].q, points [i].n);
			i++;
		}
		while ((fscanf (fp, "%d", &points [i].phase) > 0) && (i < max_num));
	}

	return i;
}

int prpd_bp_neural_network::prpd_bp_load_data_file (c_bp_neural_data pdata [], c_prpd_discharge_type_file ptf [], int f_num)
{
	int file_index, data_index;
	FILE * fp;

	data_index = 0;
	for (file_index = 0; file_index < f_num; file_index++) {
		fp = fopen (ptf [file_index].prpd_file, "r");
		if (fp != NULL) {
			printf ("load data file %s\n", ptf [file_index].prpd_file);
			prpd_bp_set_x_file (&pdata [data_index], fp);
			fclose (fp);
			prpd_bp_set_o (&pdata [data_index], ptf [file_index].dsc_type);
			data_index++;
		}
		else {
			printf ("fail to open %s\n", ptf [file_index].prpd_file);
		}
	}

	return data_index;
}

void prpd_bp_neural_network::prpd_bp_translation_phase (c_prpd_point prpd_points [], int data_num, int move_phase)
{
	int i, phase_mov;
	
	for (i = 0; i < data_num; i++) {
		phase_mov = prpd_points [i].phase + move_phase;
		if (phase_mov < 0 || phase_mov >= 360) {
			phase_mov = abs (phase_mov % 360);
		}
		prpd_points [i].phase = phase_mov;
	}
}

void prpd_bp_neural_network::prpd_bp_set_x_file (c_bp_neural_data * pdata, FILE * fp)
{
	int N;
	c_prpd_point prpd_points [MAX_PRPD_NUM_PER_FILE];

	N = prpd_bp_init_points_file (fp, prpd_points, MAX_PRPD_NUM_PER_FILE);

	//printf ("datanum %d\n", N);
	
	prpd_bp_calc_feature_phase_related (pdata, prpd_points, N);
}

void prpd_bp_neural_network::prpd_bp_set_x_buf (c_bp_neural_data * pdata, c_prpd_point prpd_points [], int data_num)
{
	prpd_bp_calc_feature_phase_related (pdata, prpd_points, data_num);
}

void prpd_bp_neural_network::prpd_bp_set_o (c_bp_neural_data * pdata, discharge_type_t dt)
{
	memset (pdata->o.vect, 0, sizeof (pdata->o.vect));
	pdata->o.vect [dt] = 1;
	//printf ("o: %d\n", (int)dt);
}

discharge_type_t prpd_bp_neural_network::prpd_bp_get_o (c_bp_neural_data * pdata)
{
	int i;

	for (i = discharge_internal; i < discharge_type_num; i++) {
		if (pdata->o.vect [i] != 0) {
			break;
		}
	}

	return (discharge_type_t)i;
}

int prpd_bp_neural_network::prpd_mode_init (const char * weight_file)
{
	/* 从权值文件载入w */
	if (bp_w_load (weight_file) < 0) {
		return -1;
	}

	return 0;
}

int prpd_bp_neural_network::prpd_study_data (c_prpd_point points [], int data_num, discharge_type_t pd_mode)
{
	c_bp_neural_data prpd_bp_data;
	
	prpd_bp_set_x_buf (&prpd_bp_data, points, data_num);
	prpd_bp_set_o (&prpd_bp_data, pd_mode);

	/* 从PRPD训练神经网络 */
	return bp_study (&prpd_bp_data, 1);
}

int prpd_bp_neural_network::prpd_study_file (c_prpd_discharge_type_file prpd_file_mode [], int file_num)
{
	c_bp_neural_data * prpd_bp_data;
	int data_num, ret;

	if (file_num <= 0) {
		return -1;
	}

	prpd_bp_data = new c_bp_neural_data [file_num];
	if (prpd_bp_data == NULL) {
		return -1;
	}
	
	/* 从文件读取PRPD数据并提取特征 */
	data_num = prpd_bp_load_data_file (prpd_bp_data, prpd_file_mode, file_num);

	/* 从PRPD训练神经网络 */
	ret = bp_study (prpd_bp_data, data_num);

	delete [] prpd_bp_data;

	return ret;
}

void prpd_bp_neural_network::prpd_result_show (discharge_type_t pd_out, char * prompt)
{
	switch ((int)pd_out) {
	case discharge_internal:
		printf ("%s: discharge_internal\n", prompt);
		break;
	case discharge_surface:
		printf ("%s: discharge_surface\n", prompt);
		break;
	case discharge_corona:
		printf ("%s: discharge_corona\n", prompt);
		break;
	case discharge_floating_potential:
		printf ("%s: discharge_floating_potential\n", prompt);
		break;
	case discharge_interference:
		printf ("%s: interference!\n", prompt);
		break;
	default:
		printf ("%s: Can't recognizied discharge\n", prompt);
		break;
	}
}

int prpd_bp_neural_network::prpd_mode (c_prpd_point prpd_points [], int data_num, discharge_type_t * pd_mode_output)
{
	c_bp_neural_data prpd_bp_data;
	int i;

	for (i = 0; i < data_num; i++) {
		if (prpd_points [i].phase < 0 || prpd_points [i].phase >= 360) {
			prpd_points [i].phase = abs (prpd_points [i].phase % 360);
		}
	}

	for (i = 0; i < PHASE_MOV_MAX_DEG; i += PHASE_MOV_DEGREE) {
		prpd_bp_set_x_buf (&prpd_bp_data, prpd_points, data_num);
		bp_judge (&prpd_bp_data);
		* pd_mode_output = prpd_bp_get_o (&prpd_bp_data);

		if (* pd_mode_output != discharge_type_num) {
			return 0;
		}
		else {
			prpd_bp_translation_phase (prpd_points, data_num, PHASE_MOV_DEGREE);
		}
	}

	return -1;
}

int prpd_bp_neural_network::prpd_mode_file (char * prpd_file, discharge_type_t * pd_mode_output)
{
	int i, N;
	c_prpd_point prpd_points [MAX_PRPD_NUM_PER_FILE];
	c_bp_neural_data pd_data;
	FILE * fp;

	fp = fopen (prpd_file, "r");
	if (fp == NULL) {
		return -1;
	}
	N = prpd_bp_init_points_file (fp, prpd_points, 0);
	fclose (fp);

	if (N <= 0) {
		return -1;
	}

	for (i = 0; i < PHASE_MOV_MAX_DEG; i += PHASE_MOV_DEGREE) {
		prpd_bp_set_x_buf (&pd_data, prpd_points, N);
		bp_judge (&pd_data);
		* pd_mode_output = prpd_bp_get_o (&pd_data);

		if (* pd_mode_output != discharge_type_num) {
			return 0;
		}
		else {
			prpd_bp_translation_phase (prpd_points, N, PHASE_MOV_DEGREE);
		}
	}

	return -1;
}

int prpd_bp_neural_network::init_pd_study_files (c_prpd_discharge_type_file * study_files, int * study_files_num, int max_files_num, const char * dir_name)
{
	DIR * dp;
	struct dirent * entry;
	struct stat statbuf;
	char cwd [200], file_dir [200];
	int dir_files_num = 0;
	discharge_type_t dt = discharge_internal;

	getcwd (cwd, sizeof (cwd));
	//printf ("cwd %s\n", cwd);

	sprintf (file_dir, "%s/%s", cwd, dir_name);
	//printf ("file dir %s\n", file_dir);
	
	* study_files_num = 0;

	if (strstr (dir_name, "corona") != NULL) {
		dt = discharge_corona;
	}
	else if (strstr (dir_name, "float") != NULL) {
		dt = discharge_floating_potential;
	}
	else if (strstr (dir_name, "internal") != NULL) {
		dt = discharge_internal;
	}
	else if (strstr (dir_name, "surface") != NULL) {
		dt = discharge_surface;
	}
	else if (strstr (dir_name, "interference") != NULL) {
		dt = discharge_interference;
	}
	
	dp = opendir (file_dir);
    if (dp != NULL) {
		chdir (file_dir);
        for (;;) {
            entry = readdir (dp);
            if ((entry == NULL) || (entry->d_name [0] == 0)) break;
			if ((strcmp (".", entry->d_name) == 0) || 
				(strcmp ("..", entry->d_name) == 0)) {
				continue;
			}
			lstat (entry->d_name, &statbuf); // 获取下一级成员属性
            if (!S_ISDIR(statbuf.st_mode)) {
				if (strstr (entry->d_name, "weight") == NULL) {
					strcpy (study_files [* study_files_num].prpd_file, file_dir);
					strcat (study_files [* study_files_num].prpd_file, "/");
					strcat (study_files [* study_files_num].prpd_file, entry->d_name);
					study_files [* study_files_num].dsc_type = dt;
					* study_files_num = * study_files_num + 1;
					if (* study_files_num >= max_files_num) {
						//printf (" study_files_num1 %d\n", * study_files_num);
						break;
					}
				}
            }
			else {
				init_pd_study_files (&study_files [* study_files_num], &dir_files_num, max_files_num - * study_files_num, entry->d_name);
				* study_files_num += dir_files_num;
				if (* study_files_num >= max_files_num) {
					//printf (" study_files_num2 %d\n", * study_files_num);
					break;
				}
			}
        }
		chdir (cwd);
		closedir (dp);

		return 0;
    }
	else {
		return -1;
	}
}

void prpd_bp_neural_network::show_pd_study_files (c_prpd_discharge_type_file * study_files, int study_files_num)
{
	int i;

	printf ("discharge files num %d\n", study_files_num);
	for (i = 0; i < study_files_num; i++) {
		printf ("file %s, type %d\n", study_files [i].prpd_file, study_files [i].dsc_type);
	}
}

