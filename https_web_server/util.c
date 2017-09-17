#include "util.h"

size_t deal_response_data(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    response_data_t *res_data = (response_data_t *)userdata;

    int count = size*nmemb;

    memcpy(res_data->data, ptr, count);

    return count;
}


char *get_random_uuid(char *str)
{
    uuid_t uuid;

    uuid_generate(uuid);
    uuid_unparse(uuid, str);

    return str;
}



char *create_sessionid(const char *isDriver, char *sessionid)
{

    char str[36] = {0};

    if (strcmp(isDriver, "yes") == 0) {
        //司机
        sprintf(sessionid, "online-driver-%s", get_random_uuid(str));
    }
    else {
        //乘客
        sprintf(sessionid, "online-user-%s", get_random_uuid(str));
    }

    return sessionid;
}

