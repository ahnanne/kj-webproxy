#include <stdio.h>
#include <arpa/inet.h>

// struct in_addr {
//     /* address in network byte order (big-endian) */
//     uint32_t s_addr;
// };
// ! previous definition is in in.h:301:8

int main(void) {
    char dotted_decimal[50];
    struct in_addr a;

    /* convert IPv4 and IPv6 addresses from text to binary form */
    inet_pton(AF_INET, "128.2.194.242", (void *)&a.s_addr); // returns 1 on success
    printf("%08x\n", a.s_addr); // f2c20280

    /* convert IPv4 and IPv6 addresses from binary to text form */
    inet_ntop(AF_INET, (void *)&a.s_addr, dotted_decimal, 50);
    printf("%s\n", dotted_decimal); // 128.2.194.242
    printf("%lu\n", sizeof(uint16_t));

    return 0;
}
