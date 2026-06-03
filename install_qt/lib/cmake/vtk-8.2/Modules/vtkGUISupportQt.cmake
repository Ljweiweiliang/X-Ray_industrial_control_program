set(vtkGUISupportQt_LOADED 1)
set(vtkGUISupportQt_DEPENDS "vtkCommonCore;vtkCommonDataModel;vtkFiltersExtraction;vtkInteractionStyle;vtkRenderingCore;vtkRenderingOpenGL2")
set(vtkGUISupportQt_LIBRARIES "vtkGUISupportQt")
set(vtkGUISupportQt_INCLUDE_DIRS "D:/VTK-8.2.0/VTK-8.2.0/Build/GUISupport/Qt;D:/VTK-8.2.0/VTK-8.2.0/GUISupport/Qt")
set(vtkGUISupportQt_LIBRARY_DIRS "")
set(vtkGUISupportQt_RUNTIME_LIBRARY_DIRS "D:/VTK-8.2.0/VTK-8.2.0/Build/bin")
set(vtkGUISupportQt_WRAP_HIERARCHY_FILE "")
set(vtkGUISupportQt_KIT "")
set(vtkGUISupportQt_TARGETS_FILE "")
set(VTK_QT_RCC_EXECUTABLE "")
set(VTK_QT_MOC_EXECUTABLE "D:/QT/5.9/msvc2015_64/bin/moc.exe")
set(VTK_QT_UIC_EXECUTABLE "")
set(VTK_QT_QMAKE_EXECUTABLE "")
set(vtkGUISupportQt_EXCLUDE_FROM_WRAPPING 1)

if(NOT Qt5_DIR)
  set(Qt5_DIR "D:/QT/5.9/msvc2015_64/lib/cmake/Qt5")
endif()

find_package(Qt5 REQUIRED QUIET COMPONENTS Widgets)


