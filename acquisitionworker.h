#ifndef ACQUISITIONWORKER_H
#define ACQUISITIONWORKER_H

#include <QObject>
#include <QMutex>

class AcquisitionWorker : public QObject
{
    Q_OBJECT

public:
    explicit AcquisitionWorker(QObject *parent = nullptr);
    ~AcquisitionWorker();

    // 线程安全地访问最新帧的 16 位原始数据
    void lockFrame(const unsigned short *&data, int &w, int &h);
    void unlockFrame();

public slots:
    void start();
    void stop();

signals:
    void frameReady();  // 仅通知，不传数据

private:
    volatile bool      m_running;
    QMutex             m_mutex;
    unsigned short    *m_frameData;
    int                m_frameWidth;
    int                m_frameHeight;
};

#endif // ACQUISITIONWORKER_H
