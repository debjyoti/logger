CXX       := xlc
CXXFLAGS  := -D_LOG_DEBUG
LINKFLAGS := -D_THREAD_SAFE
INCLUDES  := 
LIBS      := -lpthread
OBJECTS   := log_level4_traceOn.o logger.o file_change.o

log_level4_traceOn: $(OBJECTS)
	$(CXX) $(LINKFLAGS) $(OBJECTS) -o log_level4_traceOn $(LIBS)

file_change: file_change.o logger.o
	$(CXX) $(LINKFLAGS) logger.o file_change.o -o file_change $(LIBS)

log_level4_traceOn.o: log_level4_traceOn.c
	$(CXX) $(INCLUDES) $(CXXFLAGS) -c log_level4_traceOn.c -o log_level4_traceOn.o

file_change.o: file_change.c
	$(CXX) $(INCLUDES) $(CXXFLAGS) -c file_change.c -o file_change.o

logger.o: ../logger.c
	$(CXX) $(CXXFLAGS) -c ../logger.c -o logger.o

clean:
	rm *.o log_level4_traceOn
