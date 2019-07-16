#include "../data.h"
#define SERVER_READ if ( !acpp_read ( request, fd_conn ) ) {SERVER_GOTO_STOP}
#define SERVER_READ_CMD char cmd[ACPP_CMD_MAX_LENGTH]; if ( !acpp_readCmd(cmd, ACPP_CMD_MAX_LENGTH, fd_conn)) {SERVER_GOTO_STOP}
#define SERVER_GOTO_STOP goto server_stop;

#define SERVER_READ_LIST(T, N, L) if ( L<=0 ) {SERVER_GOTO_STOP}T N##_arr[L];T##List N##l;N##l.item=N##_arr;N##l.max_length=L;N##l.length=0; acpp_read##T##List ( &N##l, fd_conn );if ( N##l.length <= 0 ) {SERVER_GOTO_STOP}
#define SERVER_READ_I1LIST(L)   SERVER_READ_LIST(I1, i1, L)
#define SERVER_READ_I2LIST(L)   SERVER_READ_LIST(I2, i2, L)
#define SERVER_READ_I1F1LIST(L) SERVER_READ_LIST(I1F1, i1f1, L)
#define SERVER_READ_I1S1LIST(L) SERVER_READ_LIST(I1S1, i1s1, L)

#define SERVER_RESPONSE_DEF_LIST(T, N, L) T N##_rarr[L];T##List N##lr; N##lr.item=N##_rarr; N##lr.max_length=L; N##lr.length=0;
#define SERVER_RESPONSE_DEF_FTSLIST(L) SERVER_RESPONSE_DEF_LIST(FTS, fts, L)

#define SERVER_RESPONSE_FTSLIST_PUSH(ID, V, TM, STATE) if(ftslr.length < ftslr.max_length){ftslr.item[ftslr.length].id = ID; ftslr.item[ftslr.length].value = V; ftslr.item[ftslr.length].tm = TM; ftslr.item[ftslr.length].state = STATE; ftslr.length++;}
#define SERVER_RESPONSE_FTSLIST_SEND acpp_sendFTSList(&ftslr, fd_conn);

#define SERVER_DEF_LIST(T,N) int alen = acpp_bufCountDataRows ( request );if ( alen<=0 ) {SERVER_GOTO_STOP}T N##_arr[alen];T##List N##l;N##l.item=N##_arr;N##l.max_length=alen;N##l.length=0; acpp_read##T##List ( &N##l, fd_conn );if ( N##l.length <= 0 ) {SERVER_GOTO_STOP}
#define SERVER_DEF_I1LIST SERVER_DEF_LIST(I1,i1)
#define SERVER_DEF_I1F1LIST SERVER_DEF_LIST(I1F1,i1f1)

//#define SERVER_DEF_I1LIST int alen = acpp_bufCountDataRows ( request );if ( alen<=0 ) {SERVER_GOTO_STOP}I1 i1_arr[alen];I1List i1l;i1l.item=i1_arr;i1l.max_length=alen;i1l.length=0;
//#define SERVER_DEF_I1F1LIST int alen = acpp_bufCountDataRows ( request );if ( alen<=0 ) {SERVER_GOTO_STOP}I1F1 i1f1_arr[alen];I1F1List i1f1l;i1f1l.item=i1f1_arr;i1f1l.max_length=alen;i1f1l.length=0;

#define SERVER_PARSE_LIST(T, N) char *ds = request;if ( !acpp_buftodata ( &ds ) ) {putsde ( "failed to find data\n" );SERVER_GOTO_STOP} acpp_get##T##List ( ds, &N##l );if ( N##l.length <= 0 ) {SERVER_GOTO_STOP}
#define SERVER_PARSE_FLOAT(N) char *ds = request;if ( !acpp_buftodata ( &ds ) ) {putsde ( "failed to find data\n" );SERVER_GOTO_STOP} double N; if(!acpp_getF ( ds, &N )){putsde ( "failed to parse data\n");SERVER_GOTO_STOP}
#define SERVER_PARSE_I1LIST SERVER_PARSE_LIST(I1, i1)
#define SERVER_PARSE_I1F1LIST SERVER_PARSE_LIST(I1F1, i1f1)
//#define SERVER_PARSE_I1LIST char *ds = request;if ( !acpp_buftodata ( &ds ) ) {putsde ( "failed to find data\n" );SERVER_GOTO_STOP} acpp_getI1List ( ds, &i1l );if ( i1l.length <= 0 ) {SERVER_GOTO_STOP}

#define SERVER_SEND_RESPONSE if ( !acpp_setEnd ( response ) ) {putsde ( "failed to add end sign\n" );SERVER_GOTO_STOP}acpp_send ( response, fd_conn );

