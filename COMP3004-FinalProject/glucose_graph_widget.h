#ifndef GLUCOSE_GRAPH_WIDGET_H
#define GLUCOSE_GRAPH_WIDGET_H

#include <QtCharts>
#include <QWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>


QT_BEGIN_NAMESPACE
namespace QtCharts { }
QT_END_NAMESPACE

using namespace QtCharts;

    class GlucoseGraphWidget : public QWidget {
    Q_OBJECT

public:
    explicit GlucoseGraphWidget(QWidget *parent = nullptr);
    void addReading(double glucose);

private:
    QLineSeries *glucoseSeries;
    QChart *chart;
    QValueAxis *axisX;
    QValueAxis *axisY;
    int timeIndex = 0;
};

#endif
