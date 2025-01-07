#define NOP 0xd503201f

#define LDPPOST_PREL 0xa8c17bfd
#define STPPRE_PREL 0xa9bf7bfd
#define LDP_PREL 0xa9407BFD
#define STP_PREL 0xa9007BFD
#define LDRW 0xb9400000
#define LDRX 0xf9400000
#define STRW 0xb9000000
#define STRX 0xf9000000
#define LDUR 0xf9400000 // bit 10, 11 must be 0
#define LDURSW 0xb8800000
#define STUR 0xf8000000 // bit 10, 11 must be 0

#define ADRP 0x90000000
#define ADD_IMM 0x91000000
#define MOV 0x52800000
#define MOV_X 0xD2800000
#define SUB 0xd1000000
#define CMP 0xf100001f

#define B 0x14000000
#define BL 0x94000000
#define RET 0xd65f03c0
