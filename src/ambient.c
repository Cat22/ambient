//  ambient is aprogram to read, display and optional graph 1 or more
//     (up to 10) temper gold v3.1 USB temperature sensors
//
//     Copyright (C) 2020  Eric Benton <erbenton @ comcast.net>
//
//     This program is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     This program is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with this program.  If not, see <http://www.gnu.org/licenses/>.


//
// srcs: ambient.c ambient.h
// gcc -std=gnu11 -Wall -o0 -g3 ambient.c -o ambient -ludev -lpthread
//
//
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <dirent.h>
#include <libudev.h>
#include <pthread.h>
#include <stdbool.h>
#include <signal.h>

#include "ambient.h"
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
void cleanup(void)
{
    cursor_control(SHOW_CURSOR);
    system("tset; reset; clear");
}
//--------------------------------------------------------------------------------
void signal_handler(int UNUSED(sig_num))
{
  exit_flag = true;
}
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------

int main(void)
{
t_info info;
char **hidraws;
int i;
int hidraw_count=0;

        hidraws = calloc(MAX_SENSORS, sizeof(char**));

        for(i=0; i<MAX_SENSORS; i++)
          hidraws[i] = calloc(32, sizeof(char *));

        find_hidraw(hidraws, &hidraw_count);

        lines_per_day = (24*3600)/sleep_time;
        lines_per_week = lines_per_day*7;
        lines_per_month = lines_per_day*21;  // 3 weeks (21 days) of 5 minute intervals

        create_plot_files(hidraw_count);

        signal(SIGINT, signal_handler);


        cursor_control(HIDE_CURSOR);
        atexit(cleanup); // so we will "for sure" restore cursor on exit

        system("clear");
        for(i=0; i<hidraw_count; i++) {
          strcpy(info.hidraw, hidraws[i]);
          info.thread_num = i+1;
          info.display_only = !enable_graphing;

          start_thread(info);
          usleep(250000); // sleep a quarter second
        }

        while(!exit_flag) {sleep(1);} // just spins here forever

        cursor_control(SHOW_CURSOR);

        for(i=0; i<MAX_SENSORS; i++)
          free(hidraws[i]);
        free(hidraws);
        printf("\n");

return 0;
}
//-----------------------------------------------------------------------------------------

void cursor_control(bool cur_cmd)
{
      if (cur_cmd)
      {
        fputs(CURSOR_ON, stdout);
      }
      else
      {
        fputs(CURSOR_OFF, stdout);
      }
}
//-----------------------------------------------------------------------------------------

void *sensor_thread(void *info)
{
char hidraw[32];
char secs[64];
char date[64];
char date_data[64];
char sensor_data[32];
int thread_num;
time_t tstart;
time_t elapsed;
int display_only = false;
char by_day_file[80];
char by_week_file[80];
char by_month_file[80];

        strcpy(hidraw, ((t_info *)info)->hidraw);
        thread_num = ((t_info *)info)->thread_num;
        display_only = ((t_info *)info)->display_only;

        // these filenames must be thread specific
        snprintf(by_day_file, sizeof(by_day_file), by_day_file_fmt, thread_num);
        snprintf(by_week_file, sizeof(by_week_file), by_week_file_fmt, thread_num);
        snprintf(by_month_file, sizeof(by_month_file), by_month_file_fmt, thread_num);

        get_date_data(secs, date, date_data);

        // all this does is update the didplay right away so
        // the user isnt left wondering if its working or not
        while( read_sensor(hidraw, sensor_data)) { sleep(1); }
        update_display(thread_num, sensor_data, secs, date);

        tstart = time(NULL);
        while(1) {
          bzero(secs, sizeof(secs));
          bzero(date, sizeof(date));
          bzero(date_data, sizeof(date_data));

          get_date_data(secs, date, date_data);
          while( read_sensor(hidraw, sensor_data)) { printf("\n\nBad sensor data\n");sleep(1); }
          update_display(thread_num, sensor_data, secs, date);
          elapsed = time(NULL) - tstart;
          if(display_only == false) {
              if((elapsed % sleep_time) == 0) { // every 5 minutes
                tstart = time(NULL);
                append_to_file(thread_num, by_day_file, date_data, sensor_data);
                append_to_file(thread_num, by_week_file, date_data, sensor_data);
                append_to_file(thread_num, by_month_file, date_data, sensor_data);
              }
              // trims from top of file
              trim_file(by_day_file, lines_per_day);
              trim_file(by_week_file, lines_per_week);
              trim_file(by_month_file, lines_per_month);
              if( (elapsed % sleep_time) == 0) { // every 5 minutes after first 10
                  do_plot(); // controlled by mutex
              }
          }
          if(exit_flag) break;
          sleep(1);
        }

return (void*)0;
}
//---------------------------------------------------------------------------------------------

void update_display(int row_offset, char *new_temperature, char *secs, char *date)
{
float tempF;

      pthread_mutex_lock(&display_mutex);

      printf("\033[%d;0H", row_offset); // move to row x and column 0

      tempF = atof(C_to_F(new_temperature));
      if(tempF < COLD)
          printf("\rsensor %d: %6s°C %s%s%6s°F%s %*s %s  ", row_offset, new_temperature, BOLD, CYAN, C_to_F(new_temperature), NORMAL, (int)strlen(date), date, secs);
      else if(tempF < COOL)
          printf("\rsensor %d: %6s°C %s%s%6s°F%s %*s %s  ", row_offset, new_temperature, BOLD, GREEN, C_to_F(new_temperature), NORMAL, (int)strlen(date), date, secs);
      else if(tempF < WARM)
          printf("\rsensor %d: %6s°C %s%s%6s°F%s %*s %s  ", row_offset, new_temperature, BOLD, YELLOW, C_to_F(new_temperature), NORMAL, (int)strlen(date), date, secs);
      else if(tempF < HOT)
          printf("\rsensor %d: %6s°C %s%s%6s°F%s %*s %s  ", row_offset, new_temperature, BOLD, ORANGE, C_to_F(new_temperature), NORMAL, (int)strlen(date), date, secs);
      else
          printf("\rsensor %d: %s%sWARNING!%s %6s°C %s%s%6s°F%s %*s %s  ", row_offset, BOLD,RED,NORMAL,new_temperature, BOLD, RED, C_to_F(new_temperature), NORMAL, (int)strlen(date), date, secs);
       fflush(stdout);

      pthread_mutex_unlock(&display_mutex);

}
//-----------------------------------------------------------------------------------------

void append_to_file(int idx, char *fname, char *date_data, char *temperature_data)
{
FILE *out;
char fname_full[256];

        sprintf(fname_full, fname, idx);
        out = fopen(fname_full, "a");
        fprintf(out, "%s %s\n", date_data, C_to_F(temperature_data));
        fclose(out);
}
//-----------------------------------------------------------------------------------------
char *C_to_F(char *tempC)
{
float C;
static char tempF[16];

       C = atof(tempC);
       snprintf(tempF, sizeof(tempF), "%.1f", (C*(9.0/5.0))+32.0);

return tempF;
}
//-----------------------------------------------------------------------------------------

void get_date_data(char *secs, char *date, char *date_data)
{

        local_time("%s", secs);
        local_time("%m/%d/%Y %H:%M:%S", date);
        local_time("%s %m/%d/%y_%H:%M:%S", date_data);
}
//-----------------------------------------------------------------------------------------

int trim_file(char *fname, int desired_lines)
{
char *tmp_file = "ambient_temperature_temporary_file.dat";
FILE *in, *out;
char buff[128];
int cur_lines;
int ct;

        cur_lines = line_count(fname);
        if(cur_lines < desired_lines) return 0;

        bzero(buff, sizeof(buff));
        unlink(tmp_file);

        in = fopen(fname, "r");
        out = fopen(tmp_file, "w");
        ct = cur_lines - desired_lines; // ct = how many lines to remove from top of file
        while(fgets(buff, sizeof(buff), in)) {
          if(ct > 0) {ct--; continue;}
          fprintf(out, "%s", buff);
        }
        fclose(in);
        fclose(out);
        mv_file(tmp_file, fname, true);
return 0;
}
//-----------------------------------------------------------------------------------------

int line_count(char *fname)
{
FILE *in;
int line_ct;
char buff[128];

        in = fopen(fname, "r");
        if(!in) { printf("failed to open %s\n", fname); exit(1);}
        line_ct = 0;
        while(fgets(buff, sizeof(buff), in))
          line_ct++;
        fclose(in);

return line_ct;
}
//-----------------------------------------------------------------------------------------

#if 0
// could be useful but not currently used
void sync_to_next_sleep_time(void)
{
        while( ((1+time(NULL)) % sleep_time) !=0 )
           sleep(0.5);
}
#endif
//-----------------------------------------------------------------------------------------

void do_plot(void)
{
        if(!enable_graphing) return; // disabled by user

        pthread_mutex_lock(&plot_mutex);

        if(enable_multi_plot)
          system("gnuplot plot-multi.scr");
        else
           system("gnuplot plot-single.scr");

        pthread_mutex_unlock(&plot_mutex);
}

//-----------------------------------------------------------------------------------------
#if 0
// not in  use but could be useful
int wait(int fd, int seconds)
{
fd_set rfds;
fd_set wfds;
struct timeval tv;
int retval;

        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);
        FD_SET(fd, &wfds);
        tv.tv_sec  = seconds;
        tv.tv_usec = 0;
        retval = select(fd+1, &rfds, &wfds, NULL, &tv);

return retval;
}
#endif
//-----------------------------------------------------------------------------------------

int read_sensor(char *hidraw, char *sensor_data)
{
const unsigned char fw_request[8] = {1,0x86,0xff,1,0,0,0,0};
const unsigned char read_sensor_req[8] = {1,0x80,0x33,1,0,0,0,0};
char buff[48];
int fd;
float t;

        errno = 0;
        fd = open(hidraw, O_RDWR);
        if(fd<0) {
          printf("\nread_sensor: unable to open %s - %s\n", hidraw,strerror(errno));
          cursor_control(SHOW_CURSOR);
          exit(1);
        }
        bzero(buff, sizeof(buff));
        strcpy(buff,"");
        while(!strstr(buff, "TEMPerGold_V3.1")) { // to synchronize device
            bzero(buff, sizeof(buff));
            write(fd, fw_request, 8);
            bzero(buff, sizeof(buff));
            read(fd, buff, 8);
            read(fd, &buff[8], 8);
        }

        bzero(buff, sizeof(buff));
        write(fd, read_sensor_req, 8);
        sleep(0.2);
        read(fd, buff, 8);
        close(fd);
        t = ((int)(((unsigned char)buff[2]<<8) | (unsigned char)buff[3]))/100.0;
        sprintf(sensor_data, "%.1f", t);
        if(t > 90.0) {
          read_sensor(hidraw, sensor_data); // try again until we get a sane reading
        }
return 0;
}
//---------------------------------------------------------------------------------------------

int compare(const void *h1, const void *h2)
{
  return strcmp(*(const char **)h1, *(const char **)h2);
}
//---------------------------------------------------------------------------------------------

// udevadm info -a -p $(udevadm info -q path -n /dev/hidraw3)
// udevadm info -q all -a /sys/class/hidraw/hidraw3|less
int  find_hidraw(char **hidraws, int *count)
{
int r=1;
int ct = 0;
struct udev *udev;
struct udev_enumerate *enumerator;
struct udev_list_entry *devices, *dev_list_entry;

    *count = 0;

    udev = udev_new();
    enumerator = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerator, "hidraw");
    udev_enumerate_scan_devices(enumerator);

    devices = udev_enumerate_get_list_entry(enumerator);

    udev_list_entry_foreach(dev_list_entry, devices){

        const char* path = udev_list_entry_get_name(dev_list_entry);
        struct udev_device* dev = udev_device_new_from_syspath(udev, path);
        // If the device matches the vendor and device IDs
         if(strstr(path, "413D:2107")) {
             ct++;
             if(ct==1) continue; // ignore first instance of the hidraw
             // it always seems to be that the second
             // hidraw of the hidraw pair associated with  the device is
             // the correct hidraw to use
             sprintf(hidraws[*count], "/dev/%s", 1+strrchr(path, '/'));
             (*count)++;
             ct = 0;
             r = 0;
         }

        udev_device_unref(dev); // Free the device if it wasn't used
    } // end for each
    udev_enumerate_unref(enumerator);

    //since udev does these backwards we need to sort them
    qsort(hidraws, *count, sizeof(char **), compare);

return r;
}
//---------------------------------------------------------------------------------------------

#include <linux/input.h>
#include <linux/hidraw.h>
#include <sys/ioctl.h>
#include <fcntl.h>
int find_hidraw2(char *hidraw_path) // << this will reliably find the right hidraw but its slow
{
char hpath[32];
int i, r, fd=-1;
struct hidraw_devinfo info;
struct hidraw_report_descriptor info2;
unsigned char request[8] = {1, 0x86, 0xff, 1, 0, 0, 0, 0};
char buff[64];

            for(i=0; i<10; i++) {
              snprintf(hpath, sizeof(hpath), "/dev/hidraw%d", i);
              if(access(hpath, R_OK)) continue;
              printf("checking %s\n", hpath);
              fd = open(hpath, O_RDWR|O_NONBLOCK);
              if (fd < 0) {
                perror("Unable to open device");
                continue;
              }
              // Get Raw Info
              r = ioctl(fd, HIDIOCGRAWINFO, &info);
              if (r < 0) {
                perror("error while getting info from device");
                continue;
              }
              close(fd);
              printf("info.bustype %d info.vendor %x info.product %x\n", info.bustype == BUS_USB, info.vendor, info.product);
              if(info.bustype == BUS_USB &&
                 info.vendor == 0x413d   &&
                 info.product == 0x2107 ) {
                      bzero(buff, sizeof(buff));
                      printf("%s\n", hpath);
                      fd = open(hpath, O_RDWR|O_NONBLOCK);
                      if (r < 0) {
                          perror("error while getting firmware info from device");
                          continue;
                       }
                       bzero(&info2, sizeof(info2));

                      sleep(.75);
                      write(fd, request, sizeof(request));
                      sleep(1);
                      read(fd, buff, 8);
                      sleep(.5);
                      read(fd, &buff[8], 8);

                      close(fd);
                      printf(">> %s\n", buff);
                      if(!strncmp(buff, "TEMPerGold_V3.1 ", strlen("TEMPerGold_V3.1 "))) {
                        strcpy(hidraw_path, hpath);
                        return 0;
                      }
                } // end if
            } // end for

return 1;
}
//---------------------------------------------------------------------------------------------

pthread_t start_thread(t_info info)
{
pthread_attr_t thread_attr;
pthread_t      thread_id;
int            err;

  pthread_attr_init(&thread_attr);
  pthread_attr_setdetachstate( &thread_attr, PTHREAD_CREATE_JOINABLE );
  // pthread_attr_setdetachstate(&ThreadAttr, PTHREAD_CREATE_DETACHED or PTHREAD_CREATE_JOINABLE );
  // pthreads are created joinable by default
  errno = 0;
  err = pthread_create(&thread_id, &thread_attr, &sensor_thread, &info);

  usleep(100*1000); //100 ms wait
  pthread_attr_destroy(&thread_attr);

  if(err != 0)    {

        switch(err)
        {
            case EAGAIN:
                printf("The system lacked the necessary resources to create\n");
                printf("another thread, or the system-imposed limit on the total\n");
                printf("number of threads in a process PTHREAD_THREADS_MAX would be exceeded.\n");
                break;
            case EINVAL:
                printf("The value specified by attr is invalid.\n");
                break;
            case EPERM:
                printf("The caller does not have appropriate permission to set the required\n");
                printf("scheduling parameters or scheduling policy.\n");
                break;
            default:
                printf("Some weird error code returned from pthread_create\n");
                break;
        }
        printf("Error Code 0x%08X %s: pthread failed to start\n",  err, strerror_r(errno, ebuf, sizeof(ebuf)));
        printf("LEAVE -1\n");
        return -1; // return negative thread id

    } // end if err

return thread_id;
}
//---------------------------------------------------------------------------------------------

char *local_time(char *format, char *result) // expects a 64 byte string
{
time_t t;
struct tm tm = {0};

      if(!format || strlen(format) == 0 ) {
          printf("missing format string for localTimef call\n");
          return NULL;
      }

      t = time(NULL);
      localtime_r(&t, &tm);
      strftime(result, 64, format, &tm);

return result;
}
//---------------------------------------------------------------------------------------------
/*
 * retrieves word #X in string, words are space or tab separated
 *  Zero based // also, removes line endings
 */
char *get_word(char *str,int word_number)
{
static char word[80];
char *vptr;
char *tmp;
int i;

      if(!str || strlen(str)==0) return NULL;
      if(word_number < 0) return NULL;

      tmp = malloc(strlen(str)+1);
      strcpy(tmp, str);
      vptr = strtok(tmp, " \t"); // vptr now points to word 0
      for(i=1; i<=word_number; i++) {
        vptr = strtok(NULL, " \t");
        if(!vptr) {
          free(tmp);
          return NULL;
        }
      }
      strcpy(word, vptr);
      free(tmp);

return trim(word);
}
//---------------------------------------------------------------------------------------------

int cpy_file(const char *src, const char *dst, int allow_overwrite)
{
FILE *in, *out;
char *buffer;
int r;

      if(!dst || strlen(dst)==0) return -1;
      if(!src || strlen(src)==0) return -2;

        if(access(src, R_OK)) return -2; // cant find src file
        if(!access(dst, F_OK)) {
          if( ! (allow_overwrite==1 && !access(dst, W_OK) )) return -1; // dst file exists and allow_overwrite is false or dst isnt writable
        }

        buffer = calloc(4096, 1);
        in = fopen(src, "r");
        out = fopen(dst, "w");
        while((r = fread(buffer, 1, 4096, in))>0) {
          fwrite(buffer, 1, r, out);
        }
        fclose(in);
        fclose(out);
        free(buffer);
return 0;
}
//---------------------------------------------------------------------------------------------

int mv_file(const char *src, const char *dst, int allow_overwrite)
{
int r;
       r = cpy_file(src, dst, allow_overwrite);
       if(r==0) unlink(src);
return r;
}
//---------------------------------------------------------------------------------------------
char *ltrim(char *str)
{

      if(!str) return str;
      if(strlen(str) == 0) return str;

      while(isspace(str[0])||str[0]=='\n') {
        memmove(str,  &str[1], strlen(str));
 }

return str;
}
//---------------------------------------------------------------------------------------------

char *rtrim(char *str)
{
char* vptr;

      if(!str) return str;
      if(strlen(str) == 0) return str;

      vptr = &str[strlen(str)-1];
      while(isspace(*vptr) && (vptr!=str)) vptr--;
      vptr++;
      *vptr = 0;

return str;

}
//---------------------------------------------------------------------------------------------

char *trim(char *str)
{
      if(!str || strlen(str) == 0) return str;
      return ltrim(rtrim(str));
}
//---------------------------------------------------------------------------------------------

int create_plot_files(int sensor_count)
{
FILE *out;
int i;

      // create a single sensor plot script
      if(access("plot-single.scr", W_OK)) {
         out = fopen("plot-single.scr", "w");
         fprintf(out, "%s", plot_single_scr);
         fclose(out);
      }

      // create the multi-sensors plot script (up to 10 sensors)
      if(access("plot-multi.scr", W_OK)) {
         out = fopen("plot-multi.scr", "w");
         if(!out) {
           printf("Unable to create plot file, writing denied\n");
           return 1;
         }

        fprintf(out, "%s", multi_plot_part1);

        for(i=2; i<=sensor_count; i++) {
          fprintf(out, multi_plot_part2, i, i, plot_colors[i-1]);
        }
        fprintf(out, "\n%s", multi_plot_part3);

        for(i=2; i<=sensor_count; i++) {
          fprintf(out, multi_plot_part4, i, i, plot_colors[i-1]);
        }
        fprintf(out, "\n%s", multi_plot_part5);

        for(i=2; i<=sensor_count; i++) {
          fprintf(out, multi_plot_part6, i, i, plot_colors[i-1]);
        }
        fprintf(out, "\n");
        fclose(out);
      } // end if access

return 0;
}
//---------------------------------------------------------------------------------------------
