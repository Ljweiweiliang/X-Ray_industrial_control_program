#ifndef OPCCLIENTMANAGER_H
#define OPCCLIENTMANAGER_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QVector>
#include <QHash>

#include <open62541/types.h>

struct UA_Client;

// 数据类型常量（与旧 MFC KepSever.dat 格式兼容）
#define T_UNDEFINED 0
#define T_BOOL      1
#define T_BYTE      2
#define T_CHAR      3
#define T_WORD      4
#define T_SHORT     5
#define T_DWORD     6
#define T_LONG      7
#define T_FLOAT     8
#define T_DOUBLE    9
#define T_DATE      10
#define T_STRING    11

// 单个标记信息
struct TagItem {
    QString name;       // 标记路径，如 "_System._ActiveTagCount"
    int     type;       // 数据类型: 使用 T_BOOL / T_BYTE / ... 常量
    QVariant value;     // 当前缓存值
    UA_NodeId *nodeId;  // 缓存已解析的 NodeId（resolveAllTags 后有效）, nullptr 表示未解析
};

class OpcClientManager : public QObject
{
    Q_OBJECT

public:
    explicit OpcClientManager(QObject *parent = nullptr);
    ~OpcClientManager();

    // 版本 & 状态
    QString getVersion();
    bool    isConnected() const;

    // ==================== 配置文件（兼容 KepSever.dat 格式）====================
    bool loadConfig(const QString &datFilePath);
    int  tagCount() const { return m_tags.size(); }
    QString tagName(int index) const;
    int     tagType(int index) const;
    QString serverName() const { return m_serverName; }
    QString groupName() const { return m_groupName; }

    // ==================== 连接管理 ====================
    bool connectToServer();
    void connectToServer(const QString &endpointUrl);
    void disconnect();

    // 连接后预解析所有 Tag 的 NodeId（在 readValue/writeValue/subscribe 前调用）
    bool resolveAllTags();

    // ==================== 同步读写（0-based）====================
    QVariant readValue(int tagIndex);
    bool     writeValue(int tagIndex, const QVariant &value);

    // ==================== 订阅（OPC UA 监视项，类似原 MFC 异步回调）====================
    bool subscribe(int tagIndex, double samplingInterval = 100.0);
    bool subscribeAll(double samplingInterval = 100.0);
    void unsubscribe(int tagIndex);
    void unsubscribeAll();

signals:
    void connectionStatusChanged(bool connected);
    // 订阅数据变化时发射（monId 对应 tagIndex）
    void dataChanged(int tagIndex, const QVariant &value);
    void logMessage(const QString &msg);

private:
    // 将标记路径逐级 browse 解析为 UA_NodeId
    UA_NodeId resolveTagPath(const QString &tagPath);
    // 释放缓存的 NodeId
    void clearCachedNodeIds();

    // open62541 数据变化回调（C 函数，转为成员函数调用）
    static void onDataChange(UA_Client *client, UA_UInt32 subId, void *subContext,
                             UA_UInt32 monId, void *monContext,
                             UA_DataValue *value);

    // 配置文件信息
    QString m_serverName;
    QString m_remoteIp;
    QString m_groupName;

    // OPC UA 客户端
    UA_Client      *m_client;
    QString         m_endpointUrl;
    bool            m_connected;

    // 标记列表
    QVector<TagItem> m_tags;

    // 订阅相关
    UA_UInt32   m_subscriptionId;               // 当前订阅 ID，0 表示无订阅
    QHash<int, UA_UInt32> m_monitoredItemIds;   // tagIndex -> monitoredItemId
};

#endif // OPCCLIENTMANAGER_H


