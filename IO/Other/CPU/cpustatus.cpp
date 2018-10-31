#include "cpustatus.h"

//#define VVPN_ZERO   0
//#define VVPN_ZERO   489
#define VVPN_ZERO   498

CPUStatus::CPUStatus()
{
    init_xadc ();
}

int CPUStatus::init_xadc()
{
    return xadc_core_init (EXADC_INIT_READ_ONLY);
}

int CPUStatus::get_cpu_temp(float *cpu_temp)
{
    if (cpu_temp != NULL) {
        * cpu_temp = xadc_touch (command_list [EParamTemp].parameter_id);
    }

    return 0;
}

int CPUStatus::get_cpu_vcc(float *cpu_vcc)
{
    if (cpu_vcc != NULL) {
        * cpu_vcc = xadc_touch (command_list [EParamVccInt].parameter_id);
    }

    return 0;
}

int CPUStatus::get_sync_vvpn(float *vvpn)
{
    if (vvpn != NULL) {
        * vvpn = xadc_touch (command_list [EParamVVpn].parameter_id) - VVPN_ZERO;
//        * vvpn = xadc_touch (command_list [EParamVVpn].parameter_id);
    }

    return 0;
}
