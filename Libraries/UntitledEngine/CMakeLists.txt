cmake_minimum_required(VERSION 3.20)

### Configure UntitledEngine ###
file( GLOB_RECURSE EngineSourceFiles 
    Source/*.cpp
    Source/*.h
)

source_group( TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${EngineSourceFiles} )

add_library( UntitledEngine ${EngineSourceFiles} )

set_property( TARGET UntitledEngine PROPERTY INTERPROCEDURAL_OPTIMIZATION_OPENGLEDITORRELEASE TRUE )
set_property( TARGET UntitledEngine PROPERTY INTERPROCEDURAL_OPTIMIZATION_OPENGLGAMERELEASE TRUE )


target_include_directories( UntitledEngine PUBLIC
    Libraries/include
    Source
)

target_link_directories( UntitledEngine PUBLIC
    Libraries/lib
)

target_link_libraries( UntitledEngine PUBLIC
    opengl32
    winmm
    Ws2_32
    XInput9_1_0
    XInput
    glew32s
    freetype
)
