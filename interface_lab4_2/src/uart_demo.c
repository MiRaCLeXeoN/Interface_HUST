#define UART_BASE 0x80111000
#define rbr     0*4
#define ier     1*4
#define fcr     2*4
#define lcr     3*4
#define mcr     4*4
#define lsr     5*4
#define msr     6*4
#define scr     7*4

#define thr     rbr
#define iir     fcr
#define dll     rbr
#define dlm     ier

#define WRITE_UART(dir, value) { (*(volatile unsigned *)(dir)) = (value); }
int main() {
    volatile unsigned int i, j;
    char res[20] = "hello world\n";

    WRITE_UART(UART_BASE+lcr, 0x00000080);
    for(j = 0; j < 1000000; j++) ;
    WRITE_UART(UART_BASE+dll, 27);
    for(j = 0; j < 1000000; j++) ;
    WRITE_UART(UART_BASE+fcr, 0x0);
    for(j = 0; j < 1000000; j++) ;
    WRITE_UART(UART_BASE+dlm, 0x00000000);
    for(j = 0; j < 1000000; j++) ;
    WRITE_UART(UART_BASE+lcr, 0x00000003);
    for(j = 0; j < 1000000; j++) ;
    WRITE_UART(UART_BASE+ier, 0x00000000);
    for(j = 0; j < 10000000; j++) ;

    while (1)
    {
        for(i = 0; res[i] != '\0'; i++) {
            WRITE_UART(UART_BASE + thr, res[i]);
            for(j = 0; j < 100000; j++) 
                ;
        }
    }

    return 0;    
}