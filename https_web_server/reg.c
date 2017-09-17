#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

#include <openssl/ssl.h>
#include <openssl/err.h>


#include <cJSON.h>
#include <curl/curl.h>
#include "util.h"



void reg_cb (struct evhttp_request *req, void *arg)
{ 
    struct evbuffer *evb = NULL;
    const char *uri = evhttp_request_get_uri (req);
    struct evhttp_uri *decoded = NULL;

    /* 判断 req 是否是GET 请求 */
    if (evhttp_request_get_command (req) == EVHTTP_REQ_GET)
    {
        struct evbuffer *buf = evbuffer_new();
        if (buf == NULL) return;
        evbuffer_add_printf(buf, "Requested: %s\n", uri);
        evhttp_send_reply(req, HTTP_OK, "OK", buf);
        return;
    }

    /* 这里只处理Post请求, Get请求，就直接return 200 OK  */
    if (evhttp_request_get_command (req) != EVHTTP_REQ_POST)
    { 
        evhttp_send_reply (req, 200, "OK", NULL);
        return;
    }

    printf ("Got a POST request for <%s>\n", uri);

    //判断此URI是否合法
    decoded = evhttp_uri_parse (uri);
    if (! decoded)
    { 
        printf ("It's not a good URI. Sending BADREQUEST\n");
        evhttp_send_error (req, HTTP_BADREQUEST, 0);
        return;
    }

    /* Decode the payload */
    struct evbuffer *buf = evhttp_request_get_input_buffer (req);
    evbuffer_add (buf, "", 1);    /* NUL-terminate the buffer */
    char *payload = (char *) evbuffer_pullup (buf, -1);
    int post_data_len = evbuffer_get_length(buf);
    char request_data_buf[4096] = {0};
    memcpy(request_data_buf, payload, post_data_len);

    printf("[post_data][%d]=\n %s\n", post_data_len, payload);


    /*
       具体的：可以根据Post的参数执行相应操作，然后将结果输出
       ...
       */

    /*
     *      * ==== 给服务端的协议 ====
     *          https://ip:port/reg [json_data]
     *         {
     *           username: "gailun",
     *           password: "123123",
     *           driver:   "yes/no",
     *           tel:      "13331333333",
     *           email:    "danbing_at@163.cn",
     *           id_card:  "2104041222121211122"
     *         }
     *                                                                       *
     *                                                                            *
     *                                                                                 */

    int is_reg_succ = 0;//0 succ -1 fail
    cJSON *root= cJSON_Parse(request_data_buf);

    //unpack json
    cJSON *username = cJSON_GetObjectItem(root, "username");
    cJSON *password = cJSON_GetObjectItem(root, "password");
    cJSON *driver = cJSON_GetObjectItem(root, "driver");
    cJSON *email = cJSON_GetObjectItem(root, "email");
    cJSON *tel = cJSON_GetObjectItem(root, "tel");
    cJSON *id_card = cJSON_GetObjectItem(root, "id_card");

    printf("username = %s\n", username->valuestring);
    printf("password = %s\n", password->valuestring);
    printf("email = %s\n", email->valuestring);
    printf("tel = %s\n", tel->valuestring);
    printf("driver = %s\n", driver->valuestring);
    printf("id_card = %s\n", id_card->valuestring);



    //向远程服务器发送一个入库请求

    /*
     *
     *    https://ip:port/persistent [json_data]  
     *        {
     *            cmd: "insert",
     *            busi: "reg",
     *            table: "OBO_TABLE_USER",

     *            username:  "盖伦",
     *            password:  "ADSWADSADWQ(MD5加密之后的)",
     *            tel     :  "13332133313",
     *            email   :  "danbing_at@163.com",
     *            id_card :  "21040418331323",
     *            driver  :  "yes",
     *         }
     *
     *
     *
     *
     * */

    cJSON* rqst_dt = cJSON_CreateObject();
    cJSON_AddStringToObject(rqst_dt, "cmd", "insert");
    cJSON_AddStringToObject(rqst_dt, "busi", "reg");
    cJSON_AddStringToObject(rqst_dt, "table", OBO_TABLE_USER);
    cJSON_AddStringToObject(rqst_dt, "username", username->valuestring);
    cJSON_AddStringToObject(rqst_dt, "password", password->valuestring);
    cJSON_AddStringToObject(rqst_dt, "tel", tel->valuestring);
    cJSON_AddStringToObject(rqst_dt, "email", email->valuestring);
    cJSON_AddStringToObject(rqst_dt, "id_card", id_card->valuestring);
    cJSON_AddStringToObject(rqst_dt, "driver", driver->valuestring);

    //向远程存储数据库发送的数据
    char *reg_data = cJSON_Print(rqst_dt);
    response_data_t res_data;

    memset(&res_data, 0, sizeof(res_data));


    CURL *curl = NULL;
    CURLcode res;

    curl = curl_easy_init();
    if (curl == NULL) {
        printf("curl init error");
        //返回一个错误json 给前端
    }

    curl_easy_setopt(curl, CURLOPT_URL, "https://101.200.170.171:8081/persistent");
    //忽略CA认证
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);

    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, reg_data);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, deal_response_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &res_data);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        printf("curl to data server perform error res = %d\n", res);
        //返回一个错误json 给前端
    }

    printf("curl to data-server succ!\n");

    //处理存储服务器返回的 存储结果
    printf("reponse_from_data = [%s]\n", res_data.data);
    cJSON *response_from_data = cJSON_Parse(res_data.data);
    cJSON *result = cJSON_GetObjectItem(response_from_data, "result");
    if (result && strcmp(result->valuestring, "ok") == 0) {
        //入库成功
        printf("insert OBO_TABLE_USER succ!\n");
        is_reg_succ = 0;
    }
    else {
        //入库失败
        printf("insert OBO_TABLE_USER fail\n");
        is_reg_succ = -1;
    }



    //TODO 如果入库成功
    //应该创建一个sessionid





 
    printf("delete json...\n");
    cJSON_Delete(root);
    cJSON_Delete(rqst_dt);
    free(reg_data);
    cJSON_Delete(response_from_data);
    printf("delete json end...\n");


    char *response_data  = NULL;

    cJSON *response_jni = cJSON_CreateObject();
    if (is_reg_succ == 0) {
        cJSON_AddStringToObject(response_jni, "result", "ok");
    }
    else {
        cJSON_AddStringToObject(response_jni, "result", "error");
        cJSON_AddStringToObject(response_jni, "reason", "insert OBO_TABLE_USER ERROR");
    }

    response_data = cJSON_Print(response_jni);

    printf("make response_data succ\n");


    /* This holds the content we're sending. */

    //HTTP header
    evhttp_add_header(evhttp_request_get_output_headers(req), "Server", MYHTTPD_SIGNATURE);
    evhttp_add_header(evhttp_request_get_output_headers(req), "Content-Type", "text/plain; charset=UTF-8");
    evhttp_add_header(evhttp_request_get_output_headers(req), "Connection", "close");

    evb = evbuffer_new ();
    evbuffer_add_printf(evb, "%s", response_data);
    //将封装好的evbuffer 发送给客户端
    evhttp_send_reply(req, HTTP_OK, "OK", evb);

    if (decoded)
        evhttp_uri_free (decoded);
    if (evb)
        evbuffer_free (evb);


    printf("[response]:\n");
    printf("%s\n", response_data);

    cJSON_Delete(response_jni);
    free(response_data);
}
