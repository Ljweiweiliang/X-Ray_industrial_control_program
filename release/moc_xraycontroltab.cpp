/****************************************************************************
** Meta object code from reading C++ file 'xraycontroltab.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../xraycontroltab.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'xraycontroltab.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_XrayControlTab_t {
    QByteArrayData data[15];
    char stringdata0[179];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_XrayControlTab_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_XrayControlTab_t qt_meta_stringdata_XrayControlTab = {
    {
QT_MOC_LITERAL(0, 0, 14), // "XrayControlTab"
QT_MOC_LITERAL(1, 15, 15), // "serialConnected"
QT_MOC_LITERAL(2, 31, 0), // ""
QT_MOC_LITERAL(3, 32, 9), // "connected"
QT_MOC_LITERAL(4, 42, 17), // "xrayStatusChanged"
QT_MOC_LITERAL(5, 60, 2), // "on"
QT_MOC_LITERAL(6, 63, 15), // "feedbackUpdated"
QT_MOC_LITERAL(7, 79, 9), // "voltageFB"
QT_MOC_LITERAL(8, 89, 9), // "currentFB"
QT_MOC_LITERAL(9, 99, 15), // "onConnectSerial"
QT_MOC_LITERAL(10, 115, 7), // "onRayOn"
QT_MOC_LITERAL(11, 123, 8), // "onRayOff"
QT_MOC_LITERAL(12, 132, 15), // "onWatchdogTimer"
QT_MOC_LITERAL(13, 148, 15), // "onFeedbackTimer"
QT_MOC_LITERAL(14, 164, 14) // "onPreheatTimer"

    },
    "XrayControlTab\0serialConnected\0\0"
    "connected\0xrayStatusChanged\0on\0"
    "feedbackUpdated\0voltageFB\0currentFB\0"
    "onConnectSerial\0onRayOn\0onRayOff\0"
    "onWatchdogTimer\0onFeedbackTimer\0"
    "onPreheatTimer"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_XrayControlTab[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   59,    2, 0x06 /* Public */,
       4,    1,   62,    2, 0x06 /* Public */,
       6,    2,   65,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       9,    0,   70,    2, 0x08 /* Private */,
      10,    0,   71,    2, 0x08 /* Private */,
      11,    0,   72,    2, 0x08 /* Private */,
      12,    0,   73,    2, 0x08 /* Private */,
      13,    0,   74,    2, 0x08 /* Private */,
      14,    0,   75,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::Bool,    3,
    QMetaType::Void, QMetaType::Bool,    5,
    QMetaType::Void, QMetaType::Int, QMetaType::Float,    7,    8,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void XrayControlTab::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        XrayControlTab *_t = static_cast<XrayControlTab *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->serialConnected((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->xrayStatusChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->feedbackUpdated((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< float(*)>(_a[2]))); break;
        case 3: _t->onConnectSerial(); break;
        case 4: _t->onRayOn(); break;
        case 5: _t->onRayOff(); break;
        case 6: _t->onWatchdogTimer(); break;
        case 7: _t->onFeedbackTimer(); break;
        case 8: _t->onPreheatTimer(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (XrayControlTab::*_t)(bool );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&XrayControlTab::serialConnected)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (XrayControlTab::*_t)(bool );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&XrayControlTab::xrayStatusChanged)) {
                *result = 1;
                return;
            }
        }
        {
            typedef void (XrayControlTab::*_t)(int , float );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&XrayControlTab::feedbackUpdated)) {
                *result = 2;
                return;
            }
        }
    }
}

const QMetaObject XrayControlTab::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_XrayControlTab.data,
      qt_meta_data_XrayControlTab,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *XrayControlTab::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *XrayControlTab::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_XrayControlTab.stringdata0))
        return static_cast<void*>(const_cast< XrayControlTab*>(this));
    return QWidget::qt_metacast(_clname);
}

int XrayControlTab::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void XrayControlTab::serialConnected(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void XrayControlTab::xrayStatusChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void XrayControlTab::feedbackUpdated(int _t1, float _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
