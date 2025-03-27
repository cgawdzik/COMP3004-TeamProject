#include "glucose_graph_widget.h"

GlucoseGraphWidget::GlucoseGraphWidget(QWidget *parent)
    : QWidget(parent),
    glucoseSeries(new QLineSeries()),
    chart(new QChart()),
    axisX(new QValueAxis()),
    axisY(new QValueAxis())
{
    chart->legend()->hide();
    chart->addSeries(glucoseSeries);
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);

    glucoseSeries->attachAxis(axisX);
    glucoseSeries->attachAxis(axisY);

    axisX->setTitleText("Time (ticks)");
    axisX->setRange(0, 100);
    axisY->setTitleText("Glucose (mmol/L)");
    axisY->setRange(2, 15);

    auto *view = new QChartView(chart);
    view->setRenderHint(QPainter::Antialiasing);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(view);
    setLayout(layout);
}

void GlucoseGraphWidget::addReading(double glucose) {
    glucoseSeries->append(timeIndex++, glucose);
    if (timeIndex > 100) {
        axisX->setRange(timeIndex - 100, timeIndex);
    }
}
