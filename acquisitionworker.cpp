#include "acquisitionworker.h"
#include <QThread>
#include <cmath>
#include <algorithm>

AcquisitionWorker::AcquisitionWorker(QObject *parent)
    : QObject(parent),
      m_running(false),
      m_frameData(nullptr),
      m_frameWidth(640),
      m_frameHeight(480)
{
}

AcquisitionWorker::~AcquisitionWorker()
{
    stop();
    delete[] m_frameData;
}

void AcquisitionWorker::start()
{
    m_running = true;
    m_frameWidth = 640;
    m_frameHeight = 640;
    m_frameData = new unsigned short[m_frameWidth * m_frameHeight];
    int frameCount = 0;

    while (m_running)
    {
        // 直接在共享缓冲区中生成 16 位数据
        {
            QMutexLocker lock(&m_mutex);
            for (int y = 0; y < m_frameHeight; ++y)
            {
                for (int x = 0; x < m_frameWidth; ++x)
                {
                    double cx = x - m_frameWidth / 2.0;
                    double cy = y - m_frameHeight / 2.0;
                    double dist = std::sqrt(cx * cx + cy * cy);
                    double angle = std::atan2(cy, cx);
                    double t = frameCount * 0.05;

                    double val = 32768.0
                               + 16384.0 * std::sin(dist * 0.02 + t)
                               + 8192.0  * std::cos(angle * 3.0 + t * 0.5)
                               + (std::rand() % 2048) - 1024;

                    val = (std::max)(0.0, (std::min)(65535.0, val));
                    m_frameData[y * m_frameWidth + x] = static_cast<unsigned short>(val);
                }
            }
        }

        emit frameReady();
        ++frameCount;
        QThread::msleep(40);
    }
}

void AcquisitionWorker::stop()
{
    m_running = false;
}

void AcquisitionWorker::lockFrame(const unsigned short *&data, int &w, int &h)
{
    m_mutex.lock();
    data = m_frameData;
    w = m_frameWidth;
    h = m_frameHeight;
}

void AcquisitionWorker::unlockFrame()
{
    m_mutex.unlock();
}
