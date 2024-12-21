# 모든 .c, .cpp 파일을 검색하되, 'ulp' 폴더는 제외합니다.
file(GLOB_RECURSE ALL_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/*.c
    ${CMAKE_CURRENT_LIST_DIR}/*.cpp
)

# 'ulp' 폴더를 제외한 파일만 선택
foreach(SOURCE_FILE ${ALL_SOURCE_FILES})
    # 'ulp' 폴더가 경로에 포함된 파일은 제외
    if(NOT ${SOURCE_FILE} MATCHES "/ulp/")
        list(APPEND COMPONENT_SOURCES ${SOURCE_FILE})
    endif()
endforeach()

# 추가된 파일 목록을 출력 (디버깅용)
message(STATUS "Source files: ${COMPONENT_SOURCES}")

# idf_component_register에 파일을 등록
idf_component_register(
    SRCS ${COMPONENT_SOURCES}
    INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR}
)

# set(ulp_app_name ulp_${COMPONENT_NAME})
# set(ulp_riscv_sources "ulp/main.c")
# set(ulp_exp_dep_srcs "app_main.c")
# ulp_embed_binary(${ulp_app_name} "${ulp_riscv_sources}" "${ulp_exp_dep_srcs}")
# target_compile_options(${COMPONENT_LIB} PRIVATE -std=gnu++23)