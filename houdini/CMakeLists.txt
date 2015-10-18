# Must be first - START
ADD_SUBDIRECTORY ( utils )
INCLUDE_DIRECTORIES ( utils )
# Must be first - END
ADD_SUBDIRECTORY ( bif2bgeo )
ADD_SUBDIRECTORY ( converter )
ADD_SUBDIRECTORY ( mantra )
ADD_SUBDIRECTORY ( translator )
