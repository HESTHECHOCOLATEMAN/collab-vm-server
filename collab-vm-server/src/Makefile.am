AUTOMAKE_OPTIONS = foreign 

#AM_CFLAGS = -Werror -Wall -pedantic
AM_CXXFLAGS = -std=c++11 -Iguacamole -Iuriparser -Irapidjson -IAsio -Iwebsocketpp -ISockets -DASIO_STANDALONE -DDATABASE_SQLITE -DLIBODB_STATIC_LIB -DLIBODB_SQLITE_STATIC_LIB -DRAPIDJSON_HAS_STDSTRING

bin_PROGRAMS = collab-vm-server

noinst_HEADERS = \
	AgentProtocol.h	\
	CollabVM.h	\
	CollabVMUser.h	\
	UserList.h	\
	Chat.h	\
	Database/Database.h	\
	Database/Config.h	\
	Database/Config-odb.hxx	\
	Database/VMSettings.h	\
	Database/VMSettings-odb.hxx	\
	VMControllers/QMP.h	\
	VMControllers/VMController.h	\
	VMControllers/QEMUController.h	\
	Sockets/SocketClient.h	\
	Sockets/TCPSocketClient.h	\
	Sockets/LocalSocketClient.h	\
	Sockets/QMPClient.h	\
	Sockets/AgentClient.h	\
	GuacVNCClient.h	\
	GuacBroadcastSocket.h	\
	GuacClient.h	\
	GuacInstructionParser.h	\
	GuacSocket.h	\
	GuacUser.h	\
	GuacWebSocket.h	\
	guacamole/unicode.h	\
	guacamole/user-constants.h	\
	guacamole/user-fntypes.h	\
	guacamole/user-types.h	\
	guacamole/audio.h	\
	guacamole/audio-fntypes.h	\
	guacamole/audio-types.h	\
	guacamole/client-constants.h	\
	guacamole/client-fntypes.h	\
	guacamole/client-types.h	\
	guacamole/config.h	\
	guacamole/error.h	\
	guacamole/error-types.h	\
	guacamole/guac_clipboard.h	\
	guacamole/guac_cursor.h	\
	guacamole/guac_display.h	\
	guacamole/guac_dot_cursor.h	\
	guacamole/guac_iconv.h	\
	guacamole/guac_list.h	\
	guacamole/guac_pointer_cursor.h	\
	guacamole/guac_rect.h	\
	guacamole/guac_string.h	\
	guacamole/guac_surface.h	\
	guacamole/hash.h	\
	guacamole/id.h	\
	guacamole/layer.h	\
	guacamole/layer-types.h	\
	guacamole/palette.h	\
	guacamole/plugin.h	\
	guacamole/plugin-constants.h	\
	guacamole/pool.h	\
	guacamole/pool-types.h	\
	guacamole/protocol.h	\
	guacamole/protocol-types.h	\
	guacamole/socket-constants.h	\
	guacamole/socket-fntypes.h	\
	guacamole/socket-types.h	\
	guacamole/stream.h	\
	guacamole/stream-types.h	\
	guacamole/timestamp.h	\
	guacamole/timestamp-types.h	\
	uriparser/UriDefsConfig.h	\
	uriparser/UriDefsUnicode.h	\
	uriparser/UriIp4.h	\
	uriparser/UriIp4Base.h	\
	uriparser/UriNormalizeBase.h	\
	uriparser/UriParseBase.h	\
	uriparser/Uri.h	\
	uriparser/UriBase.h	\
	uriparser/UriCommon.h	\
	uriparser/UriDefsAnsi.h

collab_vm_server_SOURCES = \
	Main.cpp	\
	CollabVM.cpp	\
	Database/Database.cpp	\
	Database/Config-odb.cxx	\
	Database/VMSettings-odb.cxx	\
	VMControllers/QMP.cpp	\
	VMControllers/QEMUController.cpp	\
	VMControllers/VMController.cpp	\
	Sockets/QMPClient.cpp	\
	Sockets/AgentClient.cpp	\
	guacamole/unicode.cpp	\
	guacamole/error.cpp	\
	guacamole/guac_clipboard.cpp	\
	guacamole/guac_cursor.cpp	\
	guacamole/guac_display.cpp	\
	guacamole/guac_dot_cursor.cpp	\
	guacamole/guac_iconv.cpp	\
	guacamole/guac_list.cpp	\
	guacamole/guac_pointer_cursor.cpp	\
	guacamole/guac_rect.cpp	\
	guacamole/guac_string.cpp	\
	guacamole/guac_surface.cpp	\
	guacamole/hash.cpp	\
	guacamole/id.cpp	\
	guacamole/palette.cpp	\
	guacamole/pool.cpp	\
	guacamole/protocol.cpp	\
	guacamole/timestamp.cpp	\
	GuacSocket.cpp	\
	GuacWebSocket.cpp	\
	GuacBroadcastSocket.cpp	\
	GuacClient.cpp	\
	GuacUser.cpp	\
	GuacVNCClient.cpp	\
	GuacInstructionParser.cpp	\
	uriparser/UriCommon.c	\
	uriparser/UriFile.c	\
	uriparser/UriNormalizeBase.c	\
	uriparser/UriParse.c	\
	uriparser/UriResolve.c	\
	uriparser/UriCompare.c	\
	uriparser/UriIp4Base.c	\
	uriparser/UriNormalize.c	\
	uriparser/UriQuery.c	\
	uriparser/UriShorten.c	\
	uriparser/UriEscape.c	\
	uriparser/UriIp4.c	\
	uriparser/UriParseBase.c	\
	uriparser/UriRecompose.c

#collab_vm_server_LDADD = -lpthread -lpng -luuid -lvncclient -lcairo

collab_vm_server_LDFLAGS = -static-libstdc++

collab_vm_server_LDADD = -lpthread /usr/local/lib/libpng.a /usr/local/lib/libz.a /usr/local/lib/libuuid.a /usr/local/lib/libvncclient.a /usr/local/lib/libturbojpeg.a /usr/local/lib/libcairo.a /usr/local/lib/libpixman-1.a /usr/local/lib/libodb-sqlite.a /usr/local/lib/libodb.a /usr/local/lib/libsqlite3.a -ldl

