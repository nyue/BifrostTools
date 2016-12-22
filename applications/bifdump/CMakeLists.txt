ADD_EXECUTABLE ( bifdump
  bifdump.cpp
  )

TARGET_LINK_LIBRARIES ( bifdump
  ${Bifrost_SDK_LIBRARIES}
  ${Tbb_TBB_LIBRARY}
  )

INSTALL ( TARGETS
  bifdump
  DESTINATION
  bin
  )
