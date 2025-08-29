set(SRCS
    text.h
    text.c
    button.h
    button.c
    darray.h
    darray.c
    node.h
    node.c
    # input.h
    # input.c
)

target_sources(${APP_NAME} PRIVATE ${SRCS})
