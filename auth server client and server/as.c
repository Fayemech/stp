#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket() and bind() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include "packet_header.h"
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <ctype.h>


#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>

#define MAXIDLEN 40

void DieWithError(char *errorMessage)  /* External error handling function */
{
    perror(errorMessage);
    exit(1);
}


int main(int argc, char* argv[])
{
    if (argc != 6) {
        fprintf(stderr,"Usage:  %s <authserverport, client_id, clientKey, server_id, serverKey>\n", argv[0]);
        exit(1);
    }
    unsigned short authserverport;
    unsigned char client_id[40];
    unsigned char server_id[40];
    strcpy(client_id, argv[2]);
    strcpy(server_id, argv[4]);
    authserverport = atoi(argv[1]);          /* local port */
    /* PAREPRE THE KEY */
    unsigned char serverKey[32];
    memset(serverKey, 0, 32);
    strcpy(serverKey, argv[5]);
    unsigned char clientKey[32];
    memset(clientKey, 0, 32);
    strcpy(clientKey, argv[3]);
    int sock;                        /* Socket */
    struct sockaddr_in AuthServAddr; /* Local address */
    struct sockaddr_in ClntAddr; /* Client address */
    unsigned int cliAddrLen;         /* Length of incoming message */
    //char echoBuffer[ECHOMAX];        /* Buffer for echo string */
    //unsigned short echoServPort;     /* Server port */
    //int recvMsgSize;                 /* Size of received message */
    unsigned char iv[16];
    memset(iv, 0, 16);

    if (strlen(client_id) > MAXIDLEN || strlen(server_id) > MAXIDLEN) {
        DieWithError("Id too long");
    }

    for (int i = 0; i < strlen(clientKey); i++) {
        if (!isascii(clientKey[i])) {
            DieWithError("clienKey should only have ASCII characters");
        }
    }

    for (int i = 0; i < strlen(serverKey); i++) {
        if (!isascii(serverKey[i])) {
            DieWithError("serverKey should only have ASCII characters");
        }
    }


    /* Create socket for sending/receiving datagrams */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    /* Construct local address structure */
    memset(&AuthServAddr, 0, sizeof(AuthServAddr));   /* Zero out structure */
    AuthServAddr.sin_family = AF_INET;                /* Internet address family */
    AuthServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    AuthServAddr.sin_port = htons(authserverport);      /* Local port */

    /* Bind to the local address */
    if (bind(sock, (struct sockaddr *) &AuthServAddr, sizeof(AuthServAddr)) < 0)
        DieWithError("bind() failed");

    cliAddrLen = sizeof(ClntAddr);

    struct as_req reqMsg;
    memset(&reqMsg, 0, sizeof (struct as_req));
    int reqMsgLen;
    reqMsgLen = sizeof (struct as_req);

    if ((reqMsgLen = recvfrom(sock, &reqMsg, reqMsgLen, 0,
            (struct sockaddr *) &ClntAddr, &cliAddrLen)) < 0)
            DieWithError("recvfrom() failed");

    // NOW WE RECEIVED AS_REQ, CHECK IF THE CLIENT ID OR SERVER ID CORRECT;
    if (strcmp(reqMsg.client_id, client_id) || strcmp(reqMsg.server_id, server_id)) {
        struct as_err errorMsg;
        int errorMsgLen;
        errorMsg.type = 3;
        errorMsgLen = sizeof (struct as_err);
        memset(&errorMsg, 0, sizeof (struct as_err));
        strcpy(errorMsg.client_id, client_id);
        int errMsgLen;
        unsigned char error_cipher[1024];
        errMsgLen = encrypt ((unsigned char *) &errorMsg, sizeof(struct as_err), clientKey, iv, error_cipher);
        if (sendto(sock, &error_cipher, errMsgLen, 0, (struct sockaddr *)
                   &ClntAddr, sizeof(ClntAddr)) != errorMsgLen)
            DieWithError("errMsg wrong");

    }
    // NOW WE PAREPRE THE AS_REP
    unsigned char AES_key[32];
    memset(AES_key, 0, 32);
    strcpy(AES_key, "0123456789012345678901234567890");


    //FINISH THE TICKET ENCRYPT WITH SERVER KEY
    struct ticket t1;
    memset(&t1, 0, sizeof (t1));
    strcpy(t1.client_id, client_id);
    strcpy(t1.server_id, server_id);
    strcpy(t1.AES_key, AES_key);
    t1.ts2 = time(NULL);
    t1.lt = LIFETIME;
    int cipherticket_len;
    unsigned char t1_cipher[1024];
    //memset(t1_cipher, 0, 1024);
    cipherticket_len = encrypt ((unsigned char *) &t1, sizeof(struct ticket), serverKey, iv, t1_cipher);    
    //NOW DEAL WITH THE CREDENTIAL
    struct credential c1;
    memset(&c1, 0, sizeof (c1));
    strcpy(c1.AES_key, AES_key);
    strcpy(c1.server_id, server_id);
    c1.ts2 = time(NULL);
    //c1.ts2 = t1.ts2;
    c1.tkt_length = cipherticket_len;
    //printf("%d\n", c1.tkt_length);
    memcpy(c1.tkt_serv, t1_cipher, cipherticket_len);
    unsigned char c1_cipher[1024];
    memset(c1_cipher, 0, 1024);
    int ciphercred_len;
    ciphercred_len = encrypt ((unsigned char *) &c1, sizeof(struct credential), clientKey, iv, c1_cipher);
    //FINISH THE CREDENTIAL, NOW SEND IT.
    struct as_rep r1;
    memset(&r1, 0, sizeof (r1));
    r1.type = 2;
    r1.cred_length = ciphercred_len;
    memcpy(r1.cred, c1_cipher, ciphercred_len);
    if (sendto(sock, &r1, sizeof (r1), 0, (struct sockaddr *)
               &ClntAddr, sizeof(ClntAddr)) != sizeof (r1))
        DieWithError("AS_REP WRONG");

    //AS SHOULD FINISH ITS WORK HERE;
    printf("%s\n", "AS finish its work");
}
