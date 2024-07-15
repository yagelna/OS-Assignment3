#include "kernel/types.h"
#include "user/user.h"
#include "kernel/spinlock.h"
#include "kernel/sleeplock.h"
#include "kernel/fs.h"
#include "kernel/file.h"
#include "kernel/fcntl.h"

#include "kernel/crypto.h"

int main(void) {
  if(open("console", O_RDWR) < 0){
    mknod("console", CONSOLE, 0);
    open("console", O_RDWR);
  }
  dup(0);  // stdout
  dup(0);  // stderr

  printf("crypto_srv: starting\n");

  if (getpid() != 2) {
    printf("crypto_srv: PID is not 2. exiting...\n");
    exit(1);
  }

  while (1) {
    struct crypto_op* req;
    uint64 size;
    if (take_shared_memory_request((void**)&req, &size) < 0) {
      printf("crypto_srv: take_shared_memory_request failed\n");
      exit(1);
    }
    //check that the sizes are within reasonable limits.
    if (size != sizeof(struct crypto_op) + req->key_size + req->data_size) {
      printf("crypto_srv: invalid request size\n");
      exit(1);
    }

    if (req->state != CRYPTO_OP_STATE_INIT) {
      req->state = CRYPTO_OP_STATE_ERROR;
      if (remove_shared_memory_request(req, size) < 0) {
        printf("crypto_srv: remove_shared_memory_request failed\n");
        exit(1);
      }
    }

    if (req->type == CRYPTO_OP_TYPE_ENCRYPT) {
      printf("crypto_srv: encrypting message\n");
      for (int i = 0; i < req->data_size; i++) {
        req->payload[req->key_size + i] ^= req->payload[i % req->key_size]; 
      }
    } 
    else if (req->type == CRYPTO_OP_TYPE_DECRYPT) {
      printf("crypto_srv: decrypting message\n"); 
      for (int i = 0; i < req->data_size; i++) {
        req->payload[req->key_size + i] ^= req->payload[i % req->key_size];
      }
    } else {
      printf("crypto_srv: unknown crypto operation type\n");
      exit(1);
    }
     
    asm volatile ("fence rw,rw" : : : "memory"); 
    req->state = CRYPTO_OP_STATE_DONE;

    if (remove_shared_memory_request(req, size) < 0) {
      printf("crypto_srv: remove_shared_memory_request failed\n");
      exit(1);
    }
  }

  exit(0);
}
