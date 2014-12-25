ADD_EXECUTABLE ( bifdump
  bifdump.cpp
  )

TARGET_LINK_LIBRARIES ( bifdump
  bifrostcore
  bifrostapi
  Base
  )
