#ifndef EDIT_OPERATIONS_H
#define EDIT_OPERATIONS_H

#include <QObject>

class QGraphicsScene;
class QGraphicsView;
class QGraphicsPixmapItem;
#include <QPoint>
#include <QRect>
#include <QWidget>

class QRubberBand;

class EditOperations : public QObject
{
    Q_OBJECT

public:
    explicit EditOperations(QObject *parent = nullptr);

    void setTarget(QGraphicsScene *scene, QGraphicsPixmapItem **pixmapItemPtr, QGraphicsView *view);

    void rotateCW();
    void rotateCCW();
    void crop();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void applyRotation(double angle);

    QGraphicsScene          *m_scene;
    QGraphicsView           *m_view;
    QGraphicsPixmapItem    **m_pixmapItemPtr;
    QRubberBand             *m_rubberBand;
    QPoint                   m_rubberBandOrigin;
    bool                     m_cropping;
};

#endif // EDIT_OPERATIONS_H
