#include <nuttx/config.h>

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>

#define AFS_SEL 4096
#define REG_LOW_MASK 0xFF00
#define REG_HIGH_MASK 0x00FF

static pid_t accelerometer_pid;

static int accelerometer_task(int argc, FAR char *argv[]) {
    printf("Starting accelerometer task\n");
    int fd, ret;
    int16_t raw_data[7];
    int16_t acc_x, acc_y, acc_z;

    fd = open("/dev/imu0", O_RDWR);
    if (fd < 0) {
        printf("Failed to open IMU\n");
        goto errout;
    }
    while (1) {
        ret = read(fd, &raw_data, sizeof(raw_data));
        if (ret != sizeof(raw_data)) {
            printf("Failed to read accelerometer data:");
            for (int i = 0; i < 7; i++) {
                printf("0x%x ", raw_data[i]);
            }
            printf("\n");
        }
        else {
            acc_x = ( (raw_data[0] & 0x00FF) << 8) + ( (raw_data[0] & REG_LOW_MASK) >> 8);
            acc_y = ( (raw_data[1] & 0x00FF) << 8) + ( (raw_data[1] & REG_LOW_MASK) >> 8);
            acc_z = ( (raw_data[2] & 0x00FF) << 8) + ( (raw_data[2] & REG_LOW_MASK) >> 8);
            printf("AccX, AccY, AccZ: %.03f %.03f %.03f\n",
                    (float)acc_x/AFS_SEL,
                    (float)acc_y/AFS_SEL,
                    (float)acc_z/AFS_SEL);
        }
        usleep(1000 * 1000L);
    }

    return 0;

errout:
  close(fd);
  printf("accelerometer_task: Terminating\n");

  return EXIT_FAILURE;
}

int main(int argc, FAR char *argv[]) {
    printf("Starting Accelerometer Task Example\n");
    accelerometer_pid = task_create(
        "Accelerometer Task",
        100,
        700,
        accelerometer_task,
        NULL);
    if (accelerometer_pid < 0) {
        printf("Failed to create task\n");
    }

    return 0;
}
