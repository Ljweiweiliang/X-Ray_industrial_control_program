#include "adjust_operations.h"
#include "windowleveldialog.h"
#include "brightnesscontrastdialog.h"
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QImage>
#include <QColor>
#include <QGraphicsView>

AdjustOperations::AdjustOperations(QObject *parent)
    : QObject(parent),
      m_scene(nullptr),
      m_pixmapItemPtr(nullptr)
{
}

void AdjustOperations::setTarget(QGraphicsScene *scene, QGraphicsPixmapItem **pixmapItemPtr)
{
    m_scene = scene;
    m_pixmapItemPtr = pixmapItemPtr;
}

void AdjustOperations::replacePixmap(const QPixmap &pixmap)
{
    if (!m_pixmapItemPtr || !(*m_pixmapItemPtr)) return;
    (*m_pixmapItemPtr)->setPixmap(pixmap);
    (*m_pixmapItemPtr)->setRotation(0);
    m_scene->setSceneRect(pixmap.rect());
}

void AdjustOperations::grayscale()
{
    if (!m_pixmapItemPtr || !(*m_pixmapItemPtr)) return;
    QImage img = (*m_pixmapItemPtr)->pixmap().toImage();
    if (img.isNull()) return;

    for (int y = 0; y < img.height(); ++y)
        for (int x = 0; x < img.width(); ++x)
        {
            QColor c(img.pixel(x, y));
            int gray = qGray(c.rgb());
            img.setPixel(x, y, qRgb(gray, gray, gray));
        }

    replacePixmap(QPixmap::fromImage(img));
}

void AdjustOperations::invert()
{
    if (!m_pixmapItemPtr || !(*m_pixmapItemPtr)) return;
    QImage img = (*m_pixmapItemPtr)->pixmap().toImage();
    if (img.isNull()) return;

    img.invertPixels();
    replacePixmap(QPixmap::fromImage(img));
}

void AdjustOperations::threshold()
{
    if (!m_pixmapItemPtr || !(*m_pixmapItemPtr)) return;
    QImage img = (*m_pixmapItemPtr)->pixmap().toImage();
    if (img.isNull()) return;

    const int thresh = 128;
    for (int y = 0; y < img.height(); ++y)
        for (int x = 0; x < img.width(); ++x)
        {
            QColor c(img.pixel(x, y));
            int gray = qGray(c.rgb());
            int val = (gray >= thresh) ? 255 : 0;
            img.setPixel(x, y, qRgb(val, val, val));
        }

    replacePixmap(QPixmap::fromImage(img));
}

void AdjustOperations::windowLevel(QWidget *parentWidget)
{
    if (!m_pixmapItemPtr || !(*m_pixmapItemPtr)) return;

    QPixmap src = (*m_pixmapItemPtr)->pixmap();
    double angle = (*m_pixmapItemPtr)->rotation();
    if (!qFuzzyCompare(angle, 0.0))
    {
        QTransform t;
        t.rotate(angle);
        src = src.transformed(t, Qt::SmoothTransformation);
    }

    int ww = 255, wl = 128;
    WindowLevelDialog dlg(ww, wl, 511, 255, parentWidget);

    connect(&dlg, &WindowLevelDialog::valuesChanged, this,
            [this, src](int width, int level) {
        // 在 8 位图上应用窗宽窗位
        QImage img = src.toImage();
        if (img.isNull()) return;
        double halfW = width / 2.0;
        double low   = level - halfW;
        double high  = level + halfW;
        for (int y = 0; y < img.height(); ++y)
            for (int x = 0; x < img.width(); ++x)
            {
                QColor c(img.pixel(x, y));
                int gray = qGray(c.rgb());
                int newVal;
                if (gray <= low) newVal = 0;
                else if (gray >= high) newVal = 255;
                else newVal = static_cast<int>((gray - low) / width * 255.0);
                img.setPixel(x, y, qRgb(newVal, newVal, newVal));
            }
        replacePixmap(QPixmap::fromImage(img));
    });

    if (dlg.exec() != QDialog::Accepted)
        replacePixmap(src);  // 取消恢复
}

void AdjustOperations::brightnessContrast(QWidget *parentWidget)
{
    if (!m_pixmapItemPtr || !(*m_pixmapItemPtr)) return;

    QPixmap src = (*m_pixmapItemPtr)->pixmap();
    double angle = (*m_pixmapItemPtr)->rotation();
    if (!qFuzzyCompare(angle, 0.0))
    {
        QTransform t;
        t.rotate(angle);
        src = src.transformed(t, Qt::SmoothTransformation);
    }

    BrightnessContrastDialog dlg(0, 0, parentWidget);

    connect(&dlg, &BrightnessContrastDialog::valuesChanged, this,
            [this, src](int brightness, int contrast) {
        QImage img = src.toImage();
        if (img.isNull()) return;
        double contrastFactor = 1.0 + contrast / 255.0;
        for (int y = 0; y < img.height(); ++y)
            for (int x = 0; x < img.width(); ++x)
            {
                QColor c(img.pixel(x, y));
                int gray = qGray(c.rgb());
                gray = gray + brightness;
                gray = static_cast<int>((gray - 128) * contrastFactor + 128);
                gray = (std::max)(0, (std::min)(255, gray));
                img.setPixel(x, y, qRgb(gray, gray, gray));
            }
        replacePixmap(QPixmap::fromImage(img));
    });

    if (dlg.exec() != QDialog::Accepted)
        replacePixmap(src);
}
