CMAKE_MINIMUM_REQUIRED ( VERSION 2.8 )
PROJECT ( BifrostTools )

FIND_PACKAGE ( Maya REQUIRED  )
FIND_PACKAGE ( Boost REQUIRED COMPONENTS program_options )
FIND_PACKAGE ( S2AA REQUIRED  )
FIND_PACKAGE ( ILMBase REQUIRED )

INCLUDE_DIRECTORIES ( ${Boost_INCLUDE_DIR} )
INCLUDE_DIRECTORIES ( ${S2AA_INCLUDE_DIR} )
INCLUDE_DIRECTORIES ( ${ILMBASE_INCLUDE_DIRS} )

IF (APPLE)
  ADD_DEFINITIONS ( -std=c++0x )
  LINK_DIRECTORIES (
    ${MAYA_LOCATION}/../plug-ins/bifrost/bin
    ${MAYA_LOCATION}/../Maya.app/Contents/MacOS 
    )
  INCLUDE_DIRECTORIES (
    ${MAYA_LOCATION}/../plug-ins/bifrost/include/common
    ${MAYA_LOCATION}/../plug-ins/bifrost/include/modules/bifrostcore/api
    ${MAYA_LOCATION}/../plug-ins/bifrost/include/modules/bifrostapi/api
    ${MAYA_LOCATION}/../plug-ins/bifrost/include/modules/aminomath/api
    )
ENDIF ()



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
