//
//  BMD split
//

#include <iostream>
#include "BMDSplit.h"

int main(int argc, const char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Too few arguments" << std::endl;

        const char* exe = argc >= 1 ? argv[0] : "bmdsplit";
        std::cerr << "Usage: " << exe << " <port> [video mode]" << std::endl;

        return 1;
    }

    uint16_t port = static_cast<uint16_t>(atoi(argv[1]));

    int32_t videoMode = 1;

    if (argc >= 3)
    {
        videoMode = atoi(argv[2]);
    }

    cppsocket::Network network;

    BMDSplit bmdSplit(network, port);
    return bmdSplit.run(videoMode) ? 0 : 1;
}
