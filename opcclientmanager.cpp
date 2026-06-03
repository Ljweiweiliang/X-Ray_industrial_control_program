#include "opcclientmanager.h"
#include "logmanager.h"

#include <QFile>
#include <QTextStream>
#include <QTextCodec>
#include <QDateTime>
#include <QDebug>

#include <open62541/client.h>
#include <open62541/client_config_default.h>
#include <open62541/client_highlevel.h>
#include <open62541/client_subscriptions.h>
#include <open62541/types.h>

// ==================== 工具函数：将 UA_Variant 转为 QVariant ====================
static QVariant uaVariantToQVariant(const UA_Variant &value)
{
    if (UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_BOOLEAN]))
        return *(UA_Boolean *)value.data ? true : false;
    else if (UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_BYTE]))
        return (int)*(UA_Byte *)value.data;
    else if (UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_SBYTE]))
        return (int)*(UA_SByte *)value.data;
    else if (UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_INT16]))
        return (int)*(UA_Int16 *)value.data;
    else if (UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_UINT16]))
        return (int)*(UA_UInt16 *)value.data;
    else if (UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_INT32]))
        return (qint32)*(UA_Int32 *)value.data;
    else if (UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_UINT32]))
        return (quint32)*(UA_UInt32 *)value.data;
    else if (UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_INT64]))
        return (qlonglong)*(UA_Int64 *)value.data;
    else if (UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_UINT64]))
        return (qulonglong)*(UA_UInt64 *)value.data;
    else if (UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_FLOAT]))
        return (double)*(UA_Float *)value.data;
    else if (UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_DOUBLE]))
        return *(UA_Double *)value.data;
    else if (UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_STRING])) {
        UA_String *s = (UA_String *)value.data;
        return QString::fromUtf8(reinterpret_cast<const char*>(s->data),
                                 static_cast<int>(s->length));
    } else if (UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_DATETIME])) {
        UA_DateTime dt = *(UA_DateTime *)value.data;
        // UA_DateTime is in 100ns intervals since Jan 1, 1601
        // Convert to QDateTime
        qint64 msecs = dt / 10000LL;            // 100ns -> ms
        qint64 epochDiff = 11644473600000LL;    // 1601-01-01 to 1970-01-01 in ms
        return QDateTime::fromMSecsSinceEpoch(msecs - epochDiff);
    }
    return QVariant();
}

// ==================== 工具函数：根据 tag type 构造写入用的 UA_Variant ====================
static UA_Variant qVariantToUaVariant(const QVariant &value, int tagType)
{
    UA_Variant uaValue;
    UA_Variant_init(&uaValue);

    // 优先按 QVariant 实际类型映射
    switch (value.type()) {
    case QVariant::Bool: {
        static UA_Boolean v = false;
        v = value.toBool() ? 1 : 0;
        UA_Variant_setScalarCopy(&uaValue, &v, &UA_TYPES[UA_TYPES_BOOLEAN]);
        return uaValue;
    }
    case QVariant::Int:
    case QVariant::LongLong: {
        // 根据 tagType 选择写入的 OPC UA 类型
        switch (tagType) {
        case T_BYTE: {
            static UA_Byte v = 0;
            v = (UA_Byte)value.toUInt();
            UA_Variant_setScalarCopy(&uaValue, &v, &UA_TYPES[UA_TYPES_BYTE]);
            return uaValue;
        }
        case T_WORD:
        case T_DWORD: {
            static UA_UInt32 v = 0;
            v = (UA_UInt32)value.toUInt();
            UA_Variant_setScalarCopy(&uaValue, &v, &UA_TYPES[UA_TYPES_UINT32]);
            return uaValue;
        }
        case T_SHORT: {
            static UA_Int16 v = 0;
            v = (UA_Int16)value.toInt();
            UA_Variant_setScalarCopy(&uaValue, &v, &UA_TYPES[UA_TYPES_INT16]);
            return uaValue;
        }
        case T_LONG:
        default: {
            static UA_Int32 v = 0;
            v = value.toInt();
            UA_Variant_setScalarCopy(&uaValue, &v, &UA_TYPES[UA_TYPES_INT32]);
            return uaValue;
        }
        }
    }
    case QVariant::Double: {
        static UA_Double v = 0.0;
        v = value.toDouble();
        UA_Variant_setScalarCopy(&uaValue, &v, &UA_TYPES[UA_TYPES_DOUBLE]);
        return uaValue;
    }
    case QVariant::String:
    default: {
        QByteArray utf8 = value.toString().toUtf8();
        UA_String v = UA_STRING_ALLOC(utf8.constData());
        UA_Variant_setScalarCopy(&uaValue, &v, &UA_TYPES[UA_TYPES_STRING]);
        UA_String_clear(&v);
        return uaValue;
    }
    }
}

// =====================================================================
// OpcClientManager 实现
// =====================================================================

OpcClientManager::OpcClientManager(QObject *parent)
    : QObject(parent),
      m_client(nullptr),
      m_connected(false),
      m_subscriptionId(0)
{
}

OpcClientManager::~OpcClientManager()
{
    disconnect();
}

QString OpcClientManager::getVersion()
{
    return QString(UA_OPEN62541_VERSION);
}

bool OpcClientManager::isConnected() const
{
    return m_connected;
}

// ==================== 配置文件解析（兼容 KepSever.dat 格式）====================
// 读取原始字节，检测编码后转为 QStringList 再逐行解析
bool OpcClientManager::loadConfig(const QString &datFilePath)
{
    m_tags.clear();
    m_serverName.clear();
    m_remoteIp.clear();
    m_groupName.clear();

    // 以二进制模式读取整个文件
    QFile file(datFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit logMessage(QString("Cannot open config: %1").arg(datFilePath));
        return false;
    }
    QByteArray rawData = file.readAll();
    file.close();

    if (rawData.isEmpty()) {
        emit logMessage("Config file is empty");
        return false;
    }

    // 尝试用 UTF-8 解码，如果包含非法序列则改用 GBK
    QTextCodec *utf8Codec = QTextCodec::codecForName("UTF-8");
    QTextCodec *gbkCodec  = QTextCodec::codecForName("GBK");
    bool useGbk = false;

    if (gbkCodec == nullptr) {
        // Qt 可能没有内置 GBK codec，改用 System
        gbkCodec = QTextCodec::codecForName("System");
    }

    // 检测 UTF-8 有效性
    QTextCodec::ConverterState state;
    utf8Codec->toUnicode(rawData.constData(), rawData.size(), &state);
    if (state.invalidChars > 0) {
        useGbk = true;
    }

    // 解码
    QString text;
    if (useGbk && gbkCodec) {
        text = gbkCodec->toUnicode(rawData);
        emit logMessage("Config loaded using GBK encoding");
    } else {
        text = utf8Codec->toUnicode(rawData);
    }

    // 按行拆分
    QStringList lines = text.split(QRegExp("[\r\n]+"), QString::SkipEmptyParts);

    // 解析头部
    QString srvName, remIp, grpName;
    int totalItems = 0;
    int lineIdx = 0;

    // 先找"总项数"
    for (; lineIdx < lines.size(); ++lineIdx) {
        const QString &l = lines[lineIdx];
        if (l.startsWith("服务器名") || l.startsWith("服务")) {
            if (++lineIdx < lines.size())
                srvName = lines[lineIdx].trimmed();
        } else if (l.startsWith("远程IP") || l.startsWith("远程")) {
            if (++lineIdx < lines.size())
                remIp = lines[lineIdx].trimmed();
        } else if (l.startsWith("组名") || l.startsWith("组")) {
            if (++lineIdx < lines.size())
                grpName = lines[lineIdx].trimmed();
        } else if (l.startsWith("总项数") || l.startsWith("总")) {
            if (++lineIdx < lines.size()) {
                totalItems = lines[lineIdx].trimmed().toInt();
            }
        } else if (l.startsWith("各项信息") || l.startsWith("各")) {
            ++lineIdx;
            break;
        }
    }

    // 读取标记条目
    for (int i = 0; i < totalItems && lineIdx < lines.size(); ++i) {
        QString tagName = lines[lineIdx].trimmed();
        if (tagName.isEmpty()) {
            ++lineIdx;
            --i;
            continue;
        }
        ++lineIdx;
        if (lineIdx >= lines.size()) break;
        QString typeStr = lines[lineIdx].trimmed();
        ++lineIdx;

        TagItem item;
        item.name   = tagName;
        item.type   = typeStr.toInt();
        item.nodeId = nullptr;
        m_tags.append(item);
    }

    // 更新成员
    m_serverName = srvName;
    m_remoteIp   = remIp;
    m_groupName  = grpName;

    // 构建 OPC UA 端点地址
    if (!m_remoteIp.isEmpty())
        m_endpointUrl = QString("opc.tcp://%1:49320").arg(m_remoteIp);
    else
        m_endpointUrl = "opc.tcp://127.0.0.1:49320";

    emit logMessage(QString("Config loaded: %1 tags from %2, endpoint=%3")
                        .arg(m_tags.size()).arg(m_serverName).arg(m_endpointUrl));
    return true;
}

QString OpcClientManager::tagName(int index) const
{
    if (index < 0 || index >= m_tags.size()) return QString();
    return m_tags[index].name;
}

int OpcClientManager::tagType(int index) const
{
    if (index < 0 || index >= m_tags.size()) return 0;
    return m_tags[index].type;
}

// ==================== 连接 ====================
bool OpcClientManager::connectToServer()
{
    connectToServer(m_endpointUrl);
    return m_connected;
}

void OpcClientManager::connectToServer(const QString &endpointUrl)
{
    if (m_connected) {
        emit logMessage("Already connected");
        return;
    }

    m_client = UA_Client_new();
    if (!m_client) {
        emit logMessage("Failed to create UA_Client");
        return;
    }

    UA_ClientConfig *config = UA_Client_getConfig(m_client);
    UA_ClientConfig_setDefault(config);

    m_endpointUrl = endpointUrl;
    emit logMessage(QString("Connecting to %1 ...").arg(m_endpointUrl));

    UA_StatusCode retval = UA_Client_connect(m_client,
        m_endpointUrl.toUtf8().constData());

    if (retval == UA_STATUSCODE_GOOD) {
        m_connected = true;
        emit logMessage(QString("Connected to %1").arg(m_endpointUrl));
        emit connectionStatusChanged(true);
    } else {
        emit logMessage(QString("Connect failed: 0x%1 (%2)")
                            .arg(retval, 8, 16, QChar('0'))
                            .arg(UA_StatusCode_name(retval)));
        UA_Client_delete(m_client);
        m_client = nullptr;
    }
}

void OpcClientManager::disconnect()
{
    // 先取消所有订阅
    unsubscribeAll();

    if (m_client) {
        if (m_connected) {
            UA_Client_disconnect(m_client);
            m_connected = false;
            emit connectionStatusChanged(false);
        }
        UA_Client_delete(m_client);
        m_client = nullptr;
    }

    // 释放缓存的 NodeId
    clearCachedNodeIds();
}

// ==================== 将标记路径转为 NodeId（成员函数版）====================
// 通过逐级浏览 Objects 文件夹来解析标记路径
UA_NodeId OpcClientManager::resolveTagPath(const QString &tagPath)
{
    QStringList parts = tagPath.split('.');
    if (parts.isEmpty())
        return UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);

    UA_NodeId currentNode = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);

    for (int idx = 0; idx < parts.size(); ++idx) {
        const QString &part = parts[idx];

        UA_BrowseRequest bReq;
        UA_BrowseRequest_init(&bReq);
        bReq.requestedMaxReferencesPerNode = 200;
        bReq.nodesToBrowse = UA_BrowseDescription_new();
        bReq.nodesToBrowseSize = 1;
        // 必须使用 UA_NodeId_copy 做深拷贝，不能直接浅拷贝赋值！
        // 否则 UA_BrowseRequest_clear 会释放 currentNode 的内部数据导致悬空指针
        UA_NodeId_copy(&currentNode, &bReq.nodesToBrowse[0].nodeId);
        bReq.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_BROWSENAME;

        UA_BrowseResponse bResp = UA_Client_Service_browse(m_client, bReq);
        UA_BrowseRequest_clear(&bReq);

        if (bResp.responseHeader.serviceResult != UA_STATUSCODE_GOOD) {
            emit logMessage(QString("Browse failed at '%1': 0x%2")
                                .arg(part)
                                .arg(bResp.responseHeader.serviceResult, 8, 16, QChar('0')));
            UA_BrowseResponse_clear(&bResp);
            UA_NodeId_clear(&currentNode);
            return UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
        }

        bool found = false;
        for (size_t i = 0; i < bResp.resultsSize && !found; ++i) {
            for (size_t j = 0; j < bResp.results[i].referencesSize; ++j) {
                UA_ReferenceDescription *ref = &bResp.results[i].references[j];
                QString browseName = QString::fromUtf8(
                    (const char*)ref->browseName.name.data,
                    (int)ref->browseName.name.length);
                if (browseName == part) {
                    UA_NodeId_clear(&currentNode);
                    UA_NodeId_copy(&ref->nodeId.nodeId, &currentNode);
                    found = true;
                    break;
                }
            }
        }
        UA_BrowseResponse_clear(&bResp);

        if (!found) {
            emit logMessage(QString("Tag path segment '%1' not found!").arg(part));
            UA_NodeId_clear(&currentNode);
            return UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
        }
    }

    return currentNode;
}

// ==================== 预解析所有 Tag 的 NodeId ====================
bool OpcClientManager::resolveAllTags()
{
    if (!m_client || !m_connected) {
        emit logMessage("Not connected, cannot resolve tags");
        return false;
    }

    // 先释放旧的 NodeId
    clearCachedNodeIds();

    int successCount = 0;
    for (int i = 0; i < m_tags.size(); ++i) {
        UA_NodeId nodeId = resolveTagPath(m_tags[i].name);
        if (nodeId.namespaceIndex == 0 &&
            nodeId.identifier.numeric == UA_NS0ID_OBJECTSFOLDER) {
            emit logMessage(QString("Tag[%1] '%2': resolution failed, skipped")
                                .arg(i).arg(m_tags[i].name));
            UA_NodeId_clear(&nodeId);
            continue;
        }
        // 缓存 NodeId（堆分配）
        m_tags[i].nodeId = (UA_NodeId*)UA_malloc(sizeof(UA_NodeId));
        UA_NodeId_copy(&nodeId, m_tags[i].nodeId);
        UA_NodeId_clear(&nodeId);
        ++successCount;
    }

    emit logMessage(QString("Resolved %1/%2 tags").arg(successCount).arg(m_tags.size()));
    return successCount > 0;
}

void OpcClientManager::clearCachedNodeIds()
{
    for (int i = 0; i < m_tags.size(); ++i) {
        if (m_tags[i].nodeId) {
            UA_NodeId_clear(m_tags[i].nodeId);
            UA_free(m_tags[i].nodeId);
            m_tags[i].nodeId = nullptr;
        }
    }
}

// ==================== 按索引读取（使用缓存的 NodeId）====================
QVariant OpcClientManager::readValue(int tagIndex)
{
    if (!m_client || !m_connected || tagIndex < 0 || tagIndex >= m_tags.size()) {
        emit logMessage("Invalid read");
        return QVariant();
    }

    // 获取 NodeId（优先用缓存的）
    UA_NodeId nodeId;
    bool needCleanup = false;

    if (m_tags[tagIndex].nodeId) {
        UA_NodeId_copy(m_tags[tagIndex].nodeId, &nodeId);
        needCleanup = true;
    } else {
        nodeId = resolveTagPath(m_tags[tagIndex].name);
        needCleanup = true;
        if (nodeId.namespaceIndex == 0 &&
            nodeId.identifier.numeric == UA_NS0ID_OBJECTSFOLDER) {
            UA_NodeId_clear(&nodeId);
            emit logMessage(QString("Tag[%1] not found: %2")
                                .arg(tagIndex).arg(m_tags[tagIndex].name));
            return QVariant();
        }
    }

    UA_Variant value;
    UA_Variant_init(&value);
    UA_StatusCode retval = UA_Client_readValueAttribute(m_client, nodeId, &value);

    if (needCleanup)
        UA_NodeId_clear(&nodeId);

    if (retval != UA_STATUSCODE_GOOD) {
        emit logMessage(QString("Read tag[%1] failed: 0x%2")
                            .arg(tagIndex).arg(retval, 8, 16, QChar('0')));
        return QVariant();
    }

    QVariant result = uaVariantToQVariant(value);
    UA_Variant_clear(&value);

    // 缓存值
    m_tags[tagIndex].value = result;
    return result;
}

// ==================== 按索引写入（使用缓存的 NodeId）====================
bool OpcClientManager::writeValue(int tagIndex, const QVariant &value)
{
    if (!m_client || !m_connected || tagIndex < 0 || tagIndex >= m_tags.size()) {
        emit logMessage("Invalid write");
        return false;
    }

    // 获取 NodeId（优先用缓存的）
    UA_NodeId nodeId;
    bool needCleanup = false;

    if (m_tags[tagIndex].nodeId) {
        UA_NodeId_copy(m_tags[tagIndex].nodeId, &nodeId);
        needCleanup = true;
    } else {
        nodeId = resolveTagPath(m_tags[tagIndex].name);
        needCleanup = true;
        if (nodeId.namespaceIndex == 0 &&
            nodeId.identifier.numeric == UA_NS0ID_OBJECTSFOLDER) {
            UA_NodeId_clear(&nodeId);
            return false;
        }
    }

    // 先读取当前值以获得准确的 OPC UA 数据类型，然后用相同类型写入
    // 这样能避免因类型不匹配（如 UInt32 vs Int32）导致写入失败
    UA_Variant currentValue;
    UA_Variant_init(&currentValue);
    UA_StatusCode readRet = UA_Client_readValueAttribute(m_client, nodeId, &currentValue);

    // 尝试用服务器实际的 Variant 类型写入
    auto writeWithType = [&](const UA_DataType *type) -> UA_StatusCode {
        UA_Variant uaValue;
        UA_Variant_init(&uaValue);

        if (type == &UA_TYPES[UA_TYPES_BOOLEAN]) {
            UA_Boolean v = value.toBool() ? 1 : 0;
            UA_Variant_setScalarCopy(&uaValue, &v, type);
        } else if (type == &UA_TYPES[UA_TYPES_BYTE]) {
            UA_Byte v = (UA_Byte)value.toUInt();
            UA_Variant_setScalarCopy(&uaValue, &v, type);
        } else if (type == &UA_TYPES[UA_TYPES_SBYTE]) {
            UA_SByte v = (UA_SByte)value.toInt();
            UA_Variant_setScalarCopy(&uaValue, &v, type);
        } else if (type == &UA_TYPES[UA_TYPES_INT16]) {
            UA_Int16 v = (UA_Int16)value.toInt();
            UA_Variant_setScalarCopy(&uaValue, &v, type);
        } else if (type == &UA_TYPES[UA_TYPES_UINT16]) {
            UA_UInt16 v = (UA_UInt16)value.toUInt();
            UA_Variant_setScalarCopy(&uaValue, &v, type);
        } else if (type == &UA_TYPES[UA_TYPES_INT32]) {
            UA_Int32 v = value.toInt();
            UA_Variant_setScalarCopy(&uaValue, &v, type);
        } else if (type == &UA_TYPES[UA_TYPES_UINT32]) {
            UA_UInt32 v = (UA_UInt32)value.toUInt();
            UA_Variant_setScalarCopy(&uaValue, &v, type);
        } else if (type == &UA_TYPES[UA_TYPES_INT64]) {
            UA_Int64 v = (UA_Int64)value.toLongLong();
            UA_Variant_setScalarCopy(&uaValue, &v, type);
        } else if (type == &UA_TYPES[UA_TYPES_UINT64]) {
            UA_UInt64 v = (UA_UInt64)value.toULongLong();
            UA_Variant_setScalarCopy(&uaValue, &v, type);
        } else if (type == &UA_TYPES[UA_TYPES_FLOAT]) {
            UA_Float v = (UA_Float)value.toDouble();
            UA_Variant_setScalarCopy(&uaValue, &v, type);
        } else if (type == &UA_TYPES[UA_TYPES_DOUBLE]) {
            UA_Double v = value.toDouble();
            UA_Variant_setScalarCopy(&uaValue, &v, type);
        } else if (type == &UA_TYPES[UA_TYPES_STRING]) {
            QByteArray utf8 = value.toString().toUtf8();
            UA_String v = UA_STRING_ALLOC(utf8.constData());
            UA_Variant_setScalarCopy(&uaValue, &v, type);
            UA_String_clear(&v);
        } else {
            UA_Variant_clear(&uaValue);
            return UA_STATUSCODE_BADTYPEMISMATCH;
        }

        UA_StatusCode ret = UA_Client_writeValueAttribute(m_client, nodeId, &uaValue);
        UA_Variant_clear(&uaValue);
        return ret;
    };

    UA_StatusCode writeRet = UA_STATUSCODE_BADUNEXPECTEDERROR;

    if (readRet == UA_STATUSCODE_GOOD && currentValue.type) {
        // 使用服务器返回的实际类型写入
        writeRet = writeWithType(currentValue.type);
        // 如果类型不匹配失败，再回退到猜测类型（兼容第一次写入）
        if (writeRet == UA_STATUSCODE_BADTYPEMISMATCH) {
            UA_Variant guess = qVariantToUaVariant(value, m_tags[tagIndex].type);
            writeRet = UA_Client_writeValueAttribute(m_client, nodeId, &guess);
            UA_Variant_clear(&guess);
        }
    } else {
        // 无法获取当前值类型，用配置文件中的 tagType 猜测
        UA_Variant guess = qVariantToUaVariant(value, m_tags[tagIndex].type);
        writeRet = UA_Client_writeValueAttribute(m_client, nodeId, &guess);
        UA_Variant_clear(&guess);
    }

    UA_Variant_clear(&currentValue);
    if (needCleanup)
        UA_NodeId_clear(&nodeId);

    if (writeRet != UA_STATUSCODE_GOOD) {
        emit logMessage(QString("Write tag[%1] failed: 0x%2 (%3)")
                            .arg(tagIndex).arg(writeRet, 8, 16, QChar('0'))
                            .arg(UA_StatusCode_name(writeRet)));
        return false;
    }

    m_tags[tagIndex].value = value;
    emit logMessage(QString("Tag[%1] = %2").arg(tagIndex).arg(value.toString()));
    return true;
}

// ==================== 订阅（OPC UA 监视项，类似原 MFC 异步回调）====================

// 静态 C 回调 —— 将 monContext（即 tagIndex 指针）转回 int，发射 dataChanged 信号
void OpcClientManager::onDataChange(UA_Client *client, UA_UInt32 subId,
                                     void *subContext, UA_UInt32 monId,
                                     void *monContext, UA_DataValue *value)
{
    Q_UNUSED(client);
    Q_UNUSED(subId);
    Q_UNUSED(monId);

    if (!subContext || !monContext || !value || !value->hasValue)
        return;

    OpcClientManager *self = static_cast<OpcClientManager*>(subContext);
    int tagIndex = static_cast<int>(reinterpret_cast<intptr_t>(monContext));

    if (tagIndex < 0 || tagIndex >= self->m_tags.size())
        return;

    QVariant val = uaVariantToQVariant(value->value);
    self->m_tags[tagIndex].value = val;
    emit self->dataChanged(tagIndex, val);
}

bool OpcClientManager::subscribe(int tagIndex, double samplingInterval)
{
    if (!m_client || !m_connected || tagIndex < 0 || tagIndex >= m_tags.size()) {
        emit logMessage("Invalid subscribe");
        return false;
    }

    if (m_monitoredItemIds.contains(tagIndex)) {
        // 已订阅，可更新采样间隔等
        return true;
    }

    // 获取 NodeId（优先用缓存）
    UA_NodeId nodeId;
    bool needCleanup = false;

    if (m_tags[tagIndex].nodeId) {
        UA_NodeId_copy(m_tags[tagIndex].nodeId, &nodeId);
        needCleanup = true;
    } else {
        nodeId = resolveTagPath(m_tags[tagIndex].name);
        needCleanup = true;
        if (nodeId.namespaceIndex == 0 &&
            nodeId.identifier.numeric == UA_NS0ID_OBJECTSFOLDER) {
            UA_NodeId_clear(&nodeId);
            return false;
        }
    }

    // 如果还没有创建订阅，先创建
    if (m_subscriptionId == 0) {
        UA_CreateSubscriptionRequest subRequest = UA_CreateSubscriptionRequest_default();
        subRequest.requestedPublishingInterval = (UA_Double)samplingInterval;
        subRequest.requestedMaxKeepAliveCount = 10;
        subRequest.requestedLifetimeCount      = 100;
        subRequest.maxNotificationsPerPublish  = 100;
        subRequest.publishingEnabled           = true;
        subRequest.priority                    = 0;

        UA_CreateSubscriptionResponse subResponse =
            UA_Client_Subscriptions_create(m_client, subRequest,
                                           this,   // subContext
                                           NULL,   // statusChangeCallback
                                           NULL);  // deleteCallback
        if (subResponse.responseHeader.serviceResult != UA_STATUSCODE_GOOD) {
            emit logMessage(QString("Create subscription failed: 0x%1")
                                .arg(subResponse.responseHeader.serviceResult, 8, 16, QChar('0')));
            if (needCleanup) UA_NodeId_clear(&nodeId);
            return false;
        }
        m_subscriptionId = subResponse.subscriptionId;
        emit logMessage(QString("Subscription created (id=%1)").arg(m_subscriptionId));
    }

    // 创建监视项
    UA_MonitoredItemCreateRequest monRequest = UA_MonitoredItemCreateRequest_default(nodeId);
    monRequest.requestedParameters.samplingInterval = samplingInterval;
    monRequest.requestedParameters.queueSize        = 1;
    monRequest.requestedParameters.discardOldest    = true;
    monRequest.monitoringMode                       = UA_MONITORINGMODE_REPORTING;

    // monContext 传 tagIndex
    void *monContext = reinterpret_cast<void*>(static_cast<intptr_t>(tagIndex));

    UA_MonitoredItemCreateResult monResult =
        UA_Client_MonitoredItems_createDataChange(
            m_client, m_subscriptionId,
            UA_TIMESTAMPSTORETURN_BOTH,
            monRequest,
            monContext,
            &OpcClientManager::onDataChange,
            NULL); // deleteCallback

    if (needCleanup) UA_NodeId_clear(&nodeId);

    if (monResult.statusCode != UA_STATUSCODE_GOOD) {
        emit logMessage(QString("Add monitored item for tag[%1] failed: 0x%2")
                            .arg(tagIndex).arg(monResult.statusCode, 8, 16, QChar('0')));
        return false;
    }

    m_monitoredItemIds[tagIndex] = monResult.monitoredItemId;
    emit logMessage(QString("Subscribed tag[%1] '%2' (monId=%3)")
                        .arg(tagIndex).arg(m_tags[tagIndex].name)
                        .arg(monResult.monitoredItemId));
    return true;
}

bool OpcClientManager::subscribeAll(double samplingInterval)
{
    if (!m_client || !m_connected) {
        emit logMessage("Not connected, cannot subscribe");
        return false;
    }

    int ok = 0;
    for (int i = 0; i < m_tags.size(); ++i) {
        if (subscribe(i, samplingInterval))
            ++ok;
    }
    emit logMessage(QString("Subscribed %1/%2 tags").arg(ok).arg(m_tags.size()));
    return ok > 0;
}

void OpcClientManager::unsubscribe(int tagIndex)
{
    if (!m_client || m_subscriptionId == 0)
        return;

    auto it = m_monitoredItemIds.find(tagIndex);
    if (it == m_monitoredItemIds.end())
        return;

    UA_Client_MonitoredItems_deleteSingle(m_client, m_subscriptionId, it.value());
    m_monitoredItemIds.erase(it);
    emit logMessage(QString("Unsubscribed tag[%1]").arg(tagIndex));
}

void OpcClientManager::unsubscribeAll()
{
    if (!m_client || m_subscriptionId == 0)
        return;

    // 删除所有监视项
    for (auto it = m_monitoredItemIds.constBegin(); it != m_monitoredItemIds.constEnd(); ++it) {
        UA_Client_MonitoredItems_deleteSingle(m_client, m_subscriptionId, it.value());
    }
    m_monitoredItemIds.clear();

    // 删除订阅
    UA_Client_Subscriptions_deleteSingle(m_client, m_subscriptionId);
    m_subscriptionId = 0;

    emit logMessage("All subscriptions removed");
}
