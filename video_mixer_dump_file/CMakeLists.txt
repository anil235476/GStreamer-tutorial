cmake_minimum_required(VERSION 3.5)
project(videoMixer)
set(EXE_NAME mixer)

add_executable(${EXE_NAME} 
main.cpp

)

add_definitions(/ZI)


# Add some compile flags to our source files.
if(MSVC)
	#set_source_files_properties(${SOURCE_FILES}
	#	PROPERTIES COMPILE_FLAGS "/W3 /WX")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
	
	target_compile_definitions(${EXE_NAME} PUBLIC
	NOMINMAX
)
else()
	set_source_files_properties(${SOURCE_FILES}
		PROPERTIES COMPILE_FLAGS -Wall -Wextra -Wpedantic)
endif()

target_include_directories(${EXE_NAME} 
PRIVATE
D:/gstreamer/1.0/x86_64/include/gstreamer-1.0
D:/gstreamer/1.0/x86_64/include/glib-2.0
D:/gstreamer/1.0/x86_64/lib/glib-2.0/include
)

target_link_directories(${EXE_NAME}
PRIVATE
D:/gstreamer/1.0/x86_64/lib
)
target_link_libraries(${EXE_NAME}
PRIVATE
gstreamer-1.0
glib-2.0
#gsttag-1.0
#gstcheck-1.0
gobject-2.0
)


