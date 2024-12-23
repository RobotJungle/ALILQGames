project(${CMAKE_PROJECT_NAME})               # Name of project

# Find the python3 package with component NumPy
# find_package(Python3 COMPONENTS Interpreter Development NumPy)
# find_package(pybind11 REQUIRED)
# find_package(matplotlibcpp17 REQUIRED)

# Find the header file of Python.h in Python3_INCLUDE_DIRS and numpy/arrayobject.h in include
# include_directories(${Python3_INCLUDE_DIRS} include)

# add_executable(main main.cpp)           # target

# add_executable(pointmassEx pointmassEx.cpp)

# add_executable(pointmass_imgui pointmass_imgui.cpp)

# add_executable(DiffDrive_imgui DiffDrive_imgui.cpp)

# add_executable(DiffDrive_imgui_ILQGame DiffDrive_imgui_ILQGame.cpp)
add_executable(DiffDriveFeedbackLin_imgui_ALILQGame DiffDriveFeedbackLin_imgui_ALILQGame.cpp)

# add_executable(homicidal_chauffeur homicidal_chauffeur.cpp)

add_executable(DiffDrive_imgui_ALILQGame DiffDrive_imgui_ALILQGame.cpp)

add_executable(3PlayerDiffDriveImguiALILQGame 3PlayerDiffDriveImguiALILQGame.cpp)

add_executable(3PlayerDiffDriveImguiALILQGameReceding 3PlayerDiffDriveImguiALILQGameReceding.cpp)

add_executable(3Player3DModelDiffDriveImguiALILQGameReceding 3Player3DModelDiffDriveImguiALILQGameReceding.cpp)

add_executable(3AgentALILQGameRHGoalChange 3AgentALILQGameRHGoalChange.cpp)

add_executable(5PlayerDiffDriveImguiALILQGameReceding 5PlayerDiffDriveImguiALILQGameReceding.cpp)

add_executable(5Player3DModelDiffDriveImguiALILQGameReceding 5Player3DModelDiffDriveImguiALILQGameReceding.cpp)

# Link executable to ALILQGames library
# target_link_libraries(main
#     PRIVATE                             # private dependency of the main executable (exectubales are usually private)
#     # ${Python3_LIBRARIES}
#     ALILQGames)

# target_link_libraries(pointmassEx
#     PRIVATE                             # private dependency of the main executable (exectubales are usually private)
#     # ${Python3_LIBRARIES}
#     # pybind11::embed
#     # matplotlibcpp17::matplotlibcpp17
#     ALILQGames)

# target_link_libraries(pointmass_imgui
#     PRIVATE                             # private dependency of the main executable (exectubales are usually private)
#     imgui 
#     glfw 
#     glad
#     ALILQGames)

# target_link_libraries(DiffDrive_imgui
#     PRIVATE                             # private dependency of the main executable (exectubales are usually private)
#     imgui 
#     glfw 
#     glad
#     ALILQGames)

# target_link_libraries(DiffDrive_imgui_ILQGame
#     PRIVATE                             # private dependency of the main executable (exectubales are usually private)
#     imgui 
#     glfw 
#     glad
#     ALILQGames)
target_link_libraries(DiffDriveFeedbackLin_imgui_ALILQGame
    PRIVATE                             # private dependency of the main executable (exectubales are usually private)
    imgui 
    glfw 
    glad
    ALILQGames)

target_link_libraries(DiffDriveFeedbackLin_imgui_ALILQGame
    PRIVATE                             # private dependency of the main executable (exectubales are usually private)
    imgui 
    glfw 
    glad
    ALILQGames)

target_link_libraries(DiffDrive_imgui_ALILQGame
    PRIVATE                             # private dependency of the main executable (exectubales are usually private)
    imgui 
    glfw 
    glad
    ALILQGames)

target_link_libraries(3PlayerDiffDriveImguiALILQGame
    PRIVATE                             # private dependency of the main executable (exectubales are usually private)
    imgui 
    glfw 
    glad
    ALILQGames)

target_link_libraries(3PlayerDiffDriveImguiALILQGameReceding
    PRIVATE                             # private dependency of the main executable (exectubales are usually private)
    imgui 
    glfw 
    glad
    ALILQGames)

target_link_libraries(3Player3DModelDiffDriveImguiALILQGameReceding
    PRIVATE                             # private dependency of the main executable (exectubales are usually private)
    imgui 
    glfw 
    glad
    ALILQGames)

target_link_libraries(3AgentALILQGameRHGoalChange
    PRIVATE                             # private dependency of the main executable (exectubales are usually private)
    imgui 
    glfw 
    glad
    ALILQGames)

target_link_libraries(5PlayerDiffDriveImguiALILQGameReceding
    PRIVATE                             # private dependency of the main executable (exectubales are usually private)
    imgui 
    glfw 
    glad
    ALILQGames)

target_link_libraries(5Player3DModelDiffDriveImguiALILQGameReceding
    PRIVATE                             # private dependency of the main executable (exectubales are usually private)
    imgui 
    glfw 
    glad
    ALILQGames)