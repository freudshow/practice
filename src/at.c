int gpofun(char *devname, int data) {
	int fd = -1;
	if ((fd = open(devname, O_RDWR | O_NDELAY)) >= 0) {
		write(fd, &data, sizeof(int));
		close(fd);
		return 1;
	}
	return 0;
}

gpofun("/dev/gpoGPRS_POWER", 0);
gpofun("/dev/gpoCSQ_GREEN", 0);
gpofun("/dev/gpoCSQ_RED", 0);
gpofun("/dev/gpoONLINE_LED", 0);

gpofun("/dev/gpoGPRS_POWER", 1);
gpofun("/dev/gpoGPRS_RST", 1);
gpofun("/dev/gpoGPRS_SWITCH", 1);
ao->state = 2;
return 3000;

