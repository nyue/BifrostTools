FIND_PACKAGE ( Arnold REQUIRED )

INCLUDE_DIRECTORIES ( ${ARNOLD_INCLUDE_DIR} )

IF (WIN32)
  # Windows
  ADD_DEFINITIONS( /D_CRT_SECURE_NO_WARNINGS )
ENDIF (WIN32)

ADD_LIBRARY ( bifrost_arnold SHARED ProcMain.cpp ProcArgs.cpp )
TARGET_LINK_LIBRARIES ( bifrost_arnold
  ${Arnold_ai_LIBRARY}
  ${Boost_LIBRARIES}
  bifrostcore
  bifrostapi
  Base
  utils
  )

SET_TARGET_PROPERTIES ( bifrost_arnold
  PROPERTIES OUTPUT_NAME bifrost
  )

ARNOLD_SET_LIBRARY_PROPERTIES ( bifrost_arnold )

INSTALL ( TARGETS
  bifrost_arnold
  RUNTIME DESTINATION dso/arnold COMPONENT Runtime
  LIBRARY DESTINATION dso/arnold COMPONENT Runtime
  )
