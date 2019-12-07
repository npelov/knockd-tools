/**
 * Compile:
 * gcc -s -o port-open port-open.c
 * cp -a port-open /usr/local/sbin
 * cd /usr/local/sbin
 * ln -sf port-open port-close
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <libgen.h>
#include <sys/types.h>
#include <unistd.h>

#define IPTABLES_OPEN_CMD "/sbin/iptables -A knock -s %s -p tcp --dport %d -j ACCEPT >/dev/null"
#define IPTABLES_CLOSE_CMD "/sbin/iptables -D knock -s %s -p tcp --dport %d -j ACCEPT >/dev/null 2>&1"

#define PROG_PORT_OPEN 1
#define PROG_PORT_CLOSE 2

char *name;
char progPath[1026];
int program = 1;
long timeout = 0;

bool verifyIp(const char *ip);
bool verifyPort(long port);

void usageOpen(){
  printf("Adds ACCEPT rule for a given IP and port using iptables. If timeout is grater than 0 starts port-close with the same parameters:\n");
  printf("Usage:\n");
  printf("       %s <ip> <port> [timeout=0]\n", name);
}

void usageClose(){
  printf("Removes the iptables ACCEPT rule, previously added with port-open after timeout seconds.\n");
  printf("Usage:\n");
  printf("       %s <ip> <port> [timeout=0]\n", name);
}

void usage() {
  switch(program){
    case PROG_PORT_OPEN:
      usageOpen();
      break;
    case PROG_PORT_CLOSE:
      usageClose();
      break;
  }
}

int portOpen(int argc, char **argv){
  long tmp;
  char **endPtr = NULL;

  if(argc < 3){
    usageOpen();
    return 1;
  }

  const char *ip = argv[1];
  if(!verifyIp(ip))
    return 1;

  tmp = strtol(argv[2], endPtr, 10);
  const int port = (int) tmp;
  if(!verifyPort(port))
    return 1;

  if(argc > 3){
    timeout=strtol(argv[3], endPtr, 10);
  }

  char *cmd = malloc(1500);
  sprintf(cmd, IPTABLES_OPEN_CMD, ip, (int)port);
//  printf("running cmd: %s\n", cmd);
  system(cmd);

  if(timeout > 0) {
    sprintf(cmd, "%sport-close %s %d %d", progPath, ip, (int)port, (int)timeout);
//    printf("port close cmd: %s\n", cmd);
    system(cmd);
  }
  free(cmd);

  return 0;
}

int portClose(int argc, char **argv){
  long tmp;
  char **endPtr = NULL;

  //printf("%d\n", argc);
  if(argc < 3){
    usageClose();
    return 1;
  }

  const char *ip = argv[1];
  if(!verifyIp(ip))
    return 1;

  tmp = strtol(argv[2], endPtr, 10);
  const int port = (int) tmp;
  if(!verifyPort(port))
    return 1;

  if(argc > 3){
    timeout=strtol(argv[3], endPtr, 10);
  }

  //printf("%d\n", timeout);
  if(timeout > 0) {
    // drop stdout and stderr
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    if(fork() != 0) {
      // parent
      return 0;
    }

    sleep(timeout);
  }
  char *cmd = malloc(1500);
  sprintf(cmd, IPTABLES_CLOSE_CMD, ip, port);
//  printf("running cmd: %s\n", cmd);
  system(cmd);
  free(cmd);
  return 0;
}

bool verifyIp(const char *ip){
  if(strlen(ip)<7){
    printf("IP should be least 7 characters\n");
    usage();
    return false;
  }
  if(strlen(ip)>15){
    printf("IP should be max 15 characters\n");
    usage();
    return false;
  }
  return true;
}

bool verifyPort(long port) {
  if(port<0) {
    printf("port cannot be negative");
    usage();
    return false;
  }
  if(port>65535) {
    printf("port must be less than 65535");
    usage();
    return false;
  }
  if(port==0) {
    printf("Port must be grater than 0");
    usage();
    return false;
  }
  return true;
}

int main(int argc, char **argv){

  if(argc<1){
    printf("argc < 1 !!!\n");
  }
  name = basename(argv[0]);
  if(strcmp(name, "port-open") == 0){
    program = PROG_PORT_OPEN;
  }else if(strcmp(name, "port-close") == 0) {
    program = PROG_PORT_CLOSE;
  }

  char *dirTemp = dirname(argv[0]);
  if (strlen(dirTemp) > 1024) {
    printf("Can't work with paths longer than 1024 chars\n");
    return 1;
  }
  if(strchr(argv[0], '/') == NULL) {
    progPath[0] = 0;
  }else {
    strncpy(progPath, dirTemp, 1024);
    int len = strlen(progPath);
    progPath[len] = '/';
    progPath[len+1] = 0;
  }

  switch(program){
    case PROG_PORT_OPEN:
      portOpen(argc, argv);
      break;
    case PROG_PORT_CLOSE:
      portClose(argc, argv);
      break;
  }

  return 0;
}

