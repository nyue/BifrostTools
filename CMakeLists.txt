CMAKE_MINIMUM_REQUIRED ( VERSION 2.8 )
PROJECT ( BifrostTools )

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
