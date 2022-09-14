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
void padding(unsigned int size, unsigned char *data, unsigned char *blocks, unsigned int BLOCKBYTES); /*�p�f�B���O�֐��i�߂�F�Ȃ��@�����P�F���b�Z�[�W�T�C�Y�@�����Q�F���b�Z�[�W�f�[�^�@�����R�F�o�͗p�̈�A�h���X�@�����S�F�S�u���b�N�Ɋ܂܂�鑍�o�C�g���j*/
unsigned int sigma(int index, unsigned int word1, unsigned int word2, unsigned int word3);
unsigned int rotL(int times, unsigned int word);

static unsigned int K[80] = { /*�萔���[�h�V�[�P���X*/ /*issue: ������for�łǂ̂悤�Ȉ����������Ă��邩���ׂ�*/
    0x5A827999, 0x5A827999, 0x5A827999, 0x5A827999, 0x5A827999, 0x5A827999, 0x5A827999, 0x5A827999, 0x5A827999, 0x5A827999,
    0x5A827999, 0x5A827999, 0x5A827999, 0x5A827999, 0x5A827999, 0x5A827999, 0x5A827999, 0x5A827999, 0x5A827999, 0x5A827999,
    0x6ED9EBA1, 0x6ED9EBA1, 0x6ED9EBA1, 0x6ED9EBA1, 0x6ED9EBA1, 0x6ED9EBA1, 0x6ED9EBA1, 0x6ED9EBA1, 0x6ED9EBA1, 0x6ED9EBA1,
    0x6ED9EBA1, 0x6ED9EBA1, 0x6ED9EBA1, 0x6ED9EBA1, 0x6ED9EBA1, 0x6ED9EBA1, 0x6ED9EBA1, 0x6ED9EBA1, 0x6ED9EBA1, 0x6ED9EBA1,
    0x8F1BBCDC, 0x8F1BBCDC, 0x8F1BBCDC, 0x8F1BBCDC, 0x8F1BBCDC, 0x8F1BBCDC, 0x8F1BBCDC, 0x8F1BBCDC, 0x8F1BBCDC, 0x8F1BBCDC,
    0x8F1BBCDC, 0x8F1BBCDC, 0x8F1BBCDC, 0x8F1BBCDC, 0x8F1BBCDC, 0x8F1BBCDC, 0x8F1BBCDC, 0x8F1BBCDC, 0x8F1BBCDC, 0x8F1BBCDC,
    0xCA62C1D6, 0xCA62C1D6, 0xCA62C1D6, 0xCA62C1D6, 0xCA62C1D6, 0xCA62C1D6, 0xCA62C1D6, 0xCA62C1D6, 0xCA62C1D6, 0xCA62C1D6,
    0xCA62C1D6, 0xCA62C1D6, 0xCA62C1D6, 0xCA62C1D6, 0xCA62C1D6, 0xCA62C1D6, 0xCA62C1D6, 0xCA62C1D6, 0xCA62C1D6, 0xCA62C1D6
};

int Nsha1(unsigned char *data, unsigned int size, unsigned int *SH0, unsigned int *SH1, unsigned int *SH2, unsigned int *SH3, unsigned int *SH4) /*��{�I�Ƀo�C�g�f�[�^������*/
{
    unsigned int numOfBlocks, BLOCKBYTES, *wordBuf, j, k; /*���b�Z�[�W�ɉ����ĕω�����u���b�N�̐�*/
    unsigned char *blocks = NULL, wordBufchar[80 * BYTES_PER_WORD]; /*���b�Z�[�W�ɉ����ē��I�Ɋm�ۂ����p�f�B���O�p��ԁi�[�������������ł��낤�j*/

    unsigned int A, B, C, D, E, TEMP;
    unsigned int H0 = 0x67452301, H1 = 0xEFCDAB89, H2 = 0x98BADCFE, H3 = 0x10325476, H4 = 0xC3D2E1F0;


    if ((size % 64) < 56) {
        numOfBlocks = size / (WORDS_PER_BLOCK * BYTES_PER_WORD) + 1; /*3���[�h�̗]�T������i�T�C�Y���p�j*/
    }
    else {
        numOfBlocks = size / (WORDS_PER_BLOCK * BYTES_PER_WORD) + 2; /*3���[�h�̗]�T���Ȃ��������1�u���b�N���₷*/
    }
    /*============================
        �u���b�N�̊m��
    ============================*/
    BLOCKBYTES = numOfBlocks * WORDS_PER_BLOCK * BYTES_PER_WORD;
    blocks = calloc(BLOCKBYTES, sizeof(char)); /*���I�m�ہi�[���������j*/
    if (blocks == NULL) {   /*�������[�e�ʂ��s�����Ă��ė̈悪�m�ۂł��Ȃ������i�ŋ߂̃}�V���ł͂Ȃ��Ǝv�����A�ꉞ�j*/
        printf("\n\n");
        printf("ERROR: Lack of Memory Resource!!");
        return DUMMY_ERROR;
    }


    /*============================
        �p�f�B���O
    ============================*/
    padding(size, data, blocks, BLOCKBYTES);    /*�p�f�B���O�֐����Ăяo��*/
    /*============================
        �v�Z
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
    unsigned char *bitSize = NULL; /*�r�b�g�T�C�Y�ɒ������߂̔z��*/
    bSize = size * 8;

    memcpy(blocks, data, size); /*�p�f�B���O�̈�փR�s�[*/
    memset(blocks + size, 0x80, 1); /*���Ɏ��ʗp�R�[�h��}���i0����n�܂��Ă邩�炱��ł悢�̂��j*/

    bitSize = (unsigned char *)&bSize; /*32bit�̈��8bit�̈�ň�������*/
    for (i = 0; i < 4; i++) { /*���g���G���f�B�A�����r�b�O�G���f�B�A���ɕϊ�����*/
        *(blocks + BLOCKBYTES - (i + 1)) = *(bitSize + i);
    }
}

/*---�O��---

[|]��bit��OR���Z��\���܂�
[&}��bit��AND���Z��\���܂�
[~]��bit��NOT���Z(1�̕␔���Ƃ�Ƃ�)��\���܂���
[^]��bit��XOR(�r���I�_���a)���Z��\���܂�
[<<][>>]�͂��ꂼ�ꍶ�_���V�t�g�A�E�_���V�t�g��\���܂��i�����Ȃ��������i�Ƃ������S���j�̂Ŋ�{�_���V�t�g�Ƃ݂Ă悢�j

���l�F
     C��32bit�����Z�͂Q����2^32�̗]��̘a
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
