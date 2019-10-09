set(HSCRIPT_SOURCE
	script.cc
        key.cc
        meta.cc
        network.cc
)

set(HSCRIPT_INCLUDE
	script.hh
	key.hh
)

add_library(hscript SHARED ${HSCRIPT_SOURCE})
target_compile_features(hscript PRIVATE cxx_nullptr)
target_compile_features(hscript PUBLIC cxx_unicode_literals)

install(TARGETS hscript DESTINATION lib)
install(FILES ${HSCRIPT_INCLUDE} DESTINATION include/hscript)
