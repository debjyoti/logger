#include <stdio.h>
#include <stdlib.h>
#include "../logger.h"

int main()
{
    int msg_count = 0;
    log_init("logger1.log");
    log_print(INFO,"----Start log for %s\n",__FILE__ );
    log_print(0,"Test log message %d level 0\n",++msg_count );
    log_print(1,"Test log message %d level 1\n",++msg_count );
    log_print(2,"Test log message %d level 2\n",++msg_count );
    log_print(3,"Test log message %d level 3\n",++msg_count );
	log_file_change("logger2.log");
    log_print(0,"Test log message %d level 0\n",++msg_count );
    log_print(4,"Test log message %d level 4\n",++msg_count );
    log_print_debug("Test log message %d level 4\n",++msg_count );
    log_exit();
    return 0;
}
