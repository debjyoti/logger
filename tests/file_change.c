#include <stdio.h>
#include <stdlib.h>
#include "../src/logger.h"

int main()
{
    log_init("../out/file_change.log");
    log_print(INFO,"Test log message on file 1\n");
	log_file_change("../out/file_change2.log");
    log_print(INFO,"Test log message on file 2\n");
    log_exit();
    return 0;
}
