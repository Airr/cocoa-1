cmake_minimum_required (VERSION 2.8)
# project (alder-pre)
set(CMAKE_C_FLAGS "-std=gnu99")
set(SRCS 
alder_adapter_cite.c
alder_adapter_cite.h
alder_adapter_cmd_check.c
alder_adapter_cmd_check.h
alder_adapter_cut.c
alder_adapter_cut.h
alder_adapter_cut_file2.c
alder_adapter_cut_file2.h
alder_adapter_cut_filter.c
alder_adapter_cut_filter.h
alder_adapter_index_illumina.c
alder_adapter_index_illumina.h
alder_adapter_option.c
alder_adapter_option.h
alder_adapter_pair.c
alder_adapter_pair.h
alder_fastq_stat.c
alder_fastq_stat.h
cmdline.c
cmdline.h
main.c
stack.c
stack.h
trie_tree_list.c
trie_tree_list.h)

ADD_EXECUTABLE(alder-pre ${SRCS})

ADD_DEFINITIONS(-g)
find_package( ZLIB REQUIRED )
if ( ZLIB_FOUND )
  include_directories( ${ZLIB_INCLUDE_DIRS} )
  target_link_libraries( alder-pre ${ZLIB_LIBRARIES} )
endif( ZLIB_FOUND )

target_link_libraries( alder-pre bstrlib nglogc fastq file logger vector )

install (TARGETS alder-pre DESTINATION bin)

enable_testing ()
add_test (alder-preRuns alder-pre)
add_test (alder-preUsage alder-pre)
set_tests_properties (alder-preUsage
  PROPERTIES
  PASS_REGULAR_EXPRESSION "no input"
  )

#define a macro to simplify adding tests
macro (do_test arg result)
  add_test (alder-preComp${arg} alder-pre ${arg})
  set_tests_properties (alder-preComp${arg}
    PROPERTIES PASS_REGULAR_EXPRESSION ${result}
    )
endmacro ()

# do a bunch of result based tests
#do_test (4 "4 is 2")

# build a CPack driven installer package
include (InstallRequiredSystemLibraries)
set (CPACK_PACKAGE_VERSION_MAJOR "${alder-pre_VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${alder-pre_VERSION_MINOR}")
set (CPACK_PACKAGE_CONTACT       "goshng@yahoo.co.kr")
include (CPack)

# enable dashboard scripting
include (CTest)

