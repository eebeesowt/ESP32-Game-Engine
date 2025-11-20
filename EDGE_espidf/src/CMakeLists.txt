# Main component CMakeLists.txt for EDGE ESP-IDF project

# Collect all source files
set(EDGE_SOURCES
    "main.cpp"
    "Core/ActivityMonitor.cpp"
    "InputManager/InputManager.cpp"
    "InputManager/EncoderDriver.cpp"
    "Renderer/Renderer.cpp"
    "Renderer/LGFXGraphicsAdapter.cpp"
    "Renderer/LGFX_Config.cpp"
    "SceneManager/Scene.cpp"
    "SceneManager/SceneManager.cpp"
    "Drivers/LEDDriver.cpp"
    "Drivers/SDCardDriver.cpp"
    "Drivers/AudioDriver.cpp"
    "Scenes/IntroScene.cpp"
    "Scenes/GameScene.cpp"
)

# Include directories
set(EDGE_INCLUDES
    "../include"
    "../include/Core"
    "../include/InputManager"
    "../include/Renderer"
    "../include/SceneManager"
    "../include/Scenes"
)

# Register component
idf_component_register(
    SRCS ${EDGE_SOURCES}
    INCLUDE_DIRS ${EDGE_INCLUDES}
    REQUIRES freertos
)

# Increase main task stack size
target_compile_definitions(${COMPONENT_LIB} PRIVATE
    CONFIG_ESP_MAIN_TASK_STACK_SIZE=8192
)
