#include "static_operations.h"
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QGraphicsLineItem>
#include <QMouseEvent>
#include <QPen>
#include <QColor>
#include <QPainter>
#include <QtMath>
#include <QVector>

StaticOperations::StaticOperations(QObject *parent)
    : QObject(parent),
      m_scene(nullptr),
      m_view(nullptr),
      m_pixmapItemPtr(nullptr),
      m_drawMode(None),
      m_tempRect(nullptr),
      m_tempLine(nullptr),
      m_calibrationRatio(1.0)
{
}

void StaticOperations::setTarget(QGraphicsScene *scene, QGraphicsPixmapItem **pixmapItemPtr, QGraphicsView *view)
{
    m_scene = scene;
    m_view = view;
    m_pixmapItemPtr = pixmapItemPtr;
}

// ==================== 绘图模式 ====================

void StaticOperations::enableDrawRect()
{
    if (!m_view || !m_pixmapItemPtr || !(*m_pixmapItemPtr)) return;
    disableDrawMode();
    m_drawMode = DrawRect;
    m_view->setDragMode(QGraphicsView::NoDrag);
    m_view->viewport()->setCursor(Qt::CrossCursor);
    m_view->viewport()->installEventFilter(this);
}

void StaticOperations::enableDrawLine()
{
    if (!m_view || !m_pixmapItemPtr || !(*m_pixmapItemPtr)) return;
    disableDrawMode();
    m_drawMode = DrawLine;
    m_view->setDragMode(QGraphicsView::NoDrag);
    m_view->viewport()->setCursor(Qt::CrossCursor);
    m_view->viewport()->installEventFilter(this);
}

void StaticOperations::enableCalibrate()
{
    if (!m_view || !m_pixmapItemPtr || !(*m_pixmapItemPtr)) return;
    disableDrawMode();
    m_drawMode = Calibrate;
    m_view->setDragMode(QGraphicsView::NoDrag);
    m_view->viewport()->setCursor(Qt::CrossCursor);
    m_view->viewport()->installEventFilter(this);
}

void StaticOperations::enableMeasure()
{
    if (!m_view || !m_pixmapItemPtr || !(*m_pixmapItemPtr)) return;
    disableDrawMode();
    m_drawMode = Measure;
    m_view->setDragMode(QGraphicsView::NoDrag);
    m_view->viewport()->setCursor(Qt::CrossCursor);
    m_view->viewport()->installEventFilter(this);
}

void StaticOperations::enableAutoWindow()
{
    if (!m_view || !m_pixmapItemPtr || !(*m_pixmapItemPtr)) return;
    disableDrawMode();
    m_drawMode = AutoWindow;
    m_view->setDragMode(QGraphicsView::NoDrag);
    m_view->viewport()->setCursor(Qt::CrossCursor);
    m_view->viewport()->installEventFilter(this);
}

void StaticOperations::disableDrawMode()
{
    m_drawMode = None;
    if (m_tempRect) { m_scene->removeItem(m_tempRect); delete m_tempRect; m_tempRect = nullptr; }
    if (m_tempLine) { m_scene->removeItem(m_tempLine); delete m_tempLine; m_tempLine = nullptr; }
    if (m_view)
    {
        m_view->setDragMode(QGraphicsView::ScrollHandDrag);
        m_view->viewport()->setCursor(Qt::ArrowCursor);
        m_view->viewport()->removeEventFilter(this);
    }
}

void StaticOperations::clearDrawnItems()
{
    for (auto &di : m_drawnItems) {
        m_scene->removeItem(di.item);
        delete di.item;
    }
    m_drawnItems.clear();
}

bool StaticOperations::undoLastDrawnItem()
{
    if (m_drawnItems.isEmpty()) return false;
    auto di = m_drawnItems.takeLast();
    m_scene->removeItem(di.item);
    delete di.item;
    return true;
}

void StaticOperations::clearItemsByTag(int tag)
{
    for (int i = m_drawnItems.size() - 1; i >= 0; --i) {
        if (m_drawnItems[i].tag == tag) {
            m_scene->removeItem(m_drawnItems[i].item);
            delete m_drawnItems[i].item;
            m_drawnItems.removeAt(i);
        }
    }
}

bool StaticOperations::eventFilter(QObject *obj, QEvent *event)
{
    if (m_drawMode == None) return QObject::eventFilter(obj, event);
    if (obj != m_view && obj != m_view->viewport())
        return QObject::eventFilter(obj, event);

    QMouseEvent *me = static_cast<QMouseEvent*>(event);
    bool isRectMode  = (m_drawMode == DrawRect || m_drawMode == AutoWindow);
    bool isLineMode  = (m_drawMode == DrawLine || m_drawMode == Calibrate || m_drawMode == Measure);

    switch (event->type())
    {
    case QEvent::MouseButtonPress:
        if (me->button() == Qt::LeftButton)
        {
            m_drawStart = m_view->mapToScene(me->pos());

            if (isRectMode)
            {
                QColor color = (m_drawMode == AutoWindow) ? Qt::yellow : Qt::red;
                m_tempRect = m_scene->addRect(QRectF(m_drawStart, QSizeF(0, 0)),
                                              QPen(color, 2));
            }
            else if (isLineMode)
            {
                QColor color = Qt::red;
                if (m_drawMode == Calibrate)      color = Qt::green;
                else if (m_drawMode == Measure)   color = Qt::blue;
                m_tempLine = m_scene->addLine(QLineF(m_drawStart, m_drawStart),
                                              QPen(color, 2));
            }
            return true;
        }
        break;

    case QEvent::MouseMove:
        if (isRectMode && m_tempRect)
        {
            QPointF pos = m_view->mapToScene(me->pos());
            m_tempRect->setRect(QRectF(m_drawStart, pos).normalized());
            return true;
        }
        else if (isLineMode && m_tempLine)
        {
            QPointF pos = m_view->mapToScene(me->pos());
            m_tempLine->setLine(QLineF(m_drawStart, pos));
            return true;
        }
        break;

    case QEvent::MouseButtonRelease:
        if (me->button() == Qt::LeftButton)
        {
            QPointF pos = m_view->mapToScene(me->pos());
            QPixmap src = (*m_pixmapItemPtr)->pixmap();

            if (isRectMode && m_tempRect)
            {
                QRectF r = QRectF(m_drawStart, pos).normalized();
                QRectF imgRect(0, 0, src.width(), src.height());
                r = r.intersected(imgRect);
                if (r.width() > 2 && r.height() > 2)
                {
                    m_tempRect->setRect(r);
                    if (m_drawMode == AutoWindow) {
                        // 只保留最新的自动调窗矩形
                        clearItemsByTag(int(AutoWindow));
                        m_drawnItems.append({m_tempRect, int(AutoWindow)});
                        m_tempRect = nullptr;
                        emit autoWindowRequested(r);
                    } else {
                        m_drawnItems.append({m_tempRect, int(m_drawMode)});
                        m_tempRect = nullptr;
                    }
                }
                else
                {
                    m_scene->removeItem(m_tempRect);
                    delete m_tempRect;
                    m_tempRect = nullptr;
                }
            }
            else if (isLineMode && m_tempLine)
            {
                QLineF line(m_drawStart, pos);
                double pixelLen = line.length();
                m_tempLine->setLine(line);
                m_drawnItems.append({m_tempLine, int(m_drawMode)});
                m_tempLine = nullptr;

                if (m_drawMode == Calibrate) {
                    emit calibrationRequested(pixelLen);
                } else if (m_drawMode == Measure) {
                    double realDist = pixelLen * m_calibrationRatio;
                    emit measurementCompleted(pixelLen, realDist);
                }
            }
            // 不自动退出绘图模式
            return true;
        }
        break;

    default:
        break;
    }

    return QObject::eventFilter(obj, event);
}

// ==================== 图像处理 ====================

QImage StaticOperations::sharpen(const QImage &src) const
{
    if (src.isNull()) return src;
    QImage dst(src.size(), src.format());
    if (src.format() != QImage::Format_Grayscale8 && src.format() != QImage::Format_Indexed8)
        dst = src.convertToFormat(QImage::Format_Grayscale8);

    int w = src.width(), h = src.height();
    // 锐化核:  [ 0 -1  0; -1  5 -1;  0 -1  0 ]
    const int kernel[3][3] = { {0, -1, 0}, {-1, 5, -1}, {0, -1, 0} };

    for (int y = 1; y < h - 1; ++y)
        for (int x = 1; x < w - 1; ++x)
        {
            int sum = 0;
            for (int ky = -1; ky <= 1; ++ky)
                for (int kx = -1; kx <= 1; ++kx)
                    sum += qGray(src.pixel(x + kx, y + ky)) * kernel[ky + 1][kx + 1];
            int v = qBound(0, sum, 255);
            dst.setPixel(x, y, qRgb(v, v, v));
        }
    // 边界复制
    for (int x = 0; x < w; ++x)
    {
        dst.setPixel(x, 0, src.pixel(x, 0));
        dst.setPixel(x, h - 1, src.pixel(x, h - 1));
    }
    for (int y = 0; y < h; ++y)
    {
        dst.setPixel(0, y, src.pixel(0, y));
        dst.setPixel(w - 1, y, src.pixel(w - 1, y));
    }
    return dst;
}

QImage StaticOperations::meanFilter(const QImage &src) const
{
    if (src.isNull()) return src;
    QImage dst(src.size(), QImage::Format_Grayscale8);
    int w = src.width(), h = src.height();
    int radius = 1; // 3x3 邻域
    int kernelSize = 3;
    int divisor = kernelSize * kernelSize;

    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
        {
            int sum = 0, count = 0;
            for (int ky = -radius; ky <= radius; ++ky)
                for (int kx = -radius; kx <= radius; ++kx)
                {
                    int px = qBound(0, x + kx, w - 1);
                    int py = qBound(0, y + ky, h - 1);
                    sum += qGray(src.pixel(px, py));
                    ++count;
                }
            int v = sum / count;
            dst.setPixel(x, y, qRgb(v, v, v));
        }
    return dst;
}

QImage StaticOperations::negative(const QImage &src) const
{
    if (src.isNull()) return src;
    QImage dst = src.copy();
    dst.invertPixels();
    return dst;
}

QImage StaticOperations::emboss(const QImage &src) const
{
    if (src.isNull()) return src;
    QImage dst(src.size(), QImage::Format_Grayscale8);
    int w = src.width(), h = src.height();
    // 浮雕核
    const int kernel[3][3] = { {-2, -1, 0}, {-1, 1, 1}, {0, 1, 2} };

    for (int y = 1; y < h - 1; ++y)
        for (int x = 1; x < w - 1; ++x)
        {
            int sum = 0;
            for (int ky = -1; ky <= 1; ++ky)
                for (int kx = -1; kx <= 1; ++kx)
                    sum += qGray(src.pixel(x + kx, y + ky)) * kernel[ky + 1][kx + 1];
            int v = qBound(0, sum + 128, 255); // +128 增加浮雕亮度
            dst.setPixel(x, y, qRgb(v, v, v));
        }
    // 边界
    for (int x = 0; x < w; ++x)
        dst.setPixel(x, 0, qRgb(128, 128, 128));
    for (int y = 0; y < h; ++y)
    {
        dst.setPixel(0, y, qRgb(128, 128, 128));
        dst.setPixel(w - 1, y, qRgb(128, 128, 128));
    }
    for (int x = 0; x < w; ++x)
        dst.setPixel(x, h - 1, qRgb(128, 128, 128));
    return dst;
}

QPixmap StaticOperations::drawHistogram(const QImage &src) const
{
    if (src.isNull()) return QPixmap();

    // 统计灰度直方图
    QVector<int> hist(256, 0);
    int w = src.width(), h = src.height();
    int totalPixels = w * h;
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
        {
            int gray = qGray(src.pixel(x, y));
            if (gray >= 0 && gray <= 255)
                ++hist[gray];
        }

    // 找到最大值用于归一化
    int maxCount = 0;
    for (int i = 0; i < 256; ++i)
        if (hist[i] > maxCount) maxCount = hist[i];

    // 绘制直方图
    int margin = 40;
    int plotW = 512, plotH = 300;
    int barW = plotW / 256;
    QPixmap canvas(plotW + margin * 2, plotH + margin * 2);
    canvas.fill(Qt::white);
    QPainter painter(&canvas);
    painter.setRenderHint(QPainter::Antialiasing);

    // 坐标轴
    painter.setPen(Qt::black);
    painter.drawLine(margin, margin, margin, plotH + margin);                // Y轴
    painter.drawLine(margin, plotH + margin, plotW + margin, plotH + margin); // X轴

    // 绘制柱状图
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(100, 140, 200));
    for (int i = 0; i < 256; ++i)
    {
        if (hist[i] == 0) continue;
        int barH = (int)((double)hist[i] / maxCount * plotH);
        painter.drawRect(margin + i * barW, plotH + margin - barH,
                         barW > 1 ? barW : 1, barH);
    }

    // 标注
    painter.setPen(Qt::black);
    QFont font = painter.font();
    font.setPointSize(9);
    painter.setFont(font);
    painter.drawText(margin, margin - 5, tr("Histogram"));
    painter.drawText(margin, plotH + margin + 15, "0");
    painter.drawText(margin + plotW - 30, plotH + margin + 15, "255");

    // Y轴刻度
    painter.drawText(margin - 30, plotH + margin, "0");
    painter.drawText(margin - 30, margin + 5, QString::number(maxCount));

    painter.end();
    return canvas;
}

// ==================== 伪彩色 ====================
QImage StaticOperations::applyPseudoColor(const QImage &src) const
{
    if (src.isNull()) return src;
    QImage dst(src.size(), QImage::Format_RGB32);
    int w = src.width(), h = src.height();

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int gray = qGray(src.pixel(x, y));
            double t = gray / 255.0;
            int r, g, b;
            // Jet colormap: blue → cyan → green → yellow → red
            if (t < 0.125) {
                r = 0; g = 0; b = qBound(0, (int)(128 + 127 * (t / 0.125)), 255);
            } else if (t < 0.375) {
                double s = (t - 0.125) / 0.25;
                r = 0; g = qBound(0, (int)(255 * s), 255); b = 255;
            } else if (t < 0.625) {
                double s = (t - 0.375) / 0.25;
                r = qBound(0, (int)(255 * s), 255); g = 255;
                b = qBound(0, (int)(255 * (1.0 - s)), 255);
            } else if (t < 0.875) {
                double s = (t - 0.625) / 0.25;
                r = 255; g = qBound(0, (int)(255 * (1.0 - s)), 255); b = 0;
            } else {
                double s = (t - 0.875) / 0.125;
                r = qBound(0, (int)(128 + 127 * (1.0 - s)), 255); g = 0; b = 0;
            }
            dst.setPixel(x, y, qRgb(r, g, b));
        }
    }
    return dst;
}

// ==================== ROI 统计（自动调窗，ImageJ 风格） ====================
void StaticOperations::computeRoiStats(const QImage &src, const QRectF &roi,
                                       int &minVal, int &maxVal) const
{
    int hist[256] = {0};
    int x0 = qMax(0, (int)roi.left());
    int y0 = qMax(0, (int)roi.top());
    int x1 = qMin(src.width() - 1, (int)roi.right());
    int y1 = qMin(src.height() - 1, (int)roi.bottom());

    int total = 0;
    for (int y = y0; y <= y1; ++y) {
        for (int x = x0; x <= x1; ++x) {
            int v = qGray(src.pixel(x, y));
            hist[qBound(0, v, 255)]++;
            total++;
        }
    }

    if (total == 0) { minVal = 0; maxVal = 255; return; }

    // 裁剪两端各 0.5% 的极端像素（ImageJ 默认行为）
    const double lowPct = 0.005, highPct = 0.995;
    int lowThresh  = qMax(1, (int)(total * lowPct));
    int highThresh = (int)(total * highPct);

    int sum = 0;
    minVal = 0;
    for (int i = 0; i < 256; ++i) {
        sum += hist[i];
        if (sum >= lowThresh)  { minVal = i; break; }
    }

    sum = 0;
    maxVal = 255;
    for (int i = 0; i < 256; ++i) {
        sum += hist[i];
        if (sum >= highThresh) { maxVal = i; break; }
    }

    if (maxVal <= minVal) { minVal = 0; maxVal = 255; }
}
