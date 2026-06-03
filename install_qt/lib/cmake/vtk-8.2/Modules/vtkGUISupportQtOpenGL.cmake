set(vtkGUISupportQtOpenGL_LOADED 1)
set(vtkGUISupportQtOpenGL_DEPENDS "vtkCommonCore;vtkGUISupportQt;vtkInteractionStyle;vtkRenderingOpenGL2")
set(vtkGUISupportQtOpenGL_LIBRARIES "vtkGUISupportQtOpenGL")
set(vtkGUISupportQtOpenGL_INCLUDE_DIRS "D:/VTK-8.2.0/VTK-8.2.0/Build/GUISupport/QtOpenGL;D:/VTK-8.2.0/VTK-8.2.0/GUISupport/QtOpenGL")
set(vtkGUISupportQtOpenGL_LIBRARY_DIRS "")
set(vtkGUISupportQtOpenGL_RUNTIME_LIBRARY_DIRS "D:/VTK-8.2.0/VTK-8.2.0/Build/bin")
set(vtkGUISupportQtOpenGL_WRAP_HIERARCHY_FILE "")
set(vtkGUISupportQtOpenGL_KIT "")
set(vtkGUISupportQtOpenGL_TARGETS_FILE "")
set(vtkGUISupportQtOpenGL_EXCLUDE_FROM_WRAPPING 1)

if(NOT Qt5_DIR)
  set(Qt5_DIR "D:/QT/5.9/msvc2015_64/lib/cmake/Qt5")
endif()

find_package(Qt5 REQUIRED QUIET COMPONENTS OpenGL)


