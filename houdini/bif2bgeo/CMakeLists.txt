HDK_ADD_EXECUTABLE ( bif2bgeo
  bif2bgeo.cpp
  )

TARGET_LINK_LIBRARIES ( bif2bgeo
  houdini_utils
  ${Boost_LIBRARIES}
  ${Bifrost_api_LIBRARY}
  ${Tbb_TBB_LIBRARY}
  )

INSTALL ( TARGETS
  bif2bgeo
  DESTINATION
  bin
  )
