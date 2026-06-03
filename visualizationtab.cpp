#include "visualizationtab.h"
#include "logmanager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QGroupBox>

// VTK 模块初始化（必须在所有 VTK include 之前）
#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2);
VTK_MODULE_INIT(vtkRenderingFreeType);
VTK_MODULE_INIT(vtkInteractionStyle);

#include <type_traits>
#include <array>
#include <vtkVersion.h>
#include <vtkObjectFactory.h>

// 渲染
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCamera.h>

// 体绘制
#include <vtkSmartVolumeMapper.h>
#include <vtkLookupTable.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>

// 数据读取
#include <vtkImageData.h>
#include <vtkMetaImageReader.h>
#include <vtkGenericDataObjectReader.h>
#include <vtkImageFlip.h>
#include <vtkImageCast.h>
#include <vtkImageActor.h>
#include <vtkImageReslice.h>
#include <vtkImageProperty.h>
#include <vtkImageMapper3D.h>
#include <vtkProperty.h>

// 切割
#include <vtkBoxWidget.h>
#include <vtkPlanes.h>
#include <vtkCommand.h>

// 切片
#include <vtkImagePlaneWidget.h>
#include <vtkCellPicker.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkInteractorStyleImage.h>

// Qt 集成
#include <QVTKOpenGLWidget.h>

// ==================== BoxWidget 回调 ====================
class vtkBoxWidgetCallback : public vtkCommand
{
public:
    static vtkBoxWidgetCallback *New() { return new vtkBoxWidgetCallback; }

    void Execute(vtkObject *caller, unsigned long, void*) override
    {
        vtkBoxWidget *widget = reinterpret_cast<vtkBoxWidget*>(caller);
        if (this->Mapper) {
            vtkSmartPointer<vtkPlanes> planes = vtkSmartPointer<vtkPlanes>::New();
            widget->GetPlanes(planes);
            this->Mapper->SetClippingPlanes(planes);
        }
    }

    void SetMapper(vtkSmartVolumeMapper *m) { this->Mapper = m; }

    vtkSmartVolumeMapper *Mapper = nullptr;
};

// ==================== 3D 视图平面拖动回调 ====================
class vtkSlicePlaneCallback : public vtkCommand
{
public:
    static vtkSlicePlaneCallback *New() { return new vtkSlicePlaneCallback; }

    void Execute(vtkObject *caller, unsigned long, void*) override
    {
        vtkImagePlaneWidget *plane = reinterpret_cast<vtkImagePlaneWidget*>(caller);
        if (m_tab) {
            m_tab->onPlaneWidgetMoved(m_axis, plane->GetSliceIndex());
        }
    }

    VisualizationTab *m_tab = nullptr;
    int m_axis = 0; // 0=X(Sagittal), 1=Y(Coronal), 2=Z(Axial)
};

// ==================== VisualizationTab ====================
VisualizationTab::VisualizationTab(QWidget *parent)
    : QWidget(parent),
      m_btnLoadData(nullptr),
      m_btnVolumeRender(nullptr),
      m_btnClipVolume(nullptr),
      m_btnTogglePlanes(nullptr),
      m_view3D(nullptr),
      m_viewAxial(nullptr),
      m_viewSagittal(nullptr),
      m_viewCoronal(nullptr),
      m_sliderAxial(nullptr),
      m_sliderSagittal(nullptr),
      m_sliderCoronal(nullptr),
      m_labelAxial(nullptr),
      m_labelSagittal(nullptr),
      m_labelCoronal(nullptr),
      m_clipEnabled(false),
      m_planesVisible(false),
      m_dims{0, 0, 0},
      m_dataLoaded(false)
{
    m_dims[0] = m_dims[1] = m_dims[2] = 0;
    setupUI();
}

VisualizationTab::~VisualizationTab()
{
    if (m_boxWidget) m_boxWidget->EnabledOff();
}

void VisualizationTab::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // ---- 按钮栏 ----
    QHBoxLayout *btnLayout = new QHBoxLayout();
    m_btnLoadData     = new QPushButton(tr("读取数据"), this);
    m_btnVolumeRender = new QPushButton(tr("体绘制"), this);
    m_btnClipVolume   = new QPushButton(tr("实体切割"), this);
    m_btnTogglePlanes = new QPushButton(tr("显示切片平面"), this);

    m_btnLoadData->setMinimumHeight(32);
    m_btnVolumeRender->setMinimumHeight(32);
    m_btnClipVolume->setMinimumHeight(32);
    m_btnVolumeRender->setEnabled(false);
    m_btnClipVolume->setEnabled(false);
    m_btnTogglePlanes->setMinimumHeight(32);
    m_btnTogglePlanes->setEnabled(false);

    btnLayout->addWidget(m_btnLoadData);
    btnLayout->addWidget(m_btnVolumeRender);
    btnLayout->addWidget(m_btnClipVolume);
    btnLayout->addWidget(m_btnTogglePlanes);
    btnLayout->addStretch();
    mainLayout->addLayout(btnLayout);


    // ---- 切片滑条 ----
    setupSliderUI(mainLayout);

    // ---- 信号连接 ----
    connect(m_btnLoadData,      &QPushButton::clicked, this, &VisualizationTab::onLoadData);
    connect(m_btnVolumeRender,  &QPushButton::clicked, this, &VisualizationTab::onVolumeRender);
    connect(m_btnClipVolume,    &QPushButton::clicked, this, &VisualizationTab::onClipVolume);
    connect(m_btnTogglePlanes,  &QPushButton::clicked, this, &VisualizationTab::onTogglePlanes);
    connect(m_sliderAxial,     &QSlider::valueChanged, this, &VisualizationTab::onSliderAxialChanged);
    connect(m_sliderSagittal,  &QSlider::valueChanged, this, &VisualizationTab::onSliderSagittalChanged);
    connect(m_sliderCoronal,   &QSlider::valueChanged, this, &VisualizationTab::onSliderCoronalChanged);
}

void VisualizationTab::setupSliderUI(QVBoxLayout *layout)
{
    QGroupBox *group = new QGroupBox(tr("切片位置"), this);
    QVBoxLayout *gLayout = new QVBoxLayout(group);
    group->setMaximumHeight(450);
    auto addSliderRow = [&](const QString &title, QSlider *&slider, QLabel *&label) {
        QHBoxLayout *row = new QHBoxLayout();
        QLabel *titleLabel = new QLabel(title, this);
        titleLabel->setMinimumWidth(60);
        slider = new QSlider(Qt::Horizontal, this);
        slider->setRange(0, 0);
        slider->setEnabled(false);
        label = new QLabel(QString("0 / 0"), this);
        label->setMinimumWidth(60);
        label->setAlignment(Qt::AlignCenter);
        row->addWidget(titleLabel);
        row->addWidget(slider, 1);
        row->addWidget(label);
        gLayout->addLayout(row);
    };

    addSliderRow(tr("轴位 Z"), m_sliderAxial, m_labelAxial);
    addSliderRow(tr("矢状 X"), m_sliderSagittal, m_labelSagittal);
    addSliderRow(tr("冠位 Y"), m_sliderCoronal, m_labelCoronal);

    layout->addWidget(group);
}

void VisualizationTab::setViewWidgets(QVTKOpenGLWidget *v3d,
                                      QVTKOpenGLWidget *axial,
                                      QVTKOpenGLWidget *sagittal,
                                      QVTKOpenGLWidget *coronal)
{
    m_view3D      = v3d;
    m_viewAxial   = axial;
    m_viewSagittal = sagittal;
    m_viewCoronal = coronal;

    // 现在可以初始化 VTK 管线
    setup3DView();
    setupSliceViews();
}

void VisualizationTab::setup3DView()
{
    m_renderWindow3D = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    m_renderer3D = vtkSmartPointer<vtkRenderer>::New();
    m_renderer3D->SetBackground(0.0, 0.0, 0.0);

    m_view3D->SetRenderWindow(m_renderWindow3D);
    m_renderWindow3D->AddRenderer(m_renderer3D);

    m_interactor3D = m_view3D->GetInteractor();
    // 设置交互样式以支持 3D 控件交互
    vtkSmartPointer<vtkInteractorStyleTrackballCamera> style =
        vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
    m_interactor3D->SetInteractorStyle(style);
}

void VisualizationTab::setupSliceViews()
{
    // 2D 视图使用 Image 交互样式：左键调窗宽窗位，中键平移，右键缩放（无旋转）
    auto setupSliceInteractor = [](vtkRenderWindowInteractor *iren) {
        vtkSmartPointer<vtkInteractorStyleImage> style =
            vtkSmartPointer<vtkInteractorStyleImage>::New();
        iren->SetInteractorStyle(style);
    };

    // 横断面 (Axial)
    m_rendererAxial = vtkSmartPointer<vtkRenderer>::New();
    m_rendererAxial->SetBackground(0.1, 0.1, 0.1);
    m_renderWindowAxial = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    m_renderWindowAxial->AddRenderer(m_rendererAxial);
    m_viewAxial->SetRenderWindow(m_renderWindowAxial);
    m_interactorAxial = m_viewAxial->GetInteractor();
    setupSliceInteractor(m_interactorAxial);

    // 矢状面 (Sagittal)
    m_rendererSagittal = vtkSmartPointer<vtkRenderer>::New();
    m_rendererSagittal->SetBackground(0.1, 0.1, 0.1);
    m_renderWindowSagittal = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    m_renderWindowSagittal->AddRenderer(m_rendererSagittal);
    m_viewSagittal->SetRenderWindow(m_renderWindowSagittal);
    m_interactorSagittal = m_viewSagittal->GetInteractor();
    setupSliceInteractor(m_interactorSagittal);

    // 冠状面 (Coronal)
    m_rendererCoronal = vtkSmartPointer<vtkRenderer>::New();
    m_rendererCoronal->SetBackground(0.1, 0.1, 0.1);
    m_renderWindowCoronal = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    m_renderWindowCoronal->AddRenderer(m_rendererCoronal);
    m_viewCoronal->SetRenderWindow(m_renderWindowCoronal);
    m_interactorCoronal = m_viewCoronal->GetInteractor();
    setupSliceInteractor(m_interactorCoronal);

    m_picker = vtkSmartPointer<vtkCellPicker>::New();
    m_picker->SetTolerance(0.005);
}

// ==================== 数据读取 ====================
void VisualizationTab::onLoadData()
{
    QString filePath = QFileDialog::getOpenFileName(
        this, tr("Open 3D Data"), QString(),
        tr("Volume Data (*.mhd *.mha *.vtk *.raw);;MHD Files (*.mhd *.mha);;VTK Files (*.vtk);;RAW Files (*.raw);;All Files (*)"));

    if (filePath.isEmpty()) return;

    QFileInfo fi(filePath);
    QString suffix = fi.suffix().toLower();

    // 清除旧数据
    if (m_volume)      { m_renderer3D->RemoveVolume(m_volume); m_volume = nullptr; }
    // 清除 3D 平面控件
    if (m_plane3D_Axial)    { m_plane3D_Axial->Off();    m_plane3D_Axial = nullptr; }
    if (m_plane3D_Sagittal) { m_plane3D_Sagittal->Off(); m_plane3D_Sagittal = nullptr; }
    if (m_plane3D_Coronal)  { m_plane3D_Coronal->Off();  m_plane3D_Coronal = nullptr; }
    if (m_actorAxial)    { m_rendererAxial->RemoveActor(m_actorAxial);    m_actorAxial = nullptr; }
    if (m_actorSagittal) { m_rendererSagittal->RemoveActor(m_actorSagittal); m_actorSagittal = nullptr; }
    if (m_actorCoronal)  { m_rendererCoronal->RemoveActor(m_actorCoronal);  m_actorCoronal = nullptr; }
    m_imageData = nullptr;
    m_flipFilter = nullptr;

    if (suffix == "mhd" || suffix == "mha")
        m_imageData = readMHD(filePath);
    else if (suffix == "vtk")
        m_imageData = readVTK(filePath);
    else if (suffix == "raw")
        m_imageData = readRAW(filePath, 512, 512, 100); // 默认尺寸，用户可按需调整
    else {
        QMessageBox::warning(this, tr("Error"), tr("Unsupported file format: %1").arg(suffix));
        return;
    }

    if (!m_imageData) {
        LogManager::instance()->logError(tr("Failed to load: %1").arg(filePath));
        return;
    }

    m_imageData->GetDimensions(m_dims);
    m_dataLoaded = true;
    LogManager::instance()->logInfo(
        tr("Loaded 3D data: %1 x %2 x %3").arg(m_dims[0]).arg(m_dims[1]).arg(m_dims[2]));

    m_btnVolumeRender->setEnabled(true);
    m_btnClipVolume->setEnabled(false);
}

vtkSmartPointer<vtkImageData> VisualizationTab::readMHD(const QString &filePath)
{
    vtkSmartPointer<vtkMetaImageReader> reader = vtkSmartPointer<vtkMetaImageReader>::New();
    reader->SetFileName(filePath.toLocal8Bit().constData());
    reader->Update();
    vtkSmartPointer<vtkImageData> img = reader->GetOutput();
    if (img) img->Register(nullptr);
    return img;
}

vtkSmartPointer<vtkImageData> VisualizationTab::readVTK(const QString &filePath)
{
    vtkNew<vtkGenericDataObjectReader> reader;
    reader->SetFileName(filePath.toLocal8Bit().constData());
    reader->Update();
    return vtkImageData::SafeDownCast(reader->GetOutput());
}

vtkSmartPointer<vtkImageData> VisualizationTab::readRAW(const QString &filePath,
                                                         int w, int h, int d)
{
    Q_UNUSED(w); Q_UNUSED(h); Q_UNUSED(d);
    // RAW 读取留空，用户可在此补充
    QMessageBox::information(this, tr("RAW Support"),
        tr("RAW file support not yet implemented.\nPlease convert to .mhd or .vtk format."));
    return nullptr;
}

// ==================== 体绘制 ====================
void VisualizationTab::onVolumeRender()
{
    if (!m_imageData) return;

    // 清除旧体
    if (m_volume) {
        m_renderer3D->RemoveVolume(m_volume);
        m_volume = nullptr;
    }

    // 翻转 Z 轴（CT 数据通常需要）
    m_flipFilter = vtkSmartPointer<vtkImageFlip>::New();
    m_flipFilter->SetFilteredAxis(2);
    m_flipFilter->SetInputData(m_imageData);
    m_flipFilter->Update();

    // Smart Volume Mapper（自动选择 GPU/CPU）
    vtkSmartPointer<vtkSmartVolumeMapper> gpuMapper =
        vtkSmartPointer<vtkSmartVolumeMapper>::New();
    gpuMapper->SetInputConnection(m_flipFilter->GetOutputPort());
    gpuMapper->SetInteractiveAdjustSampleDistances(0);
    m_volumeMapper = gpuMapper;

    // 传递函数 - 颜色
    m_colorFunc = vtkSmartPointer<vtkColorTransferFunction>::New();
    m_colorFunc->AddRGBPoint(0,    0.00, 0.00, 0.00);
    m_colorFunc->AddRGBPoint(64,   1.00, 0.52, 0.30);
    m_colorFunc->AddRGBPoint(190,  1.00, 1.00, 1.00);
    m_colorFunc->AddRGBPoint(220,  0.20, 0.20, 0.20);

    // 传递函数 - 不透明度
    m_opacityFunc = vtkSmartPointer<vtkPiecewiseFunction>::New();
    m_opacityFunc->AddPoint(40,  0.00);
    m_opacityFunc->AddPoint(50,  0.50);
    m_opacityFunc->AddPoint(60,  0.70);

    // 传递函数 - 梯度不透明度
    m_gradientOpacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
    m_gradientOpacity->AddPoint(1,   0.0);
    m_gradientOpacity->AddPoint(100, 0.5);
    m_gradientOpacity->AddPoint(200, 1.0);

    m_volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
    m_volumeProperty->SetColor(m_colorFunc);
    m_volumeProperty->SetScalarOpacity(m_opacityFunc);
    m_volumeProperty->SetGradientOpacity(m_gradientOpacity);
    m_volumeProperty->SetInterpolationTypeToLinear();
    m_volumeProperty->ShadeOn();
    m_volumeProperty->SetAmbient(0.8);
    m_volumeProperty->SetDiffuse(1.0);
    m_volumeProperty->SetSpecular(0.2);

    m_volume = vtkSmartPointer<vtkVolume>::New();
    m_volume->SetMapper(m_volumeMapper);
    m_volume->SetProperty(m_volumeProperty);

    m_renderer3D->AddVolume(m_volume);
    m_renderer3D->ResetCamera();
    m_renderWindow3D->Render();

    // ---- 正交切片（ImagePlaneWidget） ----
    int dims[3];
    m_flipFilter->GetOutput()->GetDimensions(dims);

    // ---- 在 3D 视图中创建可拖动的切片平面 ----
    auto create3DPlane = [&](vtkSmartPointer<vtkImagePlaneWidget> &plane,
                             int orientation, int sliceIdx,
                             int axis, const char *key)
    {
        plane = vtkSmartPointer<vtkImagePlaneWidget>::New();
        plane->SetInputConnection(m_flipFilter->GetOutputPort());
        plane->SetPlaneOrientation(orientation);
        plane->SetSliceIndex(sliceIdx);
        plane->SetPicker(m_picker);
        plane->SetInteractor(m_interactor3D);
        plane->SetDefaultRenderer(m_renderer3D);
        plane->SetKeyPressActivationValue(0);  // 禁用按键激活，直接鼠标拖动
        plane->SetLeftButtonAction(1);   // VTK_SLICE_MOTION_ACTION：拖动切片
        plane->SetMiddleButtonAction(2); // VTK_WINDOW_LEVEL_ACTION：调窗
        plane->SetRightButtonAction(0);  // VTK_CURSOR_ACTION：禁用旋转
        plane->GetTexturePlaneProperty()->SetOpacity(m_planesVisible ? 0.3 : 0.0);
        plane->GetPlaneProperty()->SetOpacity(m_planesVisible ? 0.6 : 0.0);
        vtkProperty* marginProp = plane->GetMarginProperty();
        marginProp->SetOpacity(0);
        plane->SetMarginSizeX(0);
        plane->SetMarginSizeY(0);

        {   // 灰度查找表（用于切片显示）
            vtkSmartPointer<vtkLookupTable> grayLut =
                vtkSmartPointer<vtkLookupTable>::New();
            grayLut->SetNumberOfTableValues(256);
            grayLut->SetHueRange(0, 0);
            grayLut->SetSaturationRange(0, 0);
            grayLut->SetValueRange(0, 1);
            grayLut->SetRange(0, 255);
            grayLut->Build();
            plane->SetLookupTable(grayLut);
        }
        plane->On();

        // 拖动回调
        vtkSmartPointer<vtkSlicePlaneCallback> cb =
            vtkSmartPointer<vtkSlicePlaneCallback>::New();
        cb->m_tab = this;
        cb->m_axis = axis;
        plane->AddObserver(vtkCommand::InteractionEvent, cb);
    };

    create3DPlane(m_plane3D_Axial,    2, dims[2] / 2, 2, "z");  // Z → Axial
    create3DPlane(m_plane3D_Sagittal, 0, dims[0] / 2, 0, "x");  // X → Sagittal
    create3DPlane(m_plane3D_Coronal,  1, dims[1] / 2, 1, "y");  // Y → Coronal

    // ---- 2D 切片视图：独立 vtkImageReslice 管线 ----
    // 强制输出 Origin=(0,0,0), Direction=单位矩阵，使 vtkImageActor
    // 始终在 XY 平面显示图像，无需依赖方向矩阵做 3D 定位。
    auto create2DView = [&](vtkSmartPointer<vtkImageReslice> &reslice,
                            vtkSmartPointer<vtkImageActor> &actor,
                            vtkRenderer *ren,
                            double d00, double d01, double d02,
                            double d10, double d11, double d12,
                            double d20, double d21, double d22,
                            double ox, double oy, double oz,
                            int extW, int extH,
                            double ww, double wl) {
        reslice = vtkSmartPointer<vtkImageReslice>::New();
        reslice->SetInputConnection(m_flipFilter->GetOutputPort());
        reslice->SetOutputDimensionality(2);
        reslice->SetResliceAxesDirectionCosines(d00,d01,d02, d10,d11,d12, d20,d21,d22);
        reslice->SetResliceAxesOrigin(ox, oy, oz);
        reslice->SetOutputExtent(0, extW - 1, 0, extH - 1, 0, 0);
        reslice->SetInterpolationModeToLinear();

        actor = vtkSmartPointer<vtkImageActor>::New();
        actor->GetMapper()->SetInputConnection(reslice->GetOutputPort());
        actor->GetProperty()->SetColorWindow(ww);
        actor->GetProperty()->SetColorLevel(wl);
        actor->GetProperty()->SetInterpolationTypeToLinear();
        ren->AddActor(actor);
    };

    const double ww = 1500.0, wl = 500.0;
    // Axial:  提取 XY 平面 (Z=dims[2]/2), 输出图像 dims[0]×dims[1]
    create2DView(m_resliceAxial, m_actorAxial, m_rendererAxial,
                 1,0,0, 0,1,0, 0,0,1,  0, 0, dims[2]/2,  dims[0], dims[1], ww, wl);
    // Sagittal: 提取 YZ 平面 (X=dims[0]/2), 输出图像 dims[1]×dims[2]
    create2DView(m_resliceSagittal, m_actorSagittal, m_rendererSagittal,
                 0,1,0, 0,0,1, 1,0,0,  dims[0]/2, 0, 0,  dims[1], dims[2], ww, wl);
    // Coronal: 提取 XZ 平面 (Y=dims[1]/2), 输出图像 dims[0]×dims[2]
    create2DView(m_resliceCoronal, m_actorCoronal, m_rendererCoronal,
                 1,0,0, 0,0,1, 0,-1,0,  0, dims[1]/2, 0,  dims[0], dims[2], ww, wl);

    // 2D 相机：读取各 reslice 输出数据的实际边界，计算相机位置使图像居中
    auto setup2DCamera = [](vtkRenderer *ren, vtkImageReslice *reslice) {
        reslice->Update();
        double bounds[6];
        reslice->GetOutput()->GetBounds(bounds);
        // bounds = [xmin, xmax, ymin, ymax, zmin, zmax]
        double cx = (bounds[0] + bounds[1]) * 0.5;
        double cy = (bounds[2] + bounds[3]) * 0.5;
        double height = bounds[3] - bounds[2];  // Y 方向高度
        vtkCamera *cam = ren->GetActiveCamera();
        cam->ParallelProjectionOn();
        cam->SetParallelScale(height * 0.5);
        cam->SetViewUp(0, 1, 0);
        cam->SetPosition(cx, cy, -1000);
        cam->SetFocalPoint(cx, cy, 0);
        cam->SetClippingRange(1, 10000);
    };

    setup2DCamera(m_rendererAxial,    m_resliceAxial);
    setup2DCamera(m_rendererSagittal, m_resliceSagittal);
    setup2DCamera(m_rendererCoronal,  m_resliceCoronal);

    m_renderWindowAxial->Render();
    m_renderWindowSagittal->Render();
    m_renderWindowCoronal->Render();

    // 启用滑条并设置范围
    m_sliderAxial->setRange(0, dims[2] - 1);
    m_sliderSagittal->setRange(0, dims[0] - 1);
    m_sliderCoronal->setRange(0, dims[1] - 1);
    m_sliderAxial->setValue(dims[2] / 2);
    m_sliderSagittal->setValue(dims[0] / 2);
    m_sliderCoronal->setValue(dims[1] / 2);
    m_sliderAxial->setEnabled(true);
    m_sliderSagittal->setEnabled(true);
    m_sliderCoronal->setEnabled(true);

    m_btnClipVolume->setEnabled(true);
    m_btnTogglePlanes->setEnabled(true);
    LogManager::instance()->logInfo(tr("Volume rendering applied"));
}

// ==================== 实体切割 ====================
void VisualizationTab::onClipVolume()
{
    if (!m_volume || !m_volumeMapper) {
        LogManager::instance()->logError(tr("No volume to clip"));
        return;
    }

    if (!m_clipEnabled) {
        // 启用切割
        m_boxWidget = vtkSmartPointer<vtkBoxWidget>::New();
        m_boxWidget->SetInteractor(m_interactor3D);
        m_boxWidget->SetPlaceFactor(1.0);
        m_boxWidget->SetInputData(m_flipFilter->GetOutput());
        m_boxWidget->SetDefaultRenderer(m_renderer3D);
        m_boxWidget->PlaceWidget();
        m_boxWidget->InsideOutOn();
        m_boxWidget->RotationEnabledOff();
        m_boxWidget->TranslationEnabledOn();

        // 回调：更新裁剪平面
        vtkSmartPointer<vtkBoxWidgetCallback> callback =
            vtkSmartPointer<vtkBoxWidgetCallback>::New();
        callback->SetMapper(
            vtkSmartVolumeMapper::SafeDownCast(m_volumeMapper));
        m_boxWidget->AddObserver(vtkCommand::InteractionEvent, callback);

        m_boxWidget->EnabledOn();
        m_clipEnabled = true;
        m_btnClipVolume->setText(tr("取消切割"));
        LogManager::instance()->logInfo(tr("Clip box enabled"));
    } else {
        // 禁用切割
        m_boxWidget->RemoveAllObservers();
        m_boxWidget->EnabledOff();
        m_boxWidget = nullptr;
        m_clipEnabled = false;
        m_btnClipVolume->setText(tr("实体切割"));

        // 清除裁剪平面
        vtkSmartVolumeMapper::SafeDownCast(m_volumeMapper)
            ->RemoveAllClippingPlanes();

        m_renderWindow3D->Render();
        LogManager::instance()->logInfo(tr("Clip box disabled"));
    }
}

// ==================== 3D 切片平面开关 ====================
void VisualizationTab::onTogglePlanes()
{
    if (!m_plane3D_Axial && !m_plane3D_Sagittal && !m_plane3D_Coronal) return;

    m_planesVisible = !m_planesVisible;

    auto setPlaneOpacity = [&](vtkSmartPointer<vtkImagePlaneWidget> &plane,
                                int orientation, int defaultSlice, bool visible) {
        if (plane) {
            plane->SetPlaneOrientation(orientation);
            plane->SetSliceIndex(defaultSlice);
            plane->SetKeyPressActivationValue(visible ? 0 : 1); // 可见时禁用按键激活
            plane->GetTexturePlaneProperty()->SetOpacity(visible ? 0.3 : 0.0);
            plane->GetPlaneProperty()->SetOpacity(visible ? 0.6 : 0.0);
        }
    };

    int axSlice = m_sliderAxial ? m_sliderAxial->value() : m_dims[2]/2;
    int sgSlice = m_sliderSagittal ? m_sliderSagittal->value() : m_dims[0]/2;
    int crSlice = m_sliderCoronal ? m_sliderCoronal->value() : m_dims[1]/2;
    setPlaneOpacity(m_plane3D_Axial,     2, axSlice, m_planesVisible);
    setPlaneOpacity(m_plane3D_Sagittal,  0, sgSlice, m_planesVisible);
    setPlaneOpacity(m_plane3D_Coronal,   1, crSlice, m_planesVisible);

    m_btnTogglePlanes->setText(m_planesVisible ? tr("隐藏切片平面") : tr("显示切片平面"));
    m_renderWindow3D->Render();
    LogManager::instance()->logInfo(m_planesVisible ?
        tr("3D slice planes shown") : tr("3D slice planes hidden"));
}

// ==================== 平面控件 ↔ 滑条双向绑定 ====================
static void SetResliceOrigin(vtkImageReslice *r, double x, double y, double z)
{
    if (!r) return;
    r->SetResliceAxesOrigin(x, y, z);
    r->Update();
}

void VisualizationTab::onPlaneWidgetMoved(int axis, int sliceIndex)
{
    // 3D 平面控件拖动 → 同步滑条 + 更新独立 2D Reslice
    switch (axis) {
    case 0: // X → Sagittal
        m_sliderSagittal->blockSignals(true);
        m_sliderSagittal->setValue(sliceIndex);
        m_sliderSagittal->blockSignals(false);
        m_labelSagittal->setText(QString("%1 / %2").arg(sliceIndex).arg(m_dims[0] - 1));
        SetResliceOrigin(m_resliceSagittal, (double)sliceIndex, 0, 0);
        m_renderWindowSagittal->Render();
        break;
    case 1: // Y → Coronal
        m_sliderCoronal->blockSignals(true);
        m_sliderCoronal->setValue(sliceIndex);
        m_sliderCoronal->blockSignals(false);
        m_labelCoronal->setText(QString("%1 / %2").arg(sliceIndex).arg(m_dims[1] - 1));
        SetResliceOrigin(m_resliceCoronal, 0, (double)sliceIndex, 0);
        m_renderWindowCoronal->Render();
        break;
    case 2: // Z → Axial
        m_sliderAxial->blockSignals(true);
        m_sliderAxial->setValue(sliceIndex);
        m_sliderAxial->blockSignals(false);
        m_labelAxial->setText(QString("%1 / %2").arg(sliceIndex).arg(m_dims[2] - 1));
        SetResliceOrigin(m_resliceAxial, 0, 0, (double)sliceIndex);
        m_renderWindowAxial->Render();
        break;
    }
    m_renderWindow3D->Render();
}

void VisualizationTab::onSliderAxialChanged(int value)
{
    if (!m_dataLoaded) return;
    m_labelAxial->setText(QString("%1 / %2").arg(value).arg(m_dims[2] - 1));
    SetResliceOrigin(m_resliceAxial, 0, 0, (double)value);
    m_renderWindowAxial->Render();
    if (m_plane3D_Axial) {
        m_plane3D_Axial->SetPlaneOrientation(2);
        m_plane3D_Axial->SetSliceIndex(value);
    }
    m_renderWindow3D->Render();
}

void VisualizationTab::onSliderSagittalChanged(int value)
{
    if (!m_dataLoaded) return;
    m_labelSagittal->setText(QString("%1 / %2").arg(value).arg(m_dims[0] - 1));
    SetResliceOrigin(m_resliceSagittal, (double)value, 0, 0);
    m_renderWindowSagittal->Render();
    if (m_plane3D_Sagittal) {
        m_plane3D_Sagittal->SetPlaneOrientation(0);
        m_plane3D_Sagittal->SetSliceIndex(value);
    }
    m_renderWindow3D->Render();
}

void VisualizationTab::onSliderCoronalChanged(int value)
{
    if (!m_dataLoaded) return;
    m_labelCoronal->setText(QString("%1 / %2").arg(value).arg(m_dims[1] - 1));
    SetResliceOrigin(m_resliceCoronal, 0, (double)value, 0);
    m_renderWindowCoronal->Render();
    if (m_plane3D_Coronal) {
        m_plane3D_Coronal->SetPlaneOrientation(1);
        m_plane3D_Coronal->SetSliceIndex(value);
    }
    m_renderWindow3D->Render();
}

void VisualizationTab::updateSliceViews()
{
    if (m_sliderAxial)    onSliderAxialChanged(m_sliderAxial->value());
    if (m_sliderSagittal) onSliderSagittalChanged(m_sliderSagittal->value());
    if (m_sliderCoronal)  onSliderCoronalChanged(m_sliderCoronal->value());
}

void VisualizationTab::update3DPlaneSliders()
{
    // 使滑条与 3D 视图中的平面控件位置同步
    if (m_plane3D_Axial && m_sliderAxial) {
        int idx = m_plane3D_Axial->GetSliceIndex();
        m_sliderAxial->blockSignals(true);
        m_sliderAxial->setValue(idx);
        m_sliderAxial->blockSignals(false);
        m_labelAxial->setText(QString("%1 / %2").arg(idx).arg(m_dims[2] - 1));
    }
    if (m_plane3D_Sagittal && m_sliderSagittal) {
        int idx = m_plane3D_Sagittal->GetSliceIndex();
        m_sliderSagittal->blockSignals(true);
        m_sliderSagittal->setValue(idx);
        m_sliderSagittal->blockSignals(false);
        m_labelSagittal->setText(QString("%1 / %2").arg(idx).arg(m_dims[0] - 1));
    }
    if (m_plane3D_Coronal && m_sliderCoronal) {
        int idx = m_plane3D_Coronal->GetSliceIndex();
        m_sliderCoronal->blockSignals(true);
        m_sliderCoronal->setValue(idx);
        m_sliderCoronal->blockSignals(false);
        m_labelCoronal->setText(QString("%1 / %2").arg(idx).arg(m_dims[1] - 1));
    }
}
