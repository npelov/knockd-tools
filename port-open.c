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

#define IPTABLES_ALLOW_CMD "/sbin/iptables -A knock_allow -s %s -j ACCEPT >/dev/null"
#define IPTABLES_REMOVE_CMD "/sbin/iptables -D knock_allow -s %s -j ACCEPT >/dev/null 2>&1"

#define IPTABLES_BLOCK_CMD "/sbin/iptables -A knock_block -s %s -j DROP >/dev/null"
#define IPTABLES_UNBLOCK_CMD "/sbin/iptables -D knock_block -s %s -j DROP >/dev/null 2>&1"

#define PROG_PORT_OPEN 1
#define PROG_PORT_CLOSE 2
#define PROG_IP_ALLOW 3
#define PROG_IP_REMOVE 4
#define PROG_IP_BLOCK 5
#define PROG_IP_UNBLOCK 6


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

void usageIpAdd(int action) {
  const char *ruleName;
  const char *removeProgName;
  switch(action) {
    case PROG_IP_ALLOW:
      ruleName = "ACCEPT";
      removeProgName = "ip-remove";
      break;
    case PROG_IP_BLOCK:
      ruleName = "REJECT";
      removeProgName = "ip-unblock";
      break;
    default:
      printf("Bad action: %d\n", action);
      return;
  }
  printf("Adds %s rule for a given IP using iptables. If timeout is grater than 0 starts %s with the same parameters:\n", ruleName, removeProgName);
  printf("Usage:\n");
  printf("       %s <ip> [timeout=0]\n", name);
}

void usageIpRemove(int action){
  const char *ruleName;
  const char *addProgName;
  switch(action) {
    case PROG_IP_REMOVE:
      ruleName = "ACCEPT";
      addProgName = "ip-remove";
      break;
    case PROG_IP_UNBLOCK:
      ruleName = "REJECT";
      addProgName = "ip-unblock";
      break;
    default:
      printf("Bad action: %d\n", action);
      return;
  }
  printf("Removes the iptables %s rule, previously added with %s after timeout seconds.\n", ruleName, addProgName);
  printf("Usage:\n");
  printf("       %s <ip> [timeout=0]\n", name);
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

int ipAllowBlock(int action, int argc, char **argv){
  char **endPtr = NULL;

  if(argc < 2){
    usageIpAdd(action);
    return 1;
  }

  const char *ip = argv[1];
  if(!verifyIp(ip))
    return 1;

  if(argc > 2){
    timeout=strtol(argv[2], endPtr, 10);
  }

  char *cmd = malloc(1500);
  const char *cmdTemplate = NULL;
  const char *closeCmdTemplate;

  switch(action) {
    case PROG_IP_ALLOW:
      cmdTemplate = IPTABLES_ALLOW_CMD;
      closeCmdTemplate = "%sip-remove %s %d";
      break;
    case PROG_IP_BLOCK:
      cmdTemplate = IPTABLES_BLOCK_CMD;
      closeCmdTemplate = "%sip-unblock %s %d";
      break;
    default:
      printf("ipAllowBlock(): invalid action %d\n", action);
      return 1;
  }
  if (cmdTemplate) {
    sprintf(cmd, cmdTemplate, ip);
//    printf("running cmd: %s\n", cmd);
    system(cmd);

    if (timeout > 0) {
      sprintf(cmd, closeCmdTemplate, progPath, ip, (int) timeout);
//      printf("ip remove cmd: %s\n", cmd);
      system(cmd);
    }
  }
  free(cmd);
  if (!cmdTemplate) {
    printf("Invalid action %d\n", action);
  }

  return 0;
}

int ipRemoveUnblock(int action, int argc, char **argv){
  char **endPtr = NULL;

  //printf("%d\n", argc);
  if(argc < 2){
    usageIpRemove(action);
    return 1;
  }

  const char *ip = argv[1];
  if(!verifyIp(ip))
    return 1;


  if(argc > 2){
    timeout=strtol(argv[2], endPtr, 10);
  }

  const char *cmdTemplate = NULL;

  switch(action) {
    case PROG_IP_REMOVE:
      cmdTemplate = IPTABLES_REMOVE_CMD;
      break;
    case PROG_IP_UNBLOCK:
      cmdTemplate = IPTABLES_UNBLOCK_CMD;
      break;
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
    // child
    sleep(timeout);
  }
  char *cmd = malloc(1500);
  sprintf(cmd, cmdTemplate, ip);
//  printf("running cmd: %s\n", cmd);
  system(cmd);
  free(cmd);
  return 0;
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
  }else if(strcmp(name, "ip-allow") == 0) {
    program = PROG_IP_ALLOW;
  }else if(strcmp(name, "ip-remove") == 0) {
    program = PROG_IP_REMOVE;
  }else if(strcmp(name, "ip-block") == 0) {
    program = PROG_IP_BLOCK;
  }else if(strcmp(name, "ip-unblock") == 0) {
    program = PROG_IP_UNBLOCK;
  }

  char *dirTemp = dirname(argv[0]);
  if (strlen(dirTemp) > 1024) {
    printf("Can't work with paths longer than 1024 chars\n");
    return 1;
  }
//  if(strchr(argv[0], '/') == NULL) {
  if(false) {
    progPath[0] = 0;
  }else {
    strncpy(progPath, dirTemp, 1024);
    int len = strlen(progPath);
    progPath[len] = '/';
    progPath[len+1] = 0;
  }
//  printf("path is %s\n", progPath);

  switch(program){
    case PROG_PORT_OPEN:
      portOpen(argc, argv);
      break;
    case PROG_PORT_CLOSE:
      portClose(argc, argv);
      break;
    case PROG_IP_ALLOW:
    case PROG_IP_BLOCK:
      ipAllowBlock(program, argc, argv);
      break;
    case PROG_IP_REMOVE:
    case PROG_IP_UNBLOCK:
      ipRemoveUnblock(program, argc, argv);
      break;
    default:
      printf("Invalid program number %d", program);
  }

  return 0;
}

