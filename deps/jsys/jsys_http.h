#pragma once

#include <jsys.h>
#include <picohttpparser.h>
#include <errno.h>
#include <stdio.h>

enum jsys_http_state {
  IN_HEADER = 1,
  IN_BODY
};

typedef int (*jsys_httpd_on_headers)(jsys_descriptor*);
typedef int (*jsys_httpd_on_body)(jsys_descriptor*, char*, size_t);
typedef int (*jsys_httpd_on_request)(jsys_descriptor*);
typedef int (*jsys_httpd_on_response)(jsys_descriptor*);
typedef int (*jsys_httpd_on_connect)(jsys_descriptor*);
typedef int (*jsys_httpd_on_end)(jsys_descriptor*);
typedef int (*jsys_httpd_on_data)(jsys_descriptor*, size_t);

typedef struct jsys_httpd_settings jsys_httpd_settings;

struct jsys_httpd_settings {
  jsys_httpd_on_headers on_headers;
  jsys_httpd_on_request on_request;
  jsys_httpd_on_response on_response;
  jsys_httpd_on_body on_body;
  jsys_httpd_on_connect on_connect;
  jsys_httpd_on_end on_end;
  jsys_httpd_on_data on_data;
  size_t buffer_size;
  int max_headers;
  int buffers;
  int domain;
  int type;
  int padding;
  void* data;
};

typedef struct __attribute__((__packed__)) {
  int minor_version;
  size_t method_len;
  size_t path_len;
  uint32_t body_length;
  uint32_t body_bytes;
  uint16_t header_size;
  size_t num_headers;
  struct phr_header* headers;
  const char* path;
  const char* method;
  void* data;
  int state;
} jsys_http_server_context;

typedef struct {
  int minor_version;
  int status;
  int state;
  size_t status_message_len;
  const char* status_message;
  size_t body_length;
  size_t body_bytes;
  size_t num_headers;
  struct phr_header* headers;
  uint16_t header_size;
  void* data;
} jsys_http_client_context;

static const char* jsys_http_header_get(struct phr_header *headers, size_t num_headers, const char *name);
static int jsys_http_on_server_data(jsys_descriptor *client, size_t bytes);
static int jsys_http_on_server_connect(jsys_descriptor* client);
static int jsys_http_on_server_end(jsys_descriptor* client);
static int jsys_http_on_client_data(jsys_descriptor *client, size_t bytes);
static int jsys_http_on_client_connect(jsys_descriptor* client);
static int jsys_http_on_client_end(jsys_descriptor* client);
static jsys_descriptor* jsys_http_create_server(jsys_loop* loop, jsys_httpd_settings* http_settings);
static jsys_descriptor* jsys_http_create_client(jsys_loop* loop, jsys_httpd_settings* http_settings);
static jsys_httpd_settings* jsys_http_create_httpd_settings();
static int jsys_http_free_server(jsys_descriptor*);
static int jsys_http_free_client(jsys_descriptor*);
static jsys_httpd_settings* jsys_http_create_httpd_settings(jsys_loop* loop);

const char* jsys_http_header_get(struct phr_header *headers, size_t num_headers, const char *name) {
  //TODO: think about how we can improve looking up headers - possibly capture known headers during parsing
  size_t i;
  for (i = 0; i < num_headers; i ++) {
    if (headers[i].name_len == strlen(name) && strncasecmp(headers[i].name, name, strlen(name)) == 0) return headers[i].value;
  }
  return NULL;
}

jsys_httpd_settings* jsys_http_create_httpd_settings(jsys_loop* loop) {
  return (jsys_httpd_settings *)loop->alloc(1, sizeof(jsys_httpd_settings), "jsys_httpd_settings");
}

int jsys_http_on_server_data(jsys_descriptor *client, size_t bytes) {
  // bytes is always number of bytes from beginning of buffer
  jsys_stream_context* context = (jsys_stream_context*)client->data;
  jsys_http_server_context* http = (jsys_http_server_context*)context->data;
  jsys_httpd_settings* http_settings = (jsys_httpd_settings *)context->settings->data;
  char* next = (char*)context->in->iov_base;
  // todo: check flag so callbacks are optional
  http_settings->on_data(client, bytes); // todo: this should have offset also
  while (bytes) {
    // todo: there is a bug if body is not in same chunk of data as headers. the body will ovewrite the headers before on_request is called
    // maybe this should be correct behaviour so user always reads headers in on_headers
    if (http->state == IN_HEADER) {
      http->num_headers = (size_t)http_settings->max_headers;
      // pico expects the buffer to have complete set of headers, from the start of the buffer
      // if headers are not complete, it will return -2 and we are forced to store the current
      // chunk and retry again from beginning when we have next chunk
      ssize_t nread = phr_parse_request(next, bytes, (const char **)&http->method, &http->method_len, (const char **)&http->path, &http->path_len, &http->minor_version, http->headers, &http->num_headers, 0);
      if (nread == 0) {
        // TODO
        fprintf(stderr, "zero parse\n");
        return -1;
      }
      if (nread == -2) { // -2 means we have not had the full request yet
        // todo: this can be optimized. we don't need to copy if we are getting many small chunks of the same request. we can just move along the buffer and read at the next offset
        char* buf = (char*)context->in->iov_base;
        memcpy(buf, next, bytes);
        context->offset = bytes;
        return 0;
      }
      if (nread < 0) {
        // this means it's a bad request so we should just return failure code
        // and jsys will end the connection
        fprintf(stderr, "read error\n");
        context->offset = bytes;
        return -1;
      }
      //todo: handle chunked
      const char *header_value = jsys_http_header_get(http->headers, http->num_headers, "Content-Length");
      if (header_value != NULL) {
        http->body_length = strtoull(header_value, NULL, 10);
      } else {
        http->body_length = 0;
      }
      http->header_size = (size_t)nread - (http->headers[0].name - next);
      http_settings->on_headers(client);
      if (http->body_length == 0) {
        // TODO: check return codes!!
        // i need to know what to do here. consumer may want to pause/not receive any more requests in the middle of a
        // chunk of data
        http_settings->on_request(client);
      }
      next += nread;
      bytes -= (size_t)nread;
      if (http->body_length > 0) {
        http->state = IN_BODY;
        http->body_bytes = 0;
      }
    }
    if (http->state == IN_BODY) {
      size_t bytes_wanted = http->body_length - http->body_bytes;
      if (bytes >= bytes_wanted) {
        bytes -= bytes_wanted;
        http_settings->on_body(client, next, bytes_wanted);
        http->body_bytes += bytes_wanted;
        http->state = IN_HEADER;
        next += bytes_wanted;
        http_settings->on_request(client);
      } else {
        http->body_bytes += bytes;
        http_settings->on_body(client, next, bytes);
        next += bytes;
        bytes = 0;
      }
    }
  }
  context->offset = 0;
  return 0;
}

int jsys_http_on_client_data(jsys_descriptor *client, size_t bytes) {
  jsys_stream_context* context = (jsys_stream_context*)client->data;
  jsys_http_client_context* http = (jsys_http_client_context*)context->data;
  jsys_httpd_settings* http_settings = (jsys_httpd_settings *)context->settings->data;
  char* next = (char*)context->in->iov_base;
  while (bytes) {
    if (http->state == IN_HEADER) {
      http->num_headers = (size_t)http_settings->max_headers;
      ssize_t nread = phr_parse_response(next, bytes, &http->minor_version, &http->status, (const char**)&http->status_message, &http->status_message_len, http->headers, &http->num_headers, 0);
      if (nread == 0) {
        // TODO
        fprintf(stderr, "zero parse\n");
        return -1;
      }
      if (nread == -2) {
        fprintf(stderr, "realloc\n");
        char* buf = (char*)client->loop->alloc(1, context->in->iov_len, "realloc_in");
        //char* old = (char*)context->in->iov_base;
        memcpy(buf, next, bytes);
        context->offset = bytes;
        context->in->iov_base = buf;
        //free(old);
        return 0;
      }
      if (nread < 0) {
        // TODO
        fprintf(stderr, "read error\n");
        context->offset = bytes;
        return -1;
      }
      //TODO: support transfer-encoding: chunked
      const char *header_value = jsys_http_header_get(http->headers, http->num_headers, "Content-Length");
      if (header_value != NULL) {
        http->body_length = strtoull(header_value, NULL, 10);
      } else {
        http->body_length = 0;
      }
      http->header_size = (size_t)nread;
      http_settings->on_headers(client);
      if (http->body_length == 0) {
        // TODO: check return codes!!
        http_settings->on_response(client);
      }
      next += nread;
      bytes -= (size_t)nread;
      if (http->body_length > 0) {
        http->state = IN_BODY;
        http->body_bytes = 0;
      }
    }
    if (http->state == IN_BODY) {
      size_t bytes_wanted = http->body_length - http->body_bytes;
      if (bytes >= bytes_wanted) {
        bytes -= bytes_wanted;
        http_settings->on_body(client, next, bytes_wanted);
        http->body_bytes += bytes_wanted;
        http->state = IN_HEADER;
        next += bytes_wanted;
        http_settings->on_response(client);
      } else {
        http->body_bytes += bytes;
        http_settings->on_body(client, next, bytes);
        next += bytes;
        bytes = 0;
      }
    }
  }
  context->offset = 0;
  return 0;
}

int jsys_http_on_server_connect(jsys_descriptor* client) {
  jsys_stream_context* context = (jsys_stream_context*)client->data;
  jsys_httpd_settings* http_settings = (jsys_httpd_settings *)context->settings->data;
  jsys_http_server_context* http = (jsys_http_server_context*)client->loop->alloc(1, sizeof(jsys_http_server_context), "jsys_http_server_context");
  http->headers = (struct phr_header*)client->loop->alloc((size_t)http_settings->max_headers, sizeof(struct phr_header), "phr_header");
  context->data = http;
  http->body_bytes = 0;
  http->state = IN_HEADER;
  http_settings->on_connect(client);
  return 0;
}

int jsys_http_on_client_connect(jsys_descriptor* client) {
  jsys_stream_context* context = (jsys_stream_context*)client->data;
  jsys_httpd_settings* http_settings = (jsys_httpd_settings *)context->settings->data;
  jsys_http_client_context* http = (jsys_http_client_context*)client->loop->alloc(1, sizeof(jsys_http_client_context), "jsys_http_client_context");
  http->headers = (struct phr_header*)client->loop->alloc((size_t)http_settings->max_headers, sizeof(struct phr_header), "phr_header");
  context->data = http;
  http->body_bytes = 0;
  http->state = IN_HEADER;
  http_settings->on_connect(client);
  return 0;
}

int jsys_http_on_server_end(jsys_descriptor* client) {
  jsys_stream_context* context = (jsys_stream_context*)client->data;
  jsys_httpd_settings* http_settings = (jsys_httpd_settings *)context->settings->data;
  jsys_http_server_context* http = (jsys_http_server_context*)context->data;
  jsys_loop* loop = context->loop;
  loop->free(http->headers, "phr_header");
  loop->free(http, "jsys_http_server_context");
  http_settings->on_end(client);
  //loop->free(context, "jsys_stream_context");
  return 0;
}

int jsys_http_on_client_end(jsys_descriptor* client) {
  jsys_stream_context* context = (jsys_stream_context*)client->data;
  jsys_httpd_settings* http_settings = (jsys_httpd_settings *)context->settings->data;
  jsys_http_client_context* http = (jsys_http_client_context*)context->data;
  jsys_loop* loop = context->loop;
  loop->free(http->headers, "phr_header");
  loop->free(http, "jsys_http_server_context");
  http_settings->on_end(client);
  loop->free(context, "jsys_stream_context");
  return 0;
}

jsys_descriptor* jsys_http_create_server(jsys_loop* loop, jsys_httpd_settings* http_settings) {
  jsys_stream_settings* settings = (jsys_stream_settings*)loop->alloc(1, sizeof(jsys_stream_settings), "jsys_stream_settings");
  settings->buffers = http_settings->buffers;
  settings->on_connect = jsys_http_on_server_connect;
  settings->on_data = jsys_http_on_server_data;
  settings->on_end = jsys_http_on_server_end;
  jsys_descriptor* server = jsys_sock_create(loop, http_settings->domain, http_settings->type);
  server->data = settings;
  settings->data = http_settings;
  return server;
}

jsys_descriptor* jsys_http_create_client(jsys_loop* loop, jsys_httpd_settings* http_settings) {
  jsys_stream_settings* settings = (jsys_stream_settings*)loop->alloc(1, sizeof(jsys_stream_settings), "jsys_stream_settings");
  settings->buffers = http_settings->buffers;
  settings->on_connect = jsys_http_on_client_connect;
  settings->on_data = jsys_http_on_client_data;
  settings->on_end = jsys_http_on_client_end;
  jsys_descriptor* client = jsys_sock_create(loop, http_settings->domain, http_settings->type);
  client->data = settings;
  settings->data = http_settings;
  return client;
}

int jsys_http_free_server(jsys_descriptor* server) {
  jsys_stream_settings* settings = (jsys_stream_settings*)server->data;
  jsys_httpd_settings* httpd_settings = (jsys_httpd_settings*)settings->data;
  jsys_loop* loop = server->loop;
  loop->free(httpd_settings, "jsys_httpd_settings");
  loop->free(settings, "jsys_stream_settings");
  return jsys_descriptor_free(server);
}

int jsys_http_free_client(jsys_descriptor* client) {
  jsys_stream_settings* settings = (jsys_stream_settings*)client->data;
  jsys_httpd_settings* httpd_settings = (jsys_httpd_settings*)settings->data;
  jsys_loop* loop = client->loop;
  loop->free(httpd_settings, "jsys_httpd_settings");
  loop->free(settings, "jsys_stream_settings");
  return jsys_descriptor_free(client);
}

