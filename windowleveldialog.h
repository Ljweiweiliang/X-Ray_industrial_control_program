#ifndef WINDOWLEVELDIALOG_H
#define WINDOWLEVELDIALOG_H

#include <QDialog>

class QSlider;
class QLabel;

class WindowLevelDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WindowLevelDialog(int initialWidth, int initialLevel,
                               int maxWidth = 65535, int maxLevel = 65535,
                               QWidget *parent = nullptr);
    int windowWidth() const { return m_windowWidth; }
    int windowLevel() const { return m_windowLevel; }

signals:
    void valuesChanged(int width, int level);

private slots:
    void onWidthChanged(int value);
    void onLevelChanged(int value);

private:
    QSlider *m_widthSlider;
    QSlider *m_levelSlider;
    QLabel  *m_widthValueLabel;
    QLabel  *m_levelValueLabel;

    int m_windowWidth;
    int m_windowLevel;
};

#endif // WINDOWLEVELDIALOG_H
