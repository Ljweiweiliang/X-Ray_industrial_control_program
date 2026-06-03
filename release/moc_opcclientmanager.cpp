/****************************************************************************
** Meta object code from reading C++ file 'opcclientmanager.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../opcclientmanager.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'opcclientmanager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_OpcClientManager_t {
    QByteArrayData data[9];
    char stringdata0[94];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_OpcClientManager_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_OpcClientManager_t qt_meta_stringdata_OpcClientManager = {
    {
QT_MOC_LITERAL(0, 0, 16), // "OpcClientManager"
QT_MOC_LITERAL(1, 17, 23), // "connectionStatusChanged"
QT_MOC_LITERAL(2, 41, 0), // ""
QT_MOC_LITERAL(3, 42, 9), // "connected"
QT_MOC_LITERAL(4, 52, 11), // "dataChanged"
QT_MOC_LITERAL(5, 64, 8), // "tagIndex"
QT_MOC_LITERAL(6, 73, 5), // "value"
QT_MOC_LITERAL(7, 79, 10), // "logMessage"
QT_MOC_LITERAL(8, 90, 3) // "msg"

    },
    "OpcClientManager\0connectionStatusChanged\0"
    "\0connected\0dataChanged\0tagIndex\0value\0"
    "logMessage\0msg"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_OpcClientManager[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   29,    2, 0x06 /* Public */,
       4,    2,   32,    2, 0x06 /* Public */,
       7,    1,   37,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Bool,    3,
    QMetaType::Void, QMetaType::Int, QMetaType::QVariant,    5,    6,
    QMetaType::Void, QMetaType::QString,    8,

       0        // eod
};

void OpcClientManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        OpcClientManager *_t = static_cast<OpcClientManager *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->connectionStatusChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->dataChanged((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< const QVariant(*)>(_a[2]))); break;
        case 2: _t->logMessage((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (OpcClientManager::*_t)(bool );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&OpcClientManager::connectionStatusChanged)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (OpcClientManager::*_t)(int , const QVariant & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&OpcClientManager::dataChanged)) {
                *result = 1;
                return;
            }
        }
        {
            typedef void (OpcClientManager::*_t)(const QString & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&OpcClientManager::logMessage)) {
                *result = 2;
                return;
            }
        }
    }
}

const QMetaObject OpcClientManager::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_OpcClientManager.data,
      qt_meta_data_OpcClientManager,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *OpcClientManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *OpcClientManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_OpcClientManager.stringdata0))
        return static_cast<void*>(const_cast< OpcClientManager*>(this));
    return QObject::qt_metacast(_clname);
}

int OpcClientManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void OpcClientManager::connectionStatusChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void OpcClientManager::dataChanged(int _t1, const QVariant & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void OpcClientManager::logMessage(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
