while [ 1 ]; do esptool.py -b 115200 write_flash --flash_size 4MB 0x00000 secfest2019_noobadge.ino.generic.bin; done
