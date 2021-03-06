cmake_minimum_required (VERSION 2.8)
project (leechcraft_azoth_otroid)
include (InitLCPlugin OPTIONAL)

include (${QT_USE_FILE})

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package (LibOTR REQUIRED)
message (STATUS "Found OTR: ${LIBOTR_LIBRARY}")

include_directories (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${LIBOTR_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)
set (OTROID_SRCS
	otroid.cpp
	)
set (OTROID_RESOURCES azothotroidresources.qrc)
CreateTrs ("azoth_otroid" "en;ru_RU;uk_UA" OTROID_COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("azoth_otroid" "en;ru_RU" "${OTROID_SRCS}" "" "")
QT4_ADD_RESOURCES (OTROID_RCCS ${OTROID_RESOURCES})

add_library (leechcraft_azoth_otroid SHARED
	${OTROID_COMPILED_TRANSLATIONS}
	${OTROID_SRCS}
	${OTROID_RCCS}
	)
target_link_libraries (leechcraft_azoth_otroid
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${LIBOTR_LIBRARY}
	leechcraft_azoth_common
	)
install (TARGETS leechcraft_azoth_otroid DESTINATION ${LC_PLUGINS_DEST})
#install (FILES azothotroidsettings.xml DESTINATION ${LC_SETTINGS_DEST})
