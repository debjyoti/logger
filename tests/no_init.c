#include <stdio.h>
#include <stdlib.h>
#include "../src/logger.h"

int main()
{
    int msg_count = 0;
    log_print(INFO, "Test INFO message %d\n", ++msg_count);
    log_print(WARNING, "Test WARN message %d\n", ++msg_count);
    log_print(ERROR, "Test ERROR message %d\n", ++msg_count);
    log_print_debug("Test DEBUG message %d\n", ++msg_count);
    log_exit();
    printf("Done\n");
    return 0;
}
