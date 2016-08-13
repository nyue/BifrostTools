# The following 2 lines are interim until
# a version of 3DelightForMaya is available
# for the same version of Maya as used in
# the parent directory
FIND_PACKAGE ( Maya REQUIRED )
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
  ${BIFROST_REQUIRED_LIBRARIES}
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

MESSAGE ( "MAYA_VERSION_NUMBER = ${MAYA_VERSION_NUMBER}" )
SET ( PACKAGING_MAYA_VERSION_NUMBER ${MAYA_VERSION_NUMBER} CACHE INTERNAL "Maya Version" FORCE)

IF (APPLE)
  INSTALL ( TARGETS BifrostTools BUNDLE
    RUNTIME DESTINATION maya/${MAYA_VERSION_NUMBER}/plug-ins COMPONENT Runtime
    LIBRARY DESTINATION maya/${MAYA_VERSION_NUMBER}/plug-ins COMPONENT Runtime
	)
ELSEIF (WIN32)
  INSTALL ( TARGETS BifrostTools
    RUNTIME DESTINATION maya/${MAYA_VERSION_NUMBER}/plug-ins COMPONENT Runtime
    LIBRARY DESTINATION maya/${MAYA_VERSION_NUMBER}/plug-ins COMPONENT Runtime
	)
ELSE (APPLE)
  INSTALL ( TARGETS BifrostTools
    RUNTIME DESTINATION maya/${MAYA_VERSION_NUMBER}/plug-ins COMPONENT Runtime
    LIBRARY DESTINATION maya/${MAYA_VERSION_NUMBER}/plug-ins COMPONENT Runtime
	)
ENDIF (APPLE)

INSTALL ( FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/AEBifrostSurfaceShapeTemplate.mel
  ${CMAKE_CURRENT_SOURCE_DIR}/BifrostDelightIntegration.mel
  DESTINATION maya/${MAYA_VERSION_NUMBER}/scripts
  )

INSTALL ( CODE
  "
  FILE (WRITE ${CMAKE_INSTALL_PREFIX}/maya/${MAYA_VERSION_NUMBER}/bifrosttools.mod \"+ bifrosttools any ${CMAKE_INSTALL_PREFIX}/maya/${MAYA_VERSION_NUMBER}\n\")
  "
  COMPONENT Runtime
)
