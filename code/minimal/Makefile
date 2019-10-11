ifndef SYSTEMCROOT
$(error SYSTEMCROOT is not set)
endif

minimal: minimal.cpp
	g++ minimal.cpp -Wall -Wno-deprecated \
		-I $(SYSTEMCROOT)/include \
		-L $(SYSTEMCROOT)/lib-gccsparcOS5/ \
		-L $(SYSTEMCROOT)/lib-linux/ \
		-L $(SYSTEMCROOT)/lib-linux64/ \
		-Xlinker -Bstatic -lsystemc -Xlinker -Bdynamic -pthread \
	-o minimal
