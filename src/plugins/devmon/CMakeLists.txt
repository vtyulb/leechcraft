cmake_minimum_required (VERSION 2.8)
project (leechcraft_devmon)
include (InitLCPlugin OPTIONAL)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package (UDev REQUIRED)

include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${UDEV_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	devmon.cpp
	udevbackend.cpp
	)
#CreateTrs("devmon" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("devmon" "en;ru_RU" "${SRCS}" "${FORMS}" "")

add_library (leechcraft_devmon SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_devmon
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${UDEV_LIBRARIES}
	)
install (TARGETS leechcraft_devmon DESTINATION ${LC_PLUGINS_DEST})
