cmake_minimum_required (VERSION 2.8)
project (leechcraft_lmp)
include (InitLCPlugin OPTIONAL)
include (CMakeDependentOption)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")

# This stuff won't be need by GStreamer 1.0.
find_package (LibXml2 REQUIRED)
find_package (GStreamer REQUIRED)
find_package (GLIB2 REQUIRED)

find_package (Taglib REQUIRED)
add_definitions (${TAGLIB_CFLAGS})

cmake_dependent_option (ENABLE_LMP_MPRIS "Enable MPRIS support for LMP" ON "NOT WIN32" OFF)

option (ENABLE_LMP_LIBGUESS "Enable tags recoding using the LibGuess library" ON)
if (ENABLE_LMP_LIBGUESS)
	find_package (LibGuess REQUIRED)
	add_definitions (-DWITH_LIBGUESS)
endif ()

set (QT_USE_QTDECLARATIVE TRUE)
set (QT_USE_QTNETWORK TRUE)
set (QT_USE_QTSQL TRUE)
set (QT_USE_QTXML TRUE)
if (ENABLE_LMP_MPRIS)
	set (QT_USE_QTDBUS TRUE)
endif ()
include (${QT_USE_FILE})

include_directories (
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${TAGLIB_INCLUDES}
	${GSTREAMER_INCLUDE_DIR}
	${GLIB2_INCLUDE_DIR}
	${LIBXML2_INCLUDE_DIR}
	${LibGuess_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (COMMON_SRCS
	mediainfo.cpp
	)
add_library (leechcraft_lmp_common STATIC
	${COMMON_SRCS}
	)
set_target_properties (leechcraft_lmp_common PROPERTIES POSITION_INDEPENDENT_CODE True)

set (SRCS
	lmp.cpp
	xmlsettingsmanager.cpp
	playertab.cpp
	player.cpp
	core.cpp
	localfileresolver.cpp
	playlistdelegate.cpp
	localcollection.cpp
	localcollectionstorage.cpp
	util.cpp
	collectiontypes.cpp
	collectiondelegate.cpp
	nowplayingwidget.cpp
	staticplaylistmanager.cpp
	playlistmanager.cpp
	similarview.cpp
	albumartmanager.cpp
	lmpsystemtrayicon.cpp
	fsbrowserwidget.cpp
	fsmodel.cpp
	rootpathsettingsmanager.cpp
	localcollectionwatcher.cpp
	recommendationswidget.cpp
	radiowidget.cpp
	releaseswidget.cpp
	playlistundocommand.cpp
	biowidget.cpp
	biopropproxy.cpp
	audiopropswidget.cpp
	playlistwidget.cpp
	aalabeleventfilter.cpp
	collectionsortermodel.cpp
	collectionstatsdialog.cpp
	eventswidget.cpp
	plmanagerwidget.cpp
	nowplayingpixmaphandler.cpp
	lmpproxy.cpp
	sortingcriteria.cpp
	sortingcriteriadialog.cpp
	similarmodel.cpp
	hypeswidget.cpp
	previewhandler.cpp
	bioviewmanager.cpp
	similarviewmanager.cpp
	artistbrowsertab.cpp
	albumartmanagerdialog.cpp
	progressmanager.cpp
	volumeslider.cpp
	seekslider.cpp
	palettefixerfilter.cpp
	playlistmodel.cpp
	volumenotifycontroller.cpp
	radiomanager.cpp
	radiocustomstreams.cpp
	radiocustomstation.cpp
	radiocustomdialog.cpp
	stdartistactionsmanager.cpp
	engine/audiosource.cpp
	engine/output.cpp
	engine/path.cpp
	engine/sourceobject.cpp
	sync/cloudwidget.cpp
	sync/clouduploadmanager.cpp
	sync/clouduploader.cpp
	sync/copymanager.cpp
	sync/devicesbrowserwidget.cpp
	sync/formats.cpp
	sync/syncmanagerbase.cpp
	sync/syncmanager.cpp
	sync/syncunmountablemanager.cpp
	sync/transcodejob.cpp
	sync/transcodemanager.cpp
	sync/transcodingparams.cpp
	sync/transcodingparamswidget.cpp
	sync/unmountabledevmanager.cpp
	sync/uploadmodel.cpp
	playlistparsers/commonpl.cpp
	playlistparsers/playlistfactory.cpp
	playlistparsers/m3u.cpp
	playlistparsers/pls.cpp
	playlistparsers/xspf.cpp
	)
set (FORMS
	playertab.ui
	nowplayingwidget.ui
	fsbrowserwidget.ui
	recommendationswidget.ui
	radiowidget.ui
	releaseswidget.ui
	biowidget.ui
	audiopropswidget.ui
	playlistwidget.ui
	collectionstatsdialog.ui
	eventswidget.ui
	plmanagerwidget.ui
	sortingcriteriadialog.ui
	hypeswidget.ui
	artistbrowsertab.ui
	albumartmanagerdialog.ui
	volumeslider.ui
	seekslider.ui
	radiocustomdialog.ui
	sync/cloudwidget.ui
	sync/devicesbrowserwidget.ui
	sync/transcodingparamswidget.ui
	)
set (RESOURCES resources.qrc)
CreateTrs ("lmp" "en;es;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("lmp" "en;ru_RU" "${SRCS}" "${FORMS}" "lmpsettings.xml")
QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

add_library (leechcraft_lmp SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	${RCCS}
	)
if (ENABLE_LMP_MPRIS)
	add_subdirectory (mpris)
	add_definitions (-DENABLE_MPRIS)
endif ()
target_link_libraries (leechcraft_lmp
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${TAGLIB_LIBRARIES}
	${GSTREAMER_LIBRARIES}
	${GLIB2_LIBRARIES}
	${LIBXML2_LIBRARIES}
	${LibGuess_LIBRARIES}

	${MPRIS_SUBLIB}
	leechcraft_lmp_common
	)

install (TARGETS leechcraft_lmp DESTINATION ${LC_PLUGINS_DEST})
install (FILES lmpsettings.xml DESTINATION ${LC_SETTINGS_DEST})
install (DIRECTORY interfaces DESTINATION include/leechcraft)
if (UNIX AND NOT APPLE)
	install (FILES freedesktop/leechcraft-lmp.desktop DESTINATION share/applications)
endif ()

option (ENABLE_LMP_DUMBSYNC "Enable DumbSync, plugin for syncing with Flash-like media players" ON)
option (ENABLE_LMP_GRAFFITI "Enable Graffiti for manipulating audio file tags" ON)
option (ENABLE_LMP_MP3TUNES "Enable plugin for syncing and using the mp3tunes.com service" ON)
option (ENABLE_LMP_MTPSYNC "Enable plugin for syncing with MTP devices" OFF)

if (ENABLE_LMP_DUMBSYNC)
	add_subdirectory (plugins/dumbsync)
endif ()
if (ENABLE_LMP_GRAFFITI)
	add_subdirectory (plugins/graffiti)
endif ()
if (ENABLE_LMP_MP3TUNES)
	add_subdirectory (plugins/mp3tunes)
endif ()
if (ENABLE_LMP_MTPSYNC)
	add_subdirectory (plugins/mtpsync)
endif ()
