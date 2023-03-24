#ifdef _WIN32
//#include <vld.h>
#endif // _WIN32

#include "Launcher.h"
#include "spdlogging.h"

int main(int argc, char *argv[])
{
    Launcher srv;

    int rc = srv.init(argc, argv);
    if (0 != rc)
    {
        SPDERROR("[main] service init error, rc:{}", rc);
        return 1;
    }
    rc = srv.open();
    if (0 != rc)
    {
        SPDERROR("[main] service open error, rc:{}", rc);
        return 1;
    }

    srv.close();

    srv.finit();

    return 0;
}