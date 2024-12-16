#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

typedef uint32_t u32;
typedef int32_t i32;

struct process
{
  u32 pid;
  u32 arrival_time;
  u32 burst_time;
  TAILQ_ENTRY(process) pointers;
  u32 start_time;
  u32 remaining_burst_time;
  u32 finish_time;
  bool started;
};
TAILQ_HEAD(process_list, process);

u32 next_int(const char **data, const char *data_end)
{
  u32 current = 0;
  bool started = false;
  while (*data != data_end)
  {
    char c = **data;

    if (c < 0x30 || c > 0x39)
    {
      if (started)
      {
        return current;
      }
    }
    else
    {
      if (!started)
      {
        current = (c - 0x30);
        started = true;
      }
      else
      {
        current *= 10;
        current += (c - 0x30);
      }
    }

    ++(*data);
  }

  printf("Reached end of file while looking for another integer\n");
  exit(EINVAL);
}

u32 next_int_from_c_str(const char *data)
{
  char c;
  u32 i = 0;
  u32 current = 0;
  bool started = false;
  while ((c = data[i++]))
  {
    if (c < 0x30 || c > 0x39)
    {
      exit(EINVAL);
    }
    if (!started)
    {
      current = (c - 0x30);
      started = true;
    }
    else
    {
      current *= 10;
      current += (c - 0x30);
    }
  }
  return current;
}

void init_processes(const char *path, struct process **process_data, u32 *process_size)
{
  int fd = open(path, O_RDONLY);
  if (fd == -1)
  {
    int err = errno;
    perror("open");
    exit(err);
  }

  struct stat st;
  if (fstat(fd, &st) == -1)
  {
    int err = errno;
    perror("stat");
    exit(err);
  }

  u32 size = st.st_size;
  const char *data_start = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (data_start == MAP_FAILED)
  {
    int err = errno;
    perror("mmap");
    exit(err);
  }

  const char *data_end = data_start + size;
  const char *data = data_start;

  *process_size = next_int(&data, data_end);
  *process_data = calloc(sizeof(struct process), *process_size);
  if (*process_data == NULL)
  {
    int err = errno;
    perror("calloc");
    exit(err);
  }
  for (u32 i = 0; i < *process_size; ++i)
  {
    (*process_data)[i].pid = next_int(&data, data_end);
    (*process_data)[i].arrival_time = next_int(&data, data_end);
    (*process_data)[i].burst_time = next_int(&data, data_end);
  }

  munmap((void *)data, size);
  close(fd);
}

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    return EINVAL;
  }
  struct process *data;

  u32 size;
  init_processes(argv[1], &data, &size);
  u32 quantum_length = next_int_from_c_str(argv[2]); 
  struct process_list list; 
  TAILQ_INIT(&list);

  u32 total_waiting_time = 0;
  u32 total_response_time = 0;

/* Your code here */
struct process *current_process = NULL;

for (u32 i = 0; i < size; i++) {
    data[i].remaining_burst_time = data[i].burst_time;
    data[i].started = false;
}

u32 current_time = 0;
u32 time_slice = 0;
bool all_done = false;
u32 num_proc_done = 0;

while (num_proc_done<size) {
    for (u32 i = 0; i < size; i++) {
        if(data[i].arrival_time == current_time && !data[i].started){
            TAILQ_INSERT_TAIL(&list, &data[i], pointers);        
            }
    }

    if(current_process!=NULL){
          if (time_slice == quantum_length && current_process != NULL) {
            TAILQ_INSERT_TAIL(&list, current_process, pointers);
            current_process = NULL;
            time_slice = 0; 
        }
    }
    if (!TAILQ_EMPTY(&list) && current_process == NULL) {
        current_process = TAILQ_FIRST(&list);
        if(current_process!=NULL){
          TAILQ_REMOVE(&list, current_process, pointers); 
          if (!current_process->started) {
              current_process->start_time = current_time;
              current_process->started = true;
              total_response_time += (current_process->start_time - current_process->arrival_time);
          }
        }
    }

    if (current_process != NULL) {
        if (current_process->remaining_burst_time > 0) {
            current_process->remaining_burst_time--;
            current_time++; 
            time_slice++;
            if (current_process->remaining_burst_time == 0) {
                current_process->finish_time = current_time;
                total_waiting_time += current_process->finish_time - current_process->arrival_time - current_process->burst_time;
                num_proc_done++;
                current_process = NULL;
                time_slice = 0;  
            }
        }
    }

}
/* End of your code here */

  printf("Average waiting time: %.2f\n", (float)total_waiting_time / (float)size);
  printf("Average response time: %.2f\n", (float)total_response_time / (float)size);

  free(data);
  return 0;
}



