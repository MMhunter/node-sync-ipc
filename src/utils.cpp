#include <node.h>
#include <uv.h>
#include <nan.h>
#include <string.h>
#include "basic.h"


#ifndef UTILS
#define UTILS

void debug(const char * format, ...) {
  if(!DEBUG) return;
     va_list args;
     va_start (args, format);
     vfprintf(stdout,format,args);
     va_end (args);
}

void error(const char * format, ...) {
   va_list args;
   va_start (args, format);
   vfprintf(stdout,format,args);
   va_end (args);
}

namespace utils{

  struct write_req_s{
      uv_write_t req;
      uv_buf_t buf;
      write_req_s* next;
      uv_stream_t* handle;
  };

  typedef struct write_req_s write_req_t;

  void free_write_req(uv_write_t *req);

  void write_cb(uv_write_t* req, int status);

  void free_write_req(uv_write_t *req) {
    write_req_t *wr = (write_req_t*) req;
    wr->next = NULL;
    wr->handle = NULL;
    free(wr->buf.base);
    free(wr);
  }

  void write(write_req_t *wreq) {
    debug("writing data %s \n", wreq->buf.base);
    uv_write(&wreq->req, wreq->handle, &wreq->buf, 1, write_cb);
  }

  void writeData(uv_stream_t * handle, char* values[], int length) {

    write_req_t *head = NULL;
    write_req_t *last = NULL;

    for(int i = 0; i < length; i ++) {

      write_req_t *wreq = (write_req_t*) malloc(sizeof(write_req_t));
      debug("prepare writing data %s \n", values[i]);
      wreq->buf = uv_buf_init(values[i], strlen(values[i]));
      wreq->next = NULL;
      wreq->handle = NULL;


      if (i == 0) {
         head = wreq;
      }
      if (last != NULL) {
         last->next = wreq;
      }
      last = wreq;
    }

    if (head != NULL) {
      head->handle = handle;
      write(head);
    }
  }

  void write_cb(uv_write_t* req, int status) {
    if (status != 0) {
         error("Client Write error %s\n", uv_err_name(status));
    }
     write_req_t* wr = (write_req_t* )req;
     if(wr->next != NULL){
          write_req_t* next = wr->next;
          next->handle = wr->handle;
          write(next);
     }
     free_write_req(req);
  }

}


#endif


