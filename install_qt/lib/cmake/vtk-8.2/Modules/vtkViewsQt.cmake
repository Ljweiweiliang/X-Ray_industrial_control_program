set(vtkViewsQt_LOADED 1)
set(vtkViewsQt_DEPENDS "vtkCommonCore;vtkCommonDataModel;vtkCommonExecutionModel;vtkFiltersExtraction;vtkFiltersGeneral;vtkGUISupportQt;vtkInfovisCore;vtkViewsCore;vtkViewsInfovis")
set(vtkViewsQt_LIBRARIES "vtkViewsQt")
set(vtkViewsQt_INCLUDE_DIRS "D:/VTK-8.2.0/VTK-8.2.0/Build/Views/Qt;D:/VTK-8.2.0/VTK-8.2.0/Views/Qt")
set(vtkViewsQt_LIBRARY_DIRS "")
set(vtkViewsQt_RUNTIME_LIBRARY_DIRS "D:/VTK-8.2.0/VTK-8.2.0/Build/bin")
set(vtkViewsQt_WRAP_HIERARCHY_FILE "")
set(vtkViewsQt_KIT "")
set(vtkViewsQt_TARGETS_FILE "")
set(vtkViewsQt_EXCLUDE_FROM_WRAPPING 1)

if(NOT Qt5_DIR)
  set(Qt5_DIR "D:/QT/5.9/msvc2015_64/lib/cmake/Qt5")
endif()

find_package(Qt5 REQUIRED QUIET COMPONENTS Widgets)


