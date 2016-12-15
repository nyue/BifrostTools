FIND_PACKAGE ( Alembic 1.5.5 REQUIRED )

ADD_EXECUTABLE ( bif2abc
  bif2abc.cpp
  Bifrost2Alembic.cpp
  )

TARGET_LINK_LIBRARIES ( bif2abc
  ${Alembic_LIBRARIES}
  ${Boost_LIBRARIES}
  ${Bifrost_SDK_LIBRARIES}
  utils
  )

INSTALL ( TARGETS
  bif2abc
  DESTINATION
  bin
  )
