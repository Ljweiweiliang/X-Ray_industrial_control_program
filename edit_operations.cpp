#include "edit_operations.h"
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QRubberBand>
#include <QMouseEvent>
#include <QRect>

EditOperations::EditOperations(QObject *parent)
    : QObject(parent),
      m_scene(nullptr),
      m_view(nullptr),
      m_pixmapItemPtr(nullptr),
      m_rubberBand(nullptr),
      m_cropping(false)
{
}

void EditOperations::setTarget(QGraphicsScene *scene, QGraphicsPixmapItem **pixmapItemPtr, QGraphicsView *view)
{
    m_scene = scene;
    m_view = view;
    m_pixmapItemPtr = pixmapItemPtr;
}

void EditOperations::rotateCW()
{
    applyRotation(90);
}

void EditOperations::rotateCCW()
{
    applyRotation(-90);
}

void EditOperations::applyRotation(double angle)
{
    if (!m_pixmapItemPtr || !(*m_pixmapItemPtr)) return;
    (*m_pixmapItemPtr)->setRotation((*m_pixmapItemPtr)->rotation() + angle);
    m_scene->setSceneRect(m_scene->itemsBoundingRect());
}

void EditOperations::crop()
{
    if (!m_pixmapItemPtr || !(*m_pixmapItemPtr) || !m_view) return;

    m_cropping = true;
    m_view->setDragMode(QGraphicsView::NoDrag);
    m_view->viewport()->setCursor(Qt::CrossCursor);
    m_view->viewport()->installEventFilter(this);

    if (!m_rubberBand)
        m_rubberBand = new QRubberBand(QRubberBand::Rectangle, m_view);
}

bool EditOperations::eventFilter(QObject *obj, QEvent *event)
{
    if (!m_cropping || (obj != m_view && obj != m_view->viewport()))
        return QObject::eventFilter(obj, event);

    QMouseEvent *me = static_cast<QMouseEvent*>(event);

    switch (event->type())
    {
    case QEvent::MouseButtonPress:
        if (me->button() == Qt::LeftButton)
        {
            m_rubberBandOrigin = me->pos();
            m_rubberBand->setGeometry(QRect(m_rubberBandOrigin, QSize()));
            m_rubberBand->show();
            return true;
        }
        break;

    case QEvent::MouseMove:
        if (m_rubberBand->isVisible())
        {
            QRect r = QRect(m_rubberBandOrigin, me->pos()).normalized();
            m_rubberBand->setGeometry(r);
            return true;
        }
        break;

    case QEvent::MouseButtonRelease:
        if (me->button() == Qt::LeftButton && m_rubberBand->isVisible())
        {
            m_rubberBand->hide();
            m_cropping = false;
            m_view->setDragMode(QGraphicsView::ScrollHandDrag);
            m_view->viewport()->setCursor(Qt::ArrowCursor);
            m_view->viewport()->removeEventFilter(this);

            QRect viewRect = m_rubberBand->geometry();
            if (viewRect.width() < 5 || viewRect.height() < 5)
                return true;

            // 将视口坐标映射到场景坐标
            QRectF sceneRect = m_view->mapToScene(viewRect).boundingRect();

            // 裁剪图像
            QPixmap src = (*m_pixmapItemPtr)->pixmap();
            QRectF imgRect(0, 0, src.width(), src.height());
            QRectF cropRect = sceneRect.intersected(imgRect);
            if (cropRect.isEmpty()) return true;

            QPixmap cropped = src.copy(cropRect.toRect());
            (*m_pixmapItemPtr)->setPixmap(cropped);
            (*m_pixmapItemPtr)->setRotation(0);
            m_scene->setSceneRect(cropped.rect());
            return true;
        }
        break;

    default:
        break;
    }

    return QObject::eventFilter(obj, event);
}
