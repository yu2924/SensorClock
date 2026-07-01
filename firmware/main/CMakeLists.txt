idf_component_register(
    SRCS
	"main.cpp"
	"gui/GuiInit.cpp"
	"modules/AlarmSounder.cpp"
    "modules/AppController.cpp"
    "modules/BoardFs.cpp"
	"modules/DataLogger.cpp"
    "modules/Dispatcher.cpp"
	"modules/HistoryServer.cpp"
    "modules/NetIf.cpp"
    "modules/PropertiesStore.cpp"
    "modules/SensorTask.cpp"
    "modules/WeatherTask.cpp"
    "modules/WebRequest.cpp"
    "utilities/platform.cpp"
	"utilities/xheap.cpp"
	# screen resolution
	# "lv_font_montserrat_latin1x_12.c"
	# "lv_font_montserrat_semibold_latin1x_12.c"
	"lv_font_montserrat_latin1x_14.c"
	"lv_font_montserrat_semibold_latin1x_14.c"
    INCLUDE_DIRS .
	# HTTPS support for HTTPd
	# EMBED_TXTFILES "certs/servercert.pem" "certs/prvtkey.pem"
	)

idf_component_get_property(LVGL_LIB lvgl__lvgl COMPONENT_LIB)
target_compile_options(
    ${LVGL_LIB} 
    PRIVATE
        -DLV_LVGL_H_INCLUDE_SIMPLE
)
