CXX=g++
CXXFILES=$$(find ${CURDIR}/src -type f -iname *.cpp -print)
CFLAGS=-std=c++17 -g $$(pkg-config --cflags --libs gtk+-3.0) -rdynamic -export-dynamic -pthread -Wall -lpthread -lgmodule-2.0 -lsteam_api -lcurl -lyajl -ldl
LDFLAGS=-L${CURDIR}/bin

$(MAKE_PATH)/bin/samrewritten:
	${CXX} ${CFLAGS} ${CXXFILES} ${LDFLAGS} -o ${CURDIR}/bin/samrewritten
