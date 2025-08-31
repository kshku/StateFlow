set(SRCS
    text.h
    text.c
    button.h
    button.c
    darray.h
    darray.c
    node.h
    node.c
    input.h
    input.c
    checkbox.h
    checkbox.c
    tline.h
    tline.c
)

target_sources(${APP_NAME} PRIVATE ${SRCS})
