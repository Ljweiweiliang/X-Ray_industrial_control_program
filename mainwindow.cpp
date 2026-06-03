#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMenu>
#include <QAction>
#include <QIcon>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QWheelEvent>
#include <QResizeEvent>
#include <QFileDialog>
#include <QScrollBar>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTransform>
#include <QColor>
#include <QFileInfo>
#include <QFile>
#include <QCoreApplication>
#include <QTabWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDialog>
#include <QThread>
#include <QInputDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QDir>
#include <QSettings>
#include <QCloseEvent>
#include <QStackedWidget>
#include <functional>
#include "edit_operations.h"
#include "adjust_operations.h"
#include "static_operations.h"
#include "acquisitionworker.h"
#include "windowleveldialog.h"
#include "brightnesscontrastdialog.h"
#include "xraycontroltab.h"
#include "visualizationtab.h"
#include "logmanager.h"
#include "opcclientmanager.h"
#include "windowleveldialog.h"
#include "brightnesscontrastdialog.h"
#include <QVTKOpenGLWidget.h>

#include <open62541/client.h>
#include <open62541/client_config_default.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_scene(nullptr),
    m_view(nullptr),
    m_pixmapItem(nullptr),
    m_zoomFactor(1.0),
    m_zoomLabel(nullptr),
    m_versionLabel(nullptr),
    m_editOps(nullptr),
    m_adjustOps(nullptr),
    m_btnConnect(nullptr),
    m_btnAcquire(nullptr),
    m_acqThread(nullptr),
    m_acqWorker(nullptr),
    m_isConnected(false),
    m_isAcquiring(false),
    m_isAcquisitionActive(false),
    m_winWidth(4096),
    m_winLevel(32768),
    m_brightness(0),
    m_contrast(0),
    m_btnDrawRect(nullptr),
    m_btnDrawLine(nullptr),
    m_btnCalibrate(nullptr),
    m_btnMeasure(nullptr),
    m_btnAutoWindow(nullptr),
    m_btnPseudoColor(nullptr),
    m_btnSharpen(nullptr),
    m_btnMeanFilter(nullptr),
    m_btnNegative(nullptr),
    m_btnEmboss(nullptr),
    m_opcClient(nullptr),
    m_isPseudoColor(false),
    m_activeFilter(FilterNone),
    m_pollTimerId(0),
    m_xrayTab(nullptr),
    m_viewStack(nullptr),
    m_3dViewContainer(nullptr),
    m_visView3D(nullptr),
    m_visViewAxial(nullptr),
    m_visViewSagittal(nullptr),
    m_visViewCoronal(nullptr),
    m_visTab(nullptr)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/resources/icons/app.ico"));
    setMinimumSize(2100, 1300);
    applyStylesheet();
    setupImageViewer();

    m_editOps = new EditOperations(this);
    m_editOps->setTarget(m_scene, &m_pixmapItem, m_view);

    m_adjustOps = new AdjustOperations(this);
    m_adjustOps->setTarget(m_scene, &m_pixmapItem);

    m_staticOps = new StaticOperations(this);
    m_staticOps->setTarget(m_scene, &m_pixmapItem, m_view);

    // 连接 StaticOperations 信号
    connect(m_staticOps, &StaticOperations::calibrationRequested,
            this, &MainWindow::onCalibrationRequested);
    connect(m_staticOps, &StaticOperations::measurementCompleted,
            this, &MainWindow::onMeasurementCompleted);
    connect(m_staticOps, &StaticOperations::autoWindowRequested,
            this, &MainWindow::onAutoWindowRequested);

    createMenus();

    // 初始化 OPC UA 管理器
    m_opcClient = new OpcClientManager(this);
    QString ver = m_opcClient->getVersion();
    LogManager::instance()->logInfo(QString("open62541 version: %1").arg(ver));

    // 加载标记配置文件
    QStringList datPaths = {
        QCoreApplication::applicationDirPath() + "/KepSever.dat",
        QCoreApplication::applicationDirPath() + "/../KepSever/KepSever.dat",
        QCoreApplication::applicationDirPath() + "/../../KepSever/KepSever.dat"
    };

    QString datPath;
    for (const QString &p : datPaths) {
        if (QFile::exists(p)) { datPath = p; break; }
    }

    if (!datPath.isEmpty()) {
        m_opcClient->loadConfig(datPath);
        LogManager::instance()->logInfo(
            QString("Loaded %1 tags from %2").arg(m_opcClient->tagCount()).arg(datPath));
    } else {
        LogManager::instance()->logError("KepSever.dat not found!");
    }

    // 加载上次保存的配置
    loadSettings();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ==================== 全局样式表 ====================
void MainWindow::applyStylesheet()
{
    setStyleSheet(R"(
        /* ===== 全局 ===== */
        QMainWindow, QWidget {
            background-color: #1e1e2e;
            color: #cdd6f4;
            font-family: "Microsoft YaHei", "Segoe UI", sans-serif;
            font-size: 13px;
        }

        /* ===== 菜单栏 ===== */
        QMenuBar {
            background-color: #181825;
            border-bottom: 1px solid #313244;
            padding: 2px 0;
        }
        QMenuBar::item {
            padding: 6px 14px;
            background: transparent;
            border-radius: 4px;
            margin: 2px 2px;
        }
        QMenuBar::item:selected {
            background: #45475a;
        }
        QMenu {
            background: #1e1e2e;
            border: 1px solid #45475a;
            border-radius: 6px;
            padding: 4px;
        }
        QMenu::item {
            padding: 6px 28px 6px 20px;
            border-radius: 4px;
        }
        QMenu::item:selected {
            background: #45475a;
        }
        QMenu::separator {
            height: 1px;
            background: #45475a;
            margin: 4px 10px;
        }

        /* ===== 状态栏 ===== */
        QStatusBar {
            background: #181825;
            border-top: 1px solid #313244;
            color: #a6adc8;
            font-size: 12px;
        }
        QStatusBar::item { border: none; }

        /* ===== Tab 控件 ===== */
        QTabWidget::pane {
            border: 1px solid #313244;
            border-radius: 6px;
            background: #1e1e2e;
            top: -1px;
        }
        QTabBar::tab {
            background: #181825;
            color: #6c7086;
            padding: 8px 18px;
            margin: 0 2px;
            border: 1px solid transparent;
            border-bottom: none;
            border-radius: 6px 6px 0 0;
            font-size: 13px;
            min-width: 70px;
        }
        QTabBar::tab:selected {
            background: #1e1e2e;
            color: #89b4fa;
            border-color: #313244;
            font-weight: bold;
        }
        QTabBar::tab:hover:!selected {
            color: #cdd6f4;
            background: #313244;
        }

        /* ===== 按钮 ===== */
        QPushButton {
            background: #313244;
            color: #cdd6f4;
            border: 1px solid #45475a;
            border-radius: 6px;
            padding: 8px 18px;
            font-size: 13px;
            min-height: 20px;
        }
        QPushButton:hover {
            background: #45475a;
            border-color: #585b70;
        }
        QPushButton:pressed {
            background: #585b70;
        }
        QPushButton:disabled {
            background: #181825;
            color: #585b70;
            border-color: #313244;
        }
        /* 采集/连接类按钮用绿色强调 */
        QPushButton[accent="true"] {
            background: #a6e3a1;
            color: #1e1e2e;
            border-color: #a6e3a1;
            font-weight: bold;
        }
        QPushButton[accent="true"]:hover {
            background: #94e2d5;
            border-color: #94e2d5;
        }
        QPushButton[accent="true"]:pressed {
            background: #74c7a4;
        }
        /* 危险/断开类按钮 */
        QPushButton[danger="true"] {
            background: #f38ba8;
            color: #1e1e2e;
            border-color: #f38ba8;
            font-weight: bold;
        }

        /* ===== 日志文本框 ===== */
        QTextEdit {
            background: #11111b;
            color: #a6adc8;
            border: 1px solid #313244;
            border-radius: 6px;
            padding: 6px 8px;
            font-family: "Consolas", "Courier New", monospace;
            font-size: 12px;
            selection-background-color: #45475a;
        }

        /* ===== 滚动条 ===== */
        QScrollBar:vertical {
            background: #1e1e2e;
            width: 10px;
            margin: 0;
            border-radius: 5px;
        }
        QScrollBar::handle:vertical {
            background: #45475a;
            min-height: 30px;
            border-radius: 5px;
        }
        QScrollBar::handle:vertical:hover { background: #585b70; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0; width: 0;
        }
        QScrollBar:horizontal {
            background: #1e1e2e;
            height: 10px;
            margin: 0;
            border-radius: 5px;
        }
        QScrollBar::handle:horizontal {
            background: #45475a;
            min-width: 30px;
            border-radius: 5px;
        }
        QScrollBar::handle:horizontal:hover { background: #585b70; }
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
            height: 0; width: 0;
        }

        /* ===== 图形视图 ===== */
        QGraphicsView {
            border: 1px solid #313244;
            border-radius: 6px;
            background: #0a0a14;
        }

        /* ===== 输入框 ===== */
        QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox {
            background: #313244;
            color: #cdd6f4;
            border: 1px solid #45475a;
            border-radius: 4px;
            padding: 4px 8px;
            font-size: 13px;
        }
        QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QComboBox:focus {
            border-color: #89b4fa;
        }

        /* ===== 滑块 ===== */
        QSlider::groove:horizontal {
            background: #313244;
            height: 6px;
            border-radius: 3px;
        }
        QSlider::handle:horizontal {
            background: #89b4fa;
            width: 16px;
            height: 16px;
            margin: -5px 0;
            border-radius: 8px;
        }
        QSlider::handle:horizontal:hover {
            background: #b4d0fb;
        }
        QSlider::sub-page:horizontal {
            background: #89b4fa;
            border-radius: 3px;
        }

        /* ===== 分组框 ===== */
        QGroupBox {
            border: 1px solid #313244;
            border-radius: 6px;
            margin-top: 14px;
            padding: 12px 8px 8px 8px;
            font-size: 13px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 0 8px;
            color: #89b4fa;
        }

        /* ===== 标签 ===== */
        QLabel {
            color: #cdd6f4;
            background: transparent;
        }

        /* ===== 分割器 ===== */
        QSplitter::handle {
            background: #313244;
            width: 2px;
        }
    )");
}

// ==================== 图像查看器 ====================
void MainWindow::setupImageViewer()
{
    m_scene = new QGraphicsScene(this);

    m_view = new QGraphicsView(m_scene);
    m_view->setRenderHint(QPainter::SmoothPixmapTransform); //当对图像进行缩放/变换时，启用平滑（双线性）滤波
    m_view->setDragMode(QGraphicsView::ScrollHandDrag);//设置拖拽模式为“手形拖动”
    m_view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);//设置变换锚点为鼠标光标下方点，效果类似于：放大时，鼠标指向的位置保持不动，周围内容向鼠标方向放大。
    m_view->setResizeAnchor(QGraphicsView::AnchorUnderMouse); //调整视图大小时的锚点为鼠标下方点
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);//禁用水平与垂直滚动条
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setFrameShape(QFrame::NoFrame);//去掉视图的边框
    m_view->setMinimumSize(1536, 1024);

    // ===== 日志区域（提前创建，方便后续所有模块输出日志）=====
    QTextEdit *logWidget = new QTextEdit(this);
    logWidget->setReadOnly(true);
    logWidget->setMaximumHeight(200);
    logWidget->setPlaceholderText(tr("Log messages..."));
    LogManager::instance()->setLogWidget(logWidget);
    LogManager::instance()->logInfo(tr("Application started"));

    // ===== 右侧 Tab 页面 =====
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setMinimumWidth(500);

    // 采集界面占位
    QWidget *tabAcq = new QWidget();
    m_tabWidget->addTab(tabAcq, tr("采集界面"));
    setupAcquisitionTab(tabAcq);

    // 静态界面
    setupStaticTab();

    // 射线控制界面
    XrayControlTab *tabXray = new XrayControlTab(this);
    m_tabWidget->addTab(tabXray, tr("射线控制"));
    m_xrayTab = tabXray;

    // ===== 创建 4 个 VTK 视图控件（用于三维可视化，放在主视图区域）=====
    m_visView3D      = new QVTKOpenGLWidget(this);
    m_visViewAxial   = new QVTKOpenGLWidget(this);
    m_visViewSagittal = new QVTKOpenGLWidget(this);
    m_visViewCoronal = new QVTKOpenGLWidget(this);

    m_visView3D->setMinimumSize(300, 250);
    m_visViewAxial->setMinimumSize(250, 200);
    m_visViewSagittal->setMinimumSize(250, 200);
    m_visViewCoronal->setMinimumSize(250, 200);

    // 将 4 个 VTK 视图放入一个容器（2×2 网格）
    m_3dViewContainer = new QWidget(this);
    QGridLayout *grid3d = new QGridLayout(m_3dViewContainer);
    grid3d->setContentsMargins(0, 0, 0, 0);
    grid3d->setSpacing(2);
    grid3d->addWidget(m_visViewAxial,    0, 0);  // 横断面   (左上)
    grid3d->addWidget(m_visViewSagittal, 0, 1);  // 矢状面   (右上)
    grid3d->addWidget(m_visViewCoronal,  1, 0);  // 冠状面   (左下)
    grid3d->addWidget(m_visView3D,       1, 1);  // 3D 视图 (右下)

    // ===== 主视图区域使用 QStackedWidget 切换 2D/3D =====
    m_viewStack = new QStackedWidget(this);
    m_viewStack->addWidget(m_view);           // Page 0: 2D 图像视图
    m_viewStack->addWidget(m_3dViewContainer); // Page 1: 3D 四视图

    // 三维可视化控制面板（仅按钮，添加到 Tab 中）
    m_visTab = new VisualizationTab(this);
    m_tabWidget->addTab(m_visTab, tr("三维可视化"));

    // 将 VTK 视图控件传入 VisualizationTab，由其初始化 VTK 管线
    m_visTab->setViewWidgets(m_visView3D, m_visViewAxial,
                             m_visViewSagittal, m_visViewCoronal);

    // 连接 Tab 切换信号：选中"三维可视化"时切换到 3D 视图，否则切回 2D
    connect(m_tabWidget, &QTabWidget::currentChanged, this, [this](int index) {
        if (m_tabWidget->widget(index) == m_visTab) {
            m_viewStack->setCurrentIndex(1);  // 显示 3D 四视图
        } else {
            m_viewStack->setCurrentIndex(0);  // 显示 2D 图像
        }
    });

    // 右侧垂直布局：上方 Tab + 下方日志
    QWidget *rightPanel = new QWidget(this);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(2);
    rightLayout->addWidget(m_tabWidget, 1);
    rightLayout->addWidget(logWidget, 0);

    // 主布局：左侧图像栈 + 右侧面板
    QWidget *container = new QWidget(this);
    QHBoxLayout *hLayout = new QHBoxLayout(container);
    hLayout->addWidget(m_viewStack, 1);
    hLayout->addWidget(rightPanel, 0);
    container->setLayout(hLayout);
    setCentralWidget(container);

    // 状态栏左下角显示缩放比例
    m_zoomLabel = new QLabel(tr("100%"));
    m_zoomLabel->setMinimumWidth(60);
    m_zoomLabel->setAlignment(Qt::AlignCenter);
    ui->statusBar->addPermanentWidget(m_zoomLabel);

    // 状态栏右下角显示版本信息
    m_versionLabel = new QLabel(tr("v1.0.0"));
    m_versionLabel->setMinimumWidth(50);
    m_versionLabel->setAlignment(Qt::AlignCenter);
    m_versionLabel->setStyleSheet("QLabel { color: #888; }");
    ui->statusBar->addPermanentWidget(m_versionLabel);
}

// ==================== 采集界面 ====================
void MainWindow::setupAcquisitionTab(QWidget *tab)
{
    QVBoxLayout *layout = new QVBoxLayout(tab);
    layout->setAlignment(Qt::AlignTop);

    m_btnConnect = new QPushButton(tr("平板连接"), tab);
    m_btnConnect->setMinimumHeight(36);
    layout->addWidget(m_btnConnect);

    m_btnAcquire = new QPushButton(tr("开始采集"), tab);
    m_btnAcquire->setMinimumHeight(36);
    m_btnAcquire->setEnabled(false);
    layout->addWidget(m_btnAcquire);


    m_btnOffset = new QPushButton(tr("校正偏置"), tab);
    m_btnOffset->setMinimumHeight(36);
    m_btnOffset->setEnabled(false);
    layout->addWidget(m_btnOffset);
    m_btnGain = new QPushButton(tr("校正增益"), tab);
    m_btnGain->setMinimumHeight(36);
    m_btnGain->setEnabled(false);
    layout->addWidget(m_btnGain);

    layout->addStretch(); //放置弹簧
    // 保存当前帧
    btnSaveFrame = new QPushButton(tr("保存当前帧"), tab);
    btnSaveFrame->setMinimumHeight(36);
    btnSaveFrame->setEnabled(false);
    layout->addWidget(btnSaveFrame);

    //添加读写opc节点测试
    layout->addStretch(); //放置弹簧
    m_opcConnect = new QPushButton(tr("连接opc服务器"), tab);
    m_opcConnect->setMinimumHeight(36);
    layout->addWidget(m_opcConnect);

    m_opcWrite = new QPushButton(tr("opc读写测试"), tab);
    m_opcWrite->setMinimumHeight(36);
    layout->addWidget(m_opcWrite);





    connect(m_btnConnect, &QPushButton::clicked, this, &MainWindow::onConnectClicked);
    connect(m_btnAcquire, &QPushButton::clicked, this, &MainWindow::onAcquireClicked);
    connect(m_btnOffset, &QPushButton::clicked, this, &MainWindow::onOffsetClicked);
    connect(m_btnGain, &QPushButton::clicked, this, &MainWindow::onGainClicked);
    connect(btnSaveFrame, &QPushButton::clicked, this, &MainWindow::onSaveFrameClicked);
    connect(m_opcConnect, &QPushButton::clicked, this, &MainWindow::onopcConnectClicked);
    connect(m_opcWrite, &QPushButton::clicked, this, &MainWindow::onopcWrite);

}

// ==================== 静态界面 ====================
void MainWindow::setupStaticTab()
{
    LogManager::instance()->logInfo("setupStaticTab() called - creating buttons...");

    QWidget *tab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(tab);
    layout->setAlignment(Qt::AlignTop);//设置布局内所有控件的对齐方式为顶部对齐（垂直方向）
    layout->setSpacing(4);//设置布局内控件之间的间距

    QPushButton *btn;

    btn = new QPushButton(tr("打开图像"), tab);
    btn->setMinimumHeight(32);
    connect(btn, &QPushButton::clicked, this, [this](){ onStaticOpen(); });
    layout->addWidget(btn);

    btn = new QPushButton(tr("保存图像"), tab);
    btn->setMinimumHeight(32);
    connect(btn, &QPushButton::clicked, this, [this](){ onStaticSave(); });
    layout->addWidget(btn);

    btn = new QPushButton(tr("画矩形"), tab);
    btn->setMinimumHeight(32);
    connect(btn, &QPushButton::clicked, this, [this](){ onStaticDrawRect(); });
    layout->addWidget(btn);
    m_btnDrawRect = btn;

    btn = new QPushButton(tr("画线"), tab);
    btn->setMinimumHeight(32);
    connect(btn, &QPushButton::clicked, this, [this](){ onStaticDrawLine(); });
    layout->addWidget(btn);
    m_btnDrawLine = btn;

    // ---- 尺寸标定 ----
    btn = new QPushButton(tr("尺寸标定"), tab);
    btn->setMinimumHeight(32);
    connect(btn, &QPushButton::clicked, this, [this](){ onStaticCalibrate(); });
    layout->addWidget(btn);
    m_btnCalibrate = btn;

    // ---- 尺寸测量 ----
    btn = new QPushButton(tr("尺寸测量"), tab);
    btn->setMinimumHeight(32);
    connect(btn, &QPushButton::clicked, this, [this](){ onStaticMeasure(); });
    layout->addWidget(btn);
    m_btnMeasure = btn;

    // ---- 自动调窗 ----
    btn = new QPushButton(tr("自动调窗"), tab);
    btn->setMinimumHeight(32);
    connect(btn, &QPushButton::clicked, this, [this](){ onStaticAutoWindow(); });
    layout->addWidget(btn);
    m_btnAutoWindow = btn;

    // ---- 伪彩色 ----
    btn = new QPushButton(tr("伪彩色"), tab);
    btn->setMinimumHeight(32);
    connect(btn, &QPushButton::clicked, this, [this](){ onStaticPseudoColor(); });
    layout->addWidget(btn);
    m_btnPseudoColor = btn;

    btn = new QPushButton(tr("锐化"), tab);
    btn->setMinimumHeight(32);
    connect(btn, &QPushButton::clicked, this, [this](){ onStaticSharpen(); });
    layout->addWidget(btn);
    m_btnSharpen = btn;

    btn = new QPushButton(tr("均值滤波"), tab);
    btn->setMinimumHeight(32);
    connect(btn, &QPushButton::clicked, this, [this](){ onStaticMeanFilter(); });
    layout->addWidget(btn);
    m_btnMeanFilter = btn;

    btn = new QPushButton(tr("负片"), tab);
    btn->setMinimumHeight(32);
    connect(btn, &QPushButton::clicked, this, [this](){ onStaticNegative(); });
    layout->addWidget(btn);
    m_btnNegative = btn;

    btn = new QPushButton(tr("浮雕"), tab);
    btn->setMinimumHeight(32);
    connect(btn, &QPushButton::clicked, this, [this](){ onStaticEmboss(); });
    layout->addWidget(btn);
    m_btnEmboss = btn;

    btn = new QPushButton(tr("绘制直方图"), tab);
    btn->setMinimumHeight(32);
    connect(btn, &QPushButton::clicked, this, [this](){ onStaticHistogram(); });
    layout->addWidget(btn);

    layout->addStretch();
    m_tabWidget->addTab(tab, tr("静态界面"));
    LogManager::instance()->logInfo(QString("Static tab added at index %1 with 13 buttons").arg(m_tabWidget->indexOf(tab)));
}

void MainWindow::onConnectClicked()
{
    m_isConnected = !m_isConnected;
    if (m_isConnected)
    {
        m_btnConnect->setText(tr("平板断开"));
        m_btnAcquire->setEnabled(true);
        m_btnGain->setEnabled(true);
        m_btnOffset->setEnabled(true);
        btnSaveFrame->setEnabled(true);
        LogManager::instance()->logSuccess(tr("Panel connected"));
    }
    else
    {
        if (m_isAcquiring) onAcquireClicked();
        m_btnConnect->setText(tr("平板连接"));
        m_btnAcquire->setEnabled(false);
        m_btnGain->setEnabled(false);
        m_btnOffset->setEnabled(false);
        btnSaveFrame->setEnabled(false);
        LogManager::instance()->logInfo(tr("Panel disconnected"));
    }
}

void MainWindow::onAcquireClicked()
{
    m_isAcquiring = !m_isAcquiring;
    if (m_isAcquiring)
    {
        m_btnAcquire->setText(tr("停止采集"));
        LogManager::instance()->logInfo(tr("Acquisition started"));

        m_acqThread = new QThread(this);
        m_acqWorker = new AcquisitionWorker();
        m_acqWorker->moveToThread(m_acqThread);
        m_isAcquisitionActive = true;

        connect(m_acqThread, &QThread::started, m_acqWorker, &AcquisitionWorker::start);
        connect(m_acqWorker, &AcquisitionWorker::frameReady, this, &MainWindow::onFrameReady);
        connect(m_acqWorker, &AcquisitionWorker::frameReady, this, [this]() {
            if (!m_isAcquiring && m_acqThread) m_acqThread->quit();
        });
        connect(m_acqThread, &QThread::finished, m_acqWorker, &QObject::deleteLater);
        connect(m_acqThread, &QThread::finished, m_acqThread, &QObject::deleteLater);
        connect(m_acqThread, &QThread::finished, this, [this]() {
            m_acqThread = nullptr;
            m_acqWorker = nullptr;
        });

        m_acqThread->start();
    }
    else
    {
        m_btnAcquire->setText(tr("开始采集"));
        LogManager::instance()->logInfo(tr("Acquisition stopped"));
        m_isAcquisitionActive = false;
        if (m_acqWorker) m_acqWorker->stop();
    }
}


void MainWindow::onOffsetClicked()
{
    //偏移校正
    LogManager::instance()->logInfo(tr("Offset calibration started"));
    // 模拟耗时操作
    QThread::sleep(2);
    LogManager::instance()->logSuccess(tr("Offset calibration completed"));
}

void MainWindow::onGainClicked()
{
    //增益校正
    LogManager::instance()->logInfo(tr("Gain calibration started"));
    // 模拟耗时操作
    QThread::sleep(2);
    LogManager::instance()->logSuccess(tr("Gain calibration completed"));
}


void MainWindow::onopcConnectClicked()
{
    LogManager::instance()->logInfo("onopcConnectClicked called");

    if (m_opcClient->isConnected()) {
        // 断开连接，停止轮询
        if (m_pollTimerId) {
            killTimer(m_pollTimerId);
            m_pollTimerId = 0;
        }
        m_opcClient->unsubscribeAll();
        m_opcClient->disconnect();
        m_opcConnect->setText(tr("Connect OPC"));
    } else {
        // 使用配置文件中的地址连接
        m_opcClient->connectToServer();
        if (m_opcClient->isConnected()) {
            m_opcConnect->setText(tr("Disconnect OPC"));
            LogManager::instance()->logInfo(
                QString("Connected, %1 tags loaded, resolving...").arg(m_opcClient->tagCount()));

            // 预解析所有 Tag 的 NodeId
            m_opcClient->resolveAllTags();

            // 订阅所有 Tag 的数据变化（OPC UA 监视项，类似原 MFC 回调）
            m_opcClient->subscribeAll(100.0);

            // 连接 dataChanged 信号
            connect(m_opcClient, &OpcClientManager::dataChanged,
                    this, [](int tagIndex, const QVariant &val) {
                LogManager::instance()->logInfo(
                    QString("[Callback] Tag[%1] changed -> %2")
                        .arg(tagIndex).arg(val.toString()));
            }, Qt::UniqueConnection);

            // 初始化轮询值数组
            int n = m_opcClient->tagCount();
            m_lastPollValues.resize(n);
            for (int i = 0; i < n; ++i)
                m_lastPollValues[i] = QVariant();

            // 启动定时轮询（500ms 间隔，遍历所有标签）
            m_pollTimerId = startTimer(200);

            LogManager::instance()->logInfo(
                "Subscriptions + polling active");
        }
    }
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

    if (!m_opcClient || !m_opcClient->isConnected())
        return;

    int n = m_opcClient->tagCount();
    if (n <= 0) return;

    // 确保数组长度与标签数一致
    if (m_lastPollValues.size() != n)
        m_lastPollValues.resize(n);

    // 遍历所有标签，检测值变化
    for (int i = 0; i < n; ++i) {
        QVariant val = m_opcClient->readValue(i);
        if (!val.isValid())
            continue;

        if (val != m_lastPollValues[i]) {
            m_lastPollValues[i] = val;
            LogManager::instance()->logInfo(
                QString("[Polling] Tag[%1] '%2' = %3")
                    .arg(i)
                    .arg(m_opcClient->tagName(i))
                    .arg(val.toString()));
        }
    }
}

void MainWindow::onopcWrite()
{
    if (!m_opcClient->isConnected()) {
        LogManager::instance()->logError("Not connected");
        return;
    }

    LogManager::instance()->logInfo(
        QString("Tags count: %1").arg(m_opcClient->tagCount()));

    if (m_opcClient->tagCount() > 0) {
        // 先预解析 NodeId（只做一次，后续读写会缓存）
        LogManager::instance()->logInfo("Resolving all tags...");
        m_opcClient->resolveAllTags();

        // 读取第1个标记，从0开始
        LogManager::instance()->logInfo(
            QString("Reading tag[1] '%1'...").arg(m_opcClient->tagName(0)));
        QVariant val = m_opcClient->readValue(1);
        LogManager::instance()->logInfo(
            QString("Tag[1] = %1").arg(val.toString()));

        // 写入第1个标记为3
        LogManager::instance()->logInfo("Writing tag[1] = 3...");
        m_opcClient->writeValue(1, 3);

        // 读回验证
        val = m_opcClient->readValue(1);
        LogManager::instance()->logInfo(
            QString("Verify tag[1] = %1").arg(val.toString()));
    }
}

// ==================== 保存当前帧 ====================
void MainWindow::onSaveFrameClicked()
{
    if (!m_pixmapItem || m_pixmapItem->pixmap().isNull()) {
        LogManager::instance()->logError(tr("No frame to save"));
        return;
    }

    // 创建 CurImage 目录
    QString dirPath = QCoreApplication::applicationDirPath() + "/CurImage";
    QDir dir(dirPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    // 时间戳命名
    QString fileName = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss_zzz") + ".png";
    QString filePath = dirPath + "/" + fileName;

    if (m_pixmapItem->pixmap().save(filePath, "PNG")) {
        LogManager::instance()->logInfo(tr("Frame saved: %1").arg(filePath));
    } else {
        LogManager::instance()->logError(tr("Failed to save frame: %1").arg(filePath));
    }
}

// ==================== 静态界面按钮事件 ====================

void MainWindow::onStaticOpen()
{
    onActionOpen();
}

void MainWindow::onStaticSave()
{
    onActionSave();
}

void MainWindow::onStaticDrawRect()
{
    if (!m_pixmapItem) return;

    bool wasDrawingRect = m_staticOps->isDrawingRect();
    m_staticOps->disableDrawMode();
    resetDrawButtonTexts();

    if (!wasDrawingRect) {
        m_staticOps->enableDrawRect();
        m_btnDrawRect->setText(tr("取消画矩形"));
        LogManager::instance()->logInfo(tr("Draw Rectangle mode: click and drag on image"));
    } else {
        LogManager::instance()->logInfo(tr("Draw Rectangle mode cancelled"));
    }
}

void MainWindow::onStaticDrawLine()
{
    if (!m_pixmapItem) return;

    bool wasDrawingLine = m_staticOps->isDrawingLine();
    m_staticOps->disableDrawMode();
    resetDrawButtonTexts();

    if (!wasDrawingLine) {
        m_staticOps->enableDrawLine();
        m_btnDrawLine->setText(tr("取消画线"));
        LogManager::instance()->logInfo(tr("Draw Line mode: click and drag on image"));
    } else {
        LogManager::instance()->logInfo(tr("Draw Line mode cancelled"));
    }
}

// ==================== 滤镜操作（互斥切换） ====================
void MainWindow::onStaticSharpen()
{
    if (!m_pixmapItem) return;
    applyFilter(FilterSharpen,
        [this](const QImage &s) { return m_staticOps->sharpen(s); },
        tr("Sharpen"));
}

void MainWindow::onStaticMeanFilter()
{
    if (!m_pixmapItem) return;
    applyFilter(FilterMean,
        [this](const QImage &s) { return m_staticOps->meanFilter(s); },
        tr("Mean filter"));
}

void MainWindow::onStaticNegative()
{
    if (!m_pixmapItem) return;
    applyFilter(FilterNegative,
        [this](const QImage &s) { return m_staticOps->negative(s); },
        tr("Negative"));
}

void MainWindow::onStaticEmboss()
{
    if (!m_pixmapItem) return;
    applyFilter(FilterEmboss,
        [this](const QImage &s) { return m_staticOps->emboss(s); },
        tr("Emboss"));
}

void MainWindow::applyFilter(ActiveFilter filter,
                              std::function<QImage(const QImage&)> func,
                              const QString &logName)
{
    if (m_activeFilter == filter) {
        // 取消当前滤镜：恢复执行前的图像
        if (!m_preFilterPixmap.isNull()) {
            m_pixmapItem->setPixmap(m_preFilterPixmap);
            m_scene->setSceneRect(m_preFilterPixmap.rect());
        }
        m_activeFilter = FilterNone;
        resetFilterButtonTexts();
        LogManager::instance()->logInfo(logName + tr(" cancelled"));
        return;
    }

    // 如果已有其他滤镜生效，先恢复
    if (m_activeFilter != FilterNone) {
        if (!m_preFilterPixmap.isNull()) {
            m_pixmapItem->setPixmap(m_preFilterPixmap);
            m_scene->setSceneRect(m_preFilterPixmap.rect());
        }
    }

    saveUndoState();
    m_preFilterPixmap = m_pixmapItem->pixmap(); // 保存当前作为恢复基线
    QImage dst = func(m_pixmapItem->pixmap().toImage());
    m_pixmapItem->setPixmap(QPixmap::fromImage(dst));
    m_scene->setSceneRect(dst.rect());
    m_activeFilter = filter;
    resetFilterButtonTexts();

    // 设置对应按钮为取消状态
    switch (filter) {
    case FilterSharpen:  m_btnSharpen->setText(tr("取消锐化"));   break;
    case FilterMean:     m_btnMeanFilter->setText(tr("取消滤波")); break;
    case FilterNegative: m_btnNegative->setText(tr("取消负片"));   break;
    case FilterEmboss:   m_btnEmboss->setText(tr("取消浮雕"));     break;
    default: break;
    }
    LogManager::instance()->logInfo(logName + tr(" applied"));
}

void MainWindow::resetFilterButtonTexts()
{
    m_btnSharpen->setText(tr("锐化"));
    m_btnMeanFilter->setText(tr("均值滤波"));
    m_btnNegative->setText(tr("负片"));
    m_btnEmboss->setText(tr("浮雕"));
}

void MainWindow::onStaticHistogram()
{
    if (!m_pixmapItem) return;
    QImage src = m_pixmapItem->pixmap().toImage();
    QPixmap hist = m_staticOps->drawHistogram(src);

    // 在独立窗口中显示直方图
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle(tr("Histogram"));
    dlg->setFixedSize(hist.width(), hist.height());
    QVBoxLayout *lay = new QVBoxLayout(dlg);
    QLabel *label = new QLabel(dlg);
    label->setPixmap(hist);
    lay->addWidget(label);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->show();
}

// ==================== 尺寸标定 ====================
void MainWindow::onStaticCalibrate()
{
    if (!m_pixmapItem) return;

    bool wasActive = m_staticOps->isCalibrating();
    m_staticOps->disableDrawMode();
    resetDrawButtonTexts();

    if (!wasActive) {
        m_staticOps->enableCalibrate();
        m_btnCalibrate->setText(tr("取消标定"));
        LogManager::instance()->logInfo(tr("Calibrate mode: draw a line, then enter real distance"));
    } else {
        // 取消标定模式时，清除所有标定线
        m_staticOps->clearItemsByTag(int(StaticOperations::Calibrate));
        LogManager::instance()->logInfo(tr("Calibrate mode cancelled"));
    }
}

void MainWindow::onCalibrationRequested(double pixelLength)
{
    bool ok = false;
    double realDist = QInputDialog::getDouble(
        this, tr("尺寸标定"),
        tr("线段像素长度: %1 px\n请输入对应的真实距离:").arg(pixelLength, 0, 'f', 1),
        10.0, 0.001, 100000.0, 2, &ok);
    if (ok && realDist > 0) {
        double ratio = realDist / pixelLength;
        m_staticOps->setCalibrationRatio(ratio);
        LogManager::instance()->logInfo(
            tr("Calibration: %1 px = %2 mm, ratio = %3 mm/px")
                .arg(pixelLength, 0, 'f', 1)
                .arg(realDist, 0, 'f', 2)
                .arg(ratio, 0, 'f', 4));
    }
}

// ==================== 尺寸测量 ====================
void MainWindow::onStaticMeasure()
{
    if (!m_pixmapItem) return;

    bool wasActive = m_staticOps->isMeasuring();
    m_staticOps->disableDrawMode();
    resetDrawButtonTexts();

    if (!wasActive) {
        m_staticOps->enableMeasure();
        m_btnMeasure->setText(tr("取消测量"));
        LogManager::instance()->logInfo(tr("Measure mode: draw a line to measure distance"));
    } else {
        LogManager::instance()->logInfo(tr("Measure mode cancelled"));
    }
}

void MainWindow::onMeasurementCompleted(double pixelLength, double realDistance)
{
    LogManager::instance()->logInfo(
        tr("Measurement: %1 px, %.2f mm (ratio=%.4f)")
            .arg(pixelLength, 0, 'f', 1)
            .arg(realDistance, 0, 'f', 2)
            .arg(m_staticOps->calibrationRatio(), 0, 'f', 4));
    QMessageBox::information(this, tr("尺寸测量"),
        tr("像素长度: %1 px\n真实距离: %2 mm\n标定系数: %3 mm/px")
            .arg(pixelLength, 0, 'f', 1)
            .arg(realDistance, 0, 'f', 2)
            .arg(m_staticOps->calibrationRatio(), 0, 'f', 4));
}

// ==================== 自动调窗 ====================
void MainWindow::onStaticAutoWindow()
{
    if (!m_pixmapItem) return;

    bool wasActive = m_staticOps->isAutoWindow();
    m_staticOps->disableDrawMode();
    resetDrawButtonTexts();

    if (!wasActive) {
        m_staticOps->enableAutoWindow();
        m_btnAutoWindow->setText(tr("取消自动调窗"));
        // 保存基线图像，后续所有调窗均基于此
        m_autoWindowBaseline = m_pixmapItem->pixmap();
        LogManager::instance()->logInfo(tr("Auto Window: draw a rectangle to set window level"));
    } else {
        // 取消自动调窗模式时，清除所有自动调窗矩形
        m_staticOps->clearItemsByTag(int(StaticOperations::AutoWindow));
        m_autoWindowBaseline = QPixmap(); // 清除基线
        LogManager::instance()->logInfo(tr("Auto Window mode cancelled"));
    }
}

void MainWindow::onAutoWindowRequested(const QRectF &roi)
{
    if (!m_pixmapItem) return;

    // 始终基于进入自动调窗模式时的基线图像计算，避免累积效应
    if (m_autoWindowBaseline.isNull()) return;
    saveUndoState();

    QImage src = m_autoWindowBaseline.toImage();
    int minVal, maxVal;
    m_staticOps->computeRoiStats(src, roi, minVal, maxVal);

    if (maxVal <= minVal) {
        LogManager::instance()->logError(tr("Auto Window: invalid ROI (min==max)"));
        return;
    }

    // 对比度拉伸
    QImage dst(src.size(), src.format());
    double scale = 255.0 / (maxVal - minVal);
    for (int y = 0; y < src.height(); ++y) {
        for (int x = 0; x < src.width(); ++x) {
            int v = qGray(src.pixel(x, y));
            int newV = qBound(0, (int)((v - minVal) * scale), 255);
            dst.setPixel(x, y, qRgb(newV, newV, newV));
        }
    }

    m_pixmapItem->setPixmap(QPixmap::fromImage(dst));
    m_scene->setSceneRect(dst.rect());
    LogManager::instance()->logInfo(
        tr("Auto Window: min=%1 max=%2 applied to whole image").arg(minVal).arg(maxVal));
}

// ==================== 伪彩色 ====================
void MainWindow::onStaticPseudoColor()
{
    if (!m_pixmapItem) return;

    if (m_isPseudoColor) {
        // 恢复原图
        if (!m_prePseudoPixmap.isNull()) {
            m_pixmapItem->setPixmap(m_prePseudoPixmap);
            m_scene->setSceneRect(m_prePseudoPixmap.rect());
        }
        m_isPseudoColor = false;
        m_btnPseudoColor->setText(tr("伪彩色"));
        LogManager::instance()->logInfo(tr("Pseudo-color removed"));
    } else {
        saveUndoState();
        m_prePseudoPixmap = m_pixmapItem->pixmap(); // 保存原始用于恢复
        QImage dst = m_staticOps->applyPseudoColor(m_pixmapItem->pixmap().toImage());
        m_pixmapItem->setPixmap(QPixmap::fromImage(dst));
        m_scene->setSceneRect(dst.rect());
        m_isPseudoColor = true;
        m_btnPseudoColor->setText(tr("取消伪彩色"));
        LogManager::instance()->logInfo(tr("Pseudo-color applied"));
    }
}

// ==================== 辅助函数 ====================
void MainWindow::resetDrawButtonTexts()
{
    m_btnDrawRect->setText(tr("画矩形"));
    m_btnDrawLine->setText(tr("画线"));
    m_btnCalibrate->setText(tr("尺寸标定"));
    m_btnMeasure->setText(tr("尺寸测量"));
    m_btnAutoWindow->setText(tr("自动调窗"));
}

void MainWindow::onFrameReady()
{
    if (!m_acqWorker) return;
    render16BitFrame();
}

void MainWindow::render16BitFrame()
{
    if (!m_acqWorker) return;

    const unsigned short *data = nullptr;
    int w = 0, h = 0;
    m_acqWorker->lockFrame(data, w, h);//获取m_frameData成员变量的数据，在这里传递给data，用于后续显示

    if (!data || w == 0 || h == 0)
    {
        m_acqWorker->unlockFrame();
        return;
    }

    double halfW = m_winWidth / 2.0;
    double low   = m_winLevel - halfW;
    double high  = m_winLevel + halfW;
    double contrastFactor = 1.0 + m_contrast / 255.0;

    QImage frame(w, h, QImage::Format_Grayscale8);
    //窗位窗宽 + 亮度对比度处理
    for (int y = 0; y < h; ++y)
    {
        unsigned char *line = frame.scanLine(y);
        for (int x = 0; x < w; ++x)
        {
            unsigned short val = data[y * w + x];

            int gray;
            if (val <= low) gray = 0;
            else if (val >= high) gray = 255;
            else gray = static_cast<int>((val - low) / m_winWidth * 255.0);

            gray = gray + m_brightness;
            gray = static_cast<int>((gray - 128) * contrastFactor + 128);
            gray = (std::max)(0, (std::min)(255, gray));

            line[x] = static_cast<unsigned char>(gray);
        }
    }
    m_acqWorker->unlockFrame();

    QPixmap pix = QPixmap::fromImage(frame);
    if (!m_pixmapItem)
    {
        m_scene->clear();
        m_pixmapItem = m_scene->addPixmap(pix);
    }
    else
    {
        m_pixmapItem->setPixmap(pix);
    }
    m_scene->setSceneRect(frame.rect());
}

void MainWindow::zoomInOut(double factor)
{
    m_zoomFactor *= factor;
    if (m_zoomFactor < 0.05)  m_zoomFactor = 0.05;
    if (m_zoomFactor > 20.0)  m_zoomFactor = 20.0;
    m_view->resetTransform();
    m_view->scale(m_zoomFactor, m_zoomFactor);
    updateZoomLabel();
}

void MainWindow::zoomIn()
{
    zoomInOut(1.25);
}

void MainWindow::zoomOut()
{
    zoomInOut(1.0 / 1.25);
}

void MainWindow::fitToWindow()
{
    if (!m_pixmapItem || !m_view) return;
    m_view->fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);
    m_zoomFactor = m_view->transform().m11();
    updateZoomLabel();
}

void MainWindow::updateZoomLabel()
{
    int percent = qRound(m_zoomFactor * 100);
    m_zoomLabel->setText(QString("%1%").arg(percent));
}

void MainWindow::restorePixmap(const QPixmap &pixmap)
{
    m_scene->clear();
    m_pixmapItem = m_scene->addPixmap(pixmap);
    m_pixmapItem->setRotation(0);
    m_scene->setSceneRect(pixmap.rect());
    fitToWindow();

    // 更新 manager 的引用
    m_editOps->setTarget(m_scene, &m_pixmapItem, m_view);
    m_adjustOps->setTarget(m_scene, &m_pixmapItem);
}

void MainWindow::wheelEvent(QWheelEvent *event)
{
    if (m_pixmapItem && event->angleDelta().y() != 0)
    {
        if (event->angleDelta().y() > 0)
            zoomIn();
        else
            zoomOut();
        event->accept();
    }
    else
    {
        QMainWindow::wheelEvent(event);
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    if (m_pixmapItem)
        fitToWindow();
}

// ==================== 创建菜单 ====================
void MainWindow::createMenus()
{
    // ---------- 文件菜单 ----------
    m_menuFile   = menuBar()->addMenu(tr("File"));
    m_actionOpen    = m_menuFile->addAction(QIcon(":/resources/icons/open.png"),      tr("Open"));
    m_actionSave    = m_menuFile->addAction(QIcon(":/resources/icons/save.png"),      tr("Save"));

    // ---------- 编辑菜单 ----------
    m_menuEdit   = menuBar()->addMenu(tr("Edit"));
    m_actionUndo   = m_menuEdit->addAction(tr("Undo"));
    m_actionReset  = m_menuEdit->addAction(tr("Reset to Original"));
    m_menuEdit->addSeparator();
    m_actionCrop      = m_menuEdit->addAction(tr("Crop"));
    m_menuEdit->addSeparator();
    m_actionRotateCW  = m_menuEdit->addAction(QIcon(":/resources/icons/rotate_cw.png"),  tr("Rotate 90 CW"));
    m_actionRotateCCW = m_menuEdit->addAction(QIcon(":/resources/icons/rotate_ccw.png"), tr("Rotate 90 CCW"));

    m_menuAdjust = menuBar()->addMenu(tr("Adjust"));
    m_actionWindowLevel = m_menuAdjust->addAction(tr("Window/Level"));
    m_actionContrastBrightness = m_menuAdjust->addAction(tr("Contrast/Brightness"));
    // ---------- 信号连接 ----------
    // 文件
    connect(m_actionOpen,    &QAction::triggered, this, &MainWindow::onActionOpen);
    connect(m_actionSave,    &QAction::triggered, this, &MainWindow::onActionSave);
    // 编辑
    connect(m_actionUndo,      &QAction::triggered, this, &MainWindow::onActionUndo);
    connect(m_actionReset,     &QAction::triggered, this, &MainWindow::onActionReset);
    connect(m_actionCrop,      &QAction::triggered, this, &MainWindow::onActionCrop);
    connect(m_actionRotateCW,  &QAction::triggered, this, &MainWindow::onActionRotateCW);
    connect(m_actionRotateCCW, &QAction::triggered, this, &MainWindow::onActionRotateCCW);
    // 调整
    connect(m_actionWindowLevel,         &QAction::triggered, this, &MainWindow::onActionWindowLevel);
    connect(m_actionContrastBrightness, &QAction::triggered, this, &MainWindow::onActionContrastBrightness);
}

// ==================== 文件图像调整 ====================
void MainWindow::applyFileImageAdjustments()
{
    if (m_sourceImage.isNull() || !m_pixmapItem) return;

    // 将 8 位灰度源图像映射到 16 位空间，统一用 16 位窗位窗宽处理
    int w = m_sourceImage.width();
    int h = m_sourceImage.height();
    QImage result(w, h, QImage::Format_Grayscale8);

    double halfW  = m_winWidth / 2.0;
    double low    = m_winLevel - halfW;
    double high   = m_winLevel + halfW;
    double contrastFactor = 1.0 + m_contrast / 255.0;

    for (int y = 0; y < h; ++y) {
        const uchar *srcLine = m_sourceImage.constScanLine(y);
        uchar *dstLine = result.scanLine(y);
        for (int x = 0; x < w; ++x) {
            // 将 8 位值放大到 16 位空间 (0~255 → 0~65535)
            int val = srcLine[x] * 257;  // 255*257=65535

            // 窗位窗宽
            int gray;
            if (val <= low) gray = 0;
            else if (val >= high) gray = 255;
            else gray = (int)((val - low) / m_winWidth * 255.0);

            // 亮度对比度
            gray = gray + m_brightness;
            gray = (int)((gray - 128) * contrastFactor + 128);
            gray = qBound(0, gray, 255);

            dstLine[x] = (uchar)gray;
        }
    }

    m_pixmapItem->setPixmap(QPixmap::fromImage(result));
    m_scene->setSceneRect(result.rect());
}

// ==================== 调整菜单 ====================
void MainWindow::onActionWindowLevel()
{
    if (!m_pixmapItem) return;
    // 统一使用 16 位范围
    WindowLevelDialog dlg(m_winWidth, m_winLevel, 65535, 65535, this);
    connect(&dlg, &WindowLevelDialog::valuesChanged, this, [this](int w, int l) {
        m_winWidth = w;
        m_winLevel = l;
        if (m_isAcquisitionActive) render16BitFrame();
        else applyFileImageAdjustments();
    });
    dlg.exec();
    m_winWidth = dlg.windowWidth();
    m_winLevel = dlg.windowLevel();
    if (m_isAcquisitionActive) render16BitFrame();
    else applyFileImageAdjustments();
}

void MainWindow::onActionContrastBrightness()
{
    if (!m_pixmapItem) return;
    BrightnessContrastDialog dlg(m_brightness, m_contrast, this);
    connect(&dlg, &BrightnessContrastDialog::valuesChanged, this, [this](int b, int c) {
        m_brightness = b;
        m_contrast = c;
        if (m_isAcquisitionActive) render16BitFrame();
        else applyFileImageAdjustments();
    });
    dlg.exec();
    m_brightness = dlg.brightness();
    m_contrast = dlg.contrast();
    if (m_isAcquisitionActive) render16BitFrame();
    else applyFileImageAdjustments();
}

// ==================== 撤回 / 重置 ====================
void MainWindow::saveUndoState()
{
    if (!m_pixmapItem) return;
    QPixmap current = m_pixmapItem->pixmap();
    double angle = m_pixmapItem->rotation();
    if (!qFuzzyCompare(angle, 0.0))
    {
        QTransform t;
        t.rotate(angle);
        current = current.transformed(t, Qt::SmoothTransformation);
    }
    m_undoPixmap = current;
}

void MainWindow::onActionUndo()
{
    // 优先撤销矢量绘制图元（矩形、线条）
    if (m_staticOps->undoLastDrawnItem()) {
        LogManager::instance()->logInfo(tr("Undo: removed last drawn shape"));
        return;
    }
    // 否则恢复图像像素
    if (m_undoPixmap.isNull()) return;
    restorePixmap(m_undoPixmap);
    m_undoPixmap = QPixmap();
}

void MainWindow::onActionReset()
{
    if (m_originalPixmap.isNull()) return;
    restorePixmap(m_originalPixmap);
    m_undoPixmap = QPixmap();
}

// ==================== File menu slots ====================
void MainWindow::onActionOpen()
{
    QString filePath = QFileDialog::getOpenFileName(
        this, tr("Open Image"), QString(),
        tr("Images (*.png *.jpg *.jpeg *.bmp *.gif *.tiff *.tif);;All Files (*)"));
    if (filePath.isEmpty()) return;

    QPixmap pixmap(filePath);
    if (pixmap.isNull()) return;

    // 保存源图像（统一以 16 位方式处理）
    m_sourceImage = pixmap.toImage().convertToFormat(QImage::Format_Grayscale8);

    // 统一使用 16 位窗位窗宽默认值
    m_winWidth   = 65535;
    m_winLevel   = 32768;
    m_brightness = 0;
    m_contrast   = 0;

    // 显示原图
    m_scene->clear();
    m_pixmapItem = m_scene->addPixmap(pixmap);
    m_scene->setSceneRect(pixmap.rect());

    m_originalPixmap = pixmap;
    m_undoPixmap = QPixmap();
    fitToWindow();

    LogManager::instance()->logSuccess(
        QString("Image loaded: %1 (%2x%3)")
            .arg(QFileInfo(filePath).fileName())
            .arg(pixmap.width()).arg(pixmap.height()));

    // 更新 manager 的引用
    m_editOps->setTarget(m_scene, &m_pixmapItem, m_view);
    m_adjustOps->setTarget(m_scene, &m_pixmapItem);
    m_staticOps->setTarget(m_scene, &m_pixmapItem, m_view);
}

void MainWindow::onActionSave()
{
    if (!m_pixmapItem) return;

    QString filePath = QFileDialog::getSaveFileName(
        this, tr("Save Image"), QString(),
        tr("PNG (*.png);;JPEG (*.jpg *.jpeg);;BMP (*.bmp)"));

    if (filePath.isEmpty()) return;

    // 获取图片并应用旋转变换
    QPixmap src = m_pixmapItem->pixmap();
    double angle = m_pixmapItem->rotation();
    if (!qFuzzyCompare(angle, 0.0))
    {
        QTransform transform;
        transform.rotate(angle);
        src = src.transformed(transform, Qt::SmoothTransformation);
    }
    src.save(filePath);
    LogManager::instance()->logSuccess(QString("Image saved: %1").arg(QFileInfo(filePath).fileName()));
}

// ==================== Edit menu slots ====================
void MainWindow::onActionCrop()
{
    if (!m_pixmapItem) return;
    saveUndoState();
    m_editOps->crop();
    fitToWindow();
}

void MainWindow::onActionRotateCW()
{
    if (!m_pixmapItem) return;
    saveUndoState();
    m_editOps->rotateCW();
    fitToWindow();
}

void MainWindow::onActionRotateCCW()
{
    if (!m_pixmapItem) return;
    saveUndoState();
    m_editOps->rotateCCW();
    fitToWindow();
}

// ==================== 配置持久化 ====================
void MainWindow::closeEvent(QCloseEvent *event)
{
    saveSettings();
    QMainWindow::closeEvent(event);
}

void MainWindow::saveSettings()
{
    QString path = QCoreApplication::applicationDirPath() + "/config.ini";
    QSettings settings(path, QSettings::IniFormat);

    // 射线控制参数
    if (m_xrayTab) m_xrayTab->saveSettings(settings);

    // 尺寸标定系数
    settings.beginGroup("Calibration");
    settings.setValue("ratio", m_staticOps->calibrationRatio());
    settings.endGroup();

    // 采集参数
    settings.beginGroup("Acquisition");
    settings.setValue("winWidth",    m_winWidth);
    settings.setValue("winLevel",    m_winLevel);
    settings.setValue("brightness",  m_brightness);
    settings.setValue("contrast",    m_contrast);
    settings.endGroup();

    LogManager::instance()->logInfo(tr("Settings saved to %1").arg(path));
}

void MainWindow::loadSettings()
{
    QString path = QCoreApplication::applicationDirPath() + "/config.ini";
    if (!QFile::exists(path)) return;

    QSettings settings(path, QSettings::IniFormat);

    // 射线控制参数
    if (m_xrayTab) m_xrayTab->loadSettings(settings);

    // 尺寸标定系数
    settings.beginGroup("Calibration");
    m_staticOps->setCalibrationRatio(
        settings.value("ratio", 1.0).toDouble());
    settings.endGroup();

    // 采集参数
    settings.beginGroup("Acquisition");
    m_winWidth   = settings.value("winWidth",   4096).toInt();
    m_winLevel   = settings.value("winLevel",   32768).toInt();
    m_brightness = settings.value("brightness", 0).toInt();
    m_contrast   = settings.value("contrast",   0).toInt();
    settings.endGroup();

    LogManager::instance()->logInfo(tr("Settings loaded from %1").arg(path));
}
