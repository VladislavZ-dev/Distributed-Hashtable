# SD-33 Project Makefile
# Lui­s Filipe Pereira dos Santos 58437
# Vladislav Zavgorodnii 59783
# Denis Bahnari 59878

BINDIR = binary
INCDIR = include
OBJDIR = object
SRCDIR = source
LIBDIR = lib

KEEPOBJS = $(OBJDIR)/block.o $(OBJDIR)/entry.o $(OBJDIR)/list.o $(OBJDIR)/table.o

LIBRARY = $(LIBDIR)/libtable.a

CLIENT_EXEC = $(BINDIR)/client_hashtable
SERVER_EXEC = $(BINDIR)/server_hashtable

CLIENT_SRC = $(SRCDIR)/client_hashtable.c $(SRCDIR)/client_network.c $(SRCDIR)/client_stub.c
SERVER_SRC = $(SRCDIR)/server_hashtable.c $(SRCDIR)/server_network.c $(SRCDIR)/server_skeleton.c

COM_SRC = $(SRCDIR)/htmessages.pb-c.c $(SRCDIR)/message.c $(SRCDIR)/stats.c

CLIENT_OBJS = $(CLIENT_SRC:$(SRCDIR)/%.c=$(OBJDIR)/%.o) $(OBJDIR)/htmessages.pb-c.o $(OBJDIR)/message.o $(OBJDIR)/stats.o $(LIBRARY)
SERVER_OBJS = $(SERVER_SRC:$(SRCDIR)/%.c=$(OBJDIR)/%.o) $(OBJDIR)/htmessages.pb-c.o $(OBJDIR)/message.o $(OBJDIR)/stats.o $(LIBRARY)

CC = gcc
CFLAGS = -I$(INCDIR)

.PHONY: all clean libtable client_hashtable server_hashtable

all: libtable $(CLIENT_EXEC) $(SERVER_EXEC)

libtable: $(KEEPOBJS)
	@ar -rcs $(LIBRARY) $(KEEPOBJS)

$(CLIENT_EXEC): $(CLIENT_OBJS)
	@$(CC) -o $@ $^ -lprotobuf-c

$(SERVER_EXEC): $(SERVER_OBJS)
	@$(CC) -o $@ $^ -lprotobuf-c

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(CLIENT_EXEC) $(SERVER_EXEC) $(LIBRARY)
	@find $(OBJDIR) -type f -name '*.o' ! -name 'block.o' ! -name 'entry.o' ! -name 'list.o' ! -name 'table.o' -delete
