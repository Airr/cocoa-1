cmake_minimum_required (VERSION 2.8)
# project (alder-kmer)
set(CMAKE_C_FLAGS "-std=gnu99")
set(SRCS 
alder_kmer_cite.c
alder_kmer_cite.h
alder_kmer_option.c
alder_kmer_option.h
cmdline.c
cmdline.h
main.c)

ADD_EXECUTABLE(alder-kmer ${SRCS})

ADD_DEFINITIONS(-g)
find_package( ZLIB REQUIRED )
if ( ZLIB_FOUND )
  include_directories( ${ZLIB_INCLUDE_DIRS} )
  target_link_libraries( alder-kmer ${ZLIB_LIBRARIES} )
endif( ZLIB_FOUND )

find_package( GSL REQUIRED )
if ( GSL_FOUND )
  include_directories( ${GSL_INCLUDE_DIRS} )
  target_link_libraries( alder-kmer ${GSL_LIBRARIES} )
endif( GSL_FOUND )

target_link_libraries( alder-kmer bit bstrlib dna encode fasta fastq file fileseq hash hashtable kmer kseq logger mcas nglogc p99 rng seq token vector)

install (TARGETS alder-kmer DESTINATION bin)

enable_testing ()
add_test (alder-kmerRuns alder-kmer)
add_test (alder-kmerUsage alder-kmer)
set_tests_properties (alder-kmerUsage
  PROPERTIES
  PASS_REGULAR_EXPRESSION "no input"
  )

#define a macro to simplify adding tests
macro (do_test arg result)
  add_test (alder-kmerComp${arg} alder-kmer ${arg})
  set_tests_properties (alder-kmerComp${arg}
    PROPERTIES PASS_REGULAR_EXPRESSION ${result}
    )
endmacro ()

# do a bunch of result based tests
#do_test (4 "4 is 2")

# build a CPack driven installer package
include (InstallRequiredSystemLibraries)
set (CPACK_PACKAGE_VERSION_MAJOR "${alder-kmer_VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${alder-kmer_VERSION_MINOR}")
set (CPACK_PACKAGE_CONTACT       "goshng@yahoo.co.kr")
include (CPack)

# enable dashboard scripting
include (CTest)

