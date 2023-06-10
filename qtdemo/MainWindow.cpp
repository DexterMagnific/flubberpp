#include "MainWindow.h"
#include "./ui_MainWindow.h"
#include "flubberpp.h"
//#include "svg_d2qpainterpath.h"

#include <QPainterPath>
#include <QGraphicsScene>
#include <QDebug>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>

// will contain result of svg string conversion to Qt shape
static QVector<QPolygonF> shapes;

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , paused(false)
  , interp(10.0f)
  , path_idx(0)
{
  ui->setupUi(this);
  QGraphicsScene *scene = new QGraphicsScene(this);
  ui->view->setScene(scene);
  ui->view->setRenderHint(QPainter::Antialiasing);
  ui->playBtn->setFocus();

  ui->colorCombo->addItem("Red", QColor(Qt::red));
  ui->colorCombo->addItem("Green", QColor(Qt::darkGreen));
  ui->colorCombo->addItem("Blue", QColor(Qt::blue));
  ui->colorCombo->addItem("Yellow", QColor(Qt::darkYellow));
  ui->colorCombo->addItem("Cyan", QColor(Qt::darkCyan));
  ui->colorCombo->addItem("Purple", QColor(QColorConstants::Svg::purple));

  ui->dataCombo->addItem("Basic shapes", ":/datasets/basic-shapes.json");
  ui->dataCombo->addItem("US States", ":/datasets/us-states.json");

  // add item to hold interpolated shape
  interpItem = new QGraphicsPolygonItem();
  interpItem->setBrush(QBrush(QColor(0,255,0,50)));
  interpItem->setPen(QPen(Qt::darkGreen,1.5f));
  scene->addItem(interpItem);

  // setup time animation
  timeAnim.setStartValue(0.f);
  timeAnim.setEndValue(1.f);
  timeAnim.setDuration(500); // seconds
  //timeAnim.setEasingCurve(QEasingCurve::InOutCubic);

  pauseTimer = new QTimer(this);
  pauseTimer->setSingleShot(true);

  // connect time animation to shape redraw
  connect(&timeAnim, &QVariantAnimation::valueChanged,
          this, &MainWindow::slot_updateShape);

  // time animation finished
  connect(&timeAnim, &QVariantAnimation::finished,
          this, &MainWindow::slot_interpolationFinished);

  // pause timer finished
  connect(pauseTimer, &QTimer::timeout,
          this, &MainWindow::slot_triggerNextInterpolation);

  // UI
  connect(ui->playBtn, &QToolButton::clicked,
          this, &MainWindow::slot_playBtnClicked);
  connect(ui->timeSlider, &QSlider::valueChanged,
          this, &::MainWindow::slot_timeSliderChanged);
  connect(ui->nextBtn, &QToolButton::clicked,
          this, &MainWindow::slot_nextBtnClicked);
  connect(ui->prevBtn, &QToolButton::clicked,
          this, &MainWindow::slot_prevBtnClicked);
  connect(ui->colorCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &MainWindow::slot_colorChanged);
  connect(ui->dataCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &MainWindow::slot_datasetChanged);

  slot_colorChanged(0);
  slot_datasetChanged(0);

  //slot_triggerNextInterpolation();
}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::slot_updateShape(const QVariant &value)
{
  if ( !paused ) {
    // only update slider from timeAnim when "playing", not when changing the slider manually
    QSignalBlocker b(ui->timeSlider);
    ui->timeSlider->setValue(value.toFloat()*1000);
  }
  ui->timeLbl->setText(QString("%1").arg(value.toFloat(),0,'g',2));

  QPolygonF poly;
  const auto &shape = interp.at(value.toFloat());
  for (const auto &pt: shape) {
    poly << QPointF(pt.x,pt.y);
  }

  interpItem->setPolygon(poly);
}

void MainWindow::slot_interpolationFinished()
{
  if ( !paused ) {
    // wait a bit and proceed with the next interpolation
    pauseTimer->start(200);
  }
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

  // be sure to draw the initial figure, in case the animation is paused
  slot_updateShape(0);

  path_idx++;

  {
    QSignalBlocker b(ui->timeSlider);
    QSignalBlocker bb(timeAnim);
    ui->timeSlider->setValue(0);
    timeAnim.setCurrentTime(0);
  }

  if ( !paused )
    timeAnim.start();
}

void MainWindow::slot_playBtnClicked(bool)
{
  paused = !paused;
  if ( paused ) {
    if ( timeAnim.state() == QAbstractAnimation::Running )
      timeAnim.pause();
    ui->playBtn->setIcon(QIcon::fromTheme("media-playback-start"));
    ui->prevBtn->setEnabled(true);
    ui->nextBtn->setEnabled(true);
    ui->timeSlider->setEnabled(true);
  } else {
    if ( timeAnim.state() == QAbstractAnimation::Paused )
      timeAnim.resume();
    else
      timeAnim.start();
    ui->playBtn->setIcon(QIcon::fromTheme("media-playback-pause"));
    ui->prevBtn->setEnabled(false);
    ui->nextBtn->setEnabled(false);
    ui->timeSlider->setEnabled(false);
  }
}

void MainWindow::slot_timeSliderChanged(int value)
{
  timeAnim.setCurrentTime((float)value*timeAnim.duration()/ui->timeSlider->maximum());
}

void MainWindow::slot_nextBtnClicked(bool)
{
  if ( timeAnim.state() == QAbstractAnimation::Running )
    timeAnim.pause();
  slot_triggerNextInterpolation();
}

void MainWindow::slot_prevBtnClicked(bool)
{
  if ( timeAnim.state() == QAbstractAnimation::Running )
    timeAnim.pause();
  path_idx = (path_idx + shapes.size() - 2)%shapes.size();
  slot_triggerNextInterpolation();
}

void MainWindow::slot_colorChanged(int idx)
{
  QColor c = ui->colorCombo->itemData(idx).value<QColor>();
  interpItem->setPen(QPen(c,2.5f));
  c.setAlpha(100);
  interpItem->setBrush(QBrush(c.lighter()));
}

void MainWindow::slot_datasetChanged(int idx)
{
  QString filename = ui->dataCombo->itemData(idx).toString();
  QFile f(filename);
  if ( !f.open(QIODevice::ReadOnly) )
    return;

  QJsonParseError err;
  QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &err);
  if ( err.error != QJsonParseError::NoError )
    return;

  if ( !doc.isArray() )
    return;

  QJsonArray jall = doc.array();

  shapes.clear();

  for (auto js: jall) {
    if ( !js.isArray() )
      continue;

    QJsonArray jpoly = js.toArray();
    QPolygonF poly;
    for (auto jpoint: jpoly) {
      if ( !jpoint.isArray() )
           continue;
      QJsonArray pt = jpoint.toArray();
      if ( pt.size() != 2 )
        continue;
      if ( !pt.at(0).isDouble() )
        continue;
      if ( !pt.at(1).isDouble() )
        continue;

      poly << QPointF(pt.at(0).toDouble(),pt.at(1).toDouble());
    }

    shapes << poly;
  }

  if ( timeAnim.state() == QAbstractAnimation::Running )
    timeAnim.pause();
  path_idx = 0;

  slot_triggerNextInterpolation();
}
