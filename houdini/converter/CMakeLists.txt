HDK_ADD_EXECUTABLE ( gbifrost
  gbifrost.cpp
  )

TARGET_LINK_LIBRARIES ( gbifrost
  ${Boost_LIBRARIES}
  ${Bifrost_api_LIBRARY}
  )

INSTALL ( TARGETS
  gbifrost
  DESTINATION
  bin
  )


