#ifndef STATIC_OPERATIONS_H
#define STATIC_OPERATIONS_H

#include <QObject>
#include <QImage>
#include <QPixmap>
#include <QList>

class QGraphicsScene;
class QGraphicsView;
class QGraphicsPixmapItem;
class QGraphicsRectItem;
class QGraphicsLineItem;
class QGraphicsItem;

class StaticOperations : public QObject
{
    Q_OBJECT

public:
    enum DrawMode { None, DrawRect, DrawLine, Calibrate, Measure, AutoWindow };

    explicit StaticOperations(QObject *parent = nullptr);

    void setTarget(QGraphicsScene *scene, QGraphicsPixmapItem **pixmapItemPtr, QGraphicsView *view);

    // 图像处理
    QImage sharpen(const QImage &src) const;
    QImage meanFilter(const QImage &src) const;
    QImage negative(const QImage &src) const;
    QImage emboss(const QImage &src) const;
    QImage applyPseudoColor(const QImage &src) const;
    QPixmap drawHistogram(const QImage &src) const;

    // ROI 统计（自动调窗用）
    void computeRoiStats(const QImage &src, const QRectF &roi,
                         int &minVal, int &maxVal) const;

    // 尺寸标定系数
    void setCalibrationRatio(double ratio) { m_calibrationRatio = ratio; }
    double calibrationRatio() const { return m_calibrationRatio; }

    // 绘图模式
    void enableDrawRect();
    void enableDrawLine();
    void enableCalibrate();
    void enableMeasure();
    void enableAutoWindow();
    void disableDrawMode();
    bool isDrawingRect()   const { return m_drawMode == DrawRect; }
    bool isDrawingLine()   const { return m_drawMode == DrawLine; }
    bool isCalibrating()   const { return m_drawMode == Calibrate; }
    bool isMeasuring()     const { return m_drawMode == Measure; }
    bool isAutoWindow()    const { return m_drawMode == AutoWindow; }
    void clearDrawnItems();
    bool undoLastDrawnItem();
    void clearItemsByTag(int tag);

signals:
    void calibrationRequested(double pixelLength);
    void measurementCompleted(double pixelLength, double realDistance);
    void autoWindowRequested(const QRectF &roi);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QGraphicsScene            *m_scene;
    QGraphicsView             *m_view;
    QGraphicsPixmapItem      **m_pixmapItemPtr;

    DrawMode m_drawMode;

    QPointF m_drawStart;
    QGraphicsRectItem  *m_tempRect;
    QGraphicsLineItem  *m_tempLine;

    struct DrawnItem {
        QGraphicsItem *item;
        int tag; // DrawMode 枚举值
    };
    QList<DrawnItem> m_drawnItems;

    double m_calibrationRatio;
};

#endif // STATIC_OPERATIONS_H
