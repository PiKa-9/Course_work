#include<stdio.h>
#include<string.h>
#include<stdbool.h>
#include<stdlib.h>
#include "sqlite3.h"
#include "database.h"

//===============================================
// One useful function
bool convert_to_id(const char *string, int *result) 
{
    *result = 0;
    if (*string == '-') {return false;}
    if (*string == '\0') {return false;}      //empty string(without digits)

    while (*string) 
    {
        if ((*string < '0') || (*string > '9')) 
        {
            return false;
        } else 
        {
            *result = *result * 10 + (*string - 48);
            string++;
        }
    }     
    return true;
}
//=============================================

int show_website_callback(void *website_count, int column_count, char **data, char **columns) 
{
    int *count = (int *) website_count;
    (*count)++;

    printf("%s. %s\n", data[0], data[1]);

    return 0;
}
void show_list_of_websites(char *searched_website_name, sqlite3 *db, char *zErrMsg, int *rc) 
{
    char *sql = sqlite3_mprintf(
"SELECT current_id, name \
FROM websites \
WHERE name LIKE '%q%%' \
LIMIT 10;", 
searched_website_name
    );

    int website_count = 0;
    *rc = sqlite3_exec(db, sql, show_website_callback, &website_count, &zErrMsg);
    if (*rc != SQLITE_OK) {
        fprintf(stderr, "\033[1;31mDatabase error:\033[0m %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }

    if (website_count == 0) 
    {
        printf("No websites found. \n");
    }
    else if (website_count == 10) 
    {
        // Show that there may be more websites, since we have printed only 10 of them
        printf("  ...\n");
    }

    printf("\n");
}


int get_website_id_callback(void *id, int column_count, char **data, char **columns) 
{
    int *website_id = (int *) id;
    convert_to_id(data[0], website_id);
    return 0;
}
int get_website_id(char *website_name, sqlite3 *db, char *zErrMsg, int *rc) 
{
    char *sql = sqlite3_mprintf(
"SELECT current_id \
FROM websites \
WHERE name LIKE '%q%%';", 
website_name
    );

    int website_id = -1;
    *rc = sqlite3_exec(db, sql, get_website_id_callback, &website_id, &zErrMsg);
    if (rc != SQLITE_OK) {free(zErrMsg);}

    return website_id;
}


int print_current_website_name_callback(void *NotUsed, int column_count, char **data, char **columns) 
{
    printf("%s", data[0]);
    return 0;
}
void print_current_website_name(const int website_id, sqlite3 *db, char *zErrMsg, int *rc) 
{
    char *sql = sqlite3_mprintf(
"SELECT name \
FROM websites \
WHERE current_id = %d;", 
website_id
    );

    *rc = sqlite3_exec(db, sql, print_current_website_name_callback, 0, &zErrMsg);
}


int check_if_website_exists_callback(void *website_exists, int column_count, char **data, char **columns) 
{
    int *does_exist = (int *) website_exists;
    *does_exist = 1;
    return 0;
}
int check_if_website_exists(const int website_id, sqlite3 *db, char *zErrMsg, int *rc) 
{
    char *sql = sqlite3_mprintf(
"SELECT id \
FROM websites \
WHERE current_id = %d;", 
website_id
    );

    int website_exists = 0;
    *rc = sqlite3_exec(db, sql, check_if_website_exists_callback, &website_exists, &zErrMsg);
    return website_exists;
}


int get_website_count_callback(void *website_count, int column_count, char **data, char **columns) 
{
    int *count = (int *) website_count;
    (*count)++; 
    return 0;
}
int get_website_count(char *website_name, sqlite3 *db, char *zErrMsg, int *rc) 
{
    char *sql = sqlite3_mprintf(
"SELECT id \
FROM websites \
WHERE name LIKE '%q%%' \
LIMIT 10;", 
website_name
    );

    int websites_count = 0;
    *rc = sqlite3_exec(db, sql, get_website_count_callback, &websites_count, &zErrMsg);
    if (*rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    
    return websites_count;
}


int show_account_callback(void *account_count, int column_count, char **data, char **columns) 
{
    int *count = (int *) account_count;
    (*count)++;

    printf("    %s.\n", data[0]);
    printf("       Username: %s\n", data[1]);
    printf("       Email: %s\n", data[2]);
    printf("       Password: %s\n", data[3]);

    return 0;
}
void show_list_of_accounts(int website_id, sqlite3 *db, char *zErrMsg, int *rc) 
{
    char *sql = sqlite3_mprintf(
"SELECT accounts.account_id, accounts.username, accounts.email, accounts.password \
FROM accounts \
INNER JOIN websites ON accounts.website_id = websites.id \
WHERE websites.current_id = %d \
ORDER BY accounts.account_id;", 
website_id
    );

    int account_count = 0;
    *rc = sqlite3_exec(db, sql, show_account_callback, &account_count, &zErrMsg);
    if (*rc != SQLITE_OK) {
        fprintf(stderr, "   \033[1;31mDatabase error:\033[0m %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }

    if (account_count == 0) 
    {
        printf("   No accounts found.\n");
    }

    printf("\n");
}


int add_website(char *website_name, sqlite3 *db, char *zErrMsg, int *rc) 
{
    char *sql = sqlite3_mprintf(
"INSERT INTO websites (current_id, name) \
VALUES (\
COALESCE((SELECT MAX(current_id) + 1 FROM websites), 1), \
'%q');", 
website_name
    );

    *rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
    if (*rc != SQLITE_OK) {
        sqlite3_free(zErrMsg);
        return 0;
    }
    return 1;
}

void add_account(int website_id, char *username, char *email, char *password, sqlite3 *db, char *zErrMsg, int *rc) 
{
    char *sql = sqlite3_mprintf(
"INSERT INTO accounts (website_id, account_id, username, email, password) \
VALUES (\
(SELECT id FROM websites WHERE current_id = %d), \
COALESCE((SELECT MAX(account_id) + 1 FROM accounts WHERE website_id = %d), 1), \
'%s', \
'%s', \
'%s');", 
website_id, website_id, username, email, password
    );

    *rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
    if (*rc != SQLITE_OK) {
        sqlite3_free(zErrMsg);
        printf("    Can't add this account. Maybe it's already on your list.\n");
    } else 
    {
       printf("    Account has been successfully added to your list!\n"); 
    }

    printf("\n");
}

int check_if_account_exists_callback(void *account_exists, int column_count, char **data, char **columns) 
{
    int *does_exist = (int *) account_exists;
    *does_exist = 1;
    return 0;
}
int check_if_account_exists(const int account_id, const int website_id, sqlite3 *db, char *zErrMsg, int *rc) 
{
    char *sql = sqlite3_mprintf(
"SELECT id \
FROM accounts \
WHERE (website_id = (SELECT id FROM websites WHERE current_id = %d)) AND (account_id = %d);", 
website_id, account_id
    );

    int account_exists = 0;
    *rc = sqlite3_exec(db, sql, check_if_account_exists_callback, &account_exists, &zErrMsg);
    return account_exists;
}
void remove_account(const int account_id, const int website_id, sqlite3 *db, char *zErrMsg, int *rc) 
{
    if (check_if_account_exists(account_id, website_id, db, zErrMsg, rc) == 0) 
    {
        printf("    There is no account with id %d on current website. Can't remove it.\n", account_id);
        return;
    }

    char *sql = sqlite3_mprintf(
"DELETE FROM accounts \
WHERE (website_id = (SELECT id FROM websites WHERE current_id = %d)) AND (account_id = %d);", 
website_id, account_id
    );

    *rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
    if (*rc != SQLITE_OK) {
        sqlite3_free(zErrMsg);
    } else 
    {
        printf("    Account with id %d has been successfully removed!\n", account_id);
    }

    printf("\n");
}

void remove_website(int website_id, sqlite3 *db, char *zErrMsg, int *rc) 
{
    printf("    Website '");
    print_current_website_name(website_id, db, zErrMsg, rc);
    printf("' has been successfully removed!\n", website_id);
    printf("\n");  

    char *sql = sqlite3_mprintf(
"DELETE FROM accounts \
WHERE website_id = (SELECT id FROM websites WHERE current_id = %d);", 
website_id
    );
    *rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);

    sql = sqlite3_mprintf(
"DELETE FROM websites WHERE current_id = %d;", 
website_id
    );
    *rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg); 
}


void update_website_ids(sqlite3 *db, char *zErrMsg, int *rc) 
{
    char *sql = sqlite3_mprintf(
"WITH subquery AS ( \
    SELECT id, ROW_NUMBER() OVER (ORDER BY id) AS new_current_id \
    FROM websites \
) \
UPDATE websites \
SET current_id = subquery.new_current_id \
FROM subquery \
WHERE websites.id = subquery.id;"
    );

    *rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
    if (*rc != SQLITE_OK) {
        sqlite3_free(zErrMsg);
        printf("\033[0;31mSomething went wrong...\033[0m\n");
    }
}
void update_account_ids(sqlite3 *db, char *zErrMsg, int *rc) 
{
    char *sql = sqlite3_mprintf(
"WITH subquery AS ( \
    SELECT id, ROW_NUMBER() OVER (PARTITION BY website_id ORDER BY account_id) AS rank \
    FROM accounts \
) \
UPDATE accounts \
SET account_id = subquery.rank \
FROM subquery \
WHERE accounts.id = subquery.id;"
    );

    *rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
    if (*rc != SQLITE_OK) {
        sqlite3_free(zErrMsg);
        printf("\033[0;31mSomething went wrong...\033[0m\n");
    }
}
