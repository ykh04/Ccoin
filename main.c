//2022 ykh04
#define	WINSOCK_USE		2.0		/* Only 32bit library */


#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <winsock2.h>

struct SOCKTHREAD {
    BOOL    isAvailableSock;
    SOCKET  sock;
};

struct SOCKTHREAD sockThread;
BOOL isExitThread = FALSE;

int main(int argc, char *argv[])
{
    int n;
    struct sockaddr_in server, receiver;
    char buf[32];
    SOCKET srvSock, lstnRecvSock;
    WSADATA wsaData;

    /*winsock2の初期化*/
    WSAStartup(MAKEWORD(2,0), &wsaData);

    /* ソケットの作成 */
    if ((srvSock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("error:%d\n",WSAGetLastError());
    }
    if ((lstnRecvSock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("error:%d\n",WSAGetLastError());
    }

    /* 接続先指定用構造体の準備 */
    server.sin_family = AF_INET;
    server.sin_port = htons(12345);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    receiver.sin_family = AF_INET;
    receiver.sin_port = htons(12346);
    receiver.sin_addr.s_addr = INADDR_ANY;
    bind(lstnRecvSock, (struct sockaddr *)&receiver, sizeof(receiver));

    /* サーバに接続 */
    connect(srvSock, (struct sockaddr *)&server, sizeof(server));

    /* サーバからデータを受信 */
    memset(buf, 0, sizeof(buf));
    n = recv(srvSock, buf, sizeof(buf), 0);

    printf("%d, %s\n", n, buf);

    /* socketの終了 */

    closesocket(srvSock);
    /*winsock2の終了*/
    WSACleanup();

    return 0;
}

DWORD WINAPI acceptThread(LPVOID lpParam)
{
    SOCKET lstnRecvSock = *((SOCKET *) lpParam), recvSock;
    struct sockaddr_in  client;
    int len;
    DWORD   isNonBlocking = 1;

    ioctlsocket(lstnRecvSock, FIONBIO, &isNonBlocking);

    listen(lstnRecvSock, 5);
    while (!isExitThread) {
        len = sizeof(client);
        recvSock = accept(lstnRecvSock, (struct sockaddr *)&client, &len);
        if (recvSock < 0) {
            sockThread.isAvailableSock = FALSE;
        }
        else {
            sockThread.sock = recvSock;
            sockThread.isAvailableSock = TRUE;
        }
    }

}

DWORD WINAPI receiveThread(LPVOID lpParam)
{

}
