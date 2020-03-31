
#include "serial.h"

int serial_init(int *fd, const char *device, const int baud, const char *config) {
    *fd = serial_open(device, baud, config);
    if (*fd < 0) {
        return 0;
    }
    return 1;
}

static speed_t baudIntToInternal(const int baud){
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

static int baudInternalToInt(const speed_t baud){
	switch (baud) {
		case B0:
			return 0;
		case B50:
             return 50;
		case B75:
            return 75;
		case B110:
            return 110;
		case B134:
            return 134;
		case B150:
            return 150;
		case B200:
            return 200;
		case B300:
            return 300;
		case B600:
            return 600;
		case B1200:
            return 1200;
		case B1800:
            return 1800;
		case B2400:
            return 2400;
		case B4800:
            return 4800;
		case B9600:
            return 9600;
		case B19200:
             return 19200;
		case B38400:
            return 38400;
		case B57600:
            return 57600;
		case B115200:
	        return 115200;
		case B230400: 
            return 230400;
    }
    return -2;
}

static int checkDataBits(const char dbits){
	switch (dbits) {
        case '5':
        case '6':
        case '7':
        case '8':
            return 1;    
    }
    return 0;
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
	if(!checkDataBits(b)){
		printde("bad data bits: found %hhd, but expected one of 5,6,7,8)", b);
		return -1;
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

	struct termios options;
    if(tcgetattr(fd, &options) != 0){
		close(fd);
		perrord("tcgetattr()");
		return -1;
	}

    cfmakeraw(&options);
    if(cfsetispeed(&options, _baud) != 0){
		close(fd);
		perrord("cfsetispeed()");
		return -1;
	}
    if(cfsetospeed(&options, _baud) != 0){
		close(fd);
		perrord("cfsetospeed()");
		return -1;
	}

    options.c_cflag |= (CLOCAL | CREAD);
    //parity
    switch(p){
		case 'N': case 'n':
	        options.c_cflag &= ~PARENB;
	        break;
        case 'E': case 'e':
	        options.c_cflag |= PARENB;
			options.c_cflag &= ~PARODD;
	        break;
        case 'O': case 'o':
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
    switch (b) {
        case '5':
            options.c_cflag |= CS5;
            break;
        case '6':
            options.c_cflag |= CS6;
            break;
        case '7':
            options.c_cflag |= CS7;
            break;
        case '8':
            options.c_cflag |= CS8;  
            break;
        default:
	        options.c_cflag |= CS8;  
	        break;  
    }
    
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);//raw input
    options.c_oflag &= ~OPOST;//output options (raw output)

    options.c_cc [VMIN] = 0;//minimum characters to read in each read call
    options.c_cc [VTIME] = 3;//read timeout between characters as 100 ms
    
    if(tcsetattr(fd, TCSANOW | TCSAFLUSH, &options) != 0){
		close(fd);
		perrord("tcsetattr()");
		return -1;
	}

    ioctl(fd, TIOCMGET, &status);

    status |= TIOCM_DTR;
    status |= TIOCM_RTS;

    //ioctl(fd, TIOCMSET, &status);//drop dtr signal

    usleep(10000); // 10mS

    return fd;
}

void serial_printOptions (const int fd){
	struct termios options;
	int n = tcgetattr(fd, &options);
	if(n != 0){
		perror("tcgetattr");
	}
	int is = baudInternalToInt(cfgetispeed(&options));
    int os = baudInternalToInt(cfgetospeed(&options));
    char *stop_bits = "?";
    if (options.c_cflag & CSTOPB){
		stop_bits = "2";
	}else {
		stop_bits = "1";
	}
	char *data_bits = "?";
	switch (options.c_cflag & CSIZE){
		case CS5:
			data_bits = "5";
			break;
		case CS6:
			data_bits = "6";
			break;
		case CS7:
			data_bits = "7";
			break;
		case CS8:
			data_bits = "8";
			break;
	}
	char *parity = "?";
	if (options.c_cflag & PARENB){
		if(options.c_cflag & PARODD){
			parity = "O";
		}else {
			parity = "E";
		}
	}else{
		parity = "N";
	}
	printf("in=%d out=%d %s%s%s\n",is, os, data_bits, parity, stop_bits);
}

void serial_flush(const int fd) {
    tcflush(fd, TCIOFLUSH);
}

int serial_canRead(int fd, int timeout_ms){
	struct pollfd fds[1];
	fds[0].fd = fd;
	fds[0].events = POLLIN;
	int ret = poll (fds, 1, timeout_ms);
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
	putsde ("not POLLIN\n");
	return 0;
}

int serial_canWrite(int fd, int timeout_ms){
	struct pollfd fds[1];
	fds[0].fd = fd;
	fds[0].events = POLLOUT;
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
    putsde("not POLLOUT");
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

size_t serial_readFromTo(int fd, char *buf, size_t buf_size, char start, char end) {
	char x;
    size_t c = 0;
    if(c >= buf_size) return c;
    while (read(fd, &x, 1) != 1 || x != start);
    buf[c] = x; c++;
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



