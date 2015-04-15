cmake_minimum_required(VERSION 2.8)

set(HEADERS
  ../html/houdini.h
  ../html/html.h
  ../src/autolink.h
  ../src/buffer.h
  ../src/html_blocks.h
  ../src/html_entities.h
  ../src/markdown.h
  ../src/stack.h
  )
set(LIBRARY_SOURCES
  ../html/houdini_href_e.c
  ../html/houdini_html_e.c
  ../html/html.c
  ../html/html_smartypants.c
  ../src/autolink.c
  ../src/buffer.c
  ../src/markdown.c
  ../src/stack.c
  ${HEADERS}
  )

set(PROGRAM "snudown-validator")
set(PROGRAM_SOURCES
  ${LIBRARY_SOURCES}
  snudown-validator.c
  )

include_directories(. ../src ../html ./build/gumbo_snudown/include ${CMAKE_CURRENT_BINARY_DIR})
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/build/gumbo_snudown/lib)

add_executable(${PROGRAM} ${PROGRAM_SOURCES})
target_link_libraries(${PROGRAM} gumbo)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -g -Wno-error=parentheses")
