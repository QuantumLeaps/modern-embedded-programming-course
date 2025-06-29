# ports/win32-qv
target_include_directories(qpc PUBLIC .)
target_sources(qpc PRIVATE
    qf_port.c
    $<$<BOOL:${QPC_CFG_GUI}>:${CMAKE_CURRENT_SOURCE_DIR}/qwin_gui.c>
    $<$<CONFIG:Spy>:${CMAKE_CURRENT_SOURCE_DIR}/qs_port.c>
)
