set(SRCS
    text.h
    text.c
    button.h
    button.c
    darray.h
    darray.c
)

target_sources(${APP_NAME} PRIVATE ${SRCS})
