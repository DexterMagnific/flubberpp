#include "MainWindow.h"
#include "./ui_MainWindow.h"
#include "flubberpp.h"
#include "svg_d2qpainterpath.h"

#include <QPainterPath>
#include <QGraphicsScene>
#include <QDebug>
#include <QTimer>

static constexpr const char *path_strings[] = {
  "M12 17.27L18.18 21l-1.64-7.03L22 9.24l-7.19-.61L12 2 9.19 8.63 2 9.24l5.46 4.73L5.82 21z",
  "M12 21.35l-1.45-1.32C5.4 15.36 2 12.28 2 8.5 2 5.42 4.42 3 7.5 3c1.74 0 3.41.81 4.5 2.09C13.09 3.81 14.76 3 16.5 3 19.58 3 22 5.42 22 8.5c0 3.78-3.4 6.86-8.55 11.54L12 21.35z",
  "M23 5.5V20c0 2.2-1.8 4-4 4h-7.3c-1.08 0-2.1-.43-2.85-1.19L1 14.83s1.26-1.23 1.3-1.25c.22-.19.49-.29.79-.29.22 0 .42.06.6.16.04.01 4.31 2.46 4.31 2.46V4c0-.83.67-1.5 1.5-1.5S11 3.17 11 4v7h1V1.5c0-.83.67-1.5 1.5-1.5S15 .67 15 1.5V11h1V2.5c0-.83.67-1.5 1.5-1.5s1.5.67 1.5 1.5V11h1V5.5c0-.83.67-1.5 1.5-1.5s1.5.67 1.5 1.5z",
  "M21 16v-2l-8-5V3.5c0-.83-.67-1.5-1.5-1.5S10 2.67 10 3.5V9l-8 5v2l8-2.5V19l-2 1.5V22l3.5-1 3.5 1v-1.5L13 19v-5.5l8 2.5z",
  "M7 2v11h3v9l7-12h-4l4-8z",
  "M12 3v10.55c-.59-.34-1.27-.55-2-.55-2.21 0-4 1.79-4 4s1.79 4 4 4 4-1.79 4-4V7h4V3h-6z",
};

static constexpr unsigned nb = sizeof(path_strings)/sizeof(*path_strings);

// will contain result of svg string conversion to Qt shape
static QVector<QPolygonF> shapes;

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , interp(10.0f)
  , path_idx(0)
{
  ui->setupUi(this);
  QGraphicsScene *scene = new QGraphicsScene(this);
  ui->view->setScene(scene);
  ui->view->setRenderHint(QPainter::Antialiasing);

  // add item to hold interpolated shape
  interpItem = new QGraphicsPolygonItem();
  interpItem->setBrush(QBrush(QColor(0,255,0,50)));
  interpItem->setPen(QPen(Qt::darkGreen,1.5f));
  scene->addItem(interpItem);

  // setup time animation
  timeAnim.setStartValue(0.f);
  timeAnim.setEndValue(1.f);
  timeAnim.setDuration(700); // seconds
  timeAnim.setEasingCurve(QEasingCurve::InOutCubic);

  // Convert SVG string paths to shapes
  convertPaths();

  connect(&timeAnim, &QVariantAnimation::finished,
          this, &MainWindow::slot_interpolationFinished);

  // connect time animation to shape redraw
  connect(&timeAnim, &QVariantAnimation::valueChanged,
          this, &MainWindow::slot_updateShape);

  slot_triggerNextInterpolation();
}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::convertPaths()
{
  // Original paths are too small, so scale them
  QTransform t;
  t.scale(20,20);

  for (unsigned i=0; i<nb; i++) {
    QPainterPath p;
    // from string to QPainterPath
    if ( svg2path(path_strings[i], p) ) {
      // scale it
      p = t.map(p);
      // turn into polygons (discretize curves), take first shape
      shapes << p.toSubpathPolygons().at(0);
    }
  }
}

void MainWindow::slot_updateShape(const QVariant &value)
{
  QPolygonF poly;
  const auto &shape = interp.at(value.toFloat());
  for (const auto &pt: shape) {
    poly << QPointF(pt.x,pt.y);
  }

  interpItem->setPolygon(poly);
}

void MainWindow::slot_interpolationFinished()
{
  // wait a bit and proceed with the next interpolation
  QTimer::singleShot(200, [&]() { slot_triggerNextInterpolation(); });
}

void MainWindow::slot_triggerNextInterpolation()
{
  QPolygonF start_shape = shapes[path_idx%shapes.size()];
  QPolygonF end_shape = shapes[(path_idx+1)%shapes.size()];

  // Now convert shapes to flubberpp shapes
  flubberpp::VectorShape from;
  for (auto p: start_shape) {
    from.push_back({(float)p.x(),(float)p.y()});
  }

  flubberpp::VectorShape to;
  for (auto p: end_shape) {
    to.push_back({(float)p.x(),(float)p.y()});
  }

  interp.setStartShape(from);
  interp.setEndShape(to);

  path_idx++;

  timeAnim.start();
}