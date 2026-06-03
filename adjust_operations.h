#ifndef ADJUST_OPERATIONS_H
#define ADJUST_OPERATIONS_H

#include <QObject>
#include <QPixmap>

class QGraphicsScene;
class QGraphicsPixmapItem;

class AdjustOperations : public QObject
{
    Q_OBJECT

public:
    explicit AdjustOperations(QObject *parent = nullptr);

    void setTarget(QGraphicsScene *scene, QGraphicsPixmapItem **pixmapItemPtr);

    void grayscale();
    void invert();
    void threshold();
    void windowLevel(QWidget *parentWidget);
    void brightnessContrast(QWidget *parentWidget);

private:
    void replacePixmap(const QPixmap &pixmap);

    QGraphicsScene          *m_scene;
    QGraphicsPixmapItem    **m_pixmapItemPtr;
};

#endif // ADJUST_OPERATIONS_H
