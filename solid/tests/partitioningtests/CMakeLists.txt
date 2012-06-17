###################################################################
# This subdirectory contains tests for the partitioning submodule #
###################################################################

# Test for device detection
set(volumedetectiontest_SRCS
    volumedetectiontest.cpp )

kde4_add_unit_test(volumedetectiontest ${volumedetectiontest_SRCS})
add_definitions(-DFAKE_PARTITIONS_XML="\\"${CMAKE_CURRENT_SOURCE_DIR}/../../solid/backends/fakehw/fakepartitions.xml\\"")
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../solid/backends/fakehw )
target_link_libraries(volumedetectiontest ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTTEST_LIBRARY} ${LIBS} solid_static)

# Test for partition formatting
set(partitionformatting_SRCS
    partitionformattingtest.cpp )

kde4_add_unit_test(partitionformattingtest ${partitionformatting_SRCS})
add_definitions(-DFAKE_PARTITIONS_XML="\\"${CMAKE_CURRENT_SOURCE_DIR}/../../solid/backends/fakehw/fakepartitions.xml\\"")
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../solid/backends/fakehw )
target_link_libraries(partitionformattingtest ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTTEST_LIBRARY} ${LIBS} solid_static)

# Test for partition creation
set(partitioncreation_SRCS
    partitioncreationtest.cpp )

kde4_add_unit_test(partitioncreationtest ${partitioncreation_SRCS})
add_definitions(-DFAKE_PARTITIONS_XML="\\"${CMAKE_CURRENT_SOURCE_DIR}/../../solid/backends/fakehw/fakepartitions.xml\\"")
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../solid/backends/fakehw )
target_link_libraries(partitioncreationtest ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTTEST_LIBRARY} ${LIBS} solid_static)

# Test for partition removal
set(partitionremoval_SRCS
    partitionremovaltest.cpp )

kde4_add_unit_test(partitionremovaltest ${partitionremoval_SRCS})
add_definitions(-DFAKE_PARTITIONS_XML="\\"${CMAKE_CURRENT_SOURCE_DIR}/../../solid/backends/fakehw/fakepartitions.xml\\"")
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../solid/backends/fakehw )
target_link_libraries(partitionremovaltest ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTTEST_LIBRARY} ${LIBS} solid_static)