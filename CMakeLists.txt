CMAKE_MINIMUM_REQUIRED ( VERSION 2.8 )
PROJECT ( BifrostTools )

FIND_PACKAGE ( Maya REQUIRED  )
FIND_PACKAGE ( Boost REQUIRED COMPONENTS program_options )
FIND_PACKAGE ( S2AA REQUIRED  )
FIND_PACKAGE ( ILMBase REQUIRED )

INCLUDE_DIRECTORIES ( ${Boost_INCLUDE_DIR} )
INCLUDE_DIRECTORIES ( ${S2AA_INCLUDE_DIR} )
INCLUDE_DIRECTORIES ( ${ILMBASE_INCLUDE_DIRS} )

ADD_DEFINITIONS ( -std=c++0x )
IF (APPLE)
  SET ( BIFROST_BASE_DIR ${MAYA_LOCATION}/.. )
ELSE ( APPLE )
  SET ( BIFROST_BASE_DIR ${MAYA_LOCATION} )
ENDIF ()

LINK_DIRECTORIES ( ${MAYA_LIBRARY_DIR} )
IF (APPLE)
  LINK_DIRECTORIES (
    ${BIFROST_BASE_DIR}/plug-ins/bifrost/bin
    ${BIFROST_BASE_DIR}/Maya.app/Contents/MacOS 
    )		     
ELSE (APPLE)	     
  LINK_DIRECTORIES (
    ${BIFROST_BASE_DIR}/plug-ins/bifrost/lib
    )
ENDIF ()

INCLUDE_DIRECTORIES (
  ${BIFROST_BASE_DIR}/plug-ins/bifrost/include/common
  ${BIFROST_BASE_DIR}/plug-ins/bifrost/include/modules/bifrostcore/api
  ${BIFROST_BASE_DIR}/plug-ins/bifrost/include/modules/bifrostapi/api
  ${BIFROST_BASE_DIR}/plug-ins/bifrost/include/modules/aminomath/api
  )

# Houdini requirement are stringent and often at odds
# with other DCC application, as such, we can't build
# them together.
# The strategy is to build and package them separately
IF ( BUILD_HOUDINI_TOOLS )
  ADD_SUBDIRECTORY ( houdini )
ELSE ( BUILD_HOUDINI_TOOLS )
  ADD_SUBDIRECTORY ( maya )
  ADD_SUBDIRECTORY ( procedural )
  ADD_SUBDIRECTORY ( applications )
ENDIF ( BUILD_HOUDINI_TOOLS )
