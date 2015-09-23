ADD_EXECUTABLE ( bifdump
  bifdump.cpp
  )

TARGET_LINK_LIBRARIES ( bifdump
  ${Bifrost_SDK_LIBRARIES}
  )

INSTALL ( TARGETS
  bifdump
  DESTINATION
  bin
  )
