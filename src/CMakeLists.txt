add_executable(${PROJ_NAME}
	""
)

target_sources(${PROJ_NAME}
	PRIVATE
		main.cpp
		Interpreter.hpp
		Interpreter.cpp
)

target_include_directories(
    ${PROJ_NAME}
    PRIVATE src/
	PRIVATE ${SDL2_PROJECT_PATH}/include
)

target_link_libraries(
	${PROJ_NAME}
	SDL2
)
	

if(CMAKE_BUILD_TYPE MATCHES "^[Rr]elease")
find_package(Doxygen)
	if(Doxygen_FOUND)
		set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/../docs/Doxyfile.in)
		set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

		configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
		message(STATUS "Doxygen build started.")

		add_custom_target(docs ALL
			COMMAND 
				${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
			WORKING_DIRECTORY
				${CMAKE_CURRENT_BINARY_DIR}
			COMMENT
				"Generating ${PROJ_NAME} documentation."
			VERBATIM
		)
	else()
		message(STATUS "Failed to find Doxygen to generate documentation")
	endif()
endif()
