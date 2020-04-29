CXX=g++ -std=c++17
RM=rm -f
RMDIR=rm -rf
HFILES:=$(shell find src/ -type f -iname *.h -print)
CXXFILES:=$(shell find src/ -type f -iname *.cpp -print)
GTKFLAGS=$(shell pkg-config gtkmm-3.0 --cflags --libs)
CXXFLAGS=$(GTKFLAGS) -Wall -lsteam_api -lcurl -lyajl -ldl
LDFLAGS=-L${CURDIR}/bin
OBJDIR=obj
OBJS=$(addprefix ${OBJDIR}/,$(subst .cpp,.o,${CXXFILES}))

all: CXXFLAGS += -O3
all: init
all: ${CURDIR}/bin/samrewritten
	@echo -e "==== Use '\033[1mmake dev\033[0m' to keep debug symbols"
	@echo -e "==== Use '\033[1mmake clean\033[0m' to remove object files"
	@echo -e "==== Nothing left to do."

dev: CXXFLAGS += -g -DDEBUG_CERR
dev: init
dev: ${CURDIR}/bin/samrewritten

init:
	git config core.hooksPath .githooks
	@echo "Hooks set to .githooks"

clean:
	${RMDIR} ${OBJDIR}

${CURDIR}/bin/samrewritten: $(OBJS)
	${CXX} -o ${CURDIR}/bin/samrewritten $(OBJS) ${LDFLAGS} ${CXXFLAGS}

${OBJDIR}/%.o: %.cpp $(HFILES)
	@mkdir -p $$(dirname $@)
	$(CXX) -c -o $@ $< ${LDFLAGS} $(CXXFLAGS)
