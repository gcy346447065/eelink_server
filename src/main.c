#include <event2/event.h>

#include "server_mc.h"
#include "server_gizwits.h"

int main(int argc, char **argv)
{
    struct event_base *base = NULL;

    setvbuf(stdout, NULL, _IONBF, 0);

    base = event_base_new();
    if (!base)
        return 1; /*XXXerr*/

    server_mc_start(base);
    
    server_gizwits_start(base);

    event_base_dispatch(base);

    zlog_fini();

    return 0;
}
