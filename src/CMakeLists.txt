include(CTest)
enable_testing()

# Segmentation part
add_library(segmentation_lib
  page_segmentation.h
  page_segmentation.cpp
  ccgraph.h
  ccgraph.cpp
  evaluation.h
  evaluation.cpp
  dsu.h
  dsu.cpp
  edge.h
  edge.cpp
)
set_property(TARGET segmentation_lib PROPERTY CXX_STANDARD 17)

add_executable(segmentation
  main_seg.cpp
)
set_property(TARGET segmentation PROPERTY CXX_STANDARD 17)

target_link_libraries(segmentation
  ${OpenCV_LIBS}
  segmentation_lib
)

# Evaluation part
add_library(evaluation_lib
  evaluation.h
  evaluation.cpp
)
set_property(TARGET evaluation_lib PROPERTY CXX_STANDARD 17)

add_executable(evaluation
  main_eval.cpp
)
set_property(TARGET evaluation PROPERTY CXX_STANDARD 17)

target_link_libraries(evaluation
  ${OpenCV_LIBS}
  evaluation_lib
)

# XML parser part
add_executable(xml_parser
  main_parser.cpp
)
set_property(TARGET xml_parser PROPERTY CXX_STANDARD 17)

target_link_libraries(xml_parser
  ${OpenCV_LIBS}
)

# Installation
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/build/bin CACHE PATH ${CMAKE_SOURCE_DIR}/build FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

install(TARGETS segmentation CONFIGURATIONS Release RUNTIME DESTINATION ".")
install(TARGETS evaluation CONFIGURATIONS Release RUNTIME DESTINATION ".")
install(TARGETS xml_parser CONFIGURATIONS Release RUNTIME DESTINATION ".")
install(DIRECTORY ${PROJECT_BINARY_DIR}/doc DESTINATION ".")
install(DIRECTORY ${PROJECT_SOURCE_DIR}/data DESTINATION ".")
install(DIRECTORY DESTINATION "./out")

# Testing
set(TestBinDir ${PROJECT_BINARY_DIR}/bin)
set(TestInDir ${PROJECT_BINARY_DIR}/bin/data/images)
set(TestResDir ${PROJECT_BINARY_DIR}/bin/data/test)
set(TestOutDir ${PROJECT_BINARY_DIR}/bin/out)

set(TestInputs "example1_263"
			   "example2_648"
			   "example3_674"
)
foreach(TestInput ${TestInputs})
  add_test(
    NAME run_${TestInput}
    COMMAND ${TestBinDir}/segmentation.exe ${TestInDir}/${TestInput}.tif ${TestOutDir}
    WORKING_DIRECTORY ${TestBinDir}
  )
  add_test(
    NAME cmp_${TestInput}
    COMMAND ${CMAKE_COMMAND} -E compare_files --ignore-eol ${TestResDir}/${TestInput}.txt ${TestOutDir}/${TestInput}.txt
    WORKING_DIRECTORY ${TestBinDir}
  )
endforeach()
