cmake_minimum_required (VERSION 2.8)
project (leechcraft_azoth_rosenthal)
include (InitLCPlugin OPTIONAL)

include (${QT_USE_FILE})

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package (Hunspell REQUIRED)
include_directories (${HUNSPELL_INCLUDE_DIR})

if (MSVC)
	add_definitions (-DWIN32 -DHUNSPELL_STATIC)
endif ()
include_directories (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)
set (ROSENTHAL_SRCS
	rosenthal.cpp
	highlighter.cpp
	xmlsettingsmanager.cpp
	)
set (ROSENTHAL_RESOURCES azothrosenthalresources.qrc)
CreateTrs ("azoth_rosenthal" "en;es;ru_RU;uk_UA" ROSENTHAL_COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("azoth_rosenthal" "en;ru_RU" "${ROSENTHAL_SRCS}" "" "azothrosenthalsettings.xml")
QT4_ADD_RESOURCES (ROSENTHAL_RCCS ${ROSENTHAL_RESOURCES})

add_library (leechcraft_azoth_rosenthal SHARED
	${ROSENTHAL_SRCS}
	${ROSENTHAL_COMPILED_TRANSLATIONS}
	${ROSENTHAL_RCCS}
	${R_HUNSPELL_SRCS}
	)
target_link_libraries (leechcraft_azoth_rosenthal
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${HUNSPELL_LIBRARIES}
	)
install (TARGETS leechcraft_azoth_rosenthal DESTINATION ${LC_PLUGINS_DEST})
install (FILES azothrosenthalsettings.xml DESTINATION ${LC_SETTINGS_DEST})
