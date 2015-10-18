HDK_ADD_EXECUTABLE ( gbifrost
  gbifrost.cpp
  )

TARGET_LINK_LIBRARIES ( gbifrost
  houdini_utils
  ${Boost_LIBRARIES}
  ${Bifrost_api_LIBRARY}
  )

INSTALL ( TARGETS
  gbifrost
  DESTINATION
  bin
  )


