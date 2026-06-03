#ifndef BRIGHTNESSCONTRASTDIALOG_H
#define BRIGHTNESSCONTRASTDIALOG_H

#include <QDialog>

class QSlider;
class QLabel;

class BrightnessContrastDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BrightnessContrastDialog(int initialBrightness, int initialContrast,
                                      QWidget *parent = nullptr);
    int brightness() const { return m_brightness; }
    int contrast() const { return m_contrast; }

signals:
    void valuesChanged(int brightness, int contrast);

private slots:
    void onBrightnessChanged(int value);
    void onContrastChanged(int value);

private:
    QSlider *m_brightnessSlider;
    QSlider *m_contrastSlider;
    QLabel  *m_brightnessValueLabel;
    QLabel  *m_contrastValueLabel;
    int m_brightness;
    int m_contrast;
};

#endif // BRIGHTNESSCONTRASTDIALOG_H
