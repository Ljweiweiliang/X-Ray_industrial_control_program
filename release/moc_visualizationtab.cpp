/****************************************************************************
** Meta object code from reading C++ file 'visualizationtab.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../visualizationtab.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'visualizationtab.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_VisualizationTab_t {
    QByteArrayData data[10];
    char stringdata0[146];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_VisualizationTab_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_VisualizationTab_t qt_meta_stringdata_VisualizationTab = {
    {
QT_MOC_LITERAL(0, 0, 16), // "VisualizationTab"
QT_MOC_LITERAL(1, 17, 10), // "onLoadData"
QT_MOC_LITERAL(2, 28, 0), // ""
QT_MOC_LITERAL(3, 29, 14), // "onVolumeRender"
QT_MOC_LITERAL(4, 44, 12), // "onClipVolume"
QT_MOC_LITERAL(5, 57, 14), // "onTogglePlanes"
QT_MOC_LITERAL(6, 72, 20), // "onSliderAxialChanged"
QT_MOC_LITERAL(7, 93, 5), // "value"
QT_MOC_LITERAL(8, 99, 23), // "onSliderSagittalChanged"
QT_MOC_LITERAL(9, 123, 22) // "onSliderCoronalChanged"

    },
    "VisualizationTab\0onLoadData\0\0"
    "onVolumeRender\0onClipVolume\0onTogglePlanes\0"
    "onSliderAxialChanged\0value\0"
    "onSliderSagittalChanged\0onSliderCoronalChanged"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_VisualizationTab[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   49,    2, 0x08 /* Private */,
       3,    0,   50,    2, 0x08 /* Private */,
       4,    0,   51,    2, 0x08 /* Private */,
       5,    0,   52,    2, 0x08 /* Private */,
       6,    1,   53,    2, 0x08 /* Private */,
       8,    1,   56,    2, 0x08 /* Private */,
       9,    1,   59,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    7,
    QMetaType::Void, QMetaType::Int,    7,
    QMetaType::Void, QMetaType::Int,    7,

       0        // eod
};

void VisualizationTab::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        VisualizationTab *_t = static_cast<VisualizationTab *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->onLoadData(); break;
        case 1: _t->onVolumeRender(); break;
        case 2: _t->onClipVolume(); break;
        case 3: _t->onTogglePlanes(); break;
        case 4: _t->onSliderAxialChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->onSliderSagittalChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->onSliderCoronalChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject VisualizationTab::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_VisualizationTab.data,
      qt_meta_data_VisualizationTab,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *VisualizationTab::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *VisualizationTab::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_VisualizationTab.stringdata0))
        return static_cast<void*>(const_cast< VisualizationTab*>(this));
    return QWidget::qt_metacast(_clname);
}

int VisualizationTab::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 7;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
