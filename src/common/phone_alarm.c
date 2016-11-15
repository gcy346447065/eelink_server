//
// Created by jk on 16-11-10.
//
#include <stdio.h>

#include "topsdk.h"
#include "phone_alarm.h"

const static char *url = "http://gw.api.taobao.com/router/rest?";
const static char *appkey = "23499944";
const static char *appsecret = "ee15cc89c463c7ce775569e6e05a4ec2";

const static char *call_number = "01053912804";
const static char *tts_template = "TTS_25315181";

int phone_alarm(const char* telphone)
{
    pTopRequest pRequest = alloc_top_request();
    pTopResponse pResponse = NULL;
    pTaobaoClient pClient = alloc_taobao_client(url, appkey, appsecret);
    set_api_name(pRequest, "alibaba.aliqin.fc.tts.num.singlecall" );
    // add_param(pRequest, "extend" , "12345" );
    // add_param(pRequest, "tts_param" , "{\"AckNum\":\"123456\"}" );
    add_param(pRequest, "called_num" , telphone);
    add_param(pRequest, "called_show_num" ,  call_number);
    add_param(pRequest, "tts_code" , tts_template);
    pResponse = top_execute(pClient,pRequest,NULL);
    printf( "ret code:%d\n" ,pResponse->code);
    if (pResponse->code == 0 ){
        pTopResponseIterator ite = init_response_iterator(pResponse);
        pResultItem pResultItem = alloc_result_item();
        while (parseNext(ite, pResultItem) == 0 ){
            printf( "%s:%s\n" ,pResultItem->key,pResultItem->value);
        }
        destroy_response_iterator(ite);
        destroy_result_item(pResultItem);
    }
    else
    {
        printf("msg = %s\nsubcode = %s\nsubmsg=%s", pResponse->msg, pResponse->subCode, pResponse->subMsg);
    }
    destroy_top_request(pRequest);
    destroy_top_response(pResponse);
    destroy_taobao_client(pClient);

}
