#include "account.h"
#include "database.h"
#include <arpa/inet.h>
#include <errno.h>
#include <p101_c/p101_stdio.h>
#include <p101_c/p101_stdlib.h>
#include <string.h>

const funcMapping acc_func[] = {
    {ACC_Create,  account_create},
    {ACC_Login,   account_login },
    {ACC_Logout,  account_logout},
    {ACC_Edit,    NULL          },
    {SYS_Success, NULL          }  // Null termination for safety
};

ssize_t account_create(request_t *request)
{
    DBO         userDB;
    DBO         index_userDB;
    char        user_name[]  = "users";
    char        index_name[] = "index_user";
    void       *existing;
    uint8_t     user_len;
    uint8_t     pass_len;
    char       *ptr;
    const char *username;
    const char *password;
    char       *copy;
    int         user_id;

    // server default to 0
    uint16_t sender_id = SERVER_ID;

    userDB.name       = user_name;
    userDB.db         = NULL;
    index_userDB.name = index_name;
    index_userDB.db   = NULL;

    copy = NULL;

    printf("in account_create %d \n", *request->client_fd);

    if(database_open(&userDB, &request->err) < 0)
    {
        perror("database error");
        request->code = SERVER_ERROR;
        goto error;
    }

    if(database_open(&index_userDB, &request->err) < 0)
    {
        perror("database error");
        request->code = SERVER_ERROR;
        goto error;
    }

    // start from username len
    ptr = (char *)request->content + HEADER_SIZE + 1;

    memcpy(&user_len, ptr, sizeof(user_len));
    ptr += sizeof(user_len);

    username = ptr;

    // start from password len
    ptr += user_len + 1;
    memcpy(&pass_len, ptr, sizeof(pass_len));
    ptr += sizeof(pass_len);

    password = ptr;

    printf("username: %.*s\n", (int)user_len, username);
    printf("password: %.*s\n", (int)pass_len, password);

    // check user exists
    existing = retrieve_byte(userDB.db, username, user_len);
    if(existing)
    {
        printf("Retrieved password: %.*s\n", (int)pass_len, (char *)existing);
        request->code = USER_EXISTS;
        free(existing);
        goto error;
    }

    (*request->user_count)++;
    *request->session_id = *request->user_count;

    printf("request->user_count: %d\n", *request->user_count);
    printf("request->session_id: %d\n", *request->session_id);

    // Store user
    if(store_byte(userDB.db, username, user_len, password, pass_len) != 0)
    {
        perror("store_byte");
        request->code = SERVER_ERROR;
        goto error;
    }

    copy = strndup(username, user_len);
    if(!copy)
    {
        perror("Failed to allocate memory");
        request->code = SERVER_ERROR;
        goto error;
    }

    // Store user index
    if(store_int(index_userDB.db, copy, *request->session_id) < 0)
    {
        perror("update user_index");
        request->code = SERVER_ERROR;
        goto error;
    }

    // for checking
    if(retrieve_int(index_userDB.db, copy, &user_id) < 0)
    {
        printf("account account retrieve_int error\n");
        request->code = SERVER_ERROR;
        goto error;
    }
    printf("account login: user_id: %.*d\n", (int)sizeof(*request->session_id), user_id);

    ptr = (char *)request->response;
    // tag
    *ptr++ = SYS_Success;
    // version
    *ptr++ = TWO;

    // sender_id
    sender_id = htons(sender_id);
    memcpy(ptr, &sender_id, sizeof(sender_id));
    ptr += sizeof(sender_id);

    // payload len
    request->response_len = htons(request->response_len);
    memcpy(ptr, &request->response_len, sizeof(request->response_len));
    ptr += sizeof(request->response_len);

    *ptr++ = ENUMERATED;
    *ptr++ = sizeof(uint8_t);
    *ptr++ = ACC_Create;

    dbm_close(userDB.db);
    dbm_close(index_userDB.db);
    free(copy);
    return 0;

error:
    dbm_close(userDB.db);
    dbm_close(index_userDB.db);
    free(copy);

    return -1;
}

ssize_t account_login(request_t *request)
{
    DBO         userDB;
    DBO         index_userDB;
    char        user_name[]  = "users";
    char        index_name[] = "index_user";
    void       *existing;
    datum       output;
    uint8_t     user_len;
    uint8_t     pass_len;
    char       *ptr;
    const char *username;
    const char *password;
    char       *copy;
    int         user_id;

    // server default to 0
    uint16_t sender_id = SERVER_ID;

    userDB.name       = user_name;
    userDB.db         = NULL;
    index_userDB.name = index_name;
    index_userDB.db   = NULL;

    copy = NULL;

    printf("in account_login %d \n", *request->client_fd);

    memset(&output, 0, sizeof(datum));

    if(database_open(&userDB, &request->err) < 0)
    {
        perror("database error");
        request->code = SERVER_ERROR;
        goto error;
    }

    if(database_open(&index_userDB, &request->err) < 0)
    {
        perror("database error");
        request->code = SERVER_ERROR;
        goto error;
    }

    // start from username len
    ptr = (char *)request->content + HEADER_SIZE + 1;

    memcpy(&user_len, ptr, sizeof(user_len));
    ptr += sizeof(user_len);

    username = ptr;

    // start from password len
    ptr += user_len + 1;
    memcpy(&pass_len, ptr, sizeof(pass_len));
    ptr += sizeof(pass_len);

    password = ptr;

    printf("username: %.*s\n", (int)user_len, username);
    printf("password: %.*s\n", (int)pass_len, password);

    // check user exists
    existing = retrieve_byte(userDB.db, username, user_len);
    if(!existing)
    {
        perror("Username not found");
        request->code = INVALID_USER_ID;
        goto error;
    }

    printf("Retrieved password: %.*s\n", (int)pass_len, (char *)existing);

    if(memcmp(existing, password, pass_len) != 0)
    {
        free(existing);
        request->code = INVALID_AUTH;
        goto error;
    }

    copy = strndup(username, user_len);
    if(!copy)
    {
        perror("Failed to allocate memory");
        request->code = SERVER_ERROR;
        goto error;
    }

    if(retrieve_int(index_userDB.db, copy, &user_id) < 0)
    {
        printf("account login retrieve_int error\n");
        free(existing);
        request->code = SERVER_ERROR;
        goto error;
    }
    printf("account login: user_id: %.*d\n", (int)sizeof(*request->session_id), user_id);

    ptr = (char *)request->response;
    // tag
    *ptr++ = ACC_Login_Success;
    // version
    *ptr++ = TWO;

    // sender_id
    sender_id = htons(sender_id);
    memcpy(ptr, &sender_id, sizeof(sender_id));
    ptr += sizeof(sender_id);

    // payload len
    request->response_len = 4;
    request->response_len = htons(request->response_len);
    memcpy(ptr, &request->response_len, sizeof(request->response_len));
    ptr += sizeof(request->response_len);

    *ptr++ = INTEGER;
    *ptr++ = sizeof(uint16_t);

    user_id = htons((uint16_t)user_id);
    memcpy(ptr, &user_id, sizeof(user_id));

    *request->session_id = ntohs((uint16_t)user_id);

    printf("session_id %d\n", *request->session_id);

    dbm_close(userDB.db);
    dbm_close(index_userDB.db);
    free(existing);
    free(copy);
    return 0;

error:
    dbm_close(userDB.db);
    dbm_close(index_userDB.db);
    free(copy);
    return -1;
}

ssize_t account_logout(request_t *request)
{
    printf("in account_logout %d \n", *request->client_fd);

    request->response_len = 0;

    request->err = 0;
    return -1;
}
