#include "../include/database.h"
#include <errno.h>
#include <fcntl.h>
#include <p101_c/p101_stdio.h>
#include <p101_c/p101_stdlib.h>
#include <string.h>
#include <sys/stat.h>

#pragma GCC diagnostic ignored "-Waggregate-return"

ssize_t database_open(DBO *dbo, int *err)
{
    dbo->db = dbm_open(dbo->name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if(!dbo->db)
    {
        perror("dbm_open failed");
        *err = errno;
        return -1;
    }
    return 0;
}

int store_string(DBM *db, const char *key, const char *value)
{
    const_datum key_datum   = MAKE_CONST_DATUM(key);
    const_datum value_datum = MAKE_CONST_DATUM(value);

    return dbm_store(db, *(datum *)&key_datum, *(datum *)&value_datum, DBM_REPLACE);
}

int store_int(DBM *db, const char *key, int value)
{
    const_datum key_datum = MAKE_CONST_DATUM(key);
    datum       value_datum;
    int         result;

    value_datum.dptr = (char *)malloc(TO_SIZE_T(sizeof(int)));

    if(value_datum.dptr == NULL)
    {
        return -1;
    }

    memcpy(value_datum.dptr, &value, sizeof(int));
    value_datum.dsize = sizeof(int);

    result = dbm_store(db, *(datum *)&key_datum, value_datum, DBM_REPLACE);

    free(value_datum.dptr);
    return result;
}

int store_byte(DBM *db, const void *key, size_t k_size, const void *value, size_t v_size)
{
    const_datum key_datum   = MAKE_CONST_DATUM_BYTE(key, k_size);
    const_datum value_datum = MAKE_CONST_DATUM_BYTE(value, v_size);

    return dbm_store(db, *(datum *)&key_datum, *(datum *)&value_datum, DBM_REPLACE);
}

char *retrieve_string(DBM *db, const char *key)
{
    const_datum key_datum;
    datum       result;
    char       *retrieved_str;

    key_datum = MAKE_CONST_DATUM(key);

    result = dbm_fetch(db, *(datum *)&key_datum);

    if(result.dptr == NULL)
    {
        return NULL;
    }

    retrieved_str = (char *)malloc(TO_SIZE_T(result.dsize));

    if(!retrieved_str)
    {
        return NULL;
    }

    memcpy(retrieved_str, result.dptr, TO_SIZE_T(result.dsize));

    return retrieved_str;
}

int retrieve_int(DBM *db, const char *key, int *result)
{
    datum       fetched;
    const_datum key_datum = MAKE_CONST_DATUM(key);

    fetched = dbm_fetch(db, *(datum *)&key_datum);

    if(fetched.dptr == NULL || fetched.dsize != sizeof(int))
    {
        return -1;
    }

    memcpy(result, fetched.dptr, sizeof(int));

    return 0;
}

void *retrieve_byte(DBM *db, const void *key, size_t size)
{
    const_datum key_datum;
    datum       result;
    char       *retrieved_str;

    key_datum = MAKE_CONST_DATUM_BYTE(key, size);

    result = dbm_fetch(db, *(datum *)&key_datum);

    if(result.dptr == NULL)
    {
        return NULL;
    }

    retrieved_str = (char *)malloc(TO_SIZE_T(result.dsize));

    if(!retrieved_str)
    {
        return NULL;
    }

    memcpy(retrieved_str, result.dptr, TO_SIZE_T(result.dsize));

    return retrieved_str;
}

ssize_t init_pk(DBO *dbo, const char *pk_name, int *pk)
{
    int err;

    if(database_open(dbo, &err) < 0)
    {
        perror("database error");
        return -1;
    }

    if(retrieve_int(dbo->db, pk_name, pk) < 0)
    {
        // *pk = 0;
        *pk = 2;
        if(store_int(dbo->db, pk_name, *pk) != 0)
        {
            return -1;
        }
    }

    printf("Retrieved user_count: %d\n", *pk);

    dbm_close(dbo->db);
    return 0;
}
