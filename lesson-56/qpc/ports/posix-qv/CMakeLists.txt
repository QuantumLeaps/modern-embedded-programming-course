# ports/posix-qv
target_include_directories(qpc PUBLIC .)
target_sources(qpc PRIVATE
    qf_port.c
    $<$<CONFIG:Spy>:${CMAKE_CURRENT_SOURCE_DIR}/qs_port.c>
)