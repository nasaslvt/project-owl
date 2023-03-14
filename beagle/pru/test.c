#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#define PRU_ADDR        0x4A300000      // Start of PRU memory Page 184 am335x TRM
#define PRU_LEN         0x80000
#define PRU0_DRAM       0x00000
#define PRU1_DRAM       0x02000

int main(int argc, char *argv[])
{
    unsigned int *pru;
    int fd;

    fd = open ("/dev/mem", O_RDWR | O_SYNC);
    if (fd == -1) {
        printf ("ERROR: could not open /dev/mem.\n\n");
        return 1;
    }
    pru = mmap (0, PRU_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, fd, PRU_ADDR);
    if (pru == MAP_FAILED) {
        printf ("ERROR: could not map memory.\n\n");
        return 1;
    }
    close(fd);

    uint32_t *pru0_dram = pru + (PRU0_DRAM + 0x200)/4; // Points to 0x200 of PRU0 memory

    while (1) {
        float dc = (float)pru0_dram[0]/(float)pru0_dram[1] * 100;
        float theta = (dc - 2.9) * 360 / (97.1 - 2.9 + 1);
        printf("Swivel Angle: %f\n", theta);
    }
}