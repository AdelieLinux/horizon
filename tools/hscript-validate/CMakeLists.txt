set(VALIDATE_SRCS
	validator.cc
)
add_executable(hscript-validate ${VALIDATE_SRCS})
target_link_libraries(hscript-validate hscript)

install(TARGETS hscript-validate DESTINATION bin)
