idf_component_register(SRCS "app_main.cpp" "task.cpp"
                        "helper/uart.cpp" "helper/system.cpp"
                        "fingerprint/reader.cpp" "fingerprint/helper.cpp"
                        "misc/pwm_audio.c"
                        "wifi/station.cpp"
                       INCLUDE_DIRS "." "fingerprint")

set(ulp_app_name ulp_${COMPONENT_NAME})
set(ulp_riscv_sources "ulp/main.c")
set(ulp_exp_dep_srcs "app_main.c")
ulp_embed_binary(${ulp_app_name} "${ulp_riscv_sources}" "${ulp_exp_dep_srcs}")

target_compile_options(${COMPONENT_LIB} PRIVATE -std=gnu++23)