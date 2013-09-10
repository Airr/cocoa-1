cmake_minimum_required (VERSION 2.8)
set(CMAKE_C_FLAGS "-std=gnu99")

set(SRCS 
alder_align_cite.c
alder_align_cite.h
alder_align_cmd_check.c
alder_align_cmd_check.h
alder_align_macro.h
alder_align_option.c
alder_align_option.h
cmdline.c
cmdline.h
main.c)

ADD_EXECUTABLE(alder_align ${SRCS})

ADD_DEFINITIONS(-g)
find_package( ZLIB REQUIRED )
if ( ZLIB_FOUND )
  include_directories( ${ZLIB_INCLUDE_DIRS} )
  target_link_libraries( alder_align ${ZLIB_LIBRARIES} )
endif( ZLIB_FOUND )

find_package(GSL REQUIRED)
include_directories(${GSL_INCLUDE_DIRS} ${GSLCBLAS_INCLUDE_DIRS})
set(LIBS ${LIBS} ${GSL_LIBRARIES} ${GSLCBLAS_LIBRARIES})
set(LIBS ${LIBS} bstrlib cigar divsufsort fasta fastq file interval logger mem palign rng ssa vector nglogc)

target_link_libraries( alder_align ${LIBS} )

install (TARGETS alder_align DESTINATION bin)

enable_testing ()
add_test (alder_alignRuns alder_align)
add_test (alder_alignUsage alder_align)
set_tests_properties (alder_alignUsage
  PROPERTIES
  PASS_REGULAR_EXPRESSION "no input"
  )

#define a macro to simplify adding tests
macro (do_test arg result)
  add_test (alder_alignComp${arg} alder_align ${arg})
  set_tests_properties (alder_alignComp${arg}
    PROPERTIES PASS_REGULAR_EXPRESSION ${result}
    )
endmacro ()

# do a bunch of result based tests
#do_test (4 "4 is 2")

# build a CPack driven installer package
include (InstallRequiredSystemLibraries)
set (CPACK_PACKAGE_VERSION_MAJOR "${alder_align_VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${alder_align_VERSION_MINOR}")
set (CPACK_PACKAGE_VERSION_PATCH "${alder_align_VERSION_PATCH}")
set (CPACK_PACKAGE_CONTACT       "goshng@yahoo.co.kr")
include (CPack)

# enable dashboard scripting
include (CTest)

