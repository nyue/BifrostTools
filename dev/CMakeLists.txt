ADD_EXECUTABLE ( bifvoxelinfo
  bifvoxelinfo_main.cpp
  )

TARGET_LINK_LIBRARIES ( bifvoxelinfo
  ${Bifrost_SDK_LIBRARIES}
  ${Boost_LIBRARIES}
  ${Tbb_TBB_LIBRARY}
  utils
  )

INSTALL ( TARGETS
  bifvoxelinfo
  DESTINATION
  bin
  )
