IF ( NOT WIN32 )
  ADD_DEFINITIONS ( -fPIC )
ENDIF ()

ADD_LIBRARY ( utils
  BifrostUtils.cpp
  )

