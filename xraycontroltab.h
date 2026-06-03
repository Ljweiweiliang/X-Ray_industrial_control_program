#ifndef XRAYCONTROLTAB_H
#define XRAYCONTROLTAB_H

#include <QWidget>

class QComboBox;
class QPushButton;
class QSpinBox;
class QLabel;
class QSerialPort;
class QTimer;
class QSettings;

class XrayControlTab : public QWidget
{
    Q_OBJECT

public:
    explicit XrayControlTab(QWidget *parent = nullptr);
    ~XrayControlTab();

    void setFeedbackDisplay(int voltageFB, float currentFB);

    void saveSettings(QSettings &settings) const;
    void loadSettings(QSettings &settings);

signals:
    void serialConnected(bool connected);
    void xrayStatusChanged(bool on);
    void feedbackUpdated(int voltageFB, float currentFB);

private slots:
    void onConnectSerial();
    void onRayOn();
    void onRayOff();

    // 定时器
    void onWatchdogTimer();      // 看门狗心跳 (1s)
    void onFeedbackTimer();      // 反馈查询 (500ms)
    void onPreheatTimer();       // 预热检查 (1s)

private:
    // 串口操作（用户根据实际协议填充）
    void sendVoltage(int kv);
    void sendCurrent(int ma);
    void sendRayOn();
    void sendRayOff();
    void sendWatchdog();         // 看门狗心跳指令
    void queryFeedback();        // 查询电压/电流反馈

    QComboBox   *m_comboPort;
    QPushButton *m_btnSerial;
    QSpinBox    *m_spinVoltage;
    QSpinBox    *m_spinCurrent;
    QLabel      *m_labelVolFB;  // 电压反馈显示
    QLabel      *m_labelCurFB;  // 电流反馈显示
    QPushButton *m_btnRayOn;
    QPushButton *m_btnRayOff;
    QSerialPort *m_serial;
    bool         m_serialConnected;

    // 射线状态
    bool m_rayOn;

    // 定时器
    QTimer *m_timerWatchdog;    // 看门狗，1秒
    QTimer *m_timerFeedback;    // 反馈查询，500ms
    QTimer *m_timerPreheat;     // 预热，1秒

    // 设置电压/电流(从spinbox读取的实际值)
    int   m_targetVoltage;
    int   m_targetCurrent;
    int   m_feedbackVoltage;
    float m_feedbackCurrent;
};

#endif // XRAYCONTROLTAB_H
