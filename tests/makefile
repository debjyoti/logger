CXX       := gcc
CXXFLAGS  := -D_LOG_DEBUG
LINKFLAGS := -D_THREAD_SAFE
INCLUDES  := 
LIBS      := -lpthread
#OBJECTS  := log_level4_traceOn.o logger.o file_change.o
EXE       :=    \
            file_change \
            level_check \
            no_init
            
all:    $(EXE)

$(EXE): %:%.o logger.o
	$(CXX) $(LINKFLAGS) $^ -o $@ $(LIBS)

.c.o : 
	$(CXX) $(INCLUDES) $(CXXFLAGS) -c $< -o $@

logger.o: ../src/logger.c
	$(CXX) $(CXXFLAGS) -c ../src/logger.c -o logger.o

clean   :
	-rm *.o bin/* out/*
#adding - infront of rm will ignore error (if file not found)

install :
	-mv $(EXE) bin/
