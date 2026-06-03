set(vtkRenderingQt_LOADED 1)
set(vtkRenderingQt_DEPENDS "vtkCommonCore;vtkCommonDataModel;vtkCommonExecutionModel;vtkCommonSystem;vtkFiltersSources;vtkFiltersTexture;vtkGUISupportQt;vtkRenderingCore;vtkRenderingLabel")
set(vtkRenderingQt_LIBRARIES "vtkRenderingQt")
set(vtkRenderingQt_INCLUDE_DIRS "D:/VTK-8.2.0/VTK-8.2.0/Build/Rendering/Qt;D:/VTK-8.2.0/VTK-8.2.0/Rendering/Qt")
set(vtkRenderingQt_LIBRARY_DIRS "")
set(vtkRenderingQt_RUNTIME_LIBRARY_DIRS "D:/VTK-8.2.0/VTK-8.2.0/Build/bin")
set(vtkRenderingQt_WRAP_HIERARCHY_FILE "")
set(vtkRenderingQt_KIT "")
set(vtkRenderingQt_TARGETS_FILE "")

if(NOT Qt5_DIR)
  set(Qt5_DIR "D:/QT/5.9/msvc2015_64/lib/cmake/Qt5")
endif()

find_package(Qt5 REQUIRED QUIET COMPONENTS Widgets)


