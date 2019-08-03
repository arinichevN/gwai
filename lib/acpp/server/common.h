#include "../data.h"
#define SERVER_CMD cmd
#define SERVER_FD fd

#define SERVER_SEND_END acpp_send(ACPP_DELIMITER_END_STR, SERVER_FD);

#define SERVER_READ_CMD char SERVER_CMD[ACPP_CMD_MAX_LENGTH]; if ( !acpp_readCmd(SERVER_CMD, ACPP_CMD_MAX_LENGTH, SERVER_FD)) {SERVER_GOTO_STOP}
#define SERVER_GOTO_STOP goto server_stop;

#define SERVER_READ_LIST(T, N, L) if ( L<=0 ) {return;}T N##_arr[L];T##List N##l;N##l.item=N##_arr;N##l.max_length=L;N##l.length=0; acpp_read##T##List ( &N##l, SERVER_FD );if ( N##l.length <= 0 ) {return;}
#define SERVER_READ_I1LIST(L)   SERVER_READ_LIST(I1, i1, L)
#define SERVER_READ_I2LIST(L)   SERVER_READ_LIST(I2, i2, L)
#define SERVER_READ_I1F1LIST(L) SERVER_READ_LIST(I1F1, i1f1, L)
#define SERVER_READ_I1S1LIST(L) SERVER_READ_LIST(I1S1, i1s1, L)

#define SERVER_RESPONSE_DEF_LIST(T, N, L) T N##_rarr[L];T##List N##lr; N##lr.item=N##_rarr; N##lr.max_length=L; N##lr.length=0;
#define SERVER_RESPONSE_DEF_FTSLIST(L) SERVER_RESPONSE_DEF_LIST(FTS, fts, L)

#define SERVER_RESPONSE_FTSLIST_PUSH(ID, V, TM, STATE) if(ftslr.length < ftslr.max_length){ftslr.item[ftslr.length].id = ID; ftslr.item[ftslr.length].value = V; ftslr.item[ftslr.length].tm = TM; ftslr.item[ftslr.length].state = STATE; ftslr.length++;}
#define SERVER_RESPONSE_FTSLIST_SEND acpp_sendFTSList(&ftslr, SERVER_FD);

#define SERVER_DEF_LIST(T,N) int alen = acpp_bufCountDataRows ( request );if ( alen<=0 ) {return;}T N##_arr[alen];T##List N##l;N##l.item=N##_arr;N##l.max_length=alen;N##l.length=0; acpp_read##T##List ( &N##l, SERVER_FD );if ( N##l.length <= 0 ) {return;}
#define SERVER_DEF_I1LIST SERVER_DEF_LIST(I1,i1)
#define SERVER_DEF_I1F1LIST SERVER_DEF_LIST(I1F1,i1f1)


