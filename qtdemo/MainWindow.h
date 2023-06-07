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

  private:
    void convertPaths();

    Ui::MainWindow *ui;
    flubberpp::SingleInterpolator interp;
    // animates time
    QVariantAnimation timeAnim;
    // graphical representation of the shape
    QGraphicsPolygonItem *interpItem;
    unsigned path_idx;
};
