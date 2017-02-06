ADD_EXECUTABLE ( bifinfo
  bifinfo.cpp
  )

TARGET_LINK_LIBRARIES ( bifinfo
  ${Bifrost_SDK_LIBRARIES}
  ${Boost_LIBRARIES}
  ${Tbb_TBB_LIBRARY}
  utils
  )

INSTALL ( TARGETS
  bifinfo
  DESTINATION
  bin
  )
