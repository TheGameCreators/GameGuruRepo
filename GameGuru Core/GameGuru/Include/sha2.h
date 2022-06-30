#ifndef SHA256_H
#define SHA256_H
 
class SHA256
{
protected:
    typedef unsigned char uint8;
    typedef unsigned int uint32;
    typedef unsigned long long uint64;
 
    const static uint32 sha256_k[];
    static const unsigned int SHA224_256_BLOCK_SIZE = (512/8);
public:
    void init();
    void update(const unsigned char *message, unsigned int len);
    void final(unsigned char *digest);
    static const unsigned int DIGEST_SIZE = ( 256 / 8);
 
protected:
    void transform(const unsigned char *message, unsigned int block_nb);
    unsigned int m_tot_len;
    unsigned int m_len;
    unsigned char m_block[2*SHA224_256_BLOCK_SIZE];
    uint32 m_h[8];
};
 
char* sha256(const char* input);

class SHA512
{
protected:
    typedef unsigned char uint8;
    typedef unsigned int uint32;
    typedef unsigned long long uint64;
 
    const static uint64 sha512_k[];
    static const unsigned int SHA384_512_BLOCK_SIZE = (1024/8);
 
public:
    void init();
    void update(const unsigned char *message, unsigned int len);
    void final(unsigned char *digest);
    static const unsigned int DIGEST_SIZE = ( 512 / 8);
 
protected:
    void transform(const unsigned char *message, unsigned int block_nb);
    unsigned int m_tot_len;
    unsigned int m_len;
    unsigned char m_block[2 * SHA384_512_BLOCK_SIZE];
    uint64 m_h[8];
};
 
 
char* sha512(const char* input);
 
#define SHA2_SHFR(x, n)    (x >> n)
#define SHA2_ROTR(x, n)   ((x >> n) | (x << ((sizeof(x) << 3) - n)))
#define SHA2_ROTL(x, n)   ((x << n) | (x >> ((sizeof(x) << 3) - n)))
#define SHA2_CH(x, y, z)  ((x & y) ^ (~x & z))
#define SHA2_MAJ(x, y, z) ((x & y) ^ (x & z) ^ (y & z))
#define SHA256_F1(x) (SHA2_ROTR(x,  2) ^ SHA2_ROTR(x, 13) ^ SHA2_ROTR(x, 22))
#define SHA256_F2(x) (SHA2_ROTR(x,  6) ^ SHA2_ROTR(x, 11) ^ SHA2_ROTR(x, 25))
#define SHA256_F3(x) (SHA2_ROTR(x,  7) ^ SHA2_ROTR(x, 18) ^ SHA2_SHFR(x,  3))
#define SHA256_F4(x) (SHA2_ROTR(x, 17) ^ SHA2_ROTR(x, 19) ^ SHA2_SHFR(x, 10))
#define SHA512_F1(x) (SHA2_ROTR(x, 28) ^ SHA2_ROTR(x, 34) ^ SHA2_ROTR(x, 39))
#define SHA512_F2(x) (SHA2_ROTR(x, 14) ^ SHA2_ROTR(x, 18) ^ SHA2_ROTR(x, 41))
#define SHA512_F3(x) (SHA2_ROTR(x,  1) ^ SHA2_ROTR(x,  8) ^ SHA2_SHFR(x,  7))
#define SHA512_F4(x) (SHA2_ROTR(x, 19) ^ SHA2_ROTR(x, 61) ^ SHA2_SHFR(x,  6))

#define SHA2_UNPACK32(x, str)                 \
{                                             \
    *((str) + 3) = (uint8) ((x)      );       \
    *((str) + 2) = (uint8) ((x) >>  8);       \
    *((str) + 1) = (uint8) ((x) >> 16);       \
    *((str) + 0) = (uint8) ((x) >> 24);       \
}
#define SHA2_PACK32(str, x)                   \
{                                             \
    *(x) =   ((uint32) *((str) + 3)      )    \
           | ((uint32) *((str) + 2) <<  8)    \
           | ((uint32) *((str) + 1) << 16)    \
           | ((uint32) *((str) + 0) << 24);   \
}

#define SHA2_UNPACK64(x, str)                 \
{                                             \
    *((str) + 7) = (uint8) ((x)      );       \
    *((str) + 6) = (uint8) ((x) >>  8);       \
    *((str) + 5) = (uint8) ((x) >> 16);       \
    *((str) + 4) = (uint8) ((x) >> 24);       \
    *((str) + 3) = (uint8) ((x) >> 32);       \
    *((str) + 2) = (uint8) ((x) >> 40);       \
    *((str) + 1) = (uint8) ((x) >> 48);       \
    *((str) + 0) = (uint8) ((x) >> 56);       \
}
#define SHA2_PACK64(str, x)                   \
{                                             \
    *(x) =   ((uint64) *((str) + 7)      )    \
           | ((uint64) *((str) + 6) <<  8)    \
           | ((uint64) *((str) + 5) << 16)    \
           | ((uint64) *((str) + 4) << 24)    \
           | ((uint64) *((str) + 3) << 32)    \
           | ((uint64) *((str) + 2) << 40)    \
           | ((uint64) *((str) + 1) << 48)    \
           | ((uint64) *((str) + 0) << 56);   \
}

#endif
