#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPixmap>
#include <QVariant>
#include <QTimer>
#include <functional>

class QAction;
class QMenu;
class QGraphicsScene;
class QGraphicsView;
class QGraphicsPixmapItem;
class QWheelEvent;
class QLabel;
class QTabWidget;
class QThread;
class QPushButton;
class QStackedWidget;
class QVTKOpenGLWidget;
class EditOperations;
class AdjustOperations;
class StaticOperations;
class AcquisitionWorker;
class OpcClientManager;
class XrayControlTab;
class VisualizationTab;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    // 文件菜单
    void onActionOpen();
    void onActionSave();

    // 编辑菜单
    void onActionUndo();
    void onActionReset();
    void onActionCrop();
    void onActionRotateCW();
    void onActionRotateCCW();

    // 采集
    void onConnectClicked();
    void onAcquireClicked();
    void onOffsetClicked();
    void onGainClicked();
    void onFrameReady();
    void onSaveFrameClicked();

    // 调整菜单
    void onActionWindowLevel();
    void onActionContrastBrightness();

    //opc测试
    void onopcConnectClicked();
    void onopcWrite();

private:
    // 滤镜切换枚举（必须在 applyFilter 声明之前）
    enum ActiveFilter { FilterNone, FilterSharpen, FilterMean, FilterNegative, FilterEmboss };

    // 静态界面按钮（通过lambda连接，不依赖MOC）
    void onStaticOpen();
    void onStaticSave();
    void onStaticDrawRect();
    void onStaticDrawLine();
    void onStaticCalibrate();
    void onStaticMeasure();
    void onStaticAutoWindow();
    void onStaticPseudoColor();
    void onStaticSharpen();
    void onStaticMeanFilter();
    void onStaticNegative();
    void onStaticEmboss();
    void onStaticHistogram();

    // StaticOperations 信号回调
    void onCalibrationRequested(double pixelLength);
    void onMeasurementCompleted(double pixelLength, double realDistance);
    void onAutoWindowRequested(const QRectF &roi);

    // OPC 定时轮询（后备回调方式）
    void timerEvent(QTimerEvent *event) override;

    void applyStylesheet();
    void createMenus();
    void setupImageViewer();
    void setupAcquisitionTab(QWidget *tab);
    void setupStaticTab();
    void resetDrawButtonTexts();
    void resetFilterButtonTexts();
    void applyFilter(ActiveFilter filter,
                     std::function<QImage(const QImage&)> func,
                     const QString &logName);
    void render16BitFrame();
    void zoomIn();
    void zoomOut();
    void zoomInOut(double factor);
    void fitToWindow();
    void updateZoomLabel();
    void saveUndoState();
    void restorePixmap(const QPixmap &pixmap);
    void saveSettings();
    void loadSettings();

    // 文件菜单
    QMenu   *m_menuFile;
    QAction *m_actionOpen;
    QAction *m_actionSave;

    // 编辑菜单
    QMenu   *m_menuEdit;
    QAction *m_actionUndo;
    QAction *m_actionReset;
    QAction *m_actionCrop;
    QAction *m_actionRotateCW;
    QAction *m_actionRotateCCW;

    //调整菜单
    QMenu  *m_menuAdjust;
    QAction *m_actionWindowLevel;
    QAction *m_actionContrastBrightness;

    // 图像查看器
    QGraphicsScene      *m_scene;
    QGraphicsView       *m_view;
    QGraphicsPixmapItem *m_pixmapItem;
    double               m_zoomFactor;
    QLabel              *m_zoomLabel;
    QLabel              *m_versionLabel;
    QTabWidget          *m_tabWidget;
    QPixmap              m_originalPixmap;
    QPixmap              m_undoPixmap;

    // 模式切换：主视图区域使用 QStackedWidget 切换 2D/3D
    QStackedWidget      *m_viewStack;
    QWidget             *m_3dViewContainer;
    QVTKOpenGLWidget    *m_visView3D;
    QVTKOpenGLWidget    *m_visViewAxial;
    QVTKOpenGLWidget    *m_visViewSagittal;
    QVTKOpenGLWidget    *m_visViewCoronal;

    // 操作管理器
    EditOperations    *m_editOps;
    AdjustOperations  *m_adjustOps;
    StaticOperations  *m_staticOps;

    // 采集
    QPushButton       *m_btnConnect;
    QPushButton       *m_btnAcquire;
    QPushButton       *m_btnOffset;
    QPushButton       *m_btnGain;
    QPushButton       *btnSaveFrame;
    QThread           *m_acqThread;
    AcquisitionWorker *m_acqWorker;
    bool               m_isConnected;
    bool               m_isAcquiring;
    bool               m_isAcquisitionActive;  // 采集中为 true，停止后立即 false
    int                m_winWidth;
    int                m_winLevel;
    int                m_brightness;
    int                m_contrast;

    //opc
    QPushButton *m_opcConnect;
    QPushButton *m_opcWrite;
    QPushButton *m_btnDrawRect;
    QPushButton *m_btnDrawLine;
    QPushButton *m_btnCalibrate;
    QPushButton *m_btnMeasure;
    QPushButton *m_btnAutoWindow;
    QPushButton *m_btnPseudoColor;
    QPushButton *m_btnSharpen;
    QPushButton *m_btnMeanFilter;
    QPushButton *m_btnNegative;
    QPushButton *m_btnEmboss;
    OpcClientManager *m_opcClient;

    // 伪彩色切换
    bool    m_isPseudoColor;
    QPixmap m_prePseudoPixmap;

    // 滤镜切换（锐化/滤波/负片/浮雕 四选一）
    ActiveFilter m_activeFilter;
    QPixmap m_preFilterPixmap;

    // 自动调窗基线（进入模式时保存，后续均基于此图像）
    QPixmap m_autoWindowBaseline;

    // 文件加载的源图像（用于窗位窗宽/亮度对比度调整）
    QImage m_sourceImage;

    void applyFileImageAdjustments();

    // OPC 定时轮询
    int              m_pollTimerId;          // 定时器 ID，0 表示未启动
    QVector<QVariant> m_lastPollValues;      // 上次轮询值（下标与 tag 索引对应）

    // 射线控制 Tab（用于配置持久化）
    XrayControlTab *m_xrayTab;

    // 三维可视化控制面板（仅按钮，视图在主区域）
    VisualizationTab *m_visTab;

    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
