# Here you choose
ISSUPER = no
ISGUI = no
################

CC = gcc
CFLAGS = -W -Wall -v
EXEC = bin/$(OSBINDIR)$(ONAME)$(SUFFIX)
OBJ = obj/*.o

TESTOS := $(shell echo %OS%)

ifeq ($(TESTOS),Windows NT)
	RM = del /q
	RM = rm -rf
	OS = Windows
	OSBINDIR = win32/
else
	UNAME := $(shell uname)
	RM = rm -rf
	OS = $(UNAME)
	OSBINDIR = linux/
endif

ifeq ($(OS),Windows)
	SUFFIX = .exe
	WINSOCKSLIB = -lws2_32
	PCAPWIN = -lwpcap
	MATHLIB = 
else
	SUFFIX =
	WINSOCKSLIB =
	PCAPWIN = -lpcap
	MATHLIB = -lm
endif


VARS = -D_REENTRANT
LIBS = $(WINSOCKSLIB) $(MATHLIB) -lpthread -Linclude/libircclient/lib/ -lircclient

GTK_LIBS = -Linclude/gtk/lib -lgobject-2.0 -lglib-2.0 -lgdk-win32-2.0 -lgtk-win32-2.0 -lgthread-2.0
GTK_INC = -Iinclude/gtk/include/atk-1.0 -Iinclude/gtk/include -Iinclude/gtk/include/gdk-pixbuf-2.0 -Iinclude/gtk/include/cairo -Iinclude/gtk/include/pango-1.0 -Iinclude/gtk/include/glib-2.0 -Iinclude/gtk/include/gtk-2.0

SUPER_LIBS = -Linclude/libpcap/Lib $(PCAPWIN)
SUPER_INC = -Iinclude/libpcap/Include

ifeq ($(ISSUPER), yes)
	SUPER_FLAGS = -DSUPER_LOIC $(SUPER_INC)
else
	SUPER_FLAGS = 
endif

ifeq ($(ISGUI), yes)
	GTK_FLAGS = -DGTK_GUI -mms-bitfields $(GTK_INC)
else
	GTK_FLAGS = 
endif

ifeq ($(ISGUI), yes)
	ifeq ($(ISSUPER), yes)
		ONAME = SuperLoicGui
		TARGET = gui
	else
		ONAME = SimpleLoicGui
		TARGET = simple_gui
	endif
else
	ifeq ($(ISSUPER), yes)
		ONAME = SuperLoic
		TARGET = cli
	else
		ONAME = SimpleLoic
		TARGET = simple_console
	endif
endif

all: $(TARGET)
	@echo $(UNAME)
	@echo target was $(TARGET)________________________________________________________
	@echo detection result was $(TESTOS)

clean:
	$(RM) obj/*.o
	

simple_console: clean core main.o
	$(CC) $(OBJ) $(VARS) $(LIBS) -o $(EXEC)
	
super_console: clean S_core superGears.o superCanon.o main.o
	$(CC) $(OBJ) $(VARS) $(SUPER_FLAGS) $(LIBS) $(SUPER_LIBS) -o $(EXEC)
	
console: super_console
cli: console
simple: simple_console




simple_gui: clean core gtk_module.o mainGtk.o
	$(CC) $(OBJ) $(VARS) $(LIBS) $(GTK_LIBS) $(GTK_FLAGS) -o $(EXEC)
	
super_gui: clean S_core superGears.o superCanon.o gtk_module.o mainGtk.o
	$(CC) $(OBJ) $(VARS) $(LIBS) $(SUPER_FLAGS) $(SUPER_LIBS) $(GTK_FLAGS) $(GTK_LIBS) -o $(EXEC)
	
gui: super_gui
gtk: gui

	
	
core: gears.o canon.o hivemind.o overlord.o config.o
S_core: gears.o canon.o hivemind.o overlord.o S_config.o

main.o:
	$(CC) -c core_src/main.c -o obj/main.o $(SUPER_FLAGS) $(GTK_FLAGS) $(CFLAGS)
	
gears.o:
	$(CC) -c core_src/gears.c -o obj/gears.o $(SUPER_FLAGS) $(GTK_FLAGS) $(CFLAGS)
	
canon.o:
	$(CC) -c core_src/canon.c -o obj/canon.o $(SUPER_FLAGS) $(GTK_FLAGS) $(CFLAGS)
	
hivemind.o:
	$(CC) -c core_src/hivemind.c -o obj/hivemind.o $(SUPER_FLAGS) $(GTK_FLAGS) $(CFLAGS)
	
overlord.o:
	$(CC) -c core_src/overlord.c -o obj/overlord.o $(SUPER_FLAGS) $(GTK_FLAGS) $(CFLAGS)
	
	
	
config.o:
	$(CC) -c core_src/config.c -o obj/config.o $(SUPER_FLAGS) $(GTK_FLAGS) $(CFLAGS) 
	
S_config.o:
	$(CC) -c core_src/config.c -o obj/config.o  $(SUPER_FLAGS) $(GTK_FLAGS) $(CFLAGS) 
	
	
	
	
superCanon.o:
	$(CC) -c core_src/superCanon.c -o obj/superCanon.o $(SUPER_FLAGS) $(GTK_FLAGS) $(CFLAGS)

superGears.o:
	$(CC) -c core_src/superGears.c -o obj/superGears.o $(SUPER_FLAGS) $(GTK_FLAGS) $(CFLAGS)

	
	
mainGtk.o:
	$(CC) -c core_src/mainGtk.c -o obj/mainGtk.o $(GTK_INC) $(GTK_FLAGS) $(SUPER_FLAGS) $(GTK_FLAGS) $(CFLAGS)
	
gtk_module.o:
	$(CC) -c core_src/gtk_module.c -o obj/gtk_module.o  $(GTK_INC) $(SUPER_FLAGS) $(GTK_FLAGS) $(CFLAGS) 
