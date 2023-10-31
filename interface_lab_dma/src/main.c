#if defined(D_NEXYS_A7)
   #include <bsp_printf.h>
   #include <bsp_mem_map.h>
   #include <bsp_version.h>
#else
   PRE_COMPILED_MSG("no platform was defined")
#endif
#include <psp_api.h>

typedef struct {
    unsigned char R;
    unsigned char G;
    unsigned char B;
} RGB;

#define N 256
#define M 256

#define GPIO_LEDs       	0x80100000
#define GPIO1_INOUT     	0x80100004
#define GPIO_SWs			0x80100008
#define GPIO2_INOUT     	0x8010000C
#define GPIO_GIER       	0x8010011C
#define GPIO_IER        	0x80100128
#define GPIO_ISR        	0x80100120

#define CANNY_EDGE_CR       0x80150000
#define CANNY_EDGE_ROWS     0x80150014
#define CANNY_EDGE_COLS     0x8015001C
#define CANNY_EDGE_LOW      0x80150024
#define CANNY_EDGE_HIG      0x8015002C

#define DMA_MM2S_DMACR      0x80140000
#define DMA_MM2S_DMASR      0x80140004
#define DMA_MM2S_SA         0x80140018
#define DMA_MM2S_SA_MSB     0x8014001C
#define DMA_MM2S_LENGTH     0x80140028

#define DMA_S2MM_DMACR      0x80140030
#define DMA_S2MM_DMASR      0x80140034
#define DMA_S2MM_DA         0x80140048
#define DMA_S2MM_DA_MSB     0x8014004C
#define DMA_S2MM_LENGTH     0x80140058

#define DMA_MM2S_ADDR		0x01000000
#define DMA_S2MM_ADDR		0x02000000

extern unsigned char TheScream_256x256[];
volatile unsigned int status;

// void WRITE_REG_BYTE(unsigned int addr, char data);
// unsigned char READ_REG_BYTE(unsigned int addr);

// void WRITE_REG_WORD(unsigned int addr, unsigned int data);
// unsigned int READ_REG_WORD(unsigned int addr);

#define WRITE_BYTE(addr, data) *(unsigned char*)(addr) = (data)
#define READ_BYTE(addr) (*(unsigned char*)(addr))

RGB ColourImage[N][M];
RGB CannyImage[N][M];

void initColourImage(RGB image[N][M]) {
    int i,j;
    for (i=0;i<N;i++)
        for (j=0; j<M; j++) {
            image[i][j].R = TheScream_256x256[(i*M + j)*3];
            image[i][j].G = TheScream_256x256[(i*M + j)*3 + 1];
            image[i][j].B = TheScream_256x256[(i*M + j)*3 + 2];
        }
}

void delay(unsigned int num) 
{
	volatile unsigned int j;
	for (j = 0; j < num; j++) ;	// delay 
}

void DDRVideoWr(unsigned int addr)
{
    int i,j;
    for (i=0; i<N; i++) {
        for (j=0; j<M; j++) {
            // WRITE_REG_BYTE((addr+(i*M + j)*3), TheScream_256x256[(i*M + j)*3]);
            // WRITE_REG_BYTE((addr+(i*M + j)*3 + 1), TheScream_256x256[(i*M + j)*3 + 1]);
            // WRITE_REG_BYTE((addr+(i*M + j)*3 + 2), TheScream_256x256[(i*M + j)*3 + 2]);
            WRITE_BYTE((addr+(i*M + j)*3), TheScream_256x256[(i*M + j)*3]);
            WRITE_BYTE((addr+(i*M + j)*3 + 1), TheScream_256x256[(i*M + j)*3 + 1]);
            WRITE_BYTE((addr+(i*M + j)*3 + 2), TheScream_256x256[(i*M + j)*3 + 2]);            
        }
    }
}

void DDRVideoRd(unsigned int addr, RGB image[N][M])
{
    int i, j;
    for (i=0; i<N; i++) {
        for (j=0; j<M; j++) {
            // image[N][M].R = READ_REG_BYTE(addr+(i*M + j)*3);
            // image[N][M].G = READ_REG_BYTE(addr+(i*M + j)*3 + 1);
            // image[N][M].B = READ_REG_BYTE(addr+(i*M + j)*3 + 2);
            image[N][M].R = READ_BYTE(addr+(i*M + j)*3);
            image[N][M].G = READ_BYTE(addr+(i*M + j)*3 + 1);
            image[N][M].B = READ_BYTE(addr+(i*M + j)*3 + 2);            
        }
    }
}

// checked
int AxiDma_ResetIsDone() 
{
    // Check transmit channel
    // Reset is done when the reset bit is low
    if(M_PSP_READ_REGISTER_32(DMA_MM2S_DMACR) & 0x00000004)
        return 0;

    // Check receive channel
    // Reset is done when the reset bit is low
    if(M_PSP_READ_REGISTER_32(DMA_S2MM_DMACR) & 0x00000004)
        return 0;

    return 1;
}

// checked
void AxiDma_Init() {
    int TimeOut = 50000;
    int tmp = 0;

    // Reset the engine so the hardware starts from a known state
    M_PSP_WRITE_REGISTER_32(DMA_MM2S_DMACR, 0x00000004);
    //M_PSP_WRITE_REGISTER_32(DMA_S2MM_DMACR, 0x00000004);

    // At the initialization time, hardware should finish reset quickly
    while (TimeOut) {
        tmp = AxiDma_ResetIsDone();
        if(tmp)
            break;
        TimeOut -= 1;
    }

    if(!TimeOut)
        printfNexys("DMA failed reset in initialize!\n");
    else
        printfNexys("DMA initialized!\n");

}

int main(void) {
    unsigned int count=0xF;
    unsigned int length = N*M*3;

    // Initialize Uart
    uartInit();
    M_PSP_WRITE_REGISTER_32(GPIO_LEDs, 0x11111111);
    // Create an NxM matrix using the input image
    initColourImage(ColourImage);

    // Copy image to DDR
    DDRVideoWr(DMA_MM2S_ADDR);

    // Initialize Canny_edge
    M_PSP_WRITE_REGISTER_32(CANNY_EDGE_ROWS, N);
    M_PSP_WRITE_REGISTER_32(CANNY_EDGE_COLS, M);
    M_PSP_WRITE_REGISTER_32(CANNY_EDGE_LOW, 20);
    M_PSP_WRITE_REGISTER_32(CANNY_EDGE_HIG, 100);

    // Initialize DMA
    AxiDma_Init();
    
    // For debug
    //status = M_PSP_READ_REGISTER_32(DMA_MM2S_DMACR);
    //status = M_PSP_READ_REGISTER_32(DMA_MM2S_DMASR);
    //status = M_PSP_READ_REGISTER_32(DMA_MM2S_SA);
    //status = M_PSP_READ_REGISTER_32(DMA_MM2S_LENGTH);

    //status = M_PSP_READ_REGISTER_32(DMA_S2MM_DMACR);
    //status = M_PSP_READ_REGISTER_32(DMA_S2MM_DMASR);
    //status = M_PSP_READ_REGISTER_32(DMA_S2MM_SA);
    //status = M_PSP_READ_REGISTER_32(DMA_S2MM_LENGTH);

    //status = M_PSP_READ_REGISTER_32(CANNY_EDGE_ROWS);
    //status = M_PSP_READ_REGISTER_32(CANNY_EDGE_COLS);
    //status = M_PSP_READ_REGISTER_32(CANNY_EDGE_LOW);
    //status = M_PSP_READ_REGISTER_32(CANNY_EDGE_HIG);
    //status = M_PSP_READ_REGISTER_32(CANNY_EDGE_CR);



    // For debug
    //status = M_PSP_READ_REGISTER_32(DMA_MM2S_DMACR);
    //status = M_PSP_READ_REGISTER_32(DMA_MM2S_DMASR);
    //status = M_PSP_READ_REGISTER_32(DMA_MM2S_SA);
    //status = M_PSP_READ_REGISTER_32(DMA_MM2S_LENGTH);

    //status = M_PSP_READ_REGISTER_32(DMA_S2MM_DMACR);
    //status = M_PSP_READ_REGISTER_32(DMA_S2MM_DMASR);
    //status = M_PSP_READ_REGISTER_32(DMA_S2MM_SA);
    //status = M_PSP_READ_REGISTER_32(DMA_S2MM_LENGTH);

    // Begin DMA receive
    M_PSP_WRITE_REGISTER_32(DMA_S2MM_DMACR, 0x00000001);
    M_PSP_WRITE_REGISTER_32(DMA_S2MM_DA, DMA_S2MM_ADDR);
    // M_PSP_WRITE_REGISTER_32(DMA_S2MM_DMACR, (M_PSP_READ_REGISTER_32(DMA_S2MM_DMACR) | 0x00000001));
    M_PSP_WRITE_REGISTER_32(DMA_S2MM_LENGTH, length);

    // Begin DMA transfer
    M_PSP_WRITE_REGISTER_32(DMA_MM2S_DMACR, 0x00000001);
    M_PSP_WRITE_REGISTER_32(DMA_MM2S_SA, DMA_MM2S_ADDR);
    // M_PSP_WRITE_REGISTER_32(DMA_MM2S_DMACR, (M_PSP_READ_REGISTER_32(DMA_MM2S_DMACR) | 0x00000001));
    M_PSP_WRITE_REGISTER_32(DMA_MM2S_LENGTH, length);

    // Begin image process
    M_PSP_WRITE_REGISTER_32(CANNY_EDGE_CR, 0x81);

    // Wait DMA finish
    while (1) {
        M_PSP_WRITE_REGISTER_32(GPIO_LEDs, count >> 0xf);
		count = count + 1;		
		// delay(1000000);
        status = M_PSP_READ_REGISTER_32(DMA_S2MM_DMASR);
        status = status & 0x00000002;
        if(status)
            break;
    }
    

    while (1) {
        M_PSP_WRITE_REGISTER_32(GPIO_LEDs, count);
		count = count + 1;		
		// delay(1000000);
        status = M_PSP_READ_REGISTER_32(DMA_MM2S_DMASR);
        status = status & 0x00000002;
        if(status)
            break;
    }

    // Copy image from DDR
    DDRVideoRd(DMA_S2MM_ADDR, CannyImage);

    delay(10000000);
    while(1) {
        M_PSP_WRITE_REGISTER_32(GPIO_LEDs, count);
		count = count + 1;		
		delay(10000);
    }

    return 0;
}


