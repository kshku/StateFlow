set(SRCS
    text.h
    text.c
    button.h
    button.c
)

target_sources(${APP_NAME} PRIVATE ${SRCS})
