#include "basechart.h"

BaseChart::BaseChart(QObject *parent) : QObject(parent)
{
    plot = NULL;
}

void BaseChart::chart_init(QWidget *, MODE mode)
{
    this->mode = mode;

    if( (mode == HFCT1 && sqlcfg->get_para()->hfct1_sql.units == Units_pC) || (mode == HFCT2 && sqlcfg->get_para()->hfct2_sql.units == Units_pC)  ){
        max_value = 6000;
        min_value = -6000;
    }
    else if(mode == AA1 || mode == AA2 || mode == AE1 || mode == AE2){
        max_value = 60;
        min_value = 0;
    }
    else{
        max_value = 60;
        min_value = -60;
    }
}

void BaseChart::hide()
{
    if(plot != NULL)
        plot->hide();
}

void BaseChart::show()
{
    if(plot != NULL)
        plot->show();
}
