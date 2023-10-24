CC = g++
CFLAGS = -c -g -Wshadow -Winit-self -Wredundant-decls -Wcast-align\
-Wundef -Wfloat-equal -Winline -Wunreachable-code -Wmissing-declarations\
-Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Weffc++ -Wmain -Wextra\
-Wall -g -pipe -fexceptions -Wcast-qual -Wconversion -Wctor-dtor-privacy\
-Wempty-body -Wformat-security -Wformat=2 -Wignored-qualifiers -Wlogical-op\
-Wno-missing-field-initializers -Wnon-virtual-dtor -Woverloaded-virtual -Wpointer-arith\
-Wsign-promo -Wstack-usage=8192 -Wstrict-aliasing -Wstrict-null-sentinel\
-Wtype-limits -Wwrite-strings -Werror=vla -D_DEBUG -D_EJUDGE_CLIENT_SIDE

all: compiler.exe discompiler.exe executor.exe

compiler.exe: compiler.o
	$(CC) compiler.o -o compiler.exe

compiler.o: compiler.cpp
	$(CC) $(CFLAGS) compiler.cpp

discompiler.exe: discompiler.o
	$(CC) discompiler.o -o discompiler.exe

discompiler.o: discompiler.cpp
	$(CC) $(CFLAGS) discompiler.cpp

executor.exe: executor.o stack.o
	$(CC) executor.o stack.o -o executor.exe

executor.o: executor.cpp
	$(CC) $(CFLAGS) executor.cpp

stack.o: stack.cpp
	$(CC) $(CFLAGS) stack.cpp

.PHONY: clean all

clean:
	del compiler.exe discompiler.exe executor.exe
	del compiler.o discompiler.o executor.o
