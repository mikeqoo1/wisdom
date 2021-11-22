#include "server.hpp"

int main(int argc, char *argv[])
{
    Server serv(6666);
    serv.servRun();
    return 0;
}
