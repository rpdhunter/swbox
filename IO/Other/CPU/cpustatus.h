#ifndef CPUSTATUS_H
#define CPUSTATUS_H

#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "xadc_core_if.h"

#define MAX_CMD_NAME_SIZE 100
#define MAX_UNIT_NAME_SIZE 50

#define VCC_INT_CMD			"xadc_get_value_vccint"
#define VCC_AUX_CMD			"xadc_get_value_vccaux"
#define VCC_BRAM_CMD		"xadc_get_value_vccbram"
#define VCC_TEMP_CMD		"xadc_get_value_temp"
#define VCC_EXT_CH_CMD		"xadc_get_value_ext_ch"

class CPUStatus
{
public:
    CPUStatus();

    int get_cpu_temp (float * cpu_temp);
    int get_cpu_vcc (float * cpu_vcc);
    int get_vvpn (float* vvpn);

private:
    struct command
    {
        const enum XADC_Param parameter_id;
        const char cmd_name[MAX_CMD_NAME_SIZE];
        const char unit[MAX_UNIT_NAME_SIZE];
    };

    struct command command_list[EParamMax] = {
                    {EParamVccInt, 	VCC_INT_CMD, "mV"},
                    {EParamVccAux, 	VCC_AUX_CMD, "mV"},
                    {EParamVccBRam, VCC_BRAM_CMD, "mV"},
                    {EParamTemp, 	VCC_TEMP_CMD, "Degree Celsius"},
                    {EParamVAux0, 	VCC_EXT_CH_CMD, "mV"},
                    {EParamVVpn, 	VCC_EXT_CH_CMD, "mV"}
    };

    int init_xadc ();
};

#endif // CPUSTATUS_H
