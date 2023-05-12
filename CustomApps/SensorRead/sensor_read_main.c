#include <nuttx/config.h>
#include <nuttx/mqueue.h>

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>

#define AFS_SEL 4096
#define REG_LOW_MASK 0xFF00
#define REG_HIGH_MASK 0x00FF
#define ACCEL_TASK_INTERVAL_MS 1000

static pid_t accelerometer_pid;
static pid_t offload_pid;

struct accelerometer_queue_msg {
    float acc_x;
    float acc_y;
    float acc_z;
};

static int offload_task(int argc, FAR char *argv[]) {
    printf("Starting offload task\n");
    int counter = 0;
    struct accelerometer_queue_msg rcv_acc_queue = {0};
    mqd_t mqd = (mqd_t)argv[0];
    int ret;

    while (1) {
        counter++;
        ret = mq_receive(mqd, (FAR char *)&rcv_acc_queue, sizeof(struct accelerometer_queue_msg), 0);
        if (ret > 0) {
            printf("AccX, AccY, AccZ: %d %.03f %.03f %.03f\n",
                   counter,
                    (float)rcv_acc_queue.acc_x/AFS_SEL,
                    (float)rcv_acc_queue.acc_y/AFS_SEL,
                    (float)rcv_acc_queue.acc_z/AFS_SEL);
        }
    }
    return 0;
}

static int accelerometer_task(int argc, FAR char *argv[]) {
    printf("Starting accelerometer task\n");
    int fd, ret;
    int16_t raw_data[7];
    static struct accelerometer_queue_msg acc_msg;
    mqd_t mqd = (mqd_t)argv[0];

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
            acc_msg.acc_x = ( (raw_data[0] & 0x00FF) << 8) + ( (raw_data[0] & REG_LOW_MASK) >> 8);
            acc_msg.acc_y = ( (raw_data[1] & 0x00FF) << 8) + ( (raw_data[1] & REG_LOW_MASK) >> 8);
            acc_msg.acc_z = ( (raw_data[2] & 0x00FF) << 8) + ( (raw_data[2] & REG_LOW_MASK) >> 8);
            ret = mq_send(mqd, (FAR const char *)&acc_msg, sizeof(struct accelerometer_queue_msg), 1);
            if (ret == (mqd_t)-1) {
                printf("Failed to send message\n");
            }
        }
        usleep(ACCEL_TASK_INTERVAL_MS * 1000L);
    }

    return 0;

errout:
  close(fd);
  printf("accelerometer_task: Terminating\n");

  return EXIT_FAILURE;
}

int main(int argc, FAR char *argv[]) {
    printf("Starting Accelerometer Task Example\n");
    char* child_argv[1];
    struct mq_attr attr;
    mqd_t mqd;
    
    printf("Opening message queue channel\n");
    attr.mq_flags   = 0;
    attr.mq_maxmsg  = 2;
    attr.mq_msgsize = sizeof(struct accelerometer_queue_msg);
    attr.mq_curmsgs = 0;
    mqd = mq_open("acc_queue", O_CREAT, 0666, &attr);
    if (mqd == (mqd_t)-1) {
        printf("Failed to start queue\n");
        return 1;
    }
    child_argv[0] = (char*)mqd;

    accelerometer_pid = task_create(
        "Accelerometer Task",
        120,
        700,
        accelerometer_task,
        child_argv);
    if (accelerometer_pid < 0) {
        printf("Failed to create accelerometer task\n");
    }

    offload_pid = task_create(
        "Offload Task",
        110,
        700,
        offload_task,
        child_argv);
    if (offload_pid < 0) {
        printf("Failed to create offload task\n");
    }

    return 0;
}
