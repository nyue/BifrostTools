# The following 2 lines are interim until
# a version of 3DelightForMaya is available
# for the same version of Maya as used in
# the parent directory
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT ( BifrostRenderManTools )
FIND_PACKAGE ( Maya REQUIRED )
FIND_PACKAGE ( OpenGL REQUIRED )
FIND_PACKAGE ( Delight REQUIRED )

ADD_DEFINITIONS ( ${MAYA_DEFINITIONS} )

INCLUDE_DIRECTORIES (
  ${DELIGHT_INCLUDE_DIR}
  )

ADD_LIBRARY ( BifrostTools SHARED
  BifrostTools.cpp
  BifrostSurfaceShape.cpp
  BifrostSurfaceShapeUI.cpp
  BifrostSurfaceShapeCacheCommand.cpp
  )

TARGET_LINK_LIBRARIES ( BifrostTools
  ${DELIGHT_3delight_LIBRARY}
  ${DELIGHTFORMAYA_3delightformaya_LIBRARY}
  ${MAYA_Foundation_LIBRARY}
  ${MAYA_OpenMaya_LIBRARY}
  ${MAYA_OpenMayaUI_LIBRARY}
  ${MAYA_OpenMayaAnim_LIBRARY}
  ${OPENGL_gl_LIBRARY}
  ${OPENGL_glu_LIBRARY}
  ${Z_z_LIBRARY}
  ${LICENSING_LIBRARY_NAME}
  ${LMX_lmxclient_LIBRARY}
  )

MAYA_SET_LIBRARY_PROPERTIES ( BifrostTools )

MESSAGE ( "MAYA_VERSION = ${MAYA_VERSION}" )
SET ( PACKAGING_MAYA_VERSION ${MAYA_VERSION} CACHE INTERNAL "Maya Version" FORCE)

IF (APPLE)
  INSTALL ( TARGETS BifrostTools BUNDLE
    RUNTIME DESTINATION maya/${MAYA_VERSION}/plug-ins COMPONENT Runtime
    LIBRARY DESTINATION maya/${MAYA_VERSION}/plug-ins COMPONENT Runtime
	)
ELSEIF (WIN32)
  INSTALL ( TARGETS BifrostTools
    RUNTIME DESTINATION maya/${MAYA_VERSION}/plug-ins COMPONENT Runtime
    LIBRARY DESTINATION maya/${MAYA_VERSION}/plug-ins COMPONENT Runtime
	)
ELSE (APPLE)
  INSTALL ( TARGETS BifrostTools
    RUNTIME DESTINATION maya/${MAYA_VERSION}/plug-ins COMPONENT Runtime
    LIBRARY DESTINATION maya/${MAYA_VERSION}/plug-ins COMPONENT Runtime
	)
ENDIF (APPLE)

INSTALL ( FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/AEBifrostSurfaceShapeTemplate.mel
  ${CMAKE_CURRENT_SOURCE_DIR}/BifrostDelightIntegration.mel
  DESTINATION maya/${MAYA_VERSION}/scripts
  )

INSTALL ( CODE
  "
  FILE (WRITE ${CMAKE_INSTALL_PREFIX}/maya/${MAYA_VERSION}/bifrosttools.mod \"+ bifrosttools any ${CMAKE_INSTALL_PREFIX}/maya/${MAYA_VERSION}\n\")
  "
  COMPONENT Runtime
)
