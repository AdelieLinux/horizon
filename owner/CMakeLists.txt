set(OWNER_SRCS
        owner.c
)
add_executable(hscript-printowner ${OWNER_SRCS})

install(TARGETS hscript-printowner DESTINATION bin)
