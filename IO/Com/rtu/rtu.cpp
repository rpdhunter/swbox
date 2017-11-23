#include "rtu.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "iec104.h"
#include "iec101.h"
#include "setting_table.h"
#include "../rdb/point_table.h"
#include "../rdb/rdb.h"
#include "../rdb/comm_head.h"
#include "../rtu/setting_value.c"

#if 0
extern "C"
{
int gpio_open (unsigned int gpio_pin /* 0 - 53, mio, 54 - 117, emio */, char * gpio_dir /* in, out */);
int gpio_close (unsigned int gpio_pin /* 0 - 53, mio, 54 - 117, emio */);
int gpio_set (unsigned int gpio_pin, unsigned int val);
int gpio_get (unsigned int gpio_pin, unsigned int * val);
int gpio_edge (unsigned int gpio_pin, char * edge /* none, rising, falling, both */);
//int init_device_setting_param();
}

#endif


Rtu::Rtu(int serial_fd)
{
    this->serial_fd = serial_fd;
    init_rtu ();
//    test_menu ();
}

#if 1

int Rtu::init_rtu()
{
    init_mem_base ();		//分配各块内存大小rdb_mem=128kB,commu_104_mem=64kB,commu_101_mem=128kB,prot_setting_mem=16kB
    show_sdram_mem();                   //show 函数，显示各mem使用情况
    init_device_setting_param();        //分配prot_setting_mem第一部分为setting_param_tables，其为一个10行的向量组，每个向量大小不一。
//    init_rdb ();                        //创建点表以及消息池，处理各事件

    /* init protocols at last */
//    init_iec104_server ();
    init_iec101_service (serial_fd);

    printf ("ftu main started\n");

    return 0;
}

int Rtu::test_menu()
{
    char in_buf [100];
    int point_no;
    unsigned int yx_value[1];
    time_type tt;
    yc_data_type yc_val;
    unsigned char qds;
    //bcr_t ym_val;

    while (1) {
        printf(" *** Protocol Test Menu ***\n");
        printf(" 1. YX Test\n");
        printf(" 2. YC Test\n");
    //	printf(" 3. YM Test\n");
        printf(" 4. RDB Data\n");
        printf(" 5. Setting and Parameter\n");
        rtu_readline(" RTU > ", in_buf, sizeof(in_buf) - 1, NULL);
        if (in_buf[0] == '1') {
            while (1) {
                printf(" *** YX Test ***\n");
                printf(" Point Number(0-%d, (-1) exit)\n", YX_VALID_NUM - 1);
                rtu_readline(" RTU > ", in_buf, sizeof(in_buf) - 1, NULL);
                if (in_buf[0] != 0) {
                    point_no = atoi(in_buf);
                    if (point_no == -1) {
                        break;
                    }
    #if 0
                    point_no = 0;
                    if (ports_string_to_pbmp(in_buf, (unsigned int *)&point_no) < 0) {
                        printf(" Invalid point number (%s)\n", in_buf);
                        break;
                    }
    #else
                    if (point_no >= YX_VALID_NUM - 1) {
                        printf(" Invalid point number (%s)\n", in_buf);
                    }
    #endif

                    printf(" New Value (1, open; 2, close)\n");
                    rtu_readline(" RTU > ", in_buf, sizeof(in_buf) - 1, NULL);
                    if (in_buf[0] == '1' ||
                        in_buf[0] == '2') {
                        yx_value[0] = in_buf[0] - '0';
                        tt.seconds = time(NULL);
                        tt.msec = 0;
    #if 0
                        for (i = 0; i < (YX_VALID_NUM < 32 ? YX_VALID_NUM : 32); i++) {
                            if (point_no & (1 << i)) {
                                printf("yx %d, set val %d\n", i, yx_value[0]);
                                yx_set_value(i, yx_value, &tt, YX_SOE_FLAG | YX_COS_FLAG);
                            }
                        }
    #else
                        printf("yx %d, set val %d\n", point_no, yx_value[0]);
                        yx_set_value(point_no, yx_value, &tt, YX_SOE_FLAG | YX_COS_FLAG);
    #endif
                    }
                    else {
                        printf("Invalid yx value\n");
                    }
                    }
                }
            }
        else if (in_buf[0] == '2') {
            while (1) {
                printf(" *** YC Test ***\n");
                printf(" Point Number(0-%d, (-1) exit)\n", YC_VALID_NUM - 1);
                rtu_readline(" RTU > ", in_buf, sizeof(in_buf) - 1, NULL);
                if (in_buf[0] != 0) {
                    point_no = atoi(in_buf);
                    if (point_no == -1) {
                        break;
                    }
    #if 0
                    point_no = 0;
                    if (ports_string_to_pbmp(in_buf, (unsigned int *)&point_no) < 0) {
                        printf(" Invalid point number (%s)\n", in_buf);
                        break;
                    }
    #else
                    if (point_no >= YC_VALID_NUM - 1) {
                        printf(" Invalid point number (%s)\n", in_buf);
                    }
    #endif

                    printf(" New Value\n");
                    rtu_readline(" RTU > ", in_buf, sizeof(in_buf) - 1, NULL);
                    if (in_buf[0] != 0) {
                        yc_val.f_val = atof(in_buf);
                        tt.seconds = time(NULL);
                        tt.msec = 0;
                        qds = 0;
    #if 0
                        for (i = 0; i < (YC_VALID_NUM < 32 ? YC_VALID_NUM : 32); i++) {
                            if (point_no & (1 << i)) {
                                printf("yc %d, set val %f\n", i, yc_val.f_val);
                                yc_set_value(i, &yc_val, qds, &tt, 1);
                            }
                        }
    #else
                        printf("yc %d, set val %f\n", point_no, yc_val.f_val);
                        yc_set_value(point_no, &yc_val, qds, &tt, 1);
    #endif
                    }
                    else {
                        printf("Invalid yx value\n");
                    }
                    }
                }
            }
        /*
        else if (in_buf[0] == '3') {
            while (1) {
                printf(" *** YM Test ***\n");
                printf(" Point Number(0-%d, (-1) exit)\n", YM_VALID_NUM - 1);
                rtu_readline(" RTU > ", in_buf, sizeof(in_buf) - 1, NULL);
                if (in_buf[0] != 0) {
                    point_no = atoi(in_buf);
                    if (point_no == -1) {
                        break;
                    }
    #if 0
                    point_no = 0;
                    if (ports_string_to_pbmp(in_buf, (unsigned int *)&point_no) < 0) {
                        printf(" Invalid point number (%s)\n", in_buf);
                        break;
                    }
    #else
                    if (point_no >= YM_VALID_NUM - 1) {
                        printf(" Invalid point number (%s)\n", in_buf);
                    }
    #endif

                    printf(" New Value\n");
                    rtu_readline(" RTU > ", in_buf, sizeof(in_buf) - 1, NULL);
                    if (in_buf[0] != 0) {
                        ym_val.counter_reading = atoi(in_buf);
                        ym_val.seq_not = 0;
    #if 0
                        for (i = 0; i < (YC_VALID_NUM < 32 ? YC_VALID_NUM : 32); i++) {
                            if (point_no & (1 << i)) {
                                printf("yc %d, set val %f\n", i, yc_val.f_val);
                                yc_set_value(i, &yc_val, qds, &tt, 1);
                            }
                        }
    #else
                        printf("ym %d, set val %d\n", point_no, ym_val.counter_reading);
                        ym_set_value(point_no, &ym_val);
    #endif
                    }
                    else {
                        printf("Invalid ym value\n");
                    }
                    }
                }
            }
        */
        else if (in_buf[0] == '4') {
            while (1) {
                printf(" *** RDB Data ***\n");
                printf(" 1. YX Data\n");
                printf(" 2. YC Data\n");
                printf(" 3. YM Data\n");
                printf(" 4. exit\n");
                rtu_readline(" RTU > ", in_buf, sizeof(in_buf) - 1, NULL);
                if (in_buf[0] == '1') {
                    rdb_yx_show();
                }
                else if (in_buf[0] == '2') {
                    rdb_yc_show();
                }
                else if (in_buf[0] == '3') {
                //	rdb_ym_show();
                }
                else if (in_buf[0] == '4') {
                    break;
                }
                else {
                    printf(" Invalid input number\n");
                }
            }
        }
        else if (in_buf[0] == '5') {
            while (1) {
                printf(" *** Setting and Parameter ***\n");
                printf(" 1. Show Data\n");
                printf(" 2. exit\n");
                rtu_readline(" RTU > ", in_buf, sizeof(in_buf) - 1, NULL);
                if (in_buf[0] == '1') {
                    setting_param_show();
                }
                else {
                    printf(" Invalid input number\n");
                }
            }
        }
        else {
            printf(" Invalid input number\n");
        }
        }

    return 0;
}

char *Rtu::rtu_console_gets(char *buf, int size)
{
    char *p = fgets(buf, size, stdin);
    if (p == NULL) {
    clearerr(stdin);
    }
    return p;
}

char *Rtu::rtu_readline(char *prompt, char *buf, int bufsize, char *defl)
{
    char *s, *full_prompt;
    const char *cont_prompt;
    char *t;
    int len;

    if (bufsize == 0) {
        return NULL;
    }

    cont_prompt = prompt[0] ? "? " : "";
    full_prompt = (char *)malloc (strlen(prompt) + (defl ? strlen(defl) : 0) + 8);
    strcpy(full_prompt, prompt);
    if (defl) {
        sprintf(full_prompt + strlen(full_prompt), "[%s] ", defl);
    }

    t = (char *)malloc(bufsize);
    printf("%s", full_prompt);

    if ((s = rtu_console_gets(t, bufsize)) == 0) {
        free(t);
    } else {
        s[bufsize - 1] = 0;
        if ((t = strchr(s, '\n')) != 0) {
            *t = 0;
            /* Replace garbage characters with spaces */
        }
        for (t = s; *t; t++) {
            if (*t < 32 && *t != 7 && *t != 9) {
                *t = ' ';
            }
        }
    }

    if (s == 0) {                       /* Handle Control-D */
        buf[0] = 0;
        /* EOF */
        buf = 0;
        goto done;
    }

    len = 0;
    if (s[0] == 0) {
        if (defl && buf != defl) {
            if ((len = strlen(defl)) >= bufsize) {
                len = bufsize - 1;
            }
            memcpy(buf, defl, len);
        }
    } else {
        if ((len = strlen(s)) >= bufsize) {
            len = bufsize - 1;
            printf("WARNING: input line truncated to %d chars\n", len);
        }
        memcpy(buf, s, len);
    }
    buf[len] = 0;

    free(s);

    /*
     * If line ends in a backslash, perform a continuation prompt.
     */

    if (strlen(buf) != 0) {
        /*
         * Ensure that there is atleast one character available
         */
        s = buf + strlen(buf) - 1;
        if (*s == '\\' /*&& rtu_readline(cont_prompt, s, bufsize - (s - buf), 0) == 0*/) {          //临时注释,为编译通过
            buf = 0;
        }
    }

done:
    free(full_prompt);

    return buf;
}

int Rtu::init_mem_base()
{
    rdb_mem_size = RDB_MEM_SIZE;
    rdb_mem_offset = 0;
#if 0	/* in the real device, using this */
    rdb_mem_base = (unsigned int *)(RDB_MEM_BASE);
#else
    rdb_mem_base = (unsigned int *)malloc (RDB_MEM_SIZE);
#endif

    commu_104_mem_size = COMMU_104_MEM_SIZE;
    commu_104_mem_offset = 0;
    commu_101_mem_size = COMMU_101_MEM_SIZE;
    commu_101_mem_offset = 0;
#if 0	/* in the real device, using this */
    commu_104_mem_base = (unsigned int *)(COMMU_104_MEM_BASE);
    commu_101_mem_base = (unsigned int *)(COMMU_101_MEM_BASE);
#else
    commu_104_mem_base = (unsigned int *)malloc (COMMU_104_MEM_SIZE);
    commu_101_mem_base = (unsigned int *)malloc (COMMU_101_MEM_SIZE);
#endif

    prot_setting_mem_size = PROT_SETTING_MEM_SIZE;
    prot_setting_mem_offset = 0;
#if 0	/* in the real device, using this */
    prot_setting_mem_base = (unsigned int *)(PROT_SETTING_MEM_BASE);
#else
    prot_setting_mem_base = (unsigned int *)malloc (PROT_SETTING_MEM_SIZE);
#endif

    return 0;
}

int Rtu::show_sdram_mem()
{
    printf ("ftu sdram memory show:\n");

    printf ("  rdb            : mem base 0x%08x, size 0x%06x, used 0x%06x\n",
        (unsigned int)rdb_mem_base, rdb_mem_size, rdb_mem_offset);
    printf ("  iec104         : mem base 0x%08x, size 0x%06x, used 0x%06x\n",
        (unsigned int)commu_104_mem_base, commu_104_mem_size, commu_104_mem_offset);
    printf ("  iec101         : mem base 0x%08x, size 0x%06x, used 0x%06x\n",
        (unsigned int)commu_101_mem_base, commu_101_mem_size, commu_101_mem_offset);
    printf ("  set param      : mem base 0x%08x, size 0x%06x, used 0x%06x\n",
        (unsigned int)prot_setting_mem_base, prot_setting_mem_size, prot_setting_mem_offset);

    printf ("\n");

    return 0;
}

#endif
