add_subdirectory(ivcore)
add_subdirectory(dvcore)
add_subdirectory(asn1editor)
add_subdirectory(asn1library)
add_subdirectory(libiveditor)
add_subdirectory(libmsceditor)
add_subdirectory(msccore)
add_subdirectory(shared)
add_subdirectory(spacecreatorsystem)

if (${QTC_FOUND})
    add_subdirectory(asn1plugin)
endif()
