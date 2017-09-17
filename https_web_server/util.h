#ifndef __OBO_BUSI_H__
#define __OBO_BUSI_H__

#define MYHTTPD_SIGNATURE   "MoCarHttpd v0.1"

#include <event2/bufferevent.h>
#include <event2/bufferevent_ssl.h>
#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/keyvalq_struct.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <uuid/uuid.h>


#define OBO_TABLE_USER "OBO_TABLE_USER"

#define DATA_SERVER_IP "101.200.1.1"
#define DATA_SERVER_PORT "8081"

#define RESPONSE_DATA_LEN 4096
#define SESSION_ID_LEN 256


typedef struct response_data
{
    char data[RESPONSE_DATA_LEN];
    int data_len;
}response_data_t;

void login_cb (struct evhttp_request *req, void *arg);
void reg_cb (struct evhttp_request *req, void *arg);

size_t deal_response_data(char *ptr, size_t size, size_t nmemb, void *userdata);

char *get_random_uuid(char *str);
char *create_sessionid(const char *isDriver, char *sessionid);




#endif
