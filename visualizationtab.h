#ifndef VISUALIZATIONTAB_H
#define VISUALIZATIONTAB_H

#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <vtkSmartPointer.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QPushButton;
class QVTKOpenGLWidget;

class vtkRenderer;
class vtkGenericOpenGLRenderWindow;
class vtkRenderWindowInteractor;
class vtkImageData;
class vtkVolume;
class vtkVolumeMapper;
class vtkVolumeProperty;
class vtkColorTransferFunction;
class vtkPiecewiseFunction;
class vtkBoxWidget;
class vtkImagePlaneWidget;
class vtkImageActor;
class vtkImageReslice;
class vtkImageFlip;
class vtkCellPicker;
class vtkSmartVolumeMapper;

class VisualizationTab : public QWidget
{
    Q_OBJECT

public:
    explicit VisualizationTab(QWidget *parent = nullptr);
    ~VisualizationTab();

public:
    // 设置外部传入的 4 个 VTK 视图控件（由 MainWindow 创建）
    void setViewWidgets(QVTKOpenGLWidget *v3d,
                        QVTKOpenGLWidget *axial,
                        QVTKOpenGLWidget *sagittal,
                        QVTKOpenGLWidget *coronal);

    // 由 VTK 回调调用（公开给 vtkSlicePlaneCallback）
    void onPlaneWidgetMoved(int axis, int sliceIndex);

private slots:
    void onLoadData();
    void onVolumeRender();
    void onClipVolume();
    void onTogglePlanes();
    void onSliderAxialChanged(int value);
    void onSliderSagittalChanged(int value);
    void onSliderCoronalChanged(int value);

private:
    void setupUI();
    void setup3DView();
    void setupSliceViews();
    void setupSliderUI(QVBoxLayout *layout);
    void updateSliceViews();
    void update3DPlaneSliders();

    // 读取各类数据格式
    vtkSmartPointer<vtkImageData> readMHD(const QString &filePath);
    vtkSmartPointer<vtkImageData> readVTK(const QString &filePath);
    vtkSmartPointer<vtkImageData> readRAW(const QString &filePath, int w, int h, int d);

    // ===== UI =====
    QPushButton *m_btnLoadData;
    QPushButton *m_btnVolumeRender;
    QPushButton *m_btnClipVolume;
    QPushButton *m_btnTogglePlanes;

    // 四个 VTK 视图（由 MainWindow 创建，通过 setViewWidgets 传入）
    QVTKOpenGLWidget *m_view3D;
    QVTKOpenGLWidget *m_viewAxial;
    QVTKOpenGLWidget *m_viewSagittal;
    QVTKOpenGLWidget *m_viewCoronal;

    // ===== 3D 体绘制相关 =====
    vtkSmartPointer<vtkRenderer>           m_renderer3D;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> m_renderWindow3D;
    vtkSmartPointer<vtkRenderWindowInteractor>    m_interactor3D;
    vtkSmartPointer<vtkImageData>          m_imageData;
    vtkSmartPointer<vtkImageFlip>          m_flipFilter;
    vtkSmartPointer<vtkVolume>             m_volume;
    vtkSmartPointer<vtkVolumeMapper>       m_volumeMapper;
    vtkSmartPointer<vtkVolumeProperty>     m_volumeProperty;
    vtkSmartPointer<vtkColorTransferFunction> m_colorFunc;
    vtkSmartPointer<vtkPiecewiseFunction>  m_opacityFunc;
    vtkSmartPointer<vtkPiecewiseFunction>  m_gradientOpacity;

    // 实体切割
    vtkSmartPointer<vtkBoxWidget>          m_boxWidget;
    bool m_clipEnabled;

    // 3D 切片平面开关
    bool m_planesVisible;

    // ===== 三个正交切片视图 =====
    vtkSmartPointer<vtkRenderer>           m_rendererAxial;
    vtkSmartPointer<vtkRenderer>           m_rendererSagittal;
    vtkSmartPointer<vtkRenderer>           m_rendererCoronal;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> m_renderWindowAxial;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> m_renderWindowSagittal;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> m_renderWindowCoronal;
    vtkSmartPointer<vtkRenderWindowInteractor> m_interactorAxial;
    vtkSmartPointer<vtkRenderWindowInteractor> m_interactorSagittal;
    vtkSmartPointer<vtkRenderWindowInteractor> m_interactorCoronal;
    // 2D 切片视图（独立 vtkImageReslice → ImageActor 管线）
    vtkSmartPointer<vtkImageReslice> m_resliceAxial;
    vtkSmartPointer<vtkImageReslice> m_resliceSagittal;
    vtkSmartPointer<vtkImageReslice> m_resliceCoronal;
    vtkSmartPointer<vtkImageActor> m_actorAxial;
    vtkSmartPointer<vtkImageActor> m_actorSagittal;
    vtkSmartPointer<vtkImageActor> m_actorCoronal;

    // 3D 视图中的平面控件（可在体绘制上拖动）
    vtkSmartPointer<vtkImagePlaneWidget>   m_plane3D_Axial;
    vtkSmartPointer<vtkImagePlaneWidget>   m_plane3D_Sagittal;
    vtkSmartPointer<vtkImagePlaneWidget>   m_plane3D_Coronal;

    vtkSmartPointer<vtkCellPicker>         m_picker;

    // 切片索引滑条
    QSlider *m_sliderAxial;
    QSlider *m_sliderSagittal;
    QSlider *m_sliderCoronal;
    QLabel  *m_labelAxial;
    QLabel  *m_labelSagittal;
    QLabel  *m_labelCoronal;

    // 数据维度缓存
    int m_dims[3];
    bool m_dataLoaded;
};

#endif // VISUALIZATIONTAB_H
