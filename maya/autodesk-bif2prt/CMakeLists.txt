ADD_EXECUTABLE ( bif2prt
  bif2prt.cpp
  )

# Library as linked in original Autodesk Makefile,
# kept for reference
#
# bifrostcore
# rtbase
# sfkernel
# compute_runtime
# aminomath
# aminobase
# Foundation
# synHub
# AppVersion
# adlmint
# Base
# awBoostLibc_system-1_52
# awBoostLibc_filesystem-1_52
# awBoostLibc_thread-1_52
# tbb
# zlib

TARGET_LINK_LIBRARIES ( bif2prt
  bifrostcore
  bifrostapi
  Base
  )

