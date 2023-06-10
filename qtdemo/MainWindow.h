#pragma once

#include <QMainWindow>
#include <QVariantAnimation>
#include <QGraphicsPolygonItem>

#include <flubberpp.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

  public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

  private slots:
    void slot_updateShape(const QVariant &value);
    void slot_interpolationFinished();
    void slot_triggerNextInterpolation();
    void slot_playBtnClicked(bool);
    void slot_timeSliderChanged(int value);
    void slot_nextBtnClicked(bool);
    void slot_prevBtnClicked(bool);
    void slot_colorChanged(int idx);
    void slot_datasetChanged(int idx);

  private:
    Ui::MainWindow *ui;
    bool paused;
    flubberpp::SingleInterpolator interp;
    // animates time from 0 to 1
    QVariantAnimation timeAnim;
    // pause timer until next interpolation
    QTimer *pauseTimer;
    // graphical representation of the shape
    QGraphicsPolygonItem *interpItem;
    unsigned path_idx;
};
