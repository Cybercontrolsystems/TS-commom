#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdlib.h>

/*
  This sample code demonstrates how to talk to an LM73 temp sensor
  chip from the TS-75XX (Cavium CPU based) series of SBCs.  The 
  LM73 uses an I2C interface, and the Cavium CPU provides a register
  interface to I2C transactions. (The Cavium data sheet calls the
  interface "TWI").
*/

void *map_phys(off_t addr,int *fd) {
  off_t page;
  unsigned char *start;

  if (*fd == -1)
    *fd = open("/dev/mem", O_RDWR|O_SYNC);
  if (*fd == -1) {
    perror("open(/dev/mem):");
    return 0;
  }
  page = addr & 0xfffff000;
  start = mmap(0, getpagesize(), PROT_READ|PROT_WRITE, MAP_SHARED, *fd, page);
  if (start == MAP_FAILED) {
    perror("mmap:");
    return 0;
  }
  start = start + (addr & 0xfff);
  return start;
}

inline void setbit(volatile unsigned *adrs,unsigned bit) {
  *adrs |= (1 << bit);
}

inline void clrbit(volatile unsigned *adrs,unsigned bit) {
  *adrs &= ~(1 << bit);
}

inline int getbit(volatile unsigned *adrs,unsigned bit) {
  return !!(*adrs & (1 << bit));
}

int fd = -1;

#define CAVIUM_DIRECT_TWI
#ifdef CAVIUM_DIRECT_TWI
volatile unsigned *reg=0;

inline void cavium_enable_TWI() {
  setbit(&reg[0],31); // enable TWI
  clrbit(&reg[0],24); // disable TWI data swap
}

void init_TWI() {
  reg = map_phys(0x71000020,&fd);
  cavium_enable_TWI();
  reg[0x14/sizeof(unsigned)] = 3; // clear any existing errors
}

void print_TWI_error() {
  int status = (reg[0x14/sizeof(unsigned)] >> 8) & 0xFF;

  printf("ERROR:");
  switch (status) {
  case 0x20: printf("Slave Address + W has been transmitted, and Slave's Not-Acknowledge(NACK) has been received.\n"); break;
  case 0x30: printf("Data byte in WR_DATA_REG has been transmitted, and NACK has been received.\n"); break;
  case 0x48: printf("Slave Address + R has been transmitted, and NACK has been received.\n"); break;
  case 0x70: printf("Bus error, SDA stuck low.\n"); break;
  case 0x90: printf("Bus error, SCL stuck low.\n"); break;
  case 0xFF: printf("None\n"); break;
  default: printf("Unknown error %X\n",status); break;
  }
}

void write_TWI_data(unsigned adrs,int len,unsigned data) {
  int limit=10000;

  clrbit(&reg[0],5); // only write
  setbit(&reg[0],4);
  reg[0] = (reg[0] & 0xFFFFFFF3) | (((len - 1) & 3)<<2); // write data length
  reg[0xC/sizeof(unsigned)] = data;
  reg[0x8/sizeof(unsigned)] = adrs;
  setbit(&reg[0],6); // start
  while(!getbit(&reg[0x14/sizeof(unsigned)],1) && --limit>0); // wait done
  if (limit <=0) { printf("timeout\n"); exit(3); }
  //printf("write %d,%X -> %X\n",len,data,reg[0x14/sizeof(unsigned)]);
  if (getbit(&reg[0x14/sizeof(unsigned)],0)) {
    print_TWI_error();
    exit(1);
  }
}

unsigned read_TWI_data(unsigned adrs,int len) {
  unsigned data;
  int limit=10000;

  reg[0] = (reg[0] & 0xFFFFFFFC) | ((len - 1) & 3); // read data length
  clrbit(&reg[0],5); // only read
  clrbit(&reg[0],4);
  setbit(&reg[0],6); // start
  while(!getbit(&reg[0x14/sizeof(unsigned)],1) && --limit>0); // wait done
  if (limit <=0) { printf("timeout\n"); exit(3); }
  if (getbit(&reg[0x14/sizeof(unsigned)],0)) {
    print_TWI_error();
    exit(1);
  }
  return reg[0x10/sizeof(unsigned)];
}
#else
// NOTE: Don't use this code. The Linux I2C interface does not
// provide a mechanism to read 16-bit values from a register -
// such attempts are converted into 2 8-bit reads, which does not
// do what we want. For example, the Id register is read as 0x01 0x01
// instead of 0x01 0x90.  Thus this code will not work.  However it
// is provided for completeness should the Linux API be fixed to
// allow us to read a 16-bit value.
#include <linux/i2c-dev.h>

void init_TWI() {
  fd = open("/dev/i2c-0", O_RDWR);
  assert(fd != 0);
}

unsigned last_adrs;

void write_TWI_data(unsigned adrs,int len,unsigned data) {
  int ret;

  last_adrs = data & 0x0F;
  adrs >>= 1; // our Cavium API expects address left-shifted one
  assert(ioctl(fd, 0x0703, adrs) == 0);
  write(fd,&data,len);
}

unsigned read_TWI_data(unsigned adrs,int len) {
  unsigned val;
  
  read(fd,&val,len);
  return val;
}
#endif

int taddr = 0x92;

int detect_LM73() {
  unsigned data;

  write_TWI_data(taddr,1,0x7);
  data = read_TWI_data(taddr,2);
  if (((data & 0xFF) != 0x01) ||  (((data >> 8) & 0xFF) != 0x90)) {
    return 0;
  }
  return 1;
}

short read_LM73_temp() {
  unsigned data;
  int limit = 10;

/* uncomment if we want to wait for temp by polling
  write_TWI_data(taddr,1,0x4);
  data = read_TWI_data(taddr,1);
  while (!(data & 1) && --limit > 0) {
    usleep(10000);
    data = read_TWI_data(taddr,1);
  }
*/
  write_TWI_data(taddr,1,0x0);
  data = read_TWI_data(taddr,2);
  return ((data & 0xFF) << 8) + ((data >> 8) & 0xFF);
}

void LM73_power(int on) {
  write_TWI_data(taddr,2,0x4001 | (((on==0)?1:0) << 15));
}

int main(int argc,char *argv[]) {
  int fd = -1;
  short temp;

  // 144, 146 (default), ..., 154 for first chip
  // 156, ..., 166 for second chip
  if (argc == 2) taddr = atoi(argv[1]);
  init_TWI();

  if (!detect_LM73()) {
    printf("failed to detect LM73 temp sensor chip\n");
    return 1;
  }
  write_TWI_data(taddr,1,0x1);

  LM73_power(0); 
  usleep(150000);
  LM73_power(1); 
  usleep(150000);

  // 0 = 0C
  // 1/128 degree per tick
  temp = read_LM73_temp();
  printf("%.2fC\n",(float)temp / 128);
  // Note:
  // Above temperatures are only accurate to within the error rate
  // of the chip (+/-1.0C to +/-2.0C depending on temperature) and
  // the number of bit being sampled (default is 11 bits)
}

