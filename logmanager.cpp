#include "logmanager.h"
#include <QTextEdit>
#include <QDateTime>
#include <QScrollBar>

LogManager *LogManager::s_instance = nullptr;

LogManager::LogManager(QObject *parent)
    : QObject(parent),
      m_logWidget(nullptr)
{
}

LogManager *LogManager::instance()
{
    if (!s_instance)
        s_instance = new LogManager();
    return s_instance;
}

void LogManager::setLogWidget(QTextEdit *widget)
{
    m_logWidget = widget;
}

QString LogManager::currentTimestamp() const
{
    return QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
}

void LogManager::log(const QString &message)
{
    QString formatted = QString("[%1] %2").arg(currentTimestamp(), message);
    if (m_logWidget)
    {
        m_logWidget->append(formatted);
        // 自动滚动到底部
        QScrollBar *sb = m_logWidget->verticalScrollBar();
        if (sb) sb->setValue(sb->maximum());
    }
    emit newLog(formatted);
}

void LogManager::logInfo(const QString &message)
{
    log(QString("INFO: %1").arg(message));
}

void LogManager::logError(const QString &message)
{
    log(QString("ERROR: %1").arg(message));
}

void LogManager::logSuccess(const QString &message)
{
    log(QString("SUCCESS: %1").arg(message));
}
