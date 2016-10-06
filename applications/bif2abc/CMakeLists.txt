IF ( BUILD_HOUDINI_TOOLS )
  # Use the Alembic files that comes with Houdini to
  # avoid symbol clash due to version/build/runtime differences
  # especially on OSX

  # For reference
  # ${Alembic_Alembic_LIBRARY}
  # ${Ilmbase_HALF_LIBRARY}
  # ${Ilmbase_IEX_LIBRARY}
  # ${HDF5_hdf5_LIBRARY}
  # ${HDF5_hdf5_hl_LIBRARY}
  
  LINK_DIRECTORIES ( $ENV{HDSO} )

  # Link via $HDSO
  SET ( BIF2ABC_ALEMBIC_LIBRARIES
    AlembicAbc
    AlembicAbcCollection
    AlembicAbcCoreAbstract
    AlembicAbcCoreFactory
    AlembicAbcCoreHDF5
    AlembicAbcCoreOgawa
    AlembicAbcGeom
    AlembicAbcMaterial
    AlembicOgawa
    AlembicUtil
    )
ELSE ( BUILD_HOUDINI_TOOLS )

  IF (USE_MAYA_ALEMBIC)
    IF ( APPLE )
      MESSAGE ( FATAL_ERROR "Maya's Alembic libraries are not usuable on OS X Yosemite due to -stdlib=libstdc++" )
    ELSE ( APPLE )
      FIND_PACKAGE ( MayaAlembic
	REQUIRED
	COMPONENTS
	Abc
	AbcGeom
	Ogawa
	
	AbcCollection
	AbcCoreAbstract
	AbcCoreFactory
	AbcCoreHDF5
	AbcCoreOgawa
	
	Half
	Iex
	Imath
	Util
	hdf5
	hdf5_hl
	)
    ENDIF ( APPLE )
    INCLUDE_DIRECTORIES ( ${MayaAlembic_INCLUDE_DIR} )
    
    # ${MayaAlembic_LIBRARIES}
    # ${Bifrost_HDF5_LIBRARIES}
    
    SET ( BIF2ABC_ALEMBIC_LIBRARIES
      ${MayaAlembic_Abc_LIBRARY}
      ${MayaAlembic_AbcGeom_LIBRARY}
      ${MayaAlembic_Ogawa_LIBRARY}
      
      ${MayaAlembic_AbcCollection_LIBRARY}
      ${MayaAlembic_AbcCoreAbstract_LIBRARY}
      ${MayaAlembic_AbcCoreFactory_LIBRARY}
      ${MayaAlembic_AbcCoreHDF5_LIBRARY}
      ${MayaAlembic_AbcCoreOgawa_LIBRARY}
      
      ${MayaAlembic_Half_LIBRARY}
      ${MayaAlembic_Iex_LIBRARY}
      ${MayaAlembic_Imath_LIBRARY}
      ${MayaAlembic_Util_LIBRARY}
      ${MayaAlembic_hdf5_LIBRARY}
      ${MayaAlembic_hdf5_hl_LIBRARY}
      ${ZLIB_LIBRARY}
      )
    
  ELSE (USE_MAYA_ALEMBIC)
    IF ( ENABLE_ALEMBIC_HDF5 )
      FIND_PACKAGE ( HDF5 REQUIRED COMPONENTS C HL)
      INCLUDE_DIRECTORIES ( ${HDF5_C_INCLUDE_DIR} ${HDF5_HL_INCLUDE_DIR} )
      ADD_DEFINITIONS ( -DBIF2ABC_ENABLE_ALEMBIC_HDF5 )
    ENDIF ( ENABLE_ALEMBIC_HDF5 )
    FIND_PACKAGE ( Alembic REQUIRED )
    INCLUDE_DIRECTORIES ( ${ALEMBIC_INCLUDE_DIR} )
    
    SET ( BIF2ABC_ALEMBIC_CORE_LIBRARIES
      ${Alembic_Alembic_LIBRARY}
      ${Ilmbase_HALF_LIBRARY}
      ${Ilmbase_IEX_LIBRARY}
      )
    IF ( ENABLE_ALEMBIC_HDF5 )
      SET ( BIF2ABC_ALEMBIC_HDF5_LIBRARIES
		${HDF5_hdf5_LIBRARY}
		${HDF5_hdf5_hl_LIBRARY}
		)
    ENDIF ( ENABLE_ALEMBIC_HDF5 )
    SET ( BIF2ABC_ALEMBIC_LIBRARIES
	  ${BIF2ABC_ALEMBIC_CORE_LIBRARIES}
	  ${BIF2ABC_ALEMBIC_HDF5_LIBRARIES}
      )
    
  ENDIF (USE_MAYA_ALEMBIC)
  
  MESSAGE ( "BIF2ABC_ALEMBIC_LIBRARIES = ${BIF2ABC_ALEMBIC_LIBRARIES}" )
ENDIF ( BUILD_HOUDINI_TOOLS )

ADD_EXECUTABLE ( bif2abc
  bif2abc.cpp
  Bifrost2Alembic.cpp
  )
MESSAGE ( "Boost_LIBRARIES = ${Boost_LIBRARIES}")
TARGET_LINK_LIBRARIES ( bif2abc
  ${BIF2ABC_ALEMBIC_LIBRARIES}
  ${Boost_LIBRARIES}
  ${Bifrost_SDK_LIBRARIES}
  utils
  )

INSTALL ( TARGETS
  bif2abc
  DESTINATION
  bin
  )
