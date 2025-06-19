#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <getopt.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>

#define DEFAULT_EEPROM_DEV "/sys/bus/i2c/devices/3-0050/eeprom"
#define EEPROM_SIZE 1024
#define COMMON_LEN 8
#define STR_MAC_LEN 17
#define LEN_OFFSET_IN_PRODUCT_AREA 1
#define LANGUAGE_CODE_OFFSET_IN_PRODUCT_AREA 2
#define MANUFACTURER_OFFSET_IN_PRODUCT_AREA 3
#define FILED_LEN_MASK (0xFF >> 2)
#define FILED_LEN_MAX 64

enum product_area_filed
{
   Manufacturer_Name = 0,
   Product_Name,
   Product_Part_Number,
   Product_Version,
   Product_serial_Number,
   Asset_Tag,
   FRU_Filed_Id,
   Mac_Addr,
   Certificate
};


unsigned char eeprom_info[EEPROM_SIZE];
unsigned char mac_addr[FILED_LEN_MASK];



static int isValidMacAddress(const char* mac) {
    int i = 0;
    int s = 0;

    while (*mac) {
       if (isxdigit(*mac)) {
          i++;
       }
       else if (*mac == ':' || *mac == '-') {

          if (i == 0 || i / 2 - 1 != s)
            break;

          ++s;
       }
       else {
           s = -1;
       }


       ++mac;
    }

    return (i == 12 && s == 5);
}


static int eeprom_read(unsigned char * device_name)
{
   int fd, cnt;
   unsigned int i, index = 0, mac_addr_index;
   char product_area_len = 0, language_code = 0, mac_addr_len = 0, tmp_filed_len = 0;
   fd = open(device_name, O_RDONLY);
   if (fd < 0) {
      return -1;
   }

   cnt = read(fd, eeprom_info, EEPROM_SIZE);
   if (cnt < 0) {
      close(fd);
      return -1;
   }
   product_area_len = eeprom_info[COMMON_LEN + LEN_OFFSET_IN_PRODUCT_AREA] * 8;
   language_code    = eeprom_info[COMMON_LEN + LANGUAGE_CODE_OFFSET_IN_PRODUCT_AREA];
   index            = COMMON_LEN + MANUFACTURER_OFFSET_IN_PRODUCT_AREA;
   /*
   *  i-tmp_filed_len:
   *  0-manuf_name_len
   *  1-product_name_len
   *  2-part_num_len
   *  3-version_len
   *  4-serial_num_len
   *  5-asset_tag_len
   *  6-fru_file_id_len
   *  7-mac_addr_len
   */
   for(i=0; i<Certificate; i++)
   {
      if(i == Mac_Addr)
      {
         mac_addr_len = (eeprom_info[index] & FILED_LEN_MASK);
         mac_addr_index =  index += sizeof(char);
      }
      else
      {
         tmp_filed_len = (eeprom_info[index] & FILED_LEN_MASK);
         index += (sizeof(char) + tmp_filed_len);
      }

   }

   memcpy(mac_addr, (unsigned char *)&eeprom_info[mac_addr_index], mac_addr_len);
   mac_addr[mac_addr_len] = '\0';
   if(isValidMacAddress(mac_addr))
   {
      printf("%s", mac_addr);
      close(fd);
      return 0;
   }
   else
   {

      return -1;
   }
}

static const struct option options[] = {
	{ "device",	required_argument,	0, 'd'},
	{ 0,  0, 0, 0},
};
int main(int argc, char *argv[])
{
   unsigned char *device_name = NULL;

   for (;;) {
		int c, idx;

		c = getopt_long(argc, argv, "d:", options, &idx);
		if (c == -1)
			break;

		switch (c) {
		case 'd':
			device_name = optarg;
			break;
		default:
         break;
		}
	}
   if(!device_name)
      device_name = DEFAULT_EEPROM_DEV;

   if(eeprom_read(device_name) == 0)
   {

      return 1;
   }
   else
   {
      return 0;
   }

}
