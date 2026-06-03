#include "xraycontroltab.h"
#include "logmanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QSpinBox>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include <QSettings>

XrayControlTab::XrayControlTab(QWidget *parent)
    : QWidget(parent),
      m_comboPort(nullptr),
      m_btnSerial(nullptr),
      m_spinVoltage(nullptr),
      m_spinCurrent(nullptr),
      m_labelVolFB(nullptr),
      m_labelCurFB(nullptr),
      m_btnRayOn(nullptr),
      m_btnRayOff(nullptr),
      m_serial(nullptr),
      m_serialConnected(false),
      m_rayOn(false),
      m_timerWatchdog(nullptr),
      m_timerFeedback(nullptr),
      m_timerPreheat(nullptr),
      m_targetVoltage(70),
      m_targetCurrent(10),
      m_feedbackVoltage(0),
      m_feedbackCurrent(0.0f)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignTop);

    // ===== 串口设置 =====
    QGroupBox *serialGroup = new QGroupBox(tr("Serial Port"), this);
    QVBoxLayout *serialLayout = new QVBoxLayout(serialGroup);
    QHBoxLayout *portLayout = new QHBoxLayout();
    portLayout->addWidget(new QLabel(tr("Port:"), this));
    m_comboPort = new QComboBox(this);
    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : ports)
        m_comboPort->addItem(info.portName());
    if (m_comboPort->count() == 0)
        m_comboPort->addItem(tr("(no port detected)"));
    portLayout->addWidget(m_comboPort, 1);
    serialLayout->addLayout(portLayout);
    m_btnSerial = new QPushButton(tr("Connect Serial"), this);
    m_btnSerial->setMinimumHeight(32);
    serialLayout->addWidget(m_btnSerial);
    mainLayout->addWidget(serialGroup);

    // ===== 射线参数 =====
    QGroupBox *rayGroup = new QGroupBox(tr("X-Ray Parameters"), this);
    QVBoxLayout *rayLayout = new QVBoxLayout(rayGroup);
    QHBoxLayout *voltLayout = new QHBoxLayout();
    voltLayout->addWidget(new QLabel(tr("Voltage (kV):"), this));
    m_spinVoltage = new QSpinBox(this);
    m_spinVoltage->setRange(10, 150);
    m_spinVoltage->setValue(70);
    m_spinVoltage->setSuffix(" kV");
    voltLayout->addWidget(m_spinVoltage, 1);
    rayLayout->addLayout(voltLayout);
    QHBoxLayout *currLayout = new QHBoxLayout();
    currLayout->addWidget(new QLabel(tr("Current (mA):"), this));
    m_spinCurrent = new QSpinBox(this);
    m_spinCurrent->setRange(1, 500);
    m_spinCurrent->setValue(10);
    m_spinCurrent->setSuffix(" mA");
    currLayout->addWidget(m_spinCurrent, 1);
    rayLayout->addLayout(currLayout);

    // 反馈显示
    QHBoxLayout *fbLayout = new QHBoxLayout();
    fbLayout->addWidget(new QLabel(tr("FB Voltage:"), this));
    m_labelVolFB = new QLabel(tr("0 kV"), this);
    m_labelVolFB->setStyleSheet("QLabel { color: #00aa00; font-weight: bold; }");
    fbLayout->addWidget(m_labelVolFB);
    fbLayout->addSpacing(10);
    fbLayout->addWidget(new QLabel(tr("FB Current:"), this));
    m_labelCurFB = new QLabel(tr("0 mA"), this);
    m_labelCurFB->setStyleSheet("QLabel { color: #00aa00; font-weight: bold; }");
    fbLayout->addWidget(m_labelCurFB);
    fbLayout->addStretch();
    rayLayout->addLayout(fbLayout);
    mainLayout->addWidget(rayGroup);

    // ===== 射线控制 =====
    QGroupBox *ctrlGroup = new QGroupBox(tr("X-Ray Control"), this);
    QVBoxLayout *ctrlLayout = new QVBoxLayout(ctrlGroup);
    m_btnRayOn = new QPushButton(tr("Ray On"), this);
    m_btnRayOn->setMinimumHeight(36);
    m_btnRayOn->setEnabled(false);
    ctrlLayout->addWidget(m_btnRayOn);
    m_btnRayOff = new QPushButton(tr("Ray Off"), this);
    m_btnRayOff->setMinimumHeight(36);
    m_btnRayOff->setEnabled(false);
    ctrlLayout->addWidget(m_btnRayOff);
    mainLayout->addWidget(ctrlGroup);
    mainLayout->addStretch();

    // ===== 定时器 =====
    m_timerWatchdog = new QTimer(this);
    m_timerWatchdog->setInterval(1000); // 1秒看门狗
    connect(m_timerWatchdog, &QTimer::timeout, this, &XrayControlTab::onWatchdogTimer);

    m_timerFeedback = new QTimer(this);
    m_timerFeedback->setInterval(500); // 500ms查反馈
    connect(m_timerFeedback, &QTimer::timeout, this, &XrayControlTab::onFeedbackTimer);

    m_timerPreheat = new QTimer(this);
    m_timerPreheat->setInterval(1000);
    connect(m_timerPreheat, &QTimer::timeout, this, &XrayControlTab::onPreheatTimer);

    // ===== 信号 =====
    connect(m_btnSerial, &QPushButton::clicked, this, &XrayControlTab::onConnectSerial);
    connect(m_btnRayOn,  &QPushButton::clicked, this, &XrayControlTab::onRayOn);
    connect(m_btnRayOff, &QPushButton::clicked, this, &XrayControlTab::onRayOff);
}

XrayControlTab::~XrayControlTab()
{
    if (m_rayOn) onRayOff();  // 确保关闭射线
    m_timerWatchdog->stop();
    m_timerFeedback->stop();
    m_timerPreheat->stop();
    if (m_serial && m_serial->isOpen())
        m_serial->close();
}

void XrayControlTab::setFeedbackDisplay(int voltageFB, float currentFB)
{
    m_labelVolFB->setText(QString("%1 kV").arg(voltageFB));
    m_labelCurFB->setText(QString("%1 mA").arg(currentFB, 0, 'f', 1));
}

// ==================== 计算校验和 (XOR bytes[1..8]) ====================
static unsigned char calcChecksum(const unsigned char *data, int start, int end)
{
    unsigned char chk = 0;
    for (int i = start; i <= end; i++)
        chk ^= data[i];
    return chk;
}

// ==================== 发送电压 (协议: 0.1kV单位) ====================
void XrayControlTab::sendVoltage(int kv)
{
    if (!m_serial || !m_serial->isOpen()) return;

    int temp = kv * 10;  // 转为 0.1kV 单位
    unsigned char cmd[11];
    cmd[0] = 0x02;                              // 帧头
    cmd[1] = '1';                               // 命令 '1'
    cmd[2] = '0';                               // 命令 '0'
    cmd[3] = ',';                               // 间隔符
    cmd[4] = (temp / 1000 % 10) + '0';          // 千位
    cmd[5] = (temp / 100  % 10) + '0';          // 百位
    cmd[6] = (temp / 10   % 10) + '0';          // 十位
    cmd[7] = (temp / 1    % 10) + '0';          // 个位
    cmd[8] = ',';                               // 间隔符
    cmd[9] = calcChecksum(cmd, 1, 8);           // 校验
    cmd[10] = 0x03;                             // 帧尾

    m_serial->write((const char*)cmd, 11);
    m_serial->waitForBytesWritten(100);

    // 调试：打印发送的十六进制数据
    QString hexDump;
    for (int i = 0; i < 11; i++)
        hexDump += QString("%1 ").arg(cmd[i], 2, 16, QChar('0'));
    LogManager::instance()->logInfo("TX: " + hexDump.toUpper());

    // 等待响应 (最多50ms)
    if (m_serial->waitForReadyRead(50))
    {
        QByteArray resp = m_serial->readAll();
        if (resp.size() >= 5 &&
            (unsigned char)resp[0] == 0x02 &&
            (unsigned char)resp[1] == '1' &&
            (unsigned char)resp[2] == '0' &&
            (unsigned char)resp[3] == ',' &&
            (unsigned char)resp[4] == '$')     // 0x24 = '$' 成功
        {
            LogManager::instance()->logSuccess(
                QString("Voltage set to %1 kV").arg(kv));
            return;
        }
    }
    LogManager::instance()->logError(
        QString("Failed to set voltage %1 kV").arg(kv));
}

// ==================== 通用指令发送 (STX + cmd + ',' + data + ',' + XOR + ETX) ====================
// 返回 true 表示收到 '$' 成功响应
static bool sendCommand(QSerialPort *serial, unsigned char c1, unsigned char c2,
                        int dataValue, int dataDigits)
{
    unsigned char cmd[11];
    cmd[0] = 0x02;
    cmd[1] = c1;
    cmd[2] = c2;
    cmd[3] = ',';
    for (int i = 0; i < dataDigits; i++) {
        int divisor = 1;
        for (int d = 0; d < dataDigits - 1 - i; d++) divisor *= 10;
        cmd[4 + i] = (dataValue / divisor % 10) + '0';
    }
    int dataEnd = 4 + dataDigits;
    cmd[dataEnd] = ',';
    unsigned char chk = 0;
    for (int i = 1; i <= dataEnd; i++) chk ^= cmd[i];
    cmd[dataEnd + 1] = chk;
    cmd[dataEnd + 2] = 0x03;

    serial->write((const char*)cmd, dataEnd + 3);
    serial->waitForBytesWritten(50);
    if (serial->waitForReadyRead(100))
    {
        QByteArray r = serial->readAll();
        return (r.size() >= 5 && (unsigned char)r[0] == 0x02 &&
                (unsigned char)r[4] == '$');
    }
    return false;
}

// ==================== 发送电流 (mA) ====================
void XrayControlTab::sendCurrent(int ma)
{
    if (!m_serial || !m_serial->isOpen()) return;
    if (sendCommand(m_serial, '1', '1', ma, 4))
        LogManager::instance()->logSuccess(QString("Current set to %1 mA").arg(ma));
    else
        LogManager::instance()->logError(QString("Failed to set current %1 mA").arg(ma));
}

// ==================== 开启射线 ====================
void XrayControlTab::sendRayOn()
{
    if (!m_serial || !m_serial->isOpen()) return;
    // TODO: 替换 '1','2' 为实际开射线命令
    if (sendCommand(m_serial, '1', '2', 1, 1))
        LogManager::instance()->logSuccess("Ray ON ACK");
    else
        LogManager::instance()->logError("Ray ON failed");
}

// ==================== 关闭射线 ====================
void XrayControlTab::sendRayOff()
{
    if (!m_serial || !m_serial->isOpen()) return;
    // TODO: 替换 '1','3' 为实际关射线命令
    if (sendCommand(m_serial, '1', '3', 0, 1))
        LogManager::instance()->logSuccess("Ray OFF ACK");
    else
        LogManager::instance()->logError("Ray OFF failed");
}

// ==================== 看门狗 ====================
void XrayControlTab::sendWatchdog()
{
    if (!m_serial || !m_serial->isOpen()) return;
    // TODO: sendCommand(m_serial, '3', '0', 0, 1);
}

// ==================== 查询反馈 ====================
void XrayControlTab::queryFeedback()
{
    if (!m_serial || !m_serial->isOpen()) return;

    // 查询电压 — 替换 '2','0' 为实际命令
    unsigned char vCmd[7] = {0x02, '2', '0', ',', '0', 0, 0x03};
    unsigned char chk = 0;
    for (int i = 1; i <= 4; i++) chk ^= vCmd[i];
    vCmd[5] = chk;
    m_serial->write((const char*)vCmd, 7);
    m_serial->waitForBytesWritten(50);
    if (m_serial->waitForReadyRead(100))
    {
        QByteArray r = m_serial->readAll();
        if (r.size() >= 8 && (unsigned char)r[0] == 0x02)
            m_feedbackVoltage = (r[4]-'0')*1000 + (r[5]-'0')*100
                              + (r[6]-'0')*10 + (r[7]-'0');
    }

    // 查询电流 — 替换 '2','1' 为实际命令
    unsigned char aCmd[7] = {0x02, '2', '1', ',', '0', 0, 0x03};
    chk = 0;
    for (int i = 1; i <= 4; i++) chk ^= aCmd[i];
    aCmd[5] = chk;
    m_serial->write((const char*)aCmd, 7);
    m_serial->waitForBytesWritten(50);
    if (m_serial->waitForReadyRead(100))
    {
        QByteArray r = m_serial->readAll();
        if (r.size() >= 8 && (unsigned char)r[0] == 0x02)
            m_feedbackCurrent = (float)((r[4]-'0')*1000 + (r[5]-'0')*100
                                      + (r[6]-'0')*10 + (r[7]-'0'));
    }

    setFeedbackDisplay(m_feedbackVoltage, m_feedbackCurrent);
}

// ==================== 按钮事件 ====================
void XrayControlTab::onConnectSerial()
{
    if (!m_serial)
        m_serial = new QSerialPort(this);

    if (!m_serialConnected)
    {
        m_serial->setPortName(m_comboPort->currentText());
        m_serial->setBaudRate(QSerialPort::Baud9600);
        m_serial->setDataBits(QSerialPort::Data8);
        m_serial->setParity(QSerialPort::NoParity);
        m_serial->setStopBits(QSerialPort::OneStop);

        if (m_serial->open(QIODevice::ReadWrite))
        {
            m_serialConnected = true;
            m_btnSerial->setText(tr("Disconnect Serial"));
            m_btnRayOn->setEnabled(true);
            m_comboPort->setEnabled(false);
            LogManager::instance()->logSuccess(
                QString("Serial port %1 connected").arg(m_comboPort->currentText()));
        }
        else
        {
            m_btnSerial->setText(tr("Connect Failed"));
            LogManager::instance()->logError(
                QString("Failed to open serial port %1").arg(m_comboPort->currentText()));
        }
    }
    else
    {
        if (m_rayOn) onRayOff();
        m_serial->close();
        m_serialConnected = false;
        m_btnSerial->setText(tr("Connect Serial"));
        m_btnRayOn->setEnabled(false);
        m_btnRayOff->setEnabled(false);
        m_comboPort->setEnabled(true);
        LogManager::instance()->logInfo(tr("Serial port disconnected"));
    }
    emit serialConnected(m_serialConnected);
}

void XrayControlTab::onRayOn()
{
    if (m_rayOn || !m_serialConnected) return;

    // 读取当前设定值
    m_targetVoltage = m_spinVoltage->value();
    m_targetCurrent = m_spinCurrent->value();

    LogManager::instance()->logInfo(
        QString("Opening X-Ray: %1 kV, %2 mA").arg(m_targetVoltage).arg(m_targetCurrent));

    // 1. 发送电压
    sendVoltage(m_targetVoltage);

    // 2. 发送电流
    sendCurrent(m_targetCurrent);

    // 3. 开启射线
    sendRayOn();

    m_rayOn = true;
    m_btnRayOn->setEnabled(false);
    m_btnRayOff->setEnabled(true);
    m_spinVoltage->setEnabled(false);
    m_spinCurrent->setEnabled(false);

    // 4. 启动定时器
    m_timerWatchdog->start();  // 看门狗心跳
    m_timerFeedback->start();  // 反馈查询
    m_timerPreheat->start();   // 预热检查

    LogManager::instance()->logInfo(tr("X-Ray ON"));
    emit xrayStatusChanged(true);
}

void XrayControlTab::onRayOff()
{
    if (!m_rayOn) return;

    LogManager::instance()->logInfo("Closing X-Ray...");

    // 1. 关闭射线
    sendRayOff();

    m_rayOn = false;
    m_btnRayOn->setEnabled(true);
    m_btnRayOff->setEnabled(false);
    m_spinVoltage->setEnabled(true);
    m_spinCurrent->setEnabled(true);

    // 2. 停止定时器
    m_timerWatchdog->stop();
    m_timerFeedback->stop();
    m_timerPreheat->stop();

    // 3. 清零反馈
    m_feedbackVoltage = 0;
    m_feedbackCurrent = 0.0f;
    setFeedbackDisplay(0, 0.0f);

    LogManager::instance()->logInfo(tr("X-Ray OFF"));
    emit xrayStatusChanged(false);
}

// ==================== 定时器事件 ====================
void XrayControlTab::onWatchdogTimer()
{
    // 看门狗心跳：每秒发送一次，保持连接
    sendWatchdog();
}

void XrayControlTab::onFeedbackTimer()
{
    // 查询反馈电压/电流
    queryFeedback();
}

void XrayControlTab::onPreheatTimer()
{
    // 预热检查：对比设定值与反馈值
    // 如果反馈电压接近设定值（误差<5kV），说明预热完成
    if (abs(m_feedbackVoltage - m_targetVoltage) <= 5 &&
        abs(m_feedbackCurrent - m_targetCurrent) <= 10)
    {
        LogManager::instance()->logSuccess("X-Ray preheat complete!");
        m_timerPreheat->stop();
    }
}

// ==================== 配置持久化 ====================
void XrayControlTab::saveSettings(QSettings &s) const
{
    s.beginGroup("XRay");
    s.setValue("voltage",    m_spinVoltage->value());
    s.setValue("current",    m_spinCurrent->value());
    s.setValue("serialPort", m_comboPort->currentText());
    s.endGroup();
}

void XrayControlTab::loadSettings(QSettings &s)
{
    s.beginGroup("XRay");
    m_spinVoltage->setValue(s.value("voltage", 70).toInt());
    m_spinCurrent->setValue(s.value("current", 10).toInt());
    QString savedPort = s.value("serialPort").toString();
    if (!savedPort.isEmpty()) {
        int idx = m_comboPort->findText(savedPort);
        if (idx >= 0) m_comboPort->setCurrentIndex(idx);
    }
    s.endGroup();
}
