#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include <QObject>
#include <QString>

class QTextEdit;

class LogManager : public QObject
{
    Q_OBJECT

public:
    static LogManager *instance();

    void setLogWidget(QTextEdit *widget);
    void log(const QString &message);
    void logInfo(const QString &message);
    void logError(const QString &message);
    void logSuccess(const QString &message);

signals:
    void newLog(const QString &formattedMessage);

private:
    explicit LogManager(QObject *parent = nullptr);

    QString currentTimestamp() const;

    static LogManager *s_instance;
    QTextEdit *m_logWidget;
};

#endif // LOGMANAGER_H
