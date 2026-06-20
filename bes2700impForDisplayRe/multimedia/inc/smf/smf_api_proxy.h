#pragma once
#ifndef __SMF_API_PROXY_H__
#define __SMF_API_PROXY_H__
#include "smf_api.h"

#if defined(WIN32)
#define __weak 
#else
#define __weak __attribute__((weak)) 
#endif
#define DefineProxySource(x) EXTERNC __weak void smf_remote_##x##_source_register() {smf_reregister("src-"#x ,"src-proxy" );}
#define DefineProxyDemuxer(x) EXTERNC __weak void smf_remote_##x##_demuxer_register() {smf_reregister("dem-"#x ,"src-proxy" );}

#define DefineProxySink(x) EXTERNC __weak void smf_remote_##x##_sink_register() {smf_reregister("sink-"#x ,"sink-proxy" );}
#define DefineProxyMuxer(x) EXTERNC __weak void smf_remote_##x##_muxer_register() {smf_reregister("mux-"#x ,"sink-proxy" );}

#define DefineProxyFilter(x) EXTERNC __weak void smf_remote_##x##_filter_register() {smf_reregister("filter-"#x ,"filter-proxy" );}
#define DefineProxyEncoder(x) EXTERNC __weak void smf_remote_##x##_encoder_register() {smf_reregister("enc-" #x ,"filter-proxy" );}
#define DefineProxyDecoder(x) EXTERNC __weak void smf_remote_##x##_decoder_register() {smf_reregister("dec-" #x ,"filter-proxy" );}


//local loop test
#define DefineProxySource_(x) EXTERNC __weak void smf_proxy_##x##_source_register() {smf_reregister("src-" #x "-proxy" ,"src-proxy" );}
#define DefineProxyDemuxer_(x) EXTERNC __weak void smf_proxy_##x##_demuxer_register() {smf_reregister("dem-"#x "-proxy" ,"src-proxy" );}

#define DefineProxySink_(x) EXTERNC __weak void smf_proxy_##x##_sink_register() {smf_reregister("sink-" #x "-proxy" ,"sink-proxy" );}
#define DefineProxyMuxer_(x) EXTERNC __weak void smf_proxy_##x##_muxer_register() {smf_reregister("mux-" #x "-proxy" ,"sink-proxy" );}

#define DefineProxyFilter_(x) EXTERNC __weak void smf_proxy_##x##_filter_register() {smf_reregister("filter-"#x "-proxy" ,"filter-proxy" );}
#define DefineProxyEncoder_(x) EXTERNC __weak void smf_proxy_##x##_encoder_register() {smf_reregister("enc-" #x "-proxy" ,"filter-proxy" );}
#define DefineProxyDecoder_(x) EXTERNC __weak void smf_proxy_##x##_decoder_register() {smf_reregister("dec-" #x "-proxy" ,"filter-proxy" );}

#endif

