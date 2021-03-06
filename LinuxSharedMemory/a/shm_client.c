#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
// TODO(student): Include necessary headers

#define SEG_SIZE 64
#define SEG_NUM 63
typedef struct {
  int valid;
  int pid;
  char birth[25];
  int start_sec;
  double start_usec;
  int elapsed_sec;
  double elapsed_msec;
} stats_t;

const char * SHM_NAME = "sadeq_patri";
const char * path_name = "/dev/shm/sadeq_patri";
const off_t length = 4096;
sem_t * mutex;
void *map;
stats_t* list;
int fd_shm;
int p_index;

void exit_handler(int sig) {
    // new routine defined here specified by sigaction() in main
    // TODO(student): critical section begins
        sem_wait(mutex);
    // client reset its segment, or mark its segment as valid
    stats_t *rm;
    rm = &list[p_index];
    rm->valid = 1;
    memset(rm->birth, '\0', sizeof(rm->birth));
    rm->elapsed_sec = 0;
    rm->elapsed_msec = 0;
    // so that the segment can be used by another client later.
        sem_post(mutex);
    // critical section ends
    exit(0);
}
void update_time() {
  stats_t *p;
  p = &list[p_index];
  struct timeval curr_time;
  (void) gettimeofday(&curr_time, 0);
  sem_wait(mutex);
  p->elapsed_sec = ((int)curr_time.tv_sec) - p->start_sec;
  p->elapsed_msec = ((double)(curr_time.tv_usec) - (p->start_usec)) / 1000.0f;
  sem_post(mutex);
}

char *get_time() {
    time_t rawtime;
    struct tm * timeinfo;
    struct tm timeinfoBuffer;
    char *result;
    time(&rawtime);
    /* call localtime */
    timeinfo = localtime_r(&rawtime , &timeinfoBuffer);
    /* allocate memory for the result of asctime call*/
    result = malloc(26 * sizeof(char));
    /* call reentrant asctime function */
    result = asctime_r(timeinfo, result);
    result[24] = '\0';
    return result;
}

int main(int argc, char *argv[]) {
    // TODO(student): Signal handling
    // Use sigaction() here and call exit_handler
    struct sigaction act;
    memset(&act, '\0', sizeof(act));
    act.sa_handler = &exit_handler;
    sigaction(SIGINT, &act, NULL);
    // TODO(student): Open the preexisting shared memory segment created by
    // shm_server
    fd_shm = shm_open(SHM_NAME, O_RDWR | O_EXCL, 0660);
      if ( fd_shm < 0 ) {
        exit(1);
      }
    // TODO(student): point the mutex to the particular segment of the shared
    // memory page
    map = mmap(NULL, (size_t) length, PROT_READ | PROT_WRITE,
      MAP_SHARED, fd_shm, 0);
    if (map <  NULL) {
      exit(1);
    }
    mutex = (sem_t*)map;
    list = (stats_t*)(mutex + 1);
    // TODO(student): critical section begins
    int found = 0;
        sem_wait(mutex);
        // client searching through the segments of the page to find a valid
        // (or available) segment and then mark this segment as invalid
        for (stats_t* p = list; p < &list[SEG_NUM]; p++) {
          // stats_t *p;
          struct timeval temp;
          if ( p->valid ) {
             p_index = p - list;
             p->valid = 0;
             p->pid = getpid();
             (void) strcpy((char*)&(p->birth), get_time());
             (void) gettimeofday(&temp, 0);
             p->start_sec = (int)temp.tv_sec;
             p->start_usec = (double)temp.tv_usec;
             found = 1;
             break;
          }
     }
     sem_post(mutex);
    if (!found )   // {
      exit(1);
//  } else {
// fprintf(stderr, "found\n");
//    }
// critical section ends
    while (1) {
        // TODO(student): fill in fields in stats_t
        update_time();
        sleep(1);
        printf("Active clients :");
        // display the PIDs of all the active clients
        for (stats_t* p = list; p < &list[SEG_NUM]; p ++) {
          printf(" ");
          if (!(p->valid)) {
            printf("%d", p->pid);
          }
        }
        printf("\n");
    }
    return 0;
}

