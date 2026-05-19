EXTRA_CFLAGS += -I/run/media/isaacb/BAC01E20C01DE403/tracy/public -DTRACY_ENABLE
TRACY_OBJ := build/us_pc/src/tracy/TracyInterface.o
O_FILES += $(TRACY_OBJ)

$(TRACY_OBJ): src/tracy/TracyInterface.cpp
	mkdir -p build/us_pc/src/tracy
	$(CXX) -c src/tracy/TracyInterface.cpp -o $@ -I/run/media/isaacb/BAC01E20C01DE403/tracy/public -DTRACY_ENABLE
