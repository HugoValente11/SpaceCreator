set(LIB_NAME spacecreatorsystem)

add_library(${LIB_NAME} STATIC)

target_sources(${LIB_NAME} PRIVATE
    ivsystemchecks.cpp
    ivsystemchecks.h
    mscsystemchecks.cpp
    mscsystemchecks.h
    spacecreatorproject.cpp
    spacecreatorproject.h
)

target_include_directories(${LIB_NAME}
    PUBLIC .
)

target_link_libraries(${LIB_NAME}
    libmsceditor
    libdveditor
    libiveditor
    ${QT_CORE}
    ${QT_GUI}
    ${QT_WIDGETS}
)
