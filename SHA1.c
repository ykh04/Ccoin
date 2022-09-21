//2022 ykh04
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define WORDS_PER_BLOCK             16
#define NUM_OF_DATAWORDS_PER_BLOCK  13
#define BYTES_PER_WORD              4
#define BITS_PER_BYTE               8
#define DUMMY_ERROR                 -1

int Nsha1(unsigned char *data, unsigned int size, unsigned int *SH0, unsigned int *SH1, unsigned int *SH2, unsigned int *SH3, unsigned int *SH4);
void padding(unsigned int size, unsigned char *data, unsigned char *blocks, unsigned int BLOCKBYTES); /*パディング関数（戻り：なし　引数１：メッセージサイズ　引数２：メッセージデータ　引数３：出力用領域アドレス　引数４：全ブロックに含まれる総バイト数）*/
unsigned int sigma(int index, unsigned int word1, unsigned int word2, unsigned int word3);
unsigned int rotL(int times, unsigned int word);

static unsigned int K[80] = { /*定数ワードシーケンス*/ /*issue: こいつがforでどのような扱われ方をしているか調べる*/
    0x5A827999, 0x5A827999, 0x5A827999, 0x5A827999, 0x5A827999, 0x5A827999, 0x5A827999, 0x5A827999, 0x5A827999, 0x5A827999,
    0x5A827999, 0x5A827999, 0x5A827999, 0x5A827999, 0x5A827999, 0x5A827999, 0x5A827999, 0x5A827999, 0x5A827999, 0x5A827999,
    0x6ED9EBA1, 0x6ED9EBA1, 0x6ED9EBA1, 0x6ED9EBA1, 0x6ED9EBA1, 0x6ED9EBA1, 0x6ED9EBA1, 0x6ED9EBA1, 0x6ED9EBA1, 0x6ED9EBA1,
    0x6ED9EBA1, 0x6ED9EBA1, 0x6ED9EBA1, 0x6ED9EBA1, 0x6ED9EBA1, 0x6ED9EBA1, 0x6ED9EBA1, 0x6ED9EBA1, 0x6ED9EBA1, 0x6ED9EBA1,
    0x8F1BBCDC, 0x8F1BBCDC, 0x8F1BBCDC, 0x8F1BBCDC, 0x8F1BBCDC, 0x8F1BBCDC, 0x8F1BBCDC, 0x8F1BBCDC, 0x8F1BBCDC, 0x8F1BBCDC,
    0x8F1BBCDC, 0x8F1BBCDC, 0x8F1BBCDC, 0x8F1BBCDC, 0x8F1BBCDC, 0x8F1BBCDC, 0x8F1BBCDC, 0x8F1BBCDC, 0x8F1BBCDC, 0x8F1BBCDC,
    0xCA62C1D6, 0xCA62C1D6, 0xCA62C1D6, 0xCA62C1D6, 0xCA62C1D6, 0xCA62C1D6, 0xCA62C1D6, 0xCA62C1D6, 0xCA62C1D6, 0xCA62C1D6,
    0xCA62C1D6, 0xCA62C1D6, 0xCA62C1D6, 0xCA62C1D6, 0xCA62C1D6, 0xCA62C1D6, 0xCA62C1D6, 0xCA62C1D6, 0xCA62C1D6, 0xCA62C1D6
};

int Nsha1(unsigned char *data, unsigned int size, unsigned int *SH0, unsigned int *SH1, unsigned int *SH2, unsigned int *SH3, unsigned int *SH4) /*基本的にバイトデータを扱う*/
{
    unsigned int numOfBlocks, BLOCKBYTES, *wordBuf, j, k; /*メッセージに応じて変化するブロックの数*/
    unsigned char *blocks = NULL, wordBufchar[80 * BYTES_PER_WORD]; /*メッセージに応じて動的に確保されるパディング用空間（ゼロ初期化されるであろう）*/

    unsigned int A, B, C, D, E, TEMP;
    unsigned int H0 = 0x67452301, H1 = 0xEFCDAB89, H2 = 0x98BADCFE, H3 = 0x10325476, H4 = 0xC3D2E1F0;


    if ((size % 64) < 56) {
        numOfBlocks = size / (WORDS_PER_BLOCK * BYTES_PER_WORD) + 1; /*3ワードの余裕がある（サイズ等用）*/
    }
    else {
        numOfBlocks = size / (WORDS_PER_BLOCK * BYTES_PER_WORD) + 2; /*3ワードの余裕がないからもう1ブロック増やす*/
    }
    /*============================
        ブロックの確保
    ============================*/
    BLOCKBYTES = numOfBlocks * WORDS_PER_BLOCK * BYTES_PER_WORD;
    blocks = calloc(BLOCKBYTES, sizeof(char)); /*動的確保（ゼロ初期化）*/
    if (blocks == NULL) {   /*メモリー容量が不足していて領域が確保できなかった（最近のマシンではないと思うが、一応）*/
        printf("\n\n");
        printf("ERROR: Lack of Memory Resource!!");
        return DUMMY_ERROR;
    }


    /*============================
        パディング
    ============================*/
    padding(size, data, blocks, BLOCKBYTES);    /*パディング関数を呼び出す*/
    /*============================
        計算
    ============================*/
    for (j = 0; j < numOfBlocks; j++) {
        memset(wordBufchar, 0, sizeof(wordBufchar));
        memcpy(wordBufchar, blocks + j * (WORDS_PER_BLOCK * BYTES_PER_WORD), WORDS_PER_BLOCK * BYTES_PER_WORD);
        wordBuf = (unsigned int *)wordBufchar;
        for (k = 16; k < 80; k++) {
            wordBuf[k] = rotL(1, wordBuf[k - 3] ^ wordBuf[k - 8] ^ wordBuf[k - 14] ^ wordBuf[k - 16]);
        }
        A = H0;
        B = H1;
        C = H2;
        D = H3;
        E = H4;
        for (k = 0; k < 80; k++) {
            TEMP = rotL(5, A) + sigma(k, B, C, D) + E + wordBuf[k] + K[k];
            E = D;
            D = C;
            C = rotL(30, B);
            B = A;
            A = TEMP;
        }
        H0 += A;
        H1 += B;
        H2 += C;
        H3 += D;
        H4 += E;
    }
    SH0 = &H0;
    SH1 = &H1;
    SH2 = &H2;
    SH3 = &H3;
    SH4 = &H4;
    free(blocks);

    return 0;
}

void padding(unsigned int size, unsigned char *data, unsigned char *blocks, unsigned int BLOCKBYTES)
{
    int i;
    unsigned int bSize;
    unsigned char *bitSize = NULL; /*ビットサイズに直すための配列*/
    bSize = size * 8;

    memcpy(blocks, data, size); /*パディング領域へコピー*/
    memset(blocks + size, 0x80, 1); /*後ろに識別用コードを挿入（0から始まってるからこれでよいのだ）*/

    bitSize = (unsigned char *)&bSize; /*32bit領域を8bit領域で扱うため*/
    for (i = 0; i < 4; i++) { /*リトルエンディアンをビッグエンディアンに変換する*/
        *(blocks + BLOCKBYTES - (i + 1)) = *(bitSize + i);
    }
}

/*---前提---

[|]はbitのOR演算を表します
[&}はbitのAND演算を表します
[~]はbitのNOT演算(1の補数をとるとも)を表します左
[^]はbitのXOR(排他的論理和)演算を表します
[<<][>>]はそれぞれ左論理シフト、右論理シフトを表します（符号なしが多い（というか全部）ので基本論理シフトとみてよい）

備考：
     Cの32bit足し算は２項の2^32の余剰の和
============================================================================*/

unsigned int sigma(int index, unsigned int word1, unsigned int word2, unsigned int word3)
{
    if (index <= 19) {
        return (word1 & word2) | ((~word1) & word3);
    }
    else if (index <= 39) {
        return word1 ^ word2 ^ word3;
    }
    else if (index <= 59) {
        return (word1 & word2) | (word1 & word3) | (word2 & word3);
    }
    else if (index <= 79) {
        return word1 ^ word2 ^ word3;
    }
    return 0;
}

unsigned int rotL(int times, unsigned int word)
{
    return (word << times) | (word >> (BYTES_PER_WORD * BITS_PER_BYTE - times));
}
