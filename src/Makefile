
CC= gcc -std=gnu99
#CFLAGS= -p -g -Wall -Wextra -DWITH_LOG -I../inc `pkg-config --cflags glib-2.0`
CFLAGS= -O2 -Wall -Wextra -DWITH_LOG -I../inc `pkg-config --cflags glib-2.0`
LDFLAGS= -L../lib
LIBS= -lm -levent -lzlog -lmosquitto -lcurl -lpaho-mqtt3c `pkg-config --libs glib-2.0`


RM= rm -f


OBJS= main.o server_mc.o log.o msg_sch_mc.o msg_proc_mc.o object_mc.o msg_mc.o leancloud_req.o leancloud_rsp.o cJSON.o yeelink_req.o yeelink_rsp.o curl.o mqtt.o msg_proc_app.o yunba_push.o env.o slb.o
TARGET= ../bin/server

all: $(TARGET)
$(TARGET): $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $(OBJS) $(LIBS) 

clean:
	$(RM) $(TARGET) $(OBJS)



# list targets that do not create files (but not all makes understand .PHONY)
.PHONY: all clean

# objects
main.o: main.c server_mc.h version.h
server_mc.o: server_mc.c server_mc.h curl.h log.h
log.o: log.c log.h
msg_sch_mc.o: msg_sch_mc.c msg_sch_mc.h msg_proc_mc.h msg_mc.h
msg_proc_mc.o: msg_proc_mc.c msg_proc_mc.h log.h object_mc.h msg_mc.h leancloud_req.h yeelink_req.h 
object_mc.o: object_mc.c object_mc.h
msg_mc.o: msg_mc.c msg_mc.h
leancloud_req.o: leancloud_req.c leancloud_req.h leancloud_rsp.h cb_ctx_mc.h object_mc.h log.h curl.h
leancloud_rsp.o: leancloud_rsp.c leancloud_rsp.h cb_ctx_mc.h object_mc.h log.h
yeelink_req.o: yeelink_req.c yeelink_req.h yeelink_rsp.h cb_ctx_mc.h object_mc.h log.h curl.h
yeelink_rsp.o: yeelink_rsp.c yeelink_rsp.h cb_ctx_mc.h object_mc.h log.h
