#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "server/server_db.h"

#define TEST_DB_PATH "tests/test_users.db"

int total = 0;
int passed = 0;

// Helper function to handle assertions
void check(int condition, const char *description)
{
    total++;
    if (condition) {
        passed++;
        printf("[PASS] %s\n", description);
    } else {
        printf("[FAIL] %s\n", description);
    }
}

// 1. Test registering a brand new user
void test_register_new_user(void)
{
    remove(TEST_DB_PATH);
    DbResult result = db_register("alice", "pwd1", TEST_DB_PATH);
    check(result == DB_OK, "register new user returns DB_OK");
}

// 2. Test logging in with correct credentials
void test_login_correct_credentials(void)
{
    remove(TEST_DB_PATH);
    DbResult result = db_register("alice", "pwd1", TEST_DB_PATH);
    check(result == DB_OK, "register new user returns DB_OK");
    
    result = db_login("alice", "pwd1", TEST_DB_PATH);
    check(result == DB_OK, "login user returns DB_OK");
}

// 3. Test logging in with an incorrect password
void test_login_wrong_password(void)
{
    remove(TEST_DB_PATH);
    db_register("alice", "pwd1", TEST_DB_PATH); // Fix: Must register before testing wrong password
    
    DbResult result = db_login("alice", "pwd2", TEST_DB_PATH);
    check(result == DB_WRONG_PASSWORD, "login user returns DB_WRONG_PASSWORD");
}

// 4. Test logging in a user that doesn't exist
void test_login_unknown_user(void)
{
    remove(TEST_DB_PATH); // Fix: Duplicate function name changed to test_login_unknown_user
    DbResult result = db_login("john", "pwd2", TEST_DB_PATH);
    check(result == DB_NOT_FOUND, "login user returns DB_NOT_FOUND");
}

// 5. Test setting a registered user online and looking up their socket descriptor
void test_set_online_and_lookup(void)
{
    remove(TEST_DB_PATH);
    db_register("alice", "pwd1", TEST_DB_PATH); // Fix: Must register user first
    
    db_set_user_status("alice", TEST_DB_PATH, "ONLINE", 42);
    int outSockfd = -1;
    DbResult result = db_lookup_sockfd("alice", TEST_DB_PATH, &outSockfd);
    check(result == DB_ONLINE && outSockfd == 42, "db_lookup returns DB_ONLINE and correct socket");
}

// 6. Test setting a user offline and verifying lookup behavior
void test_set_offline_and_lookup(void)
{
    remove(TEST_DB_PATH);
    db_register("alice", "pwd1", TEST_DB_PATH);
    
    db_set_user_status("alice", TEST_DB_PATH, "OFFLINE", -1);
    int outSockfd = 0;
    DbResult result = db_lookup_sockfd("alice", TEST_DB_PATH, &outSockfd);
    check(result == DB_OFFLINE, "offline lookup returns DB_OFFLINE");
}

// 7. Test looking up a user that was never registered
void test_lookup_unknown_user(void)
{
    remove(TEST_DB_PATH);
    int outSockfd = -1;
    DbResult result = db_lookup_sockfd("ghost", TEST_DB_PATH, &outSockfd);
    check(result == DB_NOT_FOUND, "lookup unknown user returns DB_NOT_FOUND");
}

// 8. Test that registering the exact same username fails as a duplicate
void test_register_duplicate_user(void)
{
    remove(TEST_DB_PATH);
    db_register("alice", "pwd1", TEST_DB_PATH);
    
    DbResult result = db_register("alice", "pwd1", TEST_DB_PATH);
    check(result == DB_DUPLICATE, "duplicate register returns DB_DUPLICATE");
}

// 9. Test that db_register outputs clean, readable data to the flat file
void test_register_writes_wellformed_line(void)
{
    remove(TEST_DB_PATH);
    db_register("alice", "pwd1", TEST_DB_PATH);
    
    FILE *fp = fopen(TEST_DB_PATH, "r");
    if (fp == NULL) {
        check(0, "file contains alice (failed to open file)");
        return;
    }
    
    char line[256];
    if (fgets(line, sizeof(line), fp) != NULL) {
        check(strstr(line, "alice") != NULL, "file contains alice");
    } else {
        check(0, "file contains alice (file was empty)");
    }
    fclose(fp);
}

int main(void)
{
    printf("--- Running Database Test Suite ---\n");
    
    // Call all 9 test functions sequentially
    test_register_new_user();
    test_login_correct_credentials();
    test_login_wrong_password();
    test_login_unknown_user();
    test_set_online_and_lookup();
    test_set_offline_and_lookup();
    test_lookup_unknown_user();
    test_register_duplicate_user();
    test_register_writes_wellformed_line();

    // Summary output
    printf("\n%d / %d tests passed\n", passed, total);
    
    if (passed == total) {
        return 0; // Success
    } else {
        return 1; // Test suite failed
    }
}
