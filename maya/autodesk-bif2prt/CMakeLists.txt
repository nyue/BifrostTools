ADD_EXECUTABLE ( bif2prt
  bif2prt.cpp
  )

TARGET_LINK_LIBRARIES ( bif2prt
  bifrostcore
  rtbase
  sfkernel
  compute_runtime
  aminomath
  aminobase
  Foundation
  synHub
  AppVersion
  adlmint
  Base
  awBoostLibc_system-1_52
  awBoostLibc_filesystem-1_52
  awBoostLibc_thread-1_52
  tbb
  zlib
  )

