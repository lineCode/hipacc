set(ClassRepresentation_SOURCES ClassRepresentation.cpp)

add_library(hipaccClassRepresentation ${ClassRepresentation_SOURCES})
add_cppcheck(hipaccClassRepresentation POSSIBLE_ERROR FAIL_ON_WARNINGS)
