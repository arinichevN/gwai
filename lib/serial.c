
#include "serial.h"

int serial_init(int *fd, const char *device, const int baud, const char *config) {
    *fd = serial_open(device, baud, config);
    if (*fd < 0) {
        return 0;
    }
    return 1;
}

static int baudIntToInternal(const int baud){
	switch (baud) {
		case 0:
			return B0;
        case 50:
            return B50;
        case 75:
            return B75;
        case 110:
            return B110;
        case 134:
            return B134;
        case 150:
            return B150;
        case 200:
            return B200;
        case 300:
            return B300;
        case 600:
            return B600;
        case 1200:
            return B1200;
        case 1800:
            return B1800;
        case 2400:
            return B2400;
        case 4800:
            return B4800;
        case 9600:
            return B9600;
        case 19200:
            return B19200;
        case 38400:
            return B38400;
        case 57600:
            return B57600;
        case 115200:
            return B115200;
        case 230400:
            return B230400;   
    }
    return -2;
}

static int dataBitsIntToInternal(const char dbits){
	switch (dbits) {
        case 5:
            return CS5;
        case 6:
            return CS6;
        case 7:
            return CS7;
        case 8:
            return CS8;    
    }
    return -3;
}
static int checkParity(const char p){
	switch (p) {
        case 'N':
        case 'O':
        case 'E':
        case 'n':
        case 'o':
        case 'e':
            return 1;
    }
    return 0;
}
static int checkStopBits(const char sb){
	switch (sb) {
        case '1':
        case '2':
            return 1;
    }
    return 0;
}
int serial_open(const char *device, const int baud, const char *config) {
    struct termios options;
    speed_t _baud;
    int status, fd;
	_baud = baudIntToInternal(baud);
    if(_baud < 0){
		putsde("bad baud");
		return -1;
	}
	char b, p, sb;
	int n = sscanf(config, "%c%c%c", &b, &p, &sb );
	if(n!= 3){
		putsde("bad config string");
		return -1;
	}
	int _db = dataBitsIntToInternal(b);
	if(b < 0){
		printde("bad data bits: found %hhd, but expected one of 5,6,7,8)", b);
		return 0;
	}
	if(!checkParity(p)){
		printde("bad parity: found %hhd, but expected one of N, O, E, n, o, e)", p);
		return -1;
	}
	if(!checkStopBits(sb)){
		printde("bad stop bits: found %hhd, but expected one of 1, 2)", sb);
		return -1;
	}
    if ((fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK)) == -1) {
        printde("failed to open: %s\n", device);
        perrord("open()");
        return -1;
    }

    fcntl(fd, F_SETFL, O_RDWR);

    tcgetattr(fd, &options);

    cfmakeraw(&options);
    cfsetispeed(&options, _baud);
    cfsetospeed(&options, _baud);

    options.c_cflag |= (CLOCAL | CREAD);
    //parity
    switch(p){
		case 'N':
        case 'n':
	        options.c_cflag &= ~PARENB;
	        break;
        case 'E':
        case 'e':
	        options.c_cflag |= PARENB;
			options.c_cflag &= ~PARODD;
	        break;
        case 'O':
        case 'o':
	        options.c_cflag |= PARENB;
			options.c_cflag |= PARODD;
	        break;
	    default:
		    options.c_cflag &= ~PARENB;
		    break;
	}
	//stop bits
	switch (sb) {
        case '1':
	        options.c_cflag &= ~CSTOPB;
	        break;
        case '2':
			options.c_cflag |= CSTOPB;
			break;
		default:
			options.c_cflag &= ~CSTOPB;
	        break;
    }
    //data bits
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= _db;
    
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);//raw input
    options.c_oflag &= ~OPOST;//output options (raw output)

    options.c_cc [VMIN] = 1;//minimum characters to read in each read call
    options.c_cc [VTIME] = 3;//read timeout between characters as 100 ms

    tcsetattr(fd, TCSANOW | TCSAFLUSH, &options);

    ioctl(fd, TIOCMGET, &status);

    status |= TIOCM_DTR;
    status |= TIOCM_RTS;

    //ioctl(fd, TIOCMSET, &status);//drop dtr signal

    usleep(10000); // 10mS

    return fd;
}

void serial_flush(const int fd) {
    tcflush(fd, TCIOFLUSH);
}

int serial_canRead(int fd, int timeout_ms){
	struct pollfd fds[1];
	/* watch stdin for input */
	fds[0].fd = fd;
	fds[0].events = POLLIN;
	/* All set, block! */
	int ret = poll (fds, 2, timeout_ms);
	if (ret == -1) {
		perrord ("poll");
		return 0;
	}
	if (!ret) {
		printde ("%d ms elapsed.\n", timeout_ms);
		return 0;
	}
	if (fds[0].revents & POLLIN){
		return 1;
	}
	putsde ("no data to read\n");
	return 0;
}

int serial_canWrite(int fd, int timeout_ms){
	struct pollfd fds[1];
	/* watch stdin for input */
	fds[0].fd = fd;
	fds[0].events = POLLOUT;
	/* All set, block! */
	int ret = poll (fds, 2, timeout_ms);
	if (ret == -1) {
		perrord ("poll");
		return 0;
	}
	if (!ret) {
		printde ("%d ms elapsed.\n", timeout_ms);
		return 0;
	}
	if (fds[0].revents & POLLOUT){
		return 1;
	}
	return 0;
}

int serial_puts(const int fd, char *str) {
   // printdo("serialPuts: %s\n", str);
    size_t n, sn;
    sn = strlen(str);
    n = write(fd, str, strlen(str));
    if (n < sn) {
        return 0;
    }
    return 1;
}

size_t serial_read(int fd, void *buf, size_t buf_size) {
    uint8_t x;
    size_t i = 0;
    uint8_t * b = (uint8_t *) buf;
    while (i < buf_size && read(fd, &x, 1) == 1) {
        b[i] = x;
        i++;
    }
	return i;
}

size_t serial_readUntil(int fd, char *buf, size_t buf_size, char end) {
    char x;
    size_t c = 0;
    while (c < buf_size && read(fd, &x, 1) == 1) {
        buf[c] = x;
        c++;
        if(x == end) break;
    }
    tcflush(fd, TCIFLUSH);//flushes data received but not read
	return c;
}

void serial_readAll(int fd) {
    char x;
    while (read(fd, &x, 1) == 1) {
        ;
    }
}



