# Set up the required libraries for GLFW3 usage/linkage - START
IF (APPLE)
  FIND_LIBRARY ( COCOA_LIBRARY Cocoa )
  FIND_LIBRARY ( IOKIT_LIBRARY IOKit )
  FIND_LIBRARY ( COREVIDEO_LIBRARY CoreVideo )
ELSEIF (UNIX AND NOT APPLE)
  SET ( GLFW3_REQUIRED_X11_LIBRARIES
    Xi
    Xrandr
    Xinerama
    Xcursor
    )
ELSE ()
ENDIF ()

SET ( GLFW3_REQUIRED_LIBRARIES
  ${OPENGL_gl_LIBRARY}
  ${OPENGL_glu_LIBRARY}
  ${GLFW3_glfw_LIBRARY}
  # UNIX
  ${GLFW3_REQUIRED_X11_LIBRARIES}
  # APPLE
  ${COCOA_LIBRARY}
  ${IOKIT_LIBRARY}
  ${COREVIDEO_LIBRARY}
  )

# Set up the required libraries for GLFW3 usage/linkage - END

ADD_EXECUTABLE ( bifplay
  main.cpp
  )

TARGET_LINK_LIBRARIES ( bifplay
  bifrostcore
  bifrostapi
  Base
  ${CMAKE_THREAD_LIBS_INIT}
  ${Boost_LIBRARIES}
  ${GLEW_GLEW_LIBRARY}
  ${GLFW3_REQUIRED_LIBRARIES}
  )

INSTALL ( TARGETS
  bifplay
  DESTINATION
  bin
  )
