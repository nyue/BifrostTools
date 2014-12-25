FIND_PACKAGE ( Arnold REQUIRED )

INCLUDE_DIRECTORIES ( ${ARNOLD_INCLUDE_DIR} )

IF (WIN32)
  # Windows
  ADD_DEFINITIONS( /D_CRT_SECURE_NO_WARNINGS )
ENDIF (WIN32)

ADD_LIBRARY ( bifrost SHARED ProcMain.cpp ProcArgs.cpp )
TARGET_LINK_LIBRARIES ( bifrost
  ${Arnold_ai_LIBRARY}
  bifrostcore
  bifrostapi
  Base
  )

ARNOLD_SET_LIBRARY_PROPERTIES ( bifrost )

INSTALL ( TARGETS
  bifrost
  DESTINATION
  dso
  )
