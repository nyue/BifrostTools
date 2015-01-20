IF (USE_MAYA_ALEMBIC)

  IF ( APPLE )
    MESSAGE ( FATAL_ERROR "Maya's Alembic libraries are not usuable on OS X Yosemite due to -stdlib=libstdc++" )
  ELSE ( APPLE )
    FIND_PACKAGE ( MayaAlembic
      REQUIRED
      COMPONENTS
      Abc AbcGeom Ogawa
      
      AbcCollection
      AbcCoreAbstract
      AbcCoreFactory
      AbcCoreHDF5
      AbcCoreOgawa
      # AbcMaterial
      
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
  
  SET ( BIF2ABC_ALEMBIC_LIBRARIES )

ELSE (USE_MAYA_ALEMBIC)

  IF (NOT WIN32)
    # On Windows, use the HDF with the Alembic library
    # to ensure version consistency
    FIND_PACKAGE ( HDF5 REQUIRED COMPONENTS C HL)
  ENDIF ()
  FIND_PACKAGE ( Alembic
    REQUIRED
    COMPONENTS
    AbcGeom Abc Ogawa
    AbcCoreHDF5 AbcCoreAbstract AbcCoreFactory AbcCoreOgawa
    Util
    ) # Order of components is CRITICAL    
  MESSAGE ( "MAYA_LOCATION = ${MAYA_LOCATION}")
  # INCLUDE_DIRECTORIES ( ${MAYA_LOCATION}/devkit/Alembic/include )
  # LINK_DIRECTORIES ( ${MAYA_LOCATION}/devkit/Alembic/lib )

  SET ( BIF2ABC_ALEMBIC_LIBRARIES
    ${Alembic_AbcCoreAbstract_LIBRARY}
    ${Alembic_AbcCoreFactory_LIBRARY}
    ${Alembic_AbcCoreHDF5_LIBRARY}
    ${Alembic_AbcCoreOgawa_LIBRARY}
    ${Alembic_AbcGeom_LIBRARY}
    ${Alembic_Abc_LIBRARY}
    ${Alembic_Ogawa_LIBRARY}
    ${Alembic_Util_LIBRARY}
    ${Ilmbase_HALF_LIBRARY}
    ${Ilmbase_IEX_LIBRARY}
    ${HDF5_hdf5_LIBRARY}
    ${HDF5_hdf5_hl_LIBRARY}
    )

ENDIF (USE_MAYA_ALEMBIC)


ADD_EXECUTABLE ( bif2abc
  main.cpp
  )

TARGET_LINK_LIBRARIES ( bif2abc
  ${BIF2ABC_ALEMBIC_LIBRARIES}
  ${Boost_LIBRARIES}
  bifrostcore
  bifrostapi
  Base
  utils
  )
