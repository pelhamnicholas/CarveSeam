PROG := npelh001_program02
CPP_FILES := npelh001_program02.cpp
CC_FLAGS := -W -Wall -Werror -ansi -pedantic
LD_LIBS := -lopencv_core -lopencv_highgui

all: $(PROG)

$(PROG):
	g++ $(CPP_FILES) -o $(PROG) $(LD_LIBS) $(CC_FLAGS)

clean:
	rm $(PROG)
