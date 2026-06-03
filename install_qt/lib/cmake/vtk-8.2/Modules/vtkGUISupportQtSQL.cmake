set(vtkGUISupportQtSQL_LOADED 1)
set(vtkGUISupportQtSQL_DEPENDS "vtkCommonCore;vtkIOSQL;vtksys")
set(vtkGUISupportQtSQL_LIBRARIES "vtkGUISupportQtSQL")
set(vtkGUISupportQtSQL_INCLUDE_DIRS "D:/VTK-8.2.0/VTK-8.2.0/Build/GUISupport/QtSQL;D:/VTK-8.2.0/VTK-8.2.0/GUISupport/QtSQL")
set(vtkGUISupportQtSQL_LIBRARY_DIRS "")
set(vtkGUISupportQtSQL_RUNTIME_LIBRARY_DIRS "D:/VTK-8.2.0/VTK-8.2.0/Build/bin")
set(vtkGUISupportQtSQL_WRAP_HIERARCHY_FILE "")
set(vtkGUISupportQtSQL_KIT "")
set(vtkGUISupportQtSQL_TARGETS_FILE "")
set(vtkGUISupportQtSQL_EXCLUDE_FROM_WRAPPING 1)

if(NOT Qt5_DIR)
  set(Qt5_DIR "D:/QT/5.9/msvc2015_64/lib/cmake/Qt5")
endif()

find_package(Qt5 REQUIRED QUIET COMPONENTS Sql Widgets)


