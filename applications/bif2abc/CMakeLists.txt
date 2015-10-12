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

  FIND_PACKAGE ( HDF5 REQUIRED COMPONENTS C HL)
  FIND_PACKAGE ( Alembic REQUIRED )
  MESSAGE ( "MAYA_LOCATION = ${MAYA_LOCATION}")
  INCLUDE_DIRECTORIES ( ${HDF5_C_INCLUDE_DIR} ${HDF5_HL_INCLUDE_DIR} )
  INCLUDE_DIRECTORIES ( ${ALEMBIC_INCLUDE_DIR} )
  # LINK_DIRECTORIES ( ${MAYA_LOCATION}/devkit/Alembic/lib )

  SET ( BIF2ABC_ALEMBIC_LIBRARIES
    ${Alembic_Alembic_LIBRARY}
    ${Ilmbase_HALF_LIBRARY}
    ${Ilmbase_IEX_LIBRARY}
    ${HDF5_hdf5_LIBRARY}
    ${HDF5_hdf5_hl_LIBRARY}
    )

ENDIF (USE_MAYA_ALEMBIC)

MESSAGE ( "BIF2ABC_ALEMBIC_LIBRARIES = ${BIF2ABC_ALEMBIC_LIBRARIES}" )

ADD_EXECUTABLE ( bif2abc
  bif2abc.cpp
  Bifrost2Alembic.cpp
  )

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
