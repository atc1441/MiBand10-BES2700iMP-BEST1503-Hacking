include_directories(
    inc
    ../inc
    ../ns_and_ec/inc
    ../speech_process_lib
    )

file(GLOB SOURCE_FILES *.cpp)

add_library(speech_sco_chain_3rd ${SOURCE_FILES})