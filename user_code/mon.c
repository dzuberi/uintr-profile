#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <x86intrin.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
// #include <x86gprintrin.h>

#include <errno.h>

#define __USE_GNU
#include <pthread.h>
#include <sched.h>

#include "../kernel_code/uittmonL.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#ifndef __NR_uintr_register_handler
#define __NR_uintr_register_handler 471
#define __NR_uintr_unregister_handler 472
#define __NR_uintr_create_fd 473
#define __NR_uintr_register_sender 474
#define __NR_uintr_unregister_sender 475
#define __NR_uintr_wait 476
#endif
#define uintr_register_handler(handler, flags) syscall(__NR_uintr_register_handler, handler, flags)
#define uintr_unregister_handler(flags) syscall(__NR_uintr_unregister_handler, flags)
#define uintr_create_fd(vector, flags) syscall(__NR_uintr_create_fd, vector, flags)
#define uintr_register_sender(fd, flags) syscall(__NR_uintr_register_sender, fd, flags)
#define uintr_unregister_sender(ipi_idx, flags) syscall(__NR_uintr_unregister_sender, ipi_idx, flags)
#define uintr_wait(flags) syscall(__NR_uintr_wait, flags)
#define NUM_TRIES 4000000
volatile uint64_t* sp = NULL;
volatile int uintr_fd, uipi_index;
pid_t receiver_pid;

volatile int received = 0;

volatile int driver_fd = -1;

volatile int initialized = 0;

struct uintr_upid {
    struct {
        volatile uint8_t status;         // bit 0: ON, bit 1: SN, bit 2-7: reserved
        volatile uint8_t reserved1;      // Reserved
        volatile uint8_t nv;             // Notification vector
        volatile uint8_t reserved2;      // Reserved
        volatile uint32_t ndst;          // Notification destination
    } nc;                       // Notification control
    volatile uint64_t puir;              // Posted user interrupt requests
};

struct uintr_upid *receiver_upid;

void driver_give_info(){
  printf("pid: %d\n",getpid());
  if (ioctl(driver_fd, IOCTL_SET_PID, getpid()) < 0) {
    perror("IOCTL_SET_PID failed");
    close(driver_fd);
    exit(1);
  }
  initialized = 1;
}

void __attribute__((interrupt))
__attribute__((target("general-regs-only", "inline-all-stringops")))
ui_handler(struct __uintr_frame *ui_frame,
	   unsigned long long vector)
{
  received = 1;
}

void reciever(){
  if (uintr_register_handler(ui_handler, 0))
    exit(-1);

  uintr_fd = uintr_create_fd(0, 0);
  if (uintr_fd < 0)
    exit(-1);

  // Enable interrupts
  driver_give_info();

  _stui();
  printf("receiver started\n");
  asm( "mov %%rsp, %0" : "=rm" ( sp ));
  asm("sfence");
  printf("sp: %llx\n", sp);
  while(!received);
}

void map_upid() {
  while(!initialized);
  uint64_t offset = 0;
  if (ioctl(driver_fd, IOCTL_GET_OFFSET, &offset) < 0) {
    perror("IOCTL_GET_OFFSET failed");
    exit(1);
  }

  printf("offset:%llx\n", offset);

  receiver_upid = (struct uintr_upid *) ((uint64_t) mmap(NULL, sizeof (struct uintr_upid), PROT_READ, MAP_SHARED, driver_fd, 0) + (uint64_t) offset);
  printf("receiver_upid:%llx\n", receiver_upid);
  if (receiver_upid == MAP_FAILED) {
    fprintf(stderr, "mmap failed: %s\n", strerror(errno));
    exit(1);
  }
}

/*void driver_invoke(){*/
/*  int fd = open("/dev/uittmon", O_RDWR);*/
/**/
/*  struct uittmon_io ioctl_args = {0};*/
/**/
/*  ioctl_args.sp = sp;*/
/*  ioctl_args.receiver_pid = receiver_pid;*/
/*  ioctl_args.uipi_index = uipi_index;*/
/*  ioctl_args.uintr_fd = uintr_fd;*/
/**/
/*  if (ioctl(fd, IOCTL_PROCESS_LIST, &ioctl_args) < 0) {*/
/*    perror("IOCTL_PROCESS_LIST failed");*/
/*    close(fd);*/
/*  }*/
/*  close(fd);*/
/*}*/


void sender_listener(){
  while(sp) printf("waiting sp = 0x%llx, on = %x\n",sp, receiver_upid->nc.status);
  map_upid();
  printf("on = %x\n", receiver_upid->nc.status);
  uipi_index = uintr_register_sender(uintr_fd, 0);
  //driver_invoke(sp,uipi_index,uintr_fd);
  //driver_invoke();
  printf("sp = 0x%llx\n", sp);
  printf("upid* = 0x%llx\n", receiver_upid);
  //uint8_t original = receiver_upid->nc.nv;
  uint8_t original = receiver_upid->nc.nv;
  printf("original = 0x%x\n", original);
  printf("on = %x\n", receiver_upid->nc.status);
  _senduipi(uipi_index);
  //uint64_t t0 = _rdtsc();
  printf("nv = 0x%x\n", receiver_upid->nc.nv);
  //while(receiver_upid->nc.nv == original);
  //uint64_t t1 = _rdtsc();
  //printf("nv = 0x%x\n", receiver_upid->nc.nv);
  //while(receiver_upid->nc.nv);
  //uint64_t t2 = _rdtsc();
  printf("final nv = 0x%x\n", receiver_upid->nc.nv);
  while(!received);
  printf("finalal nv = 0x%x\n", receiver_upid->nc.nv);
  //printf("\
  //  t0=%lu\n\
  //  t1=%lu\n\
  //  t1=%lu\n\
  //",t0, t1, t2);
  //while(1);
  //exit(0);
}

int main(){
  driver_fd = open("/dev/uittmon", O_RDWR);
  int core1 = 56,core2 = 1;
  core2=0;
  printf("Give me two sibling cores: <number> <number>\n");
  //scanf("%d %d ", core1,core2);
  //set_thread_affinity(core1); 
  receiver_pid = getpid();

  //bind sender to core 1
  pthread_attr_t attrs;
  memset(&attrs, 0, sizeof(attrs));
  pthread_attr_init(&attrs);
  cpu_set_t set;
  CPU_ZERO(&set);
  CPU_SET(core2, &set);
  pthread_attr_setaffinity_np(&attrs, sizeof(set), &set);
  pthread_t pt;

  //get receiver pid
  receiver_pid = getpid();

  //bind receiver to core 56 (sibling)
  CPU_ZERO(&set);
  CPU_SET(core1, &set);
  pthread_setaffinity_np(pthread_self(), sizeof(set), &set);


  if (pthread_create(&pt, &attrs, &sender_listener, NULL)) exit(-1);
  reciever();
  pthread_join(pt,NULL);
  close(driver_fd);
}
