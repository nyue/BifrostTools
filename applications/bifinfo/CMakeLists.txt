ADD_EXECUTABLE ( bifinfo
  bifinfo.cpp
  )

TARGET_LINK_LIBRARIES ( bifinfo
  bifrostcore
  bifrostapi
  Base
  ${Boost_LIBRARIES}
  utils
  )

