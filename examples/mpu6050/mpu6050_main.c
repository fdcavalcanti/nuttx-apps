/****************************************************************************
 * apps/examples/ina226/ina226_main.c
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include <nuttx/config.h>
#include <nuttx/sensors/mpu60x0.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifndef CONFIG_MPU6050_DEVICE_PATH
#  define CONFIG_MPU6050_DEVICE_PATH "/dev/imu0"
#endif
#define AFS_SEL 4096
/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * mpu6050_main
 ****************************************************************************/ 

int main(int argc, FAR char *argv[])
{
  // struct sensor_data_s *data;
  uint8_t data[14];
  int fd;
  int ret;

  fd = open(CONFIG_MPU6050_DEVICE_PATH, O_RDWR | O_NONBLOCK);
  if (fd < 0)
    {
      int errcode = errno;
      printf("ERROR: Failed to open %s: %d\n",
             CONFIG_MPU6050_DEVICE_PATH, errcode);
      return EXIT_FAILURE;
    }

    ret = read(fd, &data, sizeof(data));
    if (ret != sizeof(data))
      {
        printf("Failed to read IMU data: %d\n", ret);
        return EXIT_FAILURE;
      }

  // Accelerometer data is received in two parts: high and low
  // registers. The high part should be left shited by 8 bits
  // and added to the lower part (see MPU6050 manual).
  int16_t acc_x = (data[0] << 8) + data[1];
  int16_t acc_y = (data[2] << 8) + data[3];
  int16_t acc_z = (data[4] << 8) + data[5];
  int16_t temperature = (data[6] << 8) + data[7];
  int16_t gyro_x = (data[8] << 8) + data[9];
  int16_t gyro_y = (data[10] << 8) + data[11];
  int16_t gyro_z = (data[12] << 8) + data[13];

  printf("Accelerometer X: %.03f\n", (float)acc_x/AFS_SEL);
  printf("Accelerometer Y: %.03f\n", (float)acc_y/AFS_SEL);
  printf("Accelerometer Z: %.03f\n", (float)acc_z/AFS_SEL);
  printf("Temperature [C]: %.03f\n", (float)temperature/340.0 + 36.53);
  printf("Gyro X: %.03f\n", (float)gyro_x/AFS_SEL);
  printf("Gyro Y: %.03f\n", (float)gyro_y/AFS_SEL);
  printf("Gyro Z: %.03f\n", (float)gyro_z/AFS_SEL);

  close(fd);
  return 0;
}
