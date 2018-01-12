#include "SocketCanPort.h"

//Constructor vacío (inicializa el can0)

SocketCanPort::SocketCanPort()
{
    Init( "can0" );
}

//Inicializa todos los puertos que introduzcas

SocketCanPort::SocketCanPort(string canPort)
{

    Init(canPort);

}

SocketCanPort::~SocketCanPort()
{
    //shutdown(portFD,0);
    close(portFD);

}

long SocketCanPort::SetFilter(uint32_t canId, uint32_t mask)
{
    //check for socket
    if(portFD<=0)
    {
        cerr << "Socket not open" << endl;
        return -1;
    }
    //filter add
    can_filter rfilter1[1];
    rfilter1[0].can_id   = canId;
    rfilter1[0].can_mask = mask;
    setsockopt(portFD, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter1, sizeof(rfilter1));
    return 0;
}

long SocketCanPort::GetMsg(uint32_t &canId, uint8_t *data, uint8_t size)
{
    //get a message from canbus and store in canId and data parameters.
    //in socketcan, only one device will be listened, given a port, trough SetFilter
    //blocking. It will wait until filtered message arrives

    if (poll(poll_set, 1, 10000))
    {

//        ioctl(portFD, FIONREAD, &buff_size);
//        cout << "fion: " << buff_size;
//        nbytes = 0;
//        for (buff_size; buff_size>0; buff_size--)
//        {
            nbytes += read(portFD, &frame, sizeof(struct can_frame));
            //return first parameter
            canId = frame.can_id;
            //return second parameter
            memcpy ( data, frame.data, frame.can_dlc );
//        }
        return nbytes;
    }
    else
    {
        return -1;
    }

}

long SocketCanPort::PutMsg(const uint32_t &canId, uint8_t * const data, uint8_t const data_size)
{
    frame.can_id = canId;

    memcpy ( frame.data,  data, data_size );
    frame.can_dlc = data_size;
//    cout << "PutMsg: " << hex << frame.can_id << dec << "  | ";
//    cout << "data: " << hex << frame.data[0] << dec << endl;

    nbytes = write(portFD, &frame, sizeof(struct can_frame) );

    /* send frame */
    if (nbytes != sizeof(struct can_frame))
    {
        perror("Write Failed");
        return 1;
    }
    return nbytes;

}


long SocketCanPort::Init(string canPort)
{
    struct sockaddr_can addr;
    struct can_frame frame;
    struct ifreq ifr;

    portFD  = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if(portFD < 0)
    {
        perror("Error while opening socket \n "
               "try: sudo ip link set can0 up txqueuelen 1000 type can bitrate 1000000");
        return -1;
    }

    canPort.copy(ifr.ifr_name,canPort.size());
    //CHECK THIS!!!
    ifr.ifr_name[canPort.size()]=NULL;
    printf("%s ifr %d\n", ifr.ifr_name, ifr.ifr_ifindex);

    ioctl(portFD, SIOCGIFINDEX, &ifr);

    addr.can_family  = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    printf("%s at index %d\n", canPort.c_str(), ifr.ifr_ifindex);

    if(bind(portFD, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("Error in socket bind");
        return -2;
    }

    poll_set[0].fd = portFD;
    poll_set[0].events = POLLIN;

    return 0;
}
