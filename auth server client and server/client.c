#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include "time.h"
#include <ctype.h>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include "packet_header.h"

#define ECHOMAX 255     /* Longest string to echo */
#define MAXIDLEN 40     /* Longest Id Length */

void DieWithError(char *errorMessage)  /* External error handling function */
{
    perror(errorMessage);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sock;                        /* Socket descriptor */
    struct sockaddr_in AuthServAddr;  /* Echo server address */
    unsigned int AuthServAddrLen;
    struct sockaddr_in APServAddr;
    unsigned int APServAddrLen;
    //unsigned short echoServPort;     /* Echo server port */
    //unsigned int fromSize;           /* In-out of address size for recvfrom() */
    //char *servIP;                    /* IP address of server */
    //char *echoString;                /* String to send to echo server */
    //char echoBuffer[ECHOMAX+1];      /* Buffer for receiving echoed string */
    //int echoStringLen;               /* Length of string to echo */
    //int respStringLen;               /* Length of received response */
    char *authServerName;
    unsigned short authServerPort;
    char *serverName;
    unsigned short serverPort;
    unsigned char client_id[40];
    unsigned char server_id[40];
    //memset(client_id, 0, sizeof (client_id));
    //memset(server_id, 0, sizeof (server_id));
    if (argc != 8)    /* Test for correct number of arguments */
    {
        fprintf(stderr,"Usage:  %s <authservername, authserverport, clientkey, server name, server port, clientID, serverID>\n", argv[0]);
        exit(1);
    }
    authServerName = argv[1];
    authServerPort = atoi(argv[2]);
    serverName = argv[4];
    serverPort = atoi(argv[5]);
    strcpy(client_id, argv[6]);
    strcpy(server_id, argv[7]);
    unsigned char clientKey[32];
    memset(clientKey, 0, 32);
    strcpy(clientKey, argv[3]);

    if (strlen(client_id) > MAXIDLEN || strlen(server_id) > MAXIDLEN) {
        DieWithError("Id too long");
    }

    for (int i = 0; i < strlen(clientKey); i++) {
        //printf("%c\n", clientKey[i]);
        if (!isascii(clientKey[i])) {
            DieWithError("clienKey should only have ASCII characters");
        }
    }

    //for (int i = 0; i < sizeof (clientKey); i++) {
    //    if (clientKey[i] <= 32 || clientKey[i] >= 128) {
    //        DieWithError("clientKey should only have ASCII characters");
    //    }
    //}
    //SEND THE AS_REQ FIRST
    struct as_req req;
    int reqLen;
    reqLen = sizeof (struct as_req);
    memset(&req, 0, sizeof (req));
    strcpy(req.client_id, client_id);
    strcpy(req.server_id, server_id);
    req.type = 1;
    req.ts1 = time(NULL);
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        DieWithError("socket1() failed");
    }
    memset(&AuthServAddr, 0, sizeof (AuthServAddr));
    AuthServAddr.sin_family = AF_INET;
    AuthServAddr.sin_addr.s_addr = inet_addr(authServerName);
    AuthServAddr.sin_port = htons(authServerPort);
    if (sendto(sock, &req, reqLen, 0, (struct sockaddr *)
               &AuthServAddr, sizeof(AuthServAddr)) != reqLen)
        DieWithError("sendto() sent a different number of bytes than expected");

    //if (bind(sock, (struct sockaddr *) &AuthServAddr, sizeof(AuthServAddr)) < 0)
    //    DieWithError("bind1() failed");
    struct as_rep as1;
    memset(&as1, 0, sizeof (as1));
    int r1MsgLen;
    if ((r1MsgLen = recvfrom(sock, &as1, sizeof (struct as_rep), 0,
            (struct sockaddr *) &AuthServAddr, &AuthServAddrLen)) < 0)
            DieWithError("recvfrom() failed");
    //printf("%d\n", r1MsgLen);
    unsigned char iv[16];
    memset(iv, 0, 16);
    //printf("%d\n", as1.cred_length);
    //printf("%d\n", as1.type);
    struct credential c1;
    memset(&c1, 0, sizeof (c1));
    int c1cipherLen;
    //MARK, PROBLEM STILL HERE, WAIT FOR NEXT SOVLE
    c1cipherLen = decrypt(as1.cred, as1.cred_length, clientKey, iv, (unsigned char *) &c1);
    //printf("%d\n", c1cipherLen);
    unsigned char AES_key[32];
    memset(AES_key, 0, sizeof (AES_key));
    strcpy(AES_key, c1.AES_key);
    //printf("%s\n", c1.AES_key);
    //printf("%s\n", AES_key);
    //NOW WE HAVE HE AES_KEY IN THE CLIENT
    //NOW DECRYPT DONE PAREPRE THE AP_REQ
    struct auth a1;
    memset(&a1, 0, sizeof (struct auth));
    strcpy(a1.client_id, client_id);
    a1.ts3 = time(NULL);
    //unsigned char nonce1 = a1.ts3;
    time_t nonce1 = a1.ts3;
    //printf("%ld\n", a1.ts3);
    int authencryptLen;
    unsigned char a1_cipher[1024];
    authencryptLen = encrypt ((unsigned char *) &a1, sizeof(struct auth), AES_key, iv, a1_cipher);
    // FINISH THE AUTH
    struct ap_req ap1;
    int ap1Len = sizeof (struct ap_req);
    memset(&ap1, 0, sizeof (struct ap_req));
    ap1.type = 4;
    ap1.tkt_length = c1.tkt_length;
    //printf("%d\n", ap1.tkt_length);
    ap1.auth_length = authencryptLen;
    //printf("%d\n", ap1.auth_length);
    memcpy(ap1.tkt_serv, c1.tkt_serv, c1.tkt_length);
    memcpy(ap1.auth, a1_cipher, ap1.auth_length);
    //strcpy(ap1.tkt_serv, c1.tkt_serv);
    //strcpy(ap1.auth, a1_cipher);
    int sock2;
    if ((sock2 = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        DieWithError("socket2() failed");
    }
    memset(&APServAddr, 0, sizeof (APServAddr));
    APServAddr.sin_family = AF_INET;
    APServAddr.sin_addr.s_addr = inet_addr(serverName);
    APServAddr.sin_port = htons(serverPort);
    if (sendto(sock2, &ap1, ap1Len, 0, (struct sockaddr *)
               &APServAddr, sizeof(APServAddr)) != ap1Len)
        DieWithError("sendto() sent a different number of bytes than expected");

    //FINISH SEND THE AP_REQ, NOW WAIT FOR THE AP_REP;
    /* Bind to the local address */
    //if (bind(sock2, (struct sockaddr *) &APServAddr, sizeof(APServAddr)) < 0)
    //    DieWithError("bind() failed");

    //NOW TRY TO RECIEVE THE AP_REP
    struct ap_rep ap2;
    int ap2Len;
    if ((ap2Len = recvfrom(sock2, &ap2, sizeof (struct ap_rep), 0,
            (struct sockaddr *) &APServAddr, &APServAddrLen)) < 0)
            DieWithError("recvfrom() failed");
    int ap2decryptLen;
    //char tmp_decrypted[1024];
    time_t nonce2;
    ap2decryptLen = decrypt(ap2.nonce, ap2.nonce_length, AES_key, iv, (unsigned char *) &nonce2);
    //int nonceLen = ap2.nonce_length;
    //long int nonce2[1024];
    //memset(&nonce2, 0, ap2decryptLen);
    //memcpy(&nonce2, tmp_decrypted, ap2decryptLen);
    //printf("%ld\n", nonce2);
    if (nonce1 + 1 != nonce2) {
        printf("%ld\n", nonce1);
        printf("%ld\n", nonce2);
        struct ap_err err1;
        memset(&err1, 0, sizeof (struct ap_err));
        err1.type = 6;
        strcpy(err1.client_id, client_id);
        if (sendto(sock2, &err1, sizeof (struct ap_err), 0, (struct sockaddr *)
                       &APServAddr, sizeof(APServAddr)) != sizeof (struct ap_err))
                DieWithError("sendto() sent a different number of bytes than expected");
        DieWithError("nonce not equal, can not auth");
    }
    //NOW WE FINISH RECEIVE THE AP_REP FROM THE SERVER
    //NOW PAREPRE THE KRB_PRV TO REQUEST DATA
    struct pdata p1;
    memset(&p1, 0, sizeof (p1));
    p1.type = 14;
    strcpy(p1.data, "One sentence");
    //printf("%s\n", p1.data);
    p1.packet_length = sizeof (p1.data);
    p1.pid = 1;
    int p1encryptLen;
    unsigned char p1_cipher[1024];
    p1encryptLen = encrypt((unsigned char *) &p1, sizeof(struct pdata), AES_key, iv, p1_cipher);
    //NOW FINISH THE PDATA, DEAL WITH THE KRB_PRV
    struct krb_prv kreq;
    memset(&kreq, 0, sizeof (kreq));
    kreq.type = 7;
    kreq.prv_length = p1encryptLen;
    memcpy(kreq.prv, p1_cipher, p1encryptLen);
    if (sendto(sock2, &kreq, sizeof (struct krb_prv), 0, (struct sockaddr *)
               &APServAddr, sizeof(APServAddr)) != sizeof (struct krb_prv))
        DieWithError("sendto() sent a different number of bytes than expected");
    //NOW FINISH SEND THE KREQ, MOVE TO WAIT THE REPLY
    struct krb_prv krep;
    int krepLen;
    if ((krepLen = recvfrom(sock2, &krep, sizeof (struct krb_prv), 0,     //THE LENGTH MAY HAVE PROBLEM
            (struct sockaddr *) &APServAddr, &APServAddrLen)) < 0)
            DieWithError("recvfrom2() failed");
    struct pdata res;
    int resLen;
    resLen = decrypt(krep.prv, krep.prv_length, AES_key, iv, (unsigned char *) &res);
    if (res.type == 15) {
        printf("%s\n", "OK");
        printf("%s\n", "Decrypt data was");
        printf("%s\n", res.data);
        printf("%s\n", "Encrypt data was");
        printf("%s\n", krep.prv);
    }
    else {
        printf("%s\n", "ABORT");
        printf("%s\n", "Wrong packet type");
    }
}

