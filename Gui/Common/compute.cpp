#include "compute.h"
#include "common.h"

Compute::Compute(QObject *parent) : QObject(parent)
{

}

QPoint Compute::trans_data(int x, int y, MODE mode)
{
    if(mode != TEV1 && mode != TEV2)
        return QPoint();

    switch (mode) {
    case TEV1:
        y = sqlcfg->get_para()->tev1_sql.gain * H_C_FACTOR * (y - sqlcfg->get_para()->tev1_sql.fpga_zero) ; //注意，脉冲计算里，忽略了噪声偏置的影响
        break;
    case TEV2:
        y = sqlcfg->get_para()->tev2_sql.gain * H_C_FACTOR * (y - sqlcfg->get_para()->tev2_sql.fpga_zero) ; //注意，脉冲计算里，忽略了噪声偏置的影响
        break;
    default:
        break;
    }
    //取DB值
    if(y>=1){
        y = ((double)20) * log10(y);
    }
    else if(y<=-1){
        y = -((double)20) * log10(-y);
    }
    else{
        y = 0;
    }
    x = Common::time_to_phase(x);
    return QPoint(x,y);
}

QVector<PC_DATA> Compute::compute_pc_1ms(QVector<int> list, int x_origin, double gain, int threshold)
{
    QVector<PC_DATA> pclist_1ms;

    if(list.length() < 2 )
        return pclist_1ms;

    PC_DATA pc_data;
    int first=0,last=0;
    for (int i = 0; i < list.length() - 1; ++i) {
        if(list.at(i) * list.at(i+1) <= 0){
            last = i+1;
            if(last-first > 1){         //至少有3个点
                QVector<int> list_1node = list.mid(first,last-first+1);

//                if(max_abs(list_1node.mid(1,list_1node.count()-2)).y() > threshold ){
                QPoint P = max_abs(list_1node.mid(1,list_1node.count()-2));
                if(P.y() > threshold ){
//                    qDebug()<<"max value:"<< P << "\tthreshold"<<threshold<<list_1node;
                    pc_data = compute_pc_1node(list_1node, x_origin, gain);  //计算一个脉冲的pC值
                    if(qAbs(pc_data.pc_value) > 10){            //认为值严格为0的点是无意义点(可加入更严格的筛选条件)
                        pclist_1ms.append( pc_data );
                    }
                }
                first = i;
            }
        }
    }

//    if(pclist_1ms.isEmpty()  && max_abs(list).y() > threshold){           //序列没有子脉冲，则把整个序列看做一个脉冲
//        pclist_1ms.append(compute_pc_1node(list, x_origin, gain) );
//    }

    return pclist_1ms;
}

PC_DATA Compute::compute_pc_1node(QVector<int> list, int x_origin, double gain)
{
    PC_DATA pc_data;
    pc_data.pc_value = 0;

    if(list.length() < 2 )
        return pc_data;

    double t1=0, t2=0, s=0;                 //前后中面积
    int first = 0, last = list.length()-1;  //前中,后中分割点
    double first_zero = first, last_zero = last, peak = first;     //第一个零点,第二个零点,峰值点

    if(list.at(0) * list.at(1) <= 0 && list.at(0) != list.at(1)){       //判断符号变化,且不同时为0
        t1 = triangle(list.at(1), list.at(0));
        first_zero = list.at(0) / (list.at(0) - list.at(1));
        first = 1;
    }

    if(list.at(last-1) * list.at(last) <= 0 && list.at(last-1) != list.at(last)){
        t2 = triangle(list.at(last-1), list.at(last));
        last_zero = last - 1 + list.at(last-1) / (list.at(last-1) - list.at(last));
        last = list.length() - 2;
    }

    s = simpson(list.mid(first,last - first + 1));

    //求峰值位置
    peak = first;
    for (int i = 0; i < last - first + 1; ++i) {
        if( qAbs(list.at(first + i))  > qAbs( list.at(peak)) ) {
            peak = first + i;
        }
    }

    //    qDebug()<<"t1="<<t1<<"\ts="<<s<<"\tt2="<<t2<<"\t"<<list << "simpson :"<<list.mid(first,last - first + 1);

    pc_data.pc_value = gain * H_C_FACTOR * (t1 + s + t2);
    pc_data.phase = x_origin;

    pc_data.rise_time = 10 * (peak - first_zero);       //单位为ns
    pc_data.fall_time = 10 * (last_zero - peak);        //单位为ns
    return pc_data;
}

//list是已标准化的脉冲序列（如何标准化，以后探讨）
//list长度至少为2
//函数实现用复化辛普生公式求积分
double Compute::simpson(QVector<int> list)
{
    if(list.length() < 2 )
        return 0;

    if(list.length() == 2)
        return (list.at(0) + list.at(1)) / 2.0 ;      //梯形公式

    int n = (list.length()-1) / 2 ;     //n至少为1
    double S = list.at(0) - list.at(2*n);
    for (int i = 1; i <= n; ++i) {
        S += 4*list.at(2*i-1) + 2*list.at(2*i);
    }
    if(list.length() % 2 == 1){     //如果长度为奇数，正好使用复化辛普生公式，否则补上一个梯形公式做结尾
        return S / 3.0;
    }
    else{
        return (list.at(2*n) + list.last() )/2.0 + S/3.0 ;
    }
}

//返回序列首（尾）遇到符号变化时，三角形的面积
//d1为靠近序列内部的数据点，d2为序列首（尾）部数据点
double Compute::triangle(double d1, double d2)
{
    return d1*d1 / (d1 - d2) / 2.0;
}

//求一组测量值的50Hz频率分量和100Hz频率分量
void Compute::compute_f_value(QVector<int> list, FFT *fft, int &v_50Hz, int &v_100Hz)
{
    QVector<double> fft_50Hz, fft_100Hz;
    for (int i = 0; i < list.count() / 32; ++i) {
        list = fft->fft32(list.mid(i*32,32));
        fft_50Hz.append(list.at(1));
        fft_100Hz.append(list.at(2));
    }
    v_50Hz = Common::avrage(fft_50Hz);
    v_100Hz = Common::avrage(fft_100Hz);
}

//寻找一个数组的最大值点
//返回值为(最大值序号,最大值)
QPoint Compute::max(QVector<int> list)
{
    QPoint P;
    if(!list.isEmpty()){
        int m = list.at(0), max_i=0;
        for (int i = 0; i < list.count(); ++i) {
            if(m < list.at(i)){
                m = list.at(i);
                max_i = i;
            }
        }
        P.setX(max_i);
        P.setY(m);
    }
    return P;
}

//寻找一个数组的最小值点
//返回值为(最小值序号,最小值)
QPoint Compute::min(QVector<int> list)
{
    QPoint P;
    if(!list.isEmpty()){
        int m = list.at(0), min_i=0;
        for (int i = 0; i < list.count(); ++i) {
            if(m > list.at(i)){
                m = list.at(i);
                min_i = i;
            }
        }
        P.setX(min_i);
        P.setY(m);
    }
    return P;
}

//寻找一个数组的绝对值最大点
//返回值为(绝对值最大点序号,最大绝对值)
QPoint Compute::max_abs(QVector<int> list)
{
    QPoint P;
    if(!list.isEmpty()){
        int m = qAbs(list.at(0) ), max_i=0;
        for (int i = 0; i < list.count(); ++i) {
            if(m < qAbs(list.at(i) ) ){
                m = qAbs(list.at(i) );
                max_i = i;
            }
        }
        P.setX(max_i);
        P.setY(m);
    }
    return P;
}

QVector<int> Compute::sim_sin(int amp, int f, int n)
{
    QVector<int> list;
    for (int i = 0; i < n; ++i) {
        list.append( amp * sin(i*f*2*3.1416/100)  );
    }
    return list;
}

QVector<int> Compute::sim_pulse(int amp, int n)
{
    QVector<int> list;
    if(n<40){
        qDebug()<<"sim_pulse input points should greater than 40!";
        return list;
    }

    list.fill(0,n);
    for (int i = 0; i < 20; ++i) {
        list[20+i] = amp * (20-i) / 20;
    }
    return list;
}
