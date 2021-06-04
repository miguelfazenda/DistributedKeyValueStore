#gcc main.c KVS-lib.c ../shared/hashtable.c ../shared/message.c -lpthread -Wall -Wextra -pedantic -g
(cd ../../KVS-lib/ ; sh build.sh)
gcc main.c ../../shared/error_codes.c -Wall -Wextra -ldl -g