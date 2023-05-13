#include <nuttx/config.h>
#include <nuttx/mqueue.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

static pid_t child_pid;
static pid_t sister_pid;

static int sister_task(int argc, FAR char *argv[]) {
  printf("Starting sister task\n");
  mqd_t mqd = (mqd_t)*argv[3];
  char message[12];
  int ret;
  while (true)
    { 
      ret = mq_receive(mqd, (char*)&message, sizeof(message), 0);
      if (ret < 0)
        {
          printf("Failed to receive\n");
          continue;
        }
      printf("Received: %s\n", message);
    }

  return 0;
}


static int child_task(int argc, FAR char *argv[]) {
  printf("Starting child task\n");
  printf("Arg Counter argc = %d\n", argc);
  for (int i = 0; i <= argc; i++)
    {
      printf("argv[%d] = %s\n", i, argv[i]);
    }
  // argv[3] is the queue descriptor address
  printf("Cast argv[3] = %d\n", (mqd_t)*argv[3]);
  mqd_t mqd = (mqd_t)*argv[3];
  char message[] = "Hello World";
  int counter = 0;
  int ret;

  while (counter < 5)
    {
      printf("Blink!\n");
      ret = mq_send(mqd, (const void*)&message, sizeof(message), 0);
      if (ret < 0)
        {
          printf("Failed to send message: %s\n", strerror(errno));
        }
      counter ++;
      usleep(1000 * 1000L);
    }

  return 0;
}

int main(int argc, FAR char *argv[]) {
  printf("Starting\n");
  char *child_argv[5];

  struct mq_attr attr;
  attr.mq_maxmsg = 2;
  attr.mq_msgsize = 5;
  attr.mq_flags = 0;
  attr.mq_curmsgs = 0;

  mqd_t mqd = mq_open(
    "QueueMain", O_RDWR | O_CREAT, 0666, &attr);
  if (mqd == (mqd_t)-1)
    {
      printf("Failed to create queue\n");
    }

  child_argv[0] = "a";
  child_argv[1] = "b";
  child_argv[2] = (char*)&mqd;
  child_argv[3] = "d";
  child_argv[4] = NULL;

  sister_pid = task_create(
    "Sister Task", 100, 200, sister_task, (char* const*) child_argv);
  if (sister_pid < 0)
    {
      printf("Failed to create sister task\n");
    }

  child_pid = task_create(
    "Child Task", 110, 200, child_task, (char* const*) child_argv);
  if (child_pid < 0)
    {
      printf("Failed to create child task\n");
    }

  return 0;
}
