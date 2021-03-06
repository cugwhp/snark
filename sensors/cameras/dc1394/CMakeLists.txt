SET( PROJECT "dc1394" )
SET( TARGET_NAME snark_${PROJECT} )

FILE( GLOB source *.cpp )
FILE( GLOB includes *.h )

SOURCE_GROUP( ${TARGET_NAME} FILES ${source} ${includes} )

ADD_LIBRARY( ${TARGET_NAME} ${source} ${includes} )
SET_TARGET_PROPERTIES( ${TARGET_NAME} PROPERTIES ${snark_LIBRARY_PROPERTIES} )
TARGET_LINK_LIBRARIES( ${TARGET_NAME} ${comma_ALL_LIBRARIES} ${snark_ALL_EXTERNAL_LIBRARIES} dc1394 )

INSTALL( FILES ${includes} DESTINATION ${snark_INSTALL_INCLUDE_DIR}/sensors/cameras/${PROJECT} )
INSTALL(
   TARGETS ${TARGET_NAME}
   RUNTIME DESTINATION ${snark_INSTALL_BIN_DIR} COMPONENT Runtime     # .exe, .dll
   LIBRARY DESTINATION ${snark_INSTALL_LIB_DIR} COMPONENT Runtime     # .so, mod.dll
   ARCHIVE DESTINATION ${snark_INSTALL_LIB_DIR} COMPONENT Development # .a, .lib
)

ADD_SUBDIRECTORY( applications )
