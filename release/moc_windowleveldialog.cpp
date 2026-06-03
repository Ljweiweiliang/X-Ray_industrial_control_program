/****************************************************************************
** Meta object code from reading C++ file 'windowleveldialog.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../windowleveldialog.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'windowleveldialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_WindowLevelDialog_t {
    QByteArrayData data[8];
    char stringdata0[81];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_WindowLevelDialog_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_WindowLevelDialog_t qt_meta_stringdata_WindowLevelDialog = {
    {
QT_MOC_LITERAL(0, 0, 17), // "WindowLevelDialog"
QT_MOC_LITERAL(1, 18, 13), // "valuesChanged"
QT_MOC_LITERAL(2, 32, 0), // ""
QT_MOC_LITERAL(3, 33, 5), // "width"
QT_MOC_LITERAL(4, 39, 5), // "level"
QT_MOC_LITERAL(5, 45, 14), // "onWidthChanged"
QT_MOC_LITERAL(6, 60, 5), // "value"
QT_MOC_LITERAL(7, 66, 14) // "onLevelChanged"

    },
    "WindowLevelDialog\0valuesChanged\0\0width\0"
    "level\0onWidthChanged\0value\0onLevelChanged"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_WindowLevelDialog[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   29,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    1,   34,    2, 0x08 /* Private */,
       7,    1,   37,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int, QMetaType::Int,    3,    4,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    6,
    QMetaType::Void, QMetaType::Int,    6,

       0        // eod
};

void WindowLevelDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        WindowLevelDialog *_t = static_cast<WindowLevelDialog *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->valuesChanged((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 1: _t->onWidthChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->onLevelChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (WindowLevelDialog::*_t)(int , int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&WindowLevelDialog::valuesChanged)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject WindowLevelDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_WindowLevelDialog.data,
      qt_meta_data_WindowLevelDialog,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *WindowLevelDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *WindowLevelDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_WindowLevelDialog.stringdata0))
        return static_cast<void*>(const_cast< WindowLevelDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int WindowLevelDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
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
void WindowLevelDialog::valuesChanged(int _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
