#
# Try to find IMAGEMAGICK(++) library
# (see www.imagemagick.org)
#
# Once run this will define:
#
# IMAGEMAGICKLIB_FOUND
# IMAGEMAGICKLIB_LIBRARIES  : contains all available Magick++ C and C++ libraries required for linking
# IMAGEMAGICKLIB_LINK_DIRECTORIES
# IMAGEMAGICKLIB_INCLUDE_DIR
#
# IMAGEMAGICKLIB_CXX_LIBRARY : ImageMagick++ lib for C++ binding
# IMAGEMAGICKLIB_C_LIBRARY   : ImageMagick   lib for C binding
# IMAGEMAGICKLIB_X11_LIBRARY : image.display needs X11
#
#
# Jan Woetzel 10/2004
# www.mip.informatik.uni-kiel.de/~jw
#
# adapted by Volker Baier 10/2007 I found it not very helpful to exit with
# an error, if imagemagick is not found...changed it to standard message
#
# tested with (from www.imagemagick.org):
# - ImageMagick-6.1.1-Q16 on WinXP, MSVS 7.1  (JW 10/2004)
# - ImageMagick 5.5.4     on Linux Suse 8.2, gcc 3.3 (JW 10/2004)
# - ImageMagcik 6.2.5     on Linux Suse 10.1, gcc 4.1.0 (JW 06/2006)
# - ImageMagick-6.2.5-Q16 on WinXP, MSVS 7.1, 8.0 (JW 06/2006)
# --------------------------------


# find the libraries main include header file
FIND_PATH(IMAGEMAGICKLIB_INCLUDE_DIR Magick++.h
  "${IMAGEMAGICKLIB_DIR}/include"
  "$ENV{IMAGEMAGICKLIB_DIR}/include"
  "$ENV{MAGICK_DIR}/include"
  "${IMAGEMAGICKLIB_HOME}/include"
  "$ENV{IMAGEMAGICKLIB_HOME}/include"
  "$ENV{MAGICK_HOME}/include"
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\ImageMagick\\Current;BinPath]/include"
  /usr/local/include
  /usr/include
  /opt/net/gcc41/ImageMagick/include
  /opt/net/gcc33/ImageMagick/include
  )
#MESSAGE("DBG IMAGEMAGICKLIB_INCLUDE_DIR=${IMAGEMAGICKLIB_INCLUDE_DIR}")


# set directories to search for libraries:
SET(IMAGEMAGICKLIB_POSSIBLE_LIBRARY_PATHS
  "${IMAGEMAGICKLIB_DIR}/lib"
  "$ENV{IMAGEMAGICKLIB_DIR}/lib"
  "$ENV{MAGICK_DIR}/lib"
  "${IMAGEMAGICKLIB_HOME}/lib"
  "$ENV{IMAGEMAGICKLIB_HOME}/lib"
  "$ENV{MAGICK_HOME}/lib"
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\ImageMagick\\Current;LibPath]/lib"
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\ImageMagick\\Current;LibPath]"
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\ImageMagick\\Current;BinPath]/lib"
  /usr/local/lib
  /usr/lib
  /opt/net/gcc41/ImageMagick/lib
  /opt/net/gcc33/ImageMagick/lib
  )
#MESSAGE("DBG IMAGEMAGICKLIB_POSSIBLE_LIBRARY_PATHS=${IMAGEMAGICKLIB_POSSIBLE_LIBRARY_PATHS}")


# find provided .lib import library files
FIND_LIBRARY(IMAGEMAGICKLIB_CXX_LIBRARY
  NAMES Magick++ CORE_RL_Magick++_ CORE_DB_Magick++_
  PATHS ${IMAGEMAGICKLIB_POSSIBLE_LIBRARY_PATHS} )
#MESSAGE("DBG IMAGEMAGICKLIB_CXX_LIBRARY=${IMAGEMAGICKLIB_CXX_LIBRARY}")


FIND_LIBRARY(IMAGEMAGICKLIB_C_LIBRARY
  NAMES Magick CORE_RL_magick_ CORE_DB_magick_
  PATHS ${IMAGEMAGICKLIB_POSSIBLE_LIBRARY_PATHS} )
#MESSAGE("DBG IMAGEMAGICKLIB_C_LIBRARY=${IMAGEMAGICKLIB_C_LIBRARY}")


FIND_LIBRARY(IMAGEMAGICKLIB_X11_LIBRARY
  NAMES X11
  PATHS
  ${IMAGEMAGICKLIB_POSSIBLE_LIBRARY_PATHS}
  /usr/X11R6/lib
  /usr/local/lib
  /usr/openwin/lib
  /usr/lib
  )
#MESSAGE("DBG IMAGEMAGICKLIB_X11_LIBRARY=${IMAGEMAGICKLIB_X11_LIBRARY}")




# --------------------------------
# decide if we found all we require:
# for debug purpose
IF(NOT IMAGEMAGICKLIB_CXX_LIBRARY)
  MESSAGE("FindImageMagickLib.cmake could not find IMAGEMAGICKLIB_CXX_LIBRARY")
ENDIF(NOT IMAGEMAGICKLIB_CXX_LIBRARY)
IF(NOT IMAGEMAGICKLIB_C_LIBRARY)
  MESSAGE("FindImageMagickLib.cmake could not find IMAGEMAGICKLIB_C_LIBRARY")
ENDIF(NOT IMAGEMAGICKLIB_C_LIBRARY)
IF(NOT IMAGEMAGICKLIB_X11_LIBRARY)
  MESSAGE("FindImageMagickLib.cmake could not find IMAGEMAGICKLIB_X11_LIBRARY")
ENDIF(NOT IMAGEMAGICKLIB_X11_LIBRARY)


# get the path(=directory) of the main library:
GET_FILENAME_COMPONENT(IMAGEMAGICKLIB_LINK_DIRECTORIES ${IMAGEMAGICKLIB_CXX_LIBRARY} PATH)

IF (IMAGEMAGICKLIB_INCLUDE_DIR AND IMAGEMAGICKLIB_CXX_LIBRARY)
  # OK:
  SET(IMAGEMAGICKLIB_FOUND TRUE)
  SET(IMAGEMAGICKLIB_LIBRARIES
    ${IMAGEMAGICKLIB_CXX_LIBRARY}
    ${IMAGEMAGICKLIB_C_LIBRARY}
    ${IMAGEMAGICKLIB_X11_LIBRARY}
    )
  # add X11 if found (for display)
  IF (IMAGEMAGICKLIB_X11_LIBRARY)
    SET(IMAGEMAGICKLIB_LIBRARIES
      ${IMAGEMAGICKLIB_LIBRARIES}
      ${IMAGEMAGICKLIB_X11_LIBRARY}
      )
  ENDIF (IMAGEMAGICKLIB_X11_LIBRARY)
ELSE (IMAGEMAGICKLIB_INCLUDE_DIR AND IMAGEMAGICKLIB_CXX_LIBRARY)
  #only for debug
  #MESSAGE("FindImageMagick could not find ImageMagick++ library  or header(s)")
  #SET(IMAGEMGICKLIB_FOUND FALSE)
ENDIF (IMAGEMAGICKLIB_INCLUDE_DIR AND IMAGEMAGICKLIB_CXX_LIBRARY)


MARK_AS_ADVANCED(
  IMAGEMAGICKLIB_INCLUDE_DIR
  IMAGEMAGICKLIB_LIBRARIES
  IMAGEMAGICKLIB_CXX_LIBRARY
  IMAGEMAGICKLIB_C_LIBRARY
  IMAGEMAGICKLIB_WAND_LIBRARY
  IMAGEMAGICKLIB_X11_LIBRARY
  IMAGEMAGICKLIB_INCLUDE_DIR
  )

