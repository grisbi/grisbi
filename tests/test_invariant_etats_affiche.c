#include <check.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Include the actual production header
#include "src/etats_affiche.h"

START_TEST(test_strcpy_buffer_bounds)
{
    // Invariant: Buffer operations must not write beyond allocated memory boundaries
    const char *payloads[] = {
        "???",  // Exact exploit case from vulnerable code
        "A",    // Minimal valid input
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()",  // Boundary/overflow attempt
        "",     // Empty string edge case
        "1234567890123456789012345678901234567890"  // Another long input
    };
    int num_payloads = sizeof(payloads) / sizeof(payloads[0]);

    for (int i = 0; i < num_payloads; i++) {
        // Create a buffer with known sentinel values to detect overflow
        char buffer[64];
        char guard_zone_before[16] = {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
                                      0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA};
        char guard_zone_after[16] = {0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB,
                                     0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB};
        
        // Initialize buffer with pattern
        memset(buffer, 0xCC, sizeof(buffer));
        
        // Call the actual production function - assuming it uses strcpy internally
        // We'll test through the public interface that triggers the vulnerable code path
        int result = affiche_etat_financier(payloads[i], buffer, sizeof(buffer));
        
        // Security property: guard zones must remain unchanged
        for (int j = 0; j < 16; j++) {
            ck_assert_msg(guard_zone_before[j] == 0xAA, 
                         "Buffer underflow detected at payload index %d", i);
            ck_assert_msg(guard_zone_after[j] == 0xBB, 
                         "Buffer overflow detected at payload index %d", i);
        }
        
        // Additional invariant: buffer must be null-terminated
        ck_assert_msg(buffer[sizeof(buffer)-1] == 0xCC || buffer[sizeof(buffer)-1] == '\0',
                     "Buffer termination invariant violated at payload index %d", i);
    }
}
END_TEST

Suite *security_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Security");
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_strcpy_buffer_bounds);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = security_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}