#include <stdio.h>
#include <nuttx/config.h>

static pid_t blink_pid;

static int blink_task(int argc, FAR char *argv[]) {
    printf("Starting blink task\n");
    while (1) {
        printf("Blink!\n");
        usleep(1000 * 1000L);
    }

    return 0;
}

int main(int argc, FAR char *argv[]) {
    printf("Creating task\n");
    blink_pid = task_create("Blink Task", 10, 200, blink_task, NULL);
    if (blink_pid < 0) {
        printf("Failed to create task\n");
    }

    return 0;
}
