#include<stdio.h>
#include "database.h"
#include "sqlite3.h"
#include "my_utils.h"

int count_rows_callback(void *rows_count, int column_count, char **data, char **columns) 
{
    int *p_count = (int *) rows_count;
    (*p_count)++;

    return 0;
}
int count_rows(char *sql_msg, sqlite3 *db, char *zErrMsg, int *rc) 
{
    int rows_count = 0;
    *rc = sqlite3_exec(db, sql_msg, count_rows_callback, &rows_count, &zErrMsg);
    if (*rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }

    return rows_count;
}


int show_website_callback(void *rows_count, int column_count, char **data, char **columns) 
{
    int *p_count = (int *) rows_count;
    (*p_count)++;

    printf("%s. %s\n", data[0], data[1]);

    return 0;
}
int show_websites(int current_id, sqlite3 *db, char *zErrMsg, int *rc) 
{
    int rows_count = 0;
    char *sql = sqlite3_mprintf(
"SELECT id, name \
FROM websites \
LIMIT 10 OFFSET %d;", 
current_id
    );

    *rc = sqlite3_exec(db, sql, show_website_callback, &rows_count, &zErrMsg);
    if (*rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }

    if (rows_count == 0) 
    {
        printf("No websites found. \n");
    }
    else if (rows_count == 10) 
    {
        // Show that there may be more websites, since we have printed only 10 of them
        printf("  ...\n");
    }

    printf("\n");
    return rows_count;
}


int show_account_callback(void *rows_count, int column_count, char **data, char **columns) 
{
    int *p_count = (int *) rows_count;
    (*p_count)++;
    if (*p_count == 1) {printf("%s:\n", data[0]);}

    printf(" %s.\n", data[4]);
    printf("    Username: %s\n", data[1]);
    printf("    Email: %s\n", data[2]);
    printf("    Password: %s\n", data[3]);

    return 0;
}
int open_website_by_id(int website_id, sqlite3 *db, char *zErrMsg, int *rc) 
{
    int rows_count = 0;
    char *sql = sqlite3_mprintf(
"SELECT websites.name, accounts.username, accounts.email, accounts.password, accounts.id \
FROM accounts \
INNER JOIN websites ON accounts.website_id = websites.id \
WHERE websites.id = %d;", 
website_id
    );

    // Firstly, check if website with 'website_id' exists
    char *sql_check = sqlite3_mprintf(
"SELECT * \
FROM websites \
WHERE id == %d", 
website_id
    );
    rows_count = count_rows(sql_check, db, zErrMsg, rc);
    if (rows_count == 0) 
    {
        printf("No website with id %d found.\n\n", website_id);
        return 0;
    }

    rows_count = 0;
    *rc = sqlite3_exec(db, sql, show_account_callback, &rows_count, &zErrMsg);
    if (*rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }

    if (rows_count == 0) 
    {
        printf("No accounts found.\n");
    }

    printf("\n");
    return 1;
}


int get_website_id_callback(void *id, int column_count, char **data, char **columns) 
{
    int *website_id = (int *) id;
    try_convert_to_id(data[0], website_id);
    
    return 0;
}
int search_website_by_name(char *website_name, sqlite3 *db, char *zErrMsg, int *rc) 
{
    int websites_count = 0;
    char *sql = sqlite3_mprintf(
"SELECT id, name \
FROM websites \
WHERE name LIKE '%%%q%%' \
LIMIT 10;", 
website_name
    );

    websites_count = count_rows(sql, db, zErrMsg, rc);
    if (websites_count == 0) 
    {
        printf("No website found.\n\n", website_name);
        // Return that no websites were found
        return -1;
    } else if (websites_count == 1) 
    {
        /* Open website by its name */
        int accounts_count = 0;
        char *sql_msg = sqlite3_mprintf(
"SELECT websites.name, accounts.username, accounts.email, accounts.password, accounts.id \
FROM accounts \
INNER JOIN websites ON accounts.website_id = websites.id \
WHERE websites.name LIKE '%%%q%%';", 
website_name
        );
        *rc = sqlite3_exec(db, sql_msg, show_account_callback, &accounts_count, &zErrMsg);
        if (*rc != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
        
        if (accounts_count == 0) 
        {
            printf("No accounts found.\n");
        }

        printf("\n");  


        /* Get the website id */
        int website_id = 0;
        sql_msg = sqlite3_mprintf(
"SELECT id \
FROM websites \
WHERE name LIKE '%%%q%%';", 
website_name
        );
        *rc = sqlite3_exec(db, sql_msg, get_website_id_callback, &website_id, &zErrMsg);
        if (*rc != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }

        return website_id;
    }

    // Otherwise we have >= 2 websites by searched name:
    websites_count = 0;
    *rc = sqlite3_exec(db, sql, show_website_callback, &websites_count, &zErrMsg);
    if (*rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }

    if (websites_count == 10) 
    {
        // Show that there may be more websites, since we have printed only 10 of them
        printf("  ...\n");
    }

    printf("\n");
    // Return that there were >=2 websites found by 'website_name'
    return -2;
}


void add_website(char *website_name, sqlite3 *db, char *zErrMsg, int *rc) 
{
    char *sql = sqlite3_mprintf(
"INSERT INTO websites (name) VALUES ('%s');", 
website_name
    );

    *rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
    if (*rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
}

void add_account(int website_id, char *username, char *email, char *password, sqlite3 *db, char *zErrMsg, int *rc) 
{
    char *sql = sqlite3_mprintf(
"INSERT INTO accounts (website_id, username, email, password) VALUES (%d, '%s', '%s', '%s');", 
website_id, username, email, password
    );

    *rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
    if (*rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
}

void remove_account(int account_id, sqlite3 *db, char *zErrMsg, int *rc) 
{
    char *sql = sqlite3_mprintf(
"DELETE FROM accounts WHERE id = %d;", 
account_id
    );

    *rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
    if (*rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
}

void remove_website(int website_id, sqlite3 *db, char *zErrMsg, int *rc) 
{
    
}