CMAKE_MINIMUM_REQUIRED ( VERSION 2.8 )
PROJECT ( BifrostTools )

IF (APPLE)
  IF (BUILD_HOUDINI_TOOLS)
    MESSAGE ( FATAL_ERROR "Due to use of Clang in building Houdini, it is not compatible with Maya 2016/Bifrost" )
  ENDIF ()
ENDIF ()

FIND_PACKAGE ( Maya REQUIRED  )
FIND_PACKAGE ( Boost 1.47 REQUIRED COMPONENTS program_options )
FIND_PACKAGE ( S2AA REQUIRED  )
IF ( BUILD_HOUDINI_TOOLS )
  FIND_PACKAGE ( HDK REQUIRED )
  INCLUDE_DIRECTORIES ( ${HDK_INCLUDE_DIR} )
ELSE ()
  FIND_PACKAGE ( ILMBase REQUIRED )
ENDIF ()
IF (WIN32)
   FIND_PACKAGE ( ZLIB REQUIRED )
   INCLUDE_DIRECTORIES ( ${ZLIB_INCLUDE_DIR} )
ENDIF ()

INCLUDE_DIRECTORIES ( ${Boost_INCLUDE_DIR} )
INCLUDE_DIRECTORIES ( ${S2AA_INCLUDE_DIR} )
INCLUDE_DIRECTORIES ( ${ILMBASE_INCLUDE_DIRS} )

IF ( WIN32 )
   ADD_DEFINITIONS ( /D_CRT_SECURE_NO_WARNINGS )
ELSE ()
   ADD_DEFINITIONS ( -std=c++0x )
ENDIF ()

SET ( BIFROST_BASE_DIR ${MAYA_LOCATION} )

LINK_DIRECTORIES ( ${MAYA_LIBRARY_DIR} )
MESSAGE ( "MAYA_LIBRARY_DIR = ${MAYA_LIBRARY_DIR}")
IF (APPLE)
  LINK_DIRECTORIES (
    ${BIFROST_BASE_DIR}/plug-ins/bifrost/bin
    ${BIFROST_BASE_DIR}/Maya.app/Contents/MacOS 
    )		     
ELSE (APPLE)	     
  SET ( CMAKE_INSTALL_RPATH
    ${BIFROST_BASE_DIR}/plug-ins/bifrost/lib
    ${BIFROST_BASE_DIR}/lib
    )
  IF (BUILD_HOUDINI_TOOLS)
    SET ( CMAKE_INSTALL_RPATH
      $ENV{HDSO}
      )
  ENDIF ()
  LINK_DIRECTORIES (
    ${BIFROST_BASE_DIR}/plug-ins/bifrost/lib
    )
ENDIF ()

INCLUDE_DIRECTORIES (
  .
  ${BIFROST_BASE_DIR}/plug-ins/bifrost/include/common
  ${BIFROST_BASE_DIR}/plug-ins/bifrost/include/modules/bifrostcore/api
  ${BIFROST_BASE_DIR}/plug-ins/bifrost/include/modules/bifrostapi/api
  ${BIFROST_BASE_DIR}/plug-ins/bifrost/include/modules/aminomath/api
  )

ADD_SUBDIRECTORY ( utils )
ADD_SUBDIRECTORY ( applications )
# Houdini requirement are stringent and often at odds
# with other DCC application, as such, we can't build
# them together.
# The strategy is to build and package them separately
IF ( BUILD_HOUDINI_TOOLS )
  ADD_SUBDIRECTORY ( houdini )
ELSE ( BUILD_HOUDINI_TOOLS )
  ADD_SUBDIRECTORY ( maya )
  ADD_SUBDIRECTORY ( procedural )
ENDIF ( BUILD_HOUDINI_TOOLS )

INCLUDE ( CPack )
