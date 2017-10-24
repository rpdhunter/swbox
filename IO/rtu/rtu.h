#ifndef RTU_H
#define RTU_H


class Rtu
{
public:
    Rtu();

#if 1
    int init_rtu();
    static int test_menu ();

private:
    unsigned int * rdb_mem_base;
    unsigned int rdb_mem_size;
    unsigned int rdb_mem_offset;

    unsigned int * commu_104_mem_base;
    unsigned int commu_104_mem_size;
    unsigned int commu_104_mem_offset;

    unsigned int * commu_101_mem_base;
    unsigned int commu_101_mem_size;
    unsigned int commu_101_mem_offset;


    static char * rtu_console_gets (char * buf, int size);
    static char * rtu_readline (char * prompt, char * buf, int bufsize, char * defl);

    int init_mem_base ();
    int show_sdram_mem ();

#endif


#if 0
    static int ports_string_to_pbmp (char * port_str, unsigned int * pbmp);
#endif

};

#endif // RTU_H
