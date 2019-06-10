#include "bpjudge.h"
#include "IO/File/testtools.h"

#define MAX_PD_FILE_NUM			40000

#define CABLE_PD_FILE		"/root/bp/weight.txt"
#define CABLE_PD_FILE_DIR		"/root/bp"

BpJudge::BpJudge(QObject *parent) : QObject(parent)
{
    p_cable_prpd_bp = new cable_bp_neural_network (&cable_prpd_bp_param);

    int cable_pd_study_files_num = 0;
    c_prpd_discharge_type_file cable_pd_study_files [MAX_PD_FILE_NUM];

    if (p_cable_prpd_bp->prpd_mode_init (CABLE_PD_FILE) < 0) {
        printf ("failed to load weight.txt\n");
        if (cable_pd_study_files_num == 0) {
            p_cable_prpd_bp->init_pd_study_files (cable_pd_study_files, &cable_pd_study_files_num, MAX_PD_FILE_NUM, CABLE_PD_FILE_DIR);
            p_cable_prpd_bp->show_pd_study_files (cable_pd_study_files, cable_pd_study_files_num);
        }
        p_cable_prpd_bp->prpd_study_file (cable_pd_study_files, cable_pd_study_files_num);
        p_cable_prpd_bp->bp_w_save (CABLE_PD_FILE);
    }
}

QString BpJudge::add_data(QVector<QPoint> list)
{

    _samples.append(list);

    if(_samples.count() > 100){
//        if(_samples.count() >= 1000 && _samples.count()%5 == 0){
//            TestTools::save_test_file(_samples);
//        }
        QString r = judge(_samples);
        while(_samples.count() > 1000){
            _samples.removeFirst();
        }
        return r;
    }
    else{
        return tr("点不足");
    }
}

QString BpJudge::judge(QVector<QwtPoint3D> samples)
{
    discharge_type_t pd_out;

    int num = samples.count();

    c_prpd_point *list =  new c_prpd_point[num];
    for (int i = 0; i < num; ++i) {
        list[i].phase = samples.at(i).x();
        list[i].q = samples.at(i).y();
        list[i].n = samples.at(i).z();
    }

    if (p_cable_prpd_bp->prpd_mode (list, num, &pd_out) == 0) {
        switch ((int)pd_out) {
        case discharge_internal:
            return tr("内部放电");
        case discharge_surface:
            return tr("表面放电");
        case discharge_corona:
            return tr("电晕放电");
        case discharge_floating_potential:
            return tr("悬浮电位");
        default:
            return tr("噪声");
        }
    }
    else {
//        printf ("fail to open or recognized %s\n", file_buf);
    }
    return QString();

}

QString BpJudge::judge(QVector<QPoint> samples)
{
    discharge_type_t pd_out;

    int num = samples.count();

    c_prpd_point *list =  new c_prpd_point[num];
    for (int i = 0; i < num; ++i) {
        list[i].phase = samples.at(i).x() % 360;
        list[i].q = qAbs(samples.at(i).y() );
        list[i].n = 1;
//        qDebug()<<list[i].phase<<"\t"<<list[i].q<<"\t"<<list[i].n ;
    }


    if (p_cable_prpd_bp->prpd_mode (list, num, &pd_out) == 0) {
        switch ((int)pd_out) {
        case discharge_internal:
            return tr("内部放电");
        case discharge_surface:
            return tr("表面放电");
        case discharge_corona:
            return tr("电晕放电");
        case discharge_floating_potential:
            return tr("悬浮电位");
        case discharge_interference:
            return tr("噪声");
        default:
            return tr("噪声");
        }
    }
    else {
//        printf ("fail to open or recognized %s\n", file_buf);
        return tr("噪声");
    }
}
