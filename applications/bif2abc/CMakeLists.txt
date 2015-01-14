FIND_PACKAGE ( HDF5 REQUIRED COMPONENTS C HL)
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

ADD_EXECUTABLE ( bif2abc
  main.cpp
  )

TARGET_LINK_LIBRARIES ( bif2abc

  ${Ilmbase_HALF_LIBRARY}

  ${Alembic_AbcGeom_LIBRARY}
  ${Alembic_Abc_LIBRARY}
  ${Alembic_Ogawa_LIBRARY}
  ${Alembic_AbcCoreHDF5_LIBRARY}
  ${Alembic_AbcCoreAbstract_LIBRARY}
  ${Alembic_AbcCoreFactory_LIBRARY}
  ${Alembic_AbcCoreOgawa_LIBRARY}
  ${Alembic_Util_LIBRARY}

  ${HDF5_hdf5_LIBRARY}
  ${HDF5_hdf5_hl_LIBRARY}

  bifrostcore
  bifrostapi
  Base
  ${Boost_LIBRARIES}
  utils

  )