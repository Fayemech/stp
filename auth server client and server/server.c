#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket() and bind() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include "packet_header.h"
#include <ctype.h>

#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>

#define ECHOMAX 255     /* Longest string to echo */

void DieWithError(char *errorMessage)  /* External error handling function */
{
    perror(errorMessage);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sock;                        /* Socket */
    struct sockaddr_in ServAddr; /* Local address */
    struct sockaddr_in ClntAddr; /* Client address */
    unsigned int cliAddrLen;         /* Length of incoming message */
    unsigned short ServPort;     /* Server port */
    int recvMsgSize;                 /* Size of received message */

    if (argc != 3)         /* Test for correct number of parameters */
    {
        fprintf(stderr,"Usage:  %s <ServerPort, SeverKey>\n", argv[0]);
        exit(1);
    }

    ServPort = atoi(argv[1]);  /* First arg:  local port */
    unsigned char serverKey[32];
    memset(serverKey, 0, 32);
    strcpy(serverKey, argv[2]);
    for (int i = 0; i < strlen(serverKey); i++) {
        //printf("%c\n", serverKey[i]);
        if (!isascii(serverKey[i])) {
            DieWithError("serverKey should only have ASCII characters");
        }
    }

    //WAIT FOR AP_REQ FIRST
    /* Create socket for sending/receiving datagrams */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    /* Construct local address structure */
    memset(&ServAddr, 0, sizeof(ServAddr));   /* Zero out structure */
    ServAddr.sin_family = AF_INET;                /* Internet address family */
    ServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    ServAddr.sin_port = htons(ServPort);      /* Local port */

    /* Bind to the local address */
    if (bind(sock, (struct sockaddr *) &ServAddr, sizeof(ServAddr)) < 0)
        DieWithError("bind() failed");
  
    cliAddrLen = sizeof(ClntAddr);
    struct ap_req ap1;
    int ap1Len = sizeof (struct ap_req);
    if ((recvMsgSize = recvfrom(sock, &ap1, ap1Len, 0,
            (struct sockaddr *) &ClntAddr, &cliAddrLen)) < 0)
            DieWithError("recvfrom() failed");
    //printf("%d\n", ap1.tkt_length);
    //printf("%d\n", ap1.auth_length);
    struct auth a1;
    struct ticket t1;
    //memset(&t1, 0, sizeof (t1));
    //memset(&a1, 0, sizeof (a1));
    int t1decryptLen;
    int a1decryptLen;
    unsigned char iv[16];
    memset(iv, 0, 16);
    //printf("%s\n", serverKey);
    char area_decrypted[1024];
    t1decryptLen = decrypt(ap1.tkt_serv, ap1.tkt_length, serverKey, iv, (unsigned char *) &area_decrypted);
    memcpy(&t1, area_decrypted, t1decryptLen);
    unsigned char AES_key[32];
    memset(AES_key, 0, sizeof (AES_key));
    strcpy(AES_key, t1.AES_key);
    //printf("%s\n", AES_key);
    //NOW THE CLIENT HAVE THE AES_KEY FOR THE FUTURE USE
    a1decryptLen = decrypt(ap1.auth, ap1.auth_length, AES_key, iv, (unsigned char *) &a1);
    if (strcmp(a1.client_id, t1.client_id)) {
        struct ap_err err1;
        memset(&err1, 0, sizeof (struct ap_err));
        err1.type = 6;
        strcpy(err1.client_id, a1.client_id);         //If the two client_id was different here, which should we use in this err msg, t1.client_id or a1.client_id?
        if (sendto(sock, &err1, sizeof (struct ap_err), 0, (struct sockaddr *)
                       &ClntAddr, sizeof(ClntAddr)) != sizeof (struct ap_err))
                DieWithError("sendto() sent a different number of bytes than expected");
    }
    //WE START TO PAREPRE THE AP_REP
    time_t tmp;
    tmp = a1.ts3 + 1;
    //printf("%ld\n", tmp);
    int nonceLen;
    unsigned char t3_cipher[1024];
    nonceLen = encrypt ((unsigned char *) &tmp, sizeof(tmp), AES_key, iv, t3_cipher);
    struct ap_rep ap2;
    memcpy(ap2.nonce, t3_cipher, nonceLen);
    //strcpy(ap2.nonce, t3_cipher);
    ap2.nonce_length = nonceLen;
    ap2.type = 5;
    //NOW WE FINISH THE AP_REP, SEND IT
    if (sendto(sock, &ap2, sizeof (struct ap_rep), 0, (struct sockaddr *)
               &ClntAddr, sizeof(ClntAddr)) != sizeof (struct ap_rep))
        DieWithError("AP_REP wrong");
    //NOW WE FINISH SEND THE AP_REP TO THE CLIENT
    //NOW WE TRY TO RECIEVE THE KRB_PRV
    struct krb_prv kreq;
    int kreqLen;
    if ((kreqLen = recvfrom(sock, &kreq, sizeof (struct krb_prv), 0,
            (struct sockaddr *) &ClntAddr, &cliAddrLen)) < 0)
            DieWithError("recvfrom() failed");
    struct pdata p1;
    int p1decryptLen;
    p1decryptLen = decrypt(kreq.prv, kreq.prv_length, AES_key, iv, (unsigned char *) &p1);
    if (p1.type == 14) {
        printf("%s\n", p1.data);
        struct pdata p2;
        memset(&p2, 0, sizeof (p2));
        p2.type = 15;
        p2.pid = p1.pid + 1;
        strcpy(p2.data, "Finally I got to send the data to the client. Succeed!");
        p2.packet_length = sizeof (p2.data);
        int p2encryptLen;
        unsigned char p2_cipher[1024];
        p2encryptLen = encrypt((unsigned char *) &p2, sizeof(struct pdata), AES_key, iv, p2_cipher);
        struct krb_prv krep;
        memset(&krep, 0, sizeof (krep));
        krep.type = 7;
        krep.prv_length = p2encryptLen;
        memcpy(krep.prv, p2_cipher, p2encryptLen);
        if (sendto(sock, &krep, sizeof (struct krb_prv), 0, (struct sockaddr *)
                   &ClntAddr, sizeof(ClntAddr)) != sizeof (struct krb_prv))
            DieWithError("AP_REP wrong");
        printf("%s\n", "server finish work here");
    }

}

