set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package (Wt REQUIRED)
add_definitions (-DQT_NO_KEYWORDS)

include_directories (${AGGREGATOR_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${Wt_INCLUDE_DIR}
	)
set (WEBACCESS_SRCS
	webaccess.cpp
	servermanager.cpp
	aggregatorapp.cpp
	readchannelsfilter.cpp
	)
set (WEBACCESS_HEADERS
	webaccess.h
	servermanager.h
	aggregatorapp.h
	readchannelsfilter.h
	)
if (NOT LC_NO_MOC)
	QT4_WRAP_CPP (WEBACCESS_MOC_SRCS ${WEBACCESS_HEADERS})
endif ()

add_library (leechcraft_aggregator_webaccess SHARED
	${WEBACCESS_COMPILED_TRANSLATIONS}
	${WEBACCESS_SRCS}
	${WEBACCESS_MOC_SRCS}
	)
target_link_libraries (leechcraft_aggregator_webaccess
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${Wt_LIBRARIES}
	)
install (TARGETS leechcraft_aggregator_webaccess DESTINATION ${LC_PLUGINS_DEST})