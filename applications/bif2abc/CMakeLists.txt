FIND_PACKAGE ( Alembic 1.5.4 REQUIRED )

INCLUDE_DIRECTORIES ( ${ALEMBIC_INCLUDE_DIR} )

ADD_EXECUTABLE ( bif2abc
  bif2abc.cpp
  Bifrost2Alembic.cpp
  )

TARGET_LINK_LIBRARIES ( bif2abc
  ${Alembic_LIBRARIES}
  ${Ilmbase_LIBRARIES}
  ${Boost_LIBRARIES}
  ${Bifrost_SDK_LIBRARIES}
  utils
  )

INSTALL ( TARGETS
  bif2abc
  DESTINATION
  bin
  )
