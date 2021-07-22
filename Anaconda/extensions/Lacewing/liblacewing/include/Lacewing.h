
/* vim: set et ts=4 sw=4 ft=cpp:
 *
 * Copyright (C) 2011 James McLaughlin.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef LacewingIncluded
#define LacewingIncluded

#ifdef Yield
    #undef Yield
#endif

#include <stdarg.h>

#ifndef _MSC_VER

    #include <inttypes.h>
    
    #define lw_i64   int64_t
    #define lw_iptr  intptr_t
    #define lw_i32   int32_t
    #define lw_i16   int16_t
    #define lw_i8    int8_t

    
#else

    #ifdef _WIN64
        #define lw_iptr __int64
    #else
        #define lw_iptr __int32
    #endif

    #define lw_i64  __int64
    #define lw_i32  __int32
    #define lw_i16  __int16
    #define lw_i8   __int8
    
#endif

#ifndef _WIN32
    #ifndef LacewingHandler
        #define LacewingHandler
    #endif

    #ifndef LacewingFunction
        #define LacewingFunction
    #endif
#else
    #define LacewingHandler __cdecl
#endif

#ifndef LacewingFunction

    #define LacewingFunction __declspec(dllimport)

    #ifndef Lacewing_NoAutoLink
        #pragma comment(lib, "Lacewing.lib")
    #endif

#endif

typedef lw_i8 lw_bool;

const lw_bool lw_true  = (lw_bool) 1;
const lw_bool lw_false = (lw_bool) 0;

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#define LacewingFlat(c) \
    typedef struct { void * reserved; void * tag; } c;

LacewingFunction    const char* lw_version                  ();
LacewingFunction        lw_i64  lw_current_thread_id        ();
LacewingFunction        lw_i64  lw_file_last_modified       (const char * filename);
LacewingFunction       lw_bool  lw_file_exists              (const char * filename);
LacewingFunction        lw_i64  lw_file_size                (const char * filename);
LacewingFunction       lw_bool  lw_path_exists              (const char * filename);
LacewingFunction          void  lw_int64_to_string          (lw_i64, char *);
LacewingFunction          void  lw_temp_path                (char * buffer);
LacewingFunction          void  lw_new_temp_file            (char * buffer);
LacewingFunction    const char* lw_guess_mime_type          (const char * filename);
LacewingFunction          void  lw_md5                      (char * output, const char * input, long length);
LacewingFunction          void  lw_md5_hex                  (char * output, const char * input, long length);
LacewingFunction          void  lw_sha1                     (char * output, const char * input, long length);
LacewingFunction          void  lw_sha1_hex                 (char * output, const char * input, long length);

/* Thread */

  LacewingFlat (lw_thread);

  LacewingFunction      lw_thread* lw_thread_new      (const char * name, void * function);
  LacewingFunction           void  lw_thread_delete   (lw_thread *);
  LacewingFunction           void  lw_thread_start    (lw_thread *, void * parameter);
  LacewingFunction        lw_bool  lw_thread_started  (lw_thread *);
  LacewingFunction           long  lw_thread_join     (lw_thread *);

/* Address */

  LacewingFlat (lw_addr);

  LacewingFunction        lw_addr* lw_addr_new       ();
  LacewingFunction        lw_addr* lw_addr_new_ip    (long ip, long port);
  LacewingFunction        lw_addr* lw_addr_new_name  (const char * hostname, long port, lw_bool blocking);
  LacewingFunction        lw_addr* lw_addr_copy      (lw_addr *);
  LacewingFunction           void  lw_addr_delete    (lw_addr *);
  LacewingFunction           long  lw_addr_port      (lw_addr *);
  LacewingFunction           void  lw_addr_set_port  (lw_addr *, long port);
  LacewingFunction        lw_bool  lw_addr_is_ready  (lw_addr *);
  LacewingFunction           long  lw_addr_ip        (lw_addr *);
  LacewingFunction  unsigned char  lw_addr_ip_byte   (lw_addr *, long index);
  LacewingFunction     const char* lw_addr_tostring  (lw_addr *);

/* EventPump */

  LacewingFlat (lw_eventpump);

  LacewingFunction   lw_eventpump* lw_eventpump_new                  ();
  LacewingFunction           void  lw_eventpump_delete               (lw_eventpump *);
  LacewingFunction           void  lw_eventpump_tick                 (lw_eventpump *);
  LacewingFunction           void  lw_eventpump_start_event_loop     (lw_eventpump *);
  LacewingFunction           void  lw_eventpump_start_sleepy_ticking (lw_eventpump *, void (LacewingHandler * on_tick_needed) (lw_eventpump *));
  LacewingFunction           void  lw_eventpump_post_eventloop_exit  (lw_eventpump *);
  LacewingFunction        lw_bool  lw_eventpump_in_use               (lw_eventpump *);
  LacewingFunction           void  lw_eventpump_set_in_use           (lw_eventpump *, lw_bool);

/* Timer */

  LacewingFlat (lw_timer);
  
  LacewingFunction       lw_timer* lw_timer_new                  ();
  LacewingFunction           void  lw_timer_delete               (lw_timer *);
  LacewingFunction           void  lw_timer_start                (lw_timer *, long milliseconds);
  LacewingFunction        lw_bool  lw_timer_started              (lw_timer *);
  LacewingFunction           void  lw_timer_stop                 (lw_timer *);
  LacewingFunction           void  lw_timer_force_tick           (lw_timer *);

  typedef void (LacewingHandler * lw_timer_handler_tick) (lw_timer *);
  LacewingFunction void lw_timer_ontick (lw_timer *, lw_timer_handler_tick);

/* Sync */

  LacewingFlat (lw_sync);
  LacewingFlat (lw_sync_lock);
  
  LacewingFunction        lw_sync* lw_sync_new                  ();
  LacewingFunction           void  lw_sync_delete               (lw_sync *);
  LacewingFunction   lw_sync_lock* lw_sync_lock_new             (lw_sync *);
  LacewingFunction           void  lw_sync_lock_delete          (lw_sync_lock *);
  LacewingFunction           void  lw_sync_lock_release         (lw_sync_lock *);

/* SpinSync */

  LacewingFlat (lw_ssync);
  LacewingFlat (lw_ssync_wlock);
  LacewingFlat (lw_ssync_rlock);
  
  LacewingFunction       lw_ssync* lw_ssync_new                 ();
  LacewingFunction           void  lw_ssync_delete              (lw_ssync *);
  LacewingFunction lw_ssync_wlock* lw_ssync_wlock_new           (lw_ssync *);
  LacewingFunction           void  lw_ssync_wlock_delete        (lw_ssync_wlock *);
  LacewingFunction           void  lw_ssync_wlock_release       (lw_ssync_wlock *);
  LacewingFunction lw_ssync_rlock* lw_ssync_rlock_new           (lw_ssync *);
  LacewingFunction           void  lw_ssync_rlock_delete        (lw_ssync_rlock *);
  LacewingFunction           void  lw_ssync_rlock_release       (lw_ssync_rlock *);

/* Event */

  LacewingFlat (lw_event);

  LacewingFunction       lw_event* lw_event_new                 ();
  LacewingFunction           void  lw_event_delete              (lw_event *);
  LacewingFunction           void  lw_event_signal              (lw_event *);
  LacewingFunction           void  lw_event_unsignal            (lw_event *);
  LacewingFunction        lw_bool  lw_event_signalled           (lw_event *);
  LacewingFunction           void  lw_event_wait                (lw_event *, long milliseconds);

/* Filter */

  LacewingFlat (lw_filter);

  LacewingFunction      lw_filter* lw_filter_new                ();
  LacewingFunction           void  lw_filter_delete             (lw_filter *);
  LacewingFunction           void  lw_filter_set_local_ip       (lw_filter *, long ip);
  LacewingFunction           long  lw_filter_get_local_ip       (lw_filter *);
  LacewingFunction           void  lw_filter_set_local          (lw_filter *, const char * name);
  LacewingFunction           void  lw_filter_set_remote_addr    (lw_filter *, lw_addr *);
  LacewingFunction        lw_addr* lw_filter_get_remote_addr    (lw_filter *);
  LacewingFunction           void  lw_filter_set_remote         (lw_filter *, const char * name);
  LacewingFunction           void  lw_filter_set_local_port     (lw_filter *, long port);
  LacewingFunction           long  lw_filter_get_local_port     (lw_filter *);
  LacewingFunction           void  lw_filter_set_reuse          (lw_filter *);
  LacewingFunction        lw_bool  lw_filter_is_reuse_set       (lw_filter *);

/* Error */

  LacewingFlat (lw_error);

  LacewingFunction       lw_error* lw_error_new                 ();
  LacewingFunction           void  lw_error_delete              (lw_error *);
  LacewingFunction           void  lw_error_add                 (lw_error *, long);
  LacewingFunction           void  lw_error_addf                (lw_error *, const char * format, ...);
  LacewingFunction     const char* lw_error_tostring            (lw_error *);
  LacewingFunction       lw_error* lw_error_clone               (lw_error *);

/* Client */

  LacewingFlat (lw_client);

  LacewingFunction      lw_client* lw_client_new                (lw_eventpump *);
  LacewingFunction           void  lw_client_delete             (lw_client *);
  LacewingFunction           void  lw_client_connect            (lw_client *, const char * host, long port);
  LacewingFunction           void  lw_client_connect_addr       (lw_client *, lw_addr *);
  LacewingFunction           void  lw_client_disconnect         (lw_client *);
  LacewingFunction        lw_bool  lw_client_connected          (lw_client *);
  LacewingFunction        lw_bool  lw_client_connecting         (lw_client *);
  LacewingFunction        lw_addr* lw_client_server_addr        (lw_client *);
  LacewingFunction           void  lw_client_send               (lw_client *, const char * data, long size);
  LacewingFunction           void  lw_client_send_text          (lw_client *, const char * text);
  LacewingFunction           void  lw_client_sendf              (lw_client *, const char * format, ...);
  LacewingFunction           void  lw_client_disable_nagling    (lw_client *);
  LacewingFunction        lw_bool  lw_client_cheap_buffering    (lw_client *);
  LacewingFunction           void  lw_client_start_buffering    (lw_client *);
  LacewingFunction           void  lw_client_flush              (lw_client *);
  
  typedef void (LacewingHandler * lw_client_handler_connect) (lw_client *);
  LacewingFunction void lw_client_onconnect (lw_client *, lw_client_handler_connect);

  typedef void (LacewingHandler * lw_client_handler_disconnect) (lw_client *);
  LacewingFunction void lw_client_ondisconnect (lw_client *, lw_client_handler_disconnect);

  typedef void (LacewingHandler * lw_client_handler_receive) (lw_client *, char * data, long size);
  LacewingFunction void lw_client_onreceive (lw_client *, lw_client_handler_receive);

  typedef void (LacewingHandler * lw_client_handler_error) (lw_client *, lw_error *);
  LacewingFunction void lw_client_onerror (lw_client *, lw_client_handler_error);

/* Server */

  LacewingFlat (lw_server);
  LacewingFlat (lw_server_client);

  LacewingFunction        lw_server* lw_server_new                      (lw_eventpump *);
  LacewingFunction             void  lw_server_delete                   (lw_server *);
  LacewingFunction             void  lw_server_host                     (lw_server *, long port);
  LacewingFunction             void  lw_server_host_ex                  (lw_server *, long port, lw_bool client_speaks_first);
  LacewingFunction             void  lw_server_host_filter              (lw_server *, lw_filter * filter);
  LacewingFunction             void  lw_server_host_filter_ex           (lw_server *, lw_filter * filter, lw_bool client_speaks_first);
  LacewingFunction             void  lw_server_unhost                   (lw_server *);
  LacewingFunction          lw_bool  lw_server_hosting                  (lw_server *);
  LacewingFunction             long  lw_server_port                     (lw_server *);
  LacewingFunction          lw_bool  lw_server_load_cert_file           (lw_server *, const char * filename, const char * passphrase);
  LacewingFunction          lw_bool  lw_server_load_sys_cert            (lw_server *, const char * store_name, const char * common_name, const char * location);
  LacewingFunction          lw_bool  lw_server_cert_loaded              (lw_server *);
  LacewingFunction           lw_i64  lw_server_bytes_sent               (lw_server *);
  LacewingFunction           lw_i64  lw_server_bytes_received           (lw_server *);
  LacewingFunction             void  lw_server_disable_nagling          (lw_server *);
  LacewingFunction          lw_addr* lw_server_client_address           (lw_server_client *);
  LacewingFunction             void  lw_server_client_send              (lw_server_client *, const char * data, long size);
  LacewingFunction             void  lw_server_client_send_writable     (lw_server_client *, char * data, long size);
  LacewingFunction             void  lw_server_client_send_text         (lw_server_client *, const char * text);
  LacewingFunction             void  lw_server_client_sendf             (lw_server_client *, const char * format, ...);
  LacewingFunction          lw_bool  lw_server_client_cheap_buffering   (lw_server_client *);
  LacewingFunction             void  lw_server_client_start_buffering   (lw_server_client *);
  LacewingFunction             void  lw_server_client_flush             (lw_server_client *);
  LacewingFunction             void  lw_server_client_disconnect        (lw_server_client *);
  LacewingFunction lw_server_client* lw_server_client_next              (lw_server_client *);

  typedef void (LacewingHandler * lw_server_handler_connect) (lw_server *, lw_server_client *);
  LacewingFunction void lw_server_onconnect (lw_server *, lw_server_handler_connect);

  typedef void (LacewingHandler * lw_server_handler_disconnect) (lw_server *, lw_server_client *);
  LacewingFunction void lw_server_ondisconnect (lw_server *, lw_server_handler_disconnect);

  typedef void (LacewingHandler * lw_server_handler_receive) (lw_server *, lw_server_client *, char * data, long size);
  LacewingFunction void lw_server_onreceive (lw_server *, lw_server_handler_receive);
  
  typedef void (LacewingHandler * lw_server_handler_error) (lw_server *, lw_error *);
  LacewingFunction void lw_server_onerror (lw_server *, lw_server_handler_error);

/* UDP */

  LacewingFlat (lw_udp);

  LacewingFunction         lw_udp* lw_udp_new                   (lw_eventpump *);
  LacewingFunction           void  lw_udp_delete                (lw_udp *);
  LacewingFunction           void  lw_udp_host                  (lw_udp *, long port);
  LacewingFunction           void  lw_udp_host_filter           (lw_udp *, lw_filter *);
  LacewingFunction           void  lw_udp_host_addr             (lw_udp *, lw_addr *);
  LacewingFunction        lw_bool  lw_udp_hosting               (lw_udp *);
  LacewingFunction           void  lw_udp_unhost                (lw_udp *);
  LacewingFunction           long  lw_udp_port                  (lw_udp *);
  LacewingFunction         lw_i64  lw_udp_bytes_sent            (lw_udp *);
  LacewingFunction         lw_i64  lw_udp_bytes_received        (lw_udp *);
  LacewingFunction           void  lw_udp_send                  (lw_udp *, lw_addr *, const char * data, long size);

  typedef void (LacewingHandler * lw_udp_handler_receive) (lw_udp *, lw_addr *, char * data, long size);
  LacewingFunction void lw_udp_onreceive (lw_udp *, lw_udp_handler_receive);

  typedef void (LacewingHandler * lw_udp_handler_error) (lw_udp *, lw_error *);
  LacewingFunction void lw_udp_onerror (lw_udp *, lw_udp_handler_error);

/* Webserver */

  LacewingFlat (lw_ws);
  LacewingFlat (lw_ws_req);
  LacewingFlat (lw_ws_req_hdr);
  LacewingFlat (lw_ws_req_param);
  LacewingFlat (lw_ws_req_cookie);
  LacewingFlat (lw_ws_req_ssnitem);
  LacewingFlat (lw_ws_upload);
  LacewingFlat (lw_ws_upload_hdr);

  LacewingFunction              lw_ws* lw_ws_new                    (lw_eventpump *);
  LacewingFunction               void  lw_ws_delete                 (lw_ws *);
  LacewingFunction               void  lw_ws_host                   (lw_ws *, long port);
  LacewingFunction               void  lw_ws_host_secure            (lw_ws *, long port);
  LacewingFunction               void  lw_ws_host_filter            (lw_ws *, lw_filter *);
  LacewingFunction               void  lw_ws_host_secure_filter     (lw_ws *, lw_filter *);
  LacewingFunction               void  lw_ws_unhost                 (lw_ws *);
  LacewingFunction               void  lw_ws_unhost_secure          (lw_ws *);
  LacewingFunction            lw_bool  lw_ws_hosting                (lw_ws *);
  LacewingFunction            lw_bool  lw_ws_hosting_secure         (lw_ws *);
  LacewingFunction               long  lw_ws_port                   (lw_ws *);
  LacewingFunction               long  lw_ws_port_secure            (lw_ws *);
  LacewingFunction            lw_bool  lw_ws_load_cert_file         (lw_ws *, const char * filename, const char * passphrase);
  LacewingFunction            lw_bool  lw_ws_load_sys_cert          (lw_ws *, const char * store_name, const char * common_name, const char * location);
  LacewingFunction            lw_bool  lw_ws_cert_loaded            (lw_ws *);
  LacewingFunction             lw_i64  lw_ws_bytes_sent             (lw_ws *);
  LacewingFunction             lw_i64  lw_ws_bytes_received         (lw_ws *);
  LacewingFunction               void  lw_ws_close_session          (lw_ws *, const char * id);
  LacewingFunction               void  lw_ws_enable_manual_finish   (lw_ws *);
  LacewingFunction               long  lw_ws_idle_timeout           (lw_ws *);
  LacewingFunction               void  lw_ws_set_idle_timeout       (lw_ws *, long seconds);  
  LacewingFunction            lw_addr* lw_ws_req_addr               (lw_ws_req *);
  LacewingFunction            lw_bool  lw_ws_req_secure             (lw_ws_req *);
  LacewingFunction         const char* lw_ws_req_url                (lw_ws_req *);
  LacewingFunction         const char* lw_ws_req_hostname           (lw_ws_req *);
  LacewingFunction               void  lw_ws_req_disconnect         (lw_ws_req *); 
  LacewingFunction               void  lw_ws_req_set_redirect       (lw_ws_req *, const char * url);
  LacewingFunction               void  lw_ws_req_set_status         (lw_ws_req *, long code, const char * message);
  LacewingFunction               void  lw_ws_req_set_mime_type      (lw_ws_req *, const char * mime_type);
  LacewingFunction               void  lw_ws_req_set_mime_type_ex   (lw_ws_req *, const char * mime_type, const char * charset);
  LacewingFunction               void  lw_ws_req_guess_mime_type    (lw_ws_req *, const char * filename);
  LacewingFunction               void  lw_ws_req_send_text          (lw_ws_req *, const char * data);
  LacewingFunction               void  lw_ws_req_send_text_const    (lw_ws_req *, const char * data);
  LacewingFunction               void  lw_ws_req_send               (lw_ws_req *, const char * data, long size);
  LacewingFunction               void  lw_ws_req_send_const         (lw_ws_req *, const char * data, long size);
  LacewingFunction               void  lw_ws_req_sendfile           (lw_ws_req *, const char * filename);
  LacewingFunction               void  lw_ws_req_sendfile_ex        (lw_ws_req *, const char * filename, lw_i64 offset, lw_i64 size);
  LacewingFunction               void  lw_ws_req_sendf              (lw_ws_req *, const char * format, ...);
  LacewingFunction               void  lw_ws_req_reset              (lw_ws_req *);
  LacewingFunction               void  lw_ws_req_finish             (lw_ws_req *);
  LacewingFunction             lw_i64  lw_ws_req_last_modified      (lw_ws_req *);
  LacewingFunction               void  lw_ws_req_set_last_modified  (lw_ws_req *, lw_i64);
  LacewingFunction               void  lw_ws_req_set_unmodified     (lw_ws_req *);
  LacewingFunction               void  lw_ws_req_set_header         (lw_ws_req *, const char * name, const char * value);
  LacewingFunction         const char* lw_ws_req_header             (lw_ws_req *, const char * name);
  LacewingFunction      lw_ws_req_hdr* lw_ws_req_first_header       (lw_ws_req *);
  LacewingFunction         const char* lw_ws_req_hdr_name           (lw_ws_req_hdr *);
  LacewingFunction         const char* lw_ws_req_hdr_value          (lw_ws_req_hdr *);
  LacewingFunction      lw_ws_req_hdr* lw_ws_req_hdr_next           (lw_ws_req_hdr *);
  LacewingFunction    lw_ws_req_param* lw_ws_req_first_GET          (lw_ws_req *);
  LacewingFunction    lw_ws_req_param* lw_ws_req_first_POST         (lw_ws_req *);
  LacewingFunction         const char* lw_ws_req_param_name         (lw_ws_req_param *);
  LacewingFunction         const char* lw_ws_req_param_value        (lw_ws_req_param *);
  LacewingFunction    lw_ws_req_param* lw_ws_req_param_next         (lw_ws_req_param *);
  LacewingFunction   lw_ws_req_cookie* lw_ws_req_first_cookie       (lw_ws_req *);
  LacewingFunction         const char* lw_ws_req_cookie_name        (lw_ws_req_cookie *);
  LacewingFunction         const char* lw_ws_req_cookie_value       (lw_ws_req_cookie *);
  LacewingFunction   lw_ws_req_cookie* lw_ws_req_cookie_next        (lw_ws_req_cookie *);
  LacewingFunction               void  lw_ws_req_set_cookie         (lw_ws_req *, const char * name, const char * value);
  LacewingFunction               void  lw_ws_req_set_cookie_ex      (lw_ws_req *, const char * name, const char * value, const char * attributes);
  LacewingFunction         const char* lw_ws_req_get_cookie         (lw_ws_req *, const char * name);
  LacewingFunction         const char* lw_ws_req_session_id         (lw_ws_req *);
  LacewingFunction               void  lw_ws_req_session_write      (lw_ws_req *, const char * name, const char * value);
  LacewingFunction         const char* lw_ws_req_session_read       (lw_ws_req *, const char * name);
  LacewingFunction               void  lw_ws_req_session_close      (lw_ws_req *);
  LacewingFunction  lw_ws_req_ssnitem* lw_ws_req_first_session_item (lw_ws_req *);
  LacewingFunction         const char* lw_ws_req_ssnitem_name       (lw_ws_req_ssnitem *);
  LacewingFunction         const char* lw_ws_req_ssnitem_value      (lw_ws_req_ssnitem *);
  LacewingFunction  lw_ws_req_ssnitem* lw_ws_req_ssnitem_next       (lw_ws_req_ssnitem *);
  LacewingFunction         const char* lw_ws_req_GET                (lw_ws_req *, const char * name);
  LacewingFunction         const char* lw_ws_req_POST               (lw_ws_req *, const char * name);
  LacewingFunction               void  lw_ws_req_disable_cache      (lw_ws_req *);
  LacewingFunction               long  lw_ws_req_idle_timeout       (lw_ws_req *);
  LacewingFunction               void  lw_ws_req_set_idle_timeout   (lw_ws_req *, long seconds);  
/*LacewingFunction               void  lw_ws_req_enable_dl_resuming (lw_ws_req *);
  LacewingFunction             lw_i64  lw_ws_req_reqrange_begin     (lw_ws_req *);
  LacewingFunction             lw_i64  lw_ws_req_reqrange_end       (lw_ws_req *);
  LacewingFunction               void  lw_ws_req_set_outgoing_range (lw_ws_req *, lw_i64 begin, lw_i64 end);*/
  LacewingFunction         const char* lw_ws_upload_form_el_name    (lw_ws_upload *);
  LacewingFunction         const char* lw_ws_upload_filename        (lw_ws_upload *);
  LacewingFunction         const char* lw_ws_upload_header          (lw_ws_upload *, const char * name);
  LacewingFunction               void  lw_ws_upload_set_autosave    (lw_ws_upload *);
  LacewingFunction         const char* lw_ws_upload_autosave_fname  (lw_ws_upload *);
  LacewingFunction   lw_ws_upload_hdr* lw_ws_upload_first_header    (lw_ws_upload *);
  LacewingFunction         const char* lw_ws_upload_hdr_name        (lw_ws_upload_hdr *);
  LacewingFunction         const char* lw_ws_upload_hdr_value       (lw_ws_upload_hdr *);
  LacewingFunction   lw_ws_upload_hdr* lw_ws_upload_hdr_next        (lw_ws_upload_hdr *);

  typedef void (LacewingHandler * lw_ws_handler_get) (lw_ws *, lw_ws_req *);
  LacewingFunction void lw_ws_onget (lw_ws *, lw_ws_handler_get);

  typedef void (LacewingHandler * lw_ws_handler_post) (lw_ws *, lw_ws_req *);
  LacewingFunction void lw_ws_onpost (lw_ws *, lw_ws_handler_post);

  typedef void (LacewingHandler * lw_ws_handler_head) (lw_ws *, lw_ws_req *);
  LacewingFunction void lw_ws_onhead (lw_ws *, lw_ws_handler_head);

  typedef void (LacewingHandler * lw_ws_handler_error) (lw_ws *, lw_error *);
  LacewingFunction void lw_ws_onerror (lw_ws *, lw_ws_handler_error);

  typedef void (LacewingHandler * lw_ws_handler_connect) (lw_ws *, lw_ws_req *);
  LacewingFunction void lw_ws_onconnect (lw_ws *, lw_ws_handler_connect);

  typedef void (LacewingHandler * lw_ws_handler_disconnect) (lw_ws *, lw_ws_req *);
  LacewingFunction void lw_ws_ondisconnect (lw_ws *, lw_ws_handler_disconnect);

  typedef void (LacewingHandler * lw_ws_handler_upload_start) (lw_ws *, lw_ws_req *, lw_ws_upload *);
  LacewingFunction void lw_ws_onuploadstart (lw_ws *, lw_ws_handler_upload_start);

  typedef void (LacewingHandler * lw_ws_handler_upload_chunk) (lw_ws *, lw_ws_req *, lw_ws_upload *, const char * data, long size);
  LacewingFunction void lw_ws_onuploadchunk (lw_ws *, lw_ws_handler_upload_chunk);

  typedef void (LacewingHandler * lw_ws_handler_upload_done) (lw_ws *, lw_ws_req *, lw_ws_upload *);
  LacewingFunction void lw_ws_onuploaddone (lw_ws *, lw_ws_handler_upload_done);

  typedef void (LacewingHandler * lw_ws_handler_upload_post) (lw_ws *, lw_ws_req *, lw_ws_upload * uploads[], long upload_count);
  LacewingFunction void lw_ws_onuploadpost (lw_ws *, lw_ws_handler_upload_post);


#ifdef __cplusplus
}

/* TODO : Other types? */

#define LacewingStream(C, F)                                \
        inline C & operator << (lw_i64 V)                   \
        {   char Buffer[64];                                \
            Int64ToString(V, Buffer);                       \
            (*this) << Buffer;                              \
            return *this;                                   \
        }                                                   \
        inline C & operator << (const char * V)             \
        {   F (V, -1);                                      \
            return *this;                                   \
        }                                                   \
        inline C & operator << (char * V)                   \
        {   F (V, -1);                                      \
            return *this;                                   \
        }                                                   \
 
namespace Lacewing
{

LacewingFunction   const char* Version                 ();
LacewingFunction       lw_i64  CurrentThreadID         ();
LacewingFunction       lw_i64  LastModified            (const char * Filename);
LacewingFunction         bool  FileExists              (const char * Filename);
LacewingFunction       lw_i64  FileSize                (const char * Filename);
LacewingFunction         bool  PathExists              (const char * Filename);
LacewingFunction         void  Int64ToString           (lw_i64, char *);
LacewingFunction         void  TempPath                (char * Buffer);
LacewingFunction         void  NewTempFile             (char * Buffer);
LacewingFunction   const char* GuessMimeType           (const char * Filename);
LacewingFunction         void  MD5                     (char * Output, const char * Input, int Length = -1);
LacewingFunction         void  MD5_Hex                 (char * Output, const char * Input, int Length = -1);
LacewingFunction         void  SHA1                    (char * Output, const char * Input, int Length = -1);
LacewingFunction         void  SHA1_Hex                (char * Output, const char * Input, int Length = -1);

struct Error
{
    void * InternalTag, * Tag;

    LacewingFunction  Error();
    LacewingFunction ~Error();
    
    LacewingFunction void Add (const char * Format, ...);
    LacewingFunction void Add (int);
    LacewingFunction void Add (const char * Format, va_list);

    LacewingFunction const char * ToString ();
    LacewingFunction operator const char * ();

    LacewingFunction Lacewing::Error * Clone();
};

/* On Windows, there's only Lacewing::EventPump, of which Lacewing::Pump is a typedef.

   On *nix, Lacewing::EventPump derives from Lacewing::Pump, which is a virtual class for
   watching FDs.  It is therefore possible to bypass Lacewing::EventPump entirely and use a custom
   class deriving from Lacewing::Pump, allowing Lacewing to be used in situations where
   EventPump is not practical (ie. when some form of event watcher already exists). */

#ifndef _WIN32

    struct Pump
    {
        void * InternalTag, * Tag;

        LacewingFunction  Pump ();
        LacewingFunction ~Pump ();
    
        LacewingFunction void Post (void * Function, void * Parameter);
        LacewingFunction virtual bool IsEventPump ();
        LacewingFunction void PostEventLoopExit ();
        LacewingFunction bool InUse ();
        LacewingFunction void InUse (bool);

        #ifdef LacewingInternal
            friend struct ::PumpInternal;
        #endif

    protected:

        LacewingFunction bool Ready (void * Tag, bool CanRead, bool CanWrite);

        virtual void * AddRead (int FD, void * Tag) = 0;
        virtual void * AddReadWrite (int FD, void * Tag) = 0;

        /* AddRead/AddReadWrite may return a pointer-sized key, which will be passed
           to Gone() when the FD is dead.  EventPump doesn't use this mechanism, but
           it may be useful for a custom Pump. */

        LacewingFunction virtual void Gone (void *);
    };

    struct EventPump : public Pump
    {
        void * EPInternalTag, * EPTag;

        LacewingFunction  EventPump (int MaxHint = 1024);
        LacewingFunction ~EventPump ();

        LacewingFunction Lacewing::Error * Tick();
        LacewingFunction Lacewing::Error * StartEventLoop();
        LacewingFunction Lacewing::Error * StartSleepyTicking(void (LacewingHandler * onTickNeeded) (Lacewing::EventPump &EventPump));
    
        LacewingFunction bool IsEventPump ();

    private:

        LacewingFunction void * AddRead (int FD, void * Tag);
        LacewingFunction void * AddReadWrite (int FD, void * Tag);
    };

#else

    struct EventPump
    {
        void * InternalTag, * Tag;

        LacewingFunction  EventPump (int MaxHint = 1024);
        LacewingFunction ~EventPump ();

        LacewingFunction Lacewing::Error * Tick();
        LacewingFunction Lacewing::Error * StartEventLoop();
        LacewingFunction Lacewing::Error * StartSleepyTicking(void (LacewingHandler * onTickNeeded) (Lacewing::EventPump &EventPump));
    
        LacewingFunction void Post (void * Function, void * Parameter);
        LacewingFunction void PostEventLoopExit ();
        LacewingFunction bool InUse ();
        LacewingFunction void InUse (bool);
    };

    typedef EventPump Pump;

#endif

struct Thread
{
    void * InternalTag, * Tag;

    LacewingFunction   Thread (const char * Name, void * Function);
    LacewingFunction ~ Thread ();

    LacewingFunction void Start (void * Parameter = 0);
    LacewingFunction bool Started ();

    LacewingFunction int Join ();
};

struct Timer
{
    void * InternalTag, * Tag;

    LacewingFunction  Timer (Pump &);
    LacewingFunction ~Timer ();

    LacewingFunction void Start    (int Milliseconds);
    LacewingFunction void Stop     ();
    LacewingFunction bool Started  ();
    
    LacewingFunction void ForceTick ();
    
    typedef void (LacewingHandler * HandlerTick) (Lacewing::Timer &Timer);
    LacewingFunction void onTick (HandlerTick);
};

struct Sync
{
    void * InternalTag, * Tag;

    LacewingFunction  Sync ();
    LacewingFunction ~Sync ();

    struct Lock
    {
        void * InternalTag;

        LacewingFunction  Lock(Lacewing::Sync &);
        LacewingFunction ~Lock();
        
        LacewingFunction void Release();
    };
};

struct SpinSync
{
    void * InternalTag, * Tag;

    LacewingFunction  SpinSync ();
    LacewingFunction ~SpinSync ();

    struct WriteLock
    {
        void * InternalTag;

        LacewingFunction  WriteLock(Lacewing::SpinSync &);
        LacewingFunction ~WriteLock();
        
        LacewingFunction void Release();
    };

    struct ReadLock
    {
        void * InternalTag;

        LacewingFunction  ReadLock(Lacewing::SpinSync &);
        LacewingFunction ~ReadLock();

        LacewingFunction void Release();
    };
};

struct Event
{
    void * InternalTag, * Tag;

    LacewingFunction Event();
    LacewingFunction ~Event();

    LacewingFunction void Signal();
    LacewingFunction void Unsignal();
    
    LacewingFunction bool Signalled();

    LacewingFunction void Wait(int Timeout = -1);
};

struct Address
{
    void * InternalTag, * Tag;

    LacewingFunction Address(const Address &);
    LacewingFunction ~Address();

    LacewingFunction Address();
    LacewingFunction Address(unsigned int IP, int Port = 0);
    LacewingFunction Address(unsigned char, unsigned char, unsigned char, unsigned char, int Port = 0);

    /* Hostname accepts formats:

          lacewing-project.org
          127.0.0.1
          lacewing-project.org:6121
          127.0.0.1:6121
          http://lacewing-project.org
          http://127.0.0.1

       Port used (in order of precedence):
     
          The port after the ":"
          The port of the service (eg 80 for http://)
          The "Port" argument
    */

    LacewingFunction Address(const char * Hostname, int Port = 0, bool Blocking = false);
 
    LacewingFunction  int Port () const;  
    LacewingFunction void Port (int Port);

    LacewingFunction         bool   Ready   () const;
    LacewingFunction unsigned int   IP      () const;
    LacewingFunction unsigned char  IP_Byte (int Index) const;

    LacewingFunction const char * ToString  () const;
    LacewingFunction operator const char *  () const;
};

struct Filter
{
    void * InternalTag, * Tag;

    LacewingFunction Filter ();
    LacewingFunction Filter (const Filter &);

    LacewingFunction ~Filter ();
    
    LacewingFunction void LocalIP(int IP);
    LacewingFunction  int LocalIP() const;

    LacewingFunction void LocalPort(int Port);
    LacewingFunction  int LocalPort() const;

    LacewingFunction void Local (const char *);
    
    LacewingFunction void Remote (const char *);
    LacewingFunction void Remote (const Address &);
    LacewingFunction Address &Remote () const;

    LacewingFunction void Reuse(bool Enabled);
    LacewingFunction bool Reuse() const;
};

struct Client
{
    void * InternalTag, * Tag;

    LacewingFunction  Client(Pump &);
    LacewingFunction ~Client();

    LacewingFunction void Connect(const char * Host, int Port);
    LacewingFunction void Connect(Address &);

    LacewingFunction void Disconnect();

    LacewingFunction bool Connected();
    LacewingFunction bool Connecting();
    
    LacewingFunction Lacewing::Address &ServerAddress();

    LacewingFunction void Send (const char * Data, int Size = -1);
    LacewingStream             (Client, Send);

    LacewingFunction void DisableNagling();
    
    LacewingFunction bool CheapBuffering ();
    LacewingFunction void StartBuffering();
    LacewingFunction void Flush();    

    typedef void (LacewingHandler * HandlerConnect)         (Lacewing::Client &Client);
    typedef void (LacewingHandler * HandlerDisconnect)      (Lacewing::Client &Client);
    typedef void (LacewingHandler * HandlerReceive)         (Lacewing::Client &Client, char * Data, int Size);
    typedef void (LacewingHandler * HandlerError)           (Lacewing::Client &Client, Lacewing::Error &);

    LacewingFunction void onConnect    (HandlerConnect);
    LacewingFunction void onDisconnect (HandlerDisconnect);
    LacewingFunction void onReceive    (HandlerReceive);
    LacewingFunction void onError      (HandlerError);
};

struct Server
{
    void * InternalTag, * Tag;

    LacewingFunction  Server(Pump &);
    LacewingFunction ~Server();

    LacewingFunction void Host    (int Port, bool ClientSpeaksFirst = false);
    LacewingFunction void Host    (Lacewing::Filter &Filter, bool ClientSpeaksFirst = false);

    LacewingFunction void Unhost  ();
    LacewingFunction bool Hosting ();
    LacewingFunction int  Port    ();

    LacewingFunction bool LoadCertificateFile   (const char * Filename,  const char * Passphrase = "");
    LacewingFunction bool LoadSystemCertificate (const char * StoreName, const char * CommonName, const char * Location = "CurrentUser");
    LacewingFunction bool CertificateLoaded     ();

    LacewingFunction lw_i64 BytesSent();
    LacewingFunction lw_i64 BytesReceived();
    
    LacewingFunction void DisableNagling ();

    struct Client
    {
        void * InternalTag, * Tag;

        LacewingFunction Lacewing::Address &GetAddress();

        LacewingFunction void Send          (const char * Data, int Size = -1);
        LacewingFunction void SendFile      (const char * Filename, lw_i64 Offset = 0, lw_i64 Size = -1);
        LacewingFunction void SendWritable  (char * Data, int Size = -1);

        LacewingFunction bool CheapBuffering ();
        LacewingFunction void StartBuffering ();
        LacewingFunction void Flush ();

        LacewingStream (Client, Send);

        LacewingFunction void Disconnect();

        LacewingFunction Client * Next ();
    };

    LacewingFunction int ClientCount();
    LacewingFunction Client * FirstClient ();

    typedef void (LacewingHandler * HandlerConnect)     (Lacewing::Server &Server, Lacewing::Server::Client &Client);
    typedef void (LacewingHandler * HandlerDisconnect)  (Lacewing::Server &Server, Lacewing::Server::Client &Client);
    typedef void (LacewingHandler * HandlerReceive)     (Lacewing::Server &Server, Lacewing::Server::Client &Client, char * Data, int Size);
    typedef void (LacewingHandler * HandlerError)       (Lacewing::Server &Server, Lacewing::Error &);
    
    LacewingFunction void onConnect     (HandlerConnect);
    LacewingFunction void onDisconnect  (HandlerDisconnect);
    LacewingFunction void onReceive     (HandlerReceive);
    LacewingFunction void onError       (HandlerError);
};

struct UDP
{
    void * InternalTag, * Tag;

    LacewingFunction  UDP (Pump &);
    LacewingFunction ~UDP ();

    LacewingFunction void Host (int Port);
    LacewingFunction void Host (Lacewing::Filter &Filter);
    LacewingFunction void Host (Address &); /* Use Port() afterwards to get the port number */

    LacewingFunction bool Hosting ();
    LacewingFunction void Unhost ();

    LacewingFunction int Port ();

    LacewingFunction lw_i64 BytesSent();
    LacewingFunction lw_i64 BytesReceived();

    LacewingFunction void Send(Lacewing::Address &Address, const char * Data, int Size = -1);

    typedef void (LacewingHandler * HandlerReceive)          (Lacewing::UDP &UDP, Lacewing::Address &From, char * Data, int Size);
    typedef void (LacewingHandler * HandlerError)            (Lacewing::UDP &UDP, Lacewing::Error &);
    
    LacewingFunction void onReceive(HandlerReceive);
    LacewingFunction void onError  (HandlerError);
};

struct Webserver
{
    void * InternalTag, * Tag;

    LacewingFunction  Webserver(Pump &);
    LacewingFunction ~Webserver();

    LacewingFunction void Host         (int Port = 80);
    LacewingFunction void HostSecure   (int Port = 443);
    
    LacewingFunction void Host         (Lacewing::Filter &Filter);
    LacewingFunction void HostSecure   (Lacewing::Filter &Filter);
    
    LacewingFunction void Unhost        ();
    LacewingFunction void UnhostSecure  ();

    LacewingFunction bool Hosting       ();
    LacewingFunction bool HostingSecure ();

    LacewingFunction int  Port          ();
    LacewingFunction int  SecurePort    ();

    LacewingFunction bool LoadCertificateFile   (const char * Filename, const char * Passphrase = "");
    LacewingFunction bool LoadSystemCertificate (const char * StoreName, const char * CommonName, const char * Location = "CurrentUser");
    LacewingFunction bool CertificateLoaded     ();

    LacewingFunction lw_i64 BytesSent();
    LacewingFunction lw_i64 BytesReceived();

    LacewingFunction void EnableManualRequestFinish ();

    LacewingFunction int  IdleTimeout ();
    LacewingFunction void IdleTimeout (int Seconds);

    struct Request
    {
        void * InternalTag, * Tag;

        LacewingFunction Lacewing::Address &GetAddress();

        LacewingFunction bool Secure ();

        LacewingFunction const char * URL();
        LacewingFunction const char * Hostname();
        
        LacewingFunction void Disconnect();

        LacewingFunction void SetRedirect (const char * URL);
        LacewingFunction void Status (int Code, const char * Message);

        LacewingFunction void SetMimeType(const char * MimeType, const char * Charset = "UTF-8");
        LacewingFunction void GuessMimeType(const char * Filename);

        LacewingFunction void Send          (const char * Data, int Size = -1);
        LacewingFunction void SendFile      (const char * Filename, lw_i64 Offset = 0, lw_i64 Size = -1);
        LacewingFunction void SendConstant  (const char * Data, int Size = -1);

        LacewingStream(Request, Send);

        LacewingFunction void Reset ();
        LacewingFunction void Finish ();

        LacewingFunction int  IdleTimeout ();
        LacewingFunction void IdleTimeout (int Seconds);

        LacewingFunction lw_i64 LastModified  ();
        LacewingFunction void   LastModified  (lw_i64 Time);
        LacewingFunction void   SetUnmodified ();
    
        LacewingFunction void DisableCache();

        LacewingFunction void   EnableDownloadResuming();
        LacewingFunction lw_i64 RequestedRangeBegin();
        LacewingFunction lw_i64 RequestedRangeEnd();
        LacewingFunction void   SetOutgoingRange(lw_i64 Begin, lw_i64 End);
        

        /** Headers **/

        struct Header
        {
            LacewingFunction const char * Name ();
            LacewingFunction const char * Value ();

            LacewingFunction Header * Next ();
        };

        LacewingFunction struct Header * FirstHeader ();

        LacewingFunction const char * Header (const char * Name);
        LacewingFunction void Header (const char * Name, const char * Value);

    
        /** Cookies **/

        struct Cookie
        {
            LacewingFunction const char * Name ();
            LacewingFunction const char * Value ();

            LacewingFunction Cookie * Next ();
        };

        LacewingFunction struct Cookie * FirstCookie ();

        LacewingFunction const char * Cookie (const char * Name);
        LacewingFunction void         Cookie (const char * Name, const char * Value);
        LacewingFunction void         Cookie (const char * Name, const char * Value, const char * Attributes);

    
        /** Sessions **/

        struct SessionItem
        {
            LacewingFunction const char * Name ();
            LacewingFunction const char * Value ();

            LacewingFunction SessionItem * Next ();
        };

        LacewingFunction SessionItem * FirstSessionItem ();

        LacewingFunction const char * Session ();
        LacewingFunction const char * Session (const char * Key);
        LacewingFunction void         Session (const char * Key, const char * Value);

        LacewingFunction void  CloseSession();

            
        /** GET/POST data **/

        struct Parameter
        {
            LacewingFunction const char * Name ();
            LacewingFunction const char * Value ();

            LacewingFunction Parameter * Next ();
        };

        LacewingFunction Parameter * GET ();
        LacewingFunction Parameter * POST ();

        LacewingFunction const char * GET  (const char * Name);
        LacewingFunction const char * POST (const char * Name);
    };

    LacewingFunction void CloseSession(const char * ID);

    struct Upload
    {
        void * InternalTag, * Tag;

        LacewingFunction const char * FormElementName();
        LacewingFunction const char * Filename();
        LacewingFunction void         SetAutoSave();
        LacewingFunction const char * GetAutoSaveFilename();

        LacewingFunction const char * Header (const char * Name);
        
        struct Header
        {
            LacewingFunction const char * Name ();
            LacewingFunction const char * Value ();

            LacewingFunction Header * Next ();
        };

        LacewingFunction struct Header * FirstHeader ();
    };

    typedef void (LacewingHandler * HandlerGet)                    (Lacewing::Webserver &Webserver, Lacewing::Webserver::Request &Request);
    typedef void (LacewingHandler * HandlerPost)                   (Lacewing::Webserver &Webserver, Lacewing::Webserver::Request &Request);
    typedef void (LacewingHandler * HandlerHead)                   (Lacewing::Webserver &Webserver, Lacewing::Webserver::Request &Request);  
    typedef void (LacewingHandler * HandlerDisconnect)             (Lacewing::Webserver &Webserver, Lacewing::Webserver::Request &Request);
    typedef void (LacewingHandler * HandlerError)                  (Lacewing::Webserver &Webserver, Lacewing::Error &);

    typedef void (LacewingHandler * HandlerUploadStart)            (Lacewing::Webserver &Webserver, Lacewing::Webserver::Request &Request,
                                                                    Lacewing::Webserver::Upload &Upload);

    typedef void (LacewingHandler * HandlerUploadChunk)            (Lacewing::Webserver &Webserver, Lacewing::Webserver::Request &Request,
                                                                    Lacewing::Webserver::Upload &Upload, const char * Data, int Size);
     
    typedef void (LacewingHandler * HandlerUploadDone)             (Lacewing::Webserver &Webserver, Lacewing::Webserver::Request &Request,
                                                                    Lacewing::Webserver::Upload &Upload);

    typedef void (LacewingHandler * HandlerUploadPost)             (Lacewing::Webserver &Webserver, Lacewing::Webserver::Request &Request,
                                                                    Lacewing::Webserver::Upload * Uploads[], int UploadCount);

    LacewingFunction void onGet              (HandlerGet);
    LacewingFunction void onUploadStart      (HandlerUploadStart);
    LacewingFunction void onUploadChunk      (HandlerUploadChunk);
    LacewingFunction void onUploadDone       (HandlerUploadDone);
    LacewingFunction void onUploadPost       (HandlerUploadPost);
    LacewingFunction void onPost             (HandlerPost);
    LacewingFunction void onHead             (HandlerHead);
    LacewingFunction void onDisconnect       (HandlerDisconnect);
    LacewingFunction void onError            (HandlerError);
};
    
struct RelayClient
{
public:

    void * InternalTag, * Tag;

    LacewingFunction  RelayClient (Pump &);
    LacewingFunction ~RelayClient ();

    LacewingFunction void Connect    (const char * Host, int Port = 6121);
    LacewingFunction void Connect    (Lacewing::Address &);

    LacewingFunction bool Connecting ();
    LacewingFunction bool Connected  ();

    LacewingFunction void Disconnect ();

    LacewingFunction Lacewing::Address &ServerAddress();

    LacewingFunction int ID ();

    LacewingFunction void Name (const char * Name);
    LacewingFunction const char * Name ();

    LacewingFunction const char * WelcomeMessage ();

    LacewingFunction void ListChannels();

    struct ChannelListing
    {
        void * InternalTag, * Tag;

        short PeerCount;
        const char * Name;

        LacewingFunction ChannelListing * Next ();
    };

    LacewingFunction int ChannelListingCount ();
    LacewingFunction ChannelListing * FirstChannelListing ();

    LacewingFunction void Join(const char * Channel, bool Hidden = false, bool AutoClose = false);

    LacewingFunction void SendServer(int Subchannel, const char * Data, int Size = -1, int Type = 0);
    LacewingFunction void BlastServer(int Subchannel, const char * Data, int Size = -1, int Type = 0);

    struct Channel
    {
        void * InternalTag, * Tag;

        LacewingFunction const char * Name();
        LacewingFunction bool IsChannelMaster();

        LacewingFunction void Send(int Subchannel, const char * Data, int Size = -1, int Type = 0);
        LacewingFunction void Blast(int Subchannel, const char * Data, int Size = -1, int Type = 0);

        struct Peer
        {
            void * InternalTag, * Tag;

            int  ID();
            bool IsChannelMaster();

            LacewingFunction void Send(int Subchannel, const char * Data, int Size = -1, int Type = 0);
            LacewingFunction void Blast(int Subchannel, const char * Data, int Size = -1, int Type = 0);

            LacewingFunction const char * Name();

            LacewingFunction Peer * Next ();
        };

        LacewingFunction int PeerCount();
        LacewingFunction Peer * FirstPeer ();

        LacewingFunction void Leave();

        LacewingFunction Channel * Next ();
    };

    LacewingFunction int ChannelCount();
    LacewingFunction Channel * FirstChannel ();

    typedef void (LacewingHandler * HandlerConnect)                  (Lacewing::RelayClient &Client);
    typedef void (LacewingHandler * HandlerConnectionDenied)         (Lacewing::RelayClient &Client, const char * DenyReason);
    typedef void (LacewingHandler * HandlerDisconnect)               (Lacewing::RelayClient &Client);
    
    typedef void (LacewingHandler * HandlerServerMessage)
            (Lacewing::RelayClient &Client, bool Blasted, int Subchannel, char * Data, int Size, int Variant);
   
    typedef void (LacewingHandler * HandlerChannelMessage)
        (Lacewing::RelayClient &Client, Lacewing::RelayClient::Channel &Channel, Lacewing::RelayClient::Channel::Peer &Peer,
            bool Blasted, int Subchannel, char * Data, int Size, int Variant);
  
    typedef void (LacewingHandler * HandlerPeerMessage)
        (Lacewing::RelayClient &Client, Lacewing::RelayClient::Channel &Channel, Lacewing::RelayClient::Channel::Peer &Peer,
            bool Blasted, int Subchannel, char * Data, int Size, int Variant);

    typedef void (LacewingHandler * HandlerServerChannelMessage)
        (Lacewing::RelayClient &Client, Lacewing::RelayClient::Channel &Channel, bool Blasted,
            int Subchannel, char * Data, int Size, int Variant);

    typedef void (LacewingHandler * HandlerError)                    (Lacewing::RelayClient &Client, Lacewing::Error &);
    typedef void (LacewingHandler * HandlerJoin)                     (Lacewing::RelayClient &Client, Lacewing::RelayClient::Channel &Target);
    typedef void (LacewingHandler * HandlerJoinDenied)               (Lacewing::RelayClient &Client, const char * ChannelName, const char * DenyReason);
    typedef void (LacewingHandler * HandlerLeave)                    (Lacewing::RelayClient &Client, Lacewing::RelayClient::Channel &Target);
    typedef void (LacewingHandler * HandlerLeaveDenied)              (Lacewing::RelayClient &Client, Lacewing::RelayClient::Channel &Target,const char * DenyReason);
    typedef void (LacewingHandler * HandlerNameSet)                  (Lacewing::RelayClient &Client);
    typedef void (LacewingHandler * HandlerNameChanged)              (Lacewing::RelayClient &Client, const char * OldName);
    typedef void (LacewingHandler * HandlerNameDenied)               (Lacewing::RelayClient &Client, const char * Name, const char * DenyReason);
    typedef void (LacewingHandler * HandlerPeerConnect)              (Lacewing::RelayClient &Client, Lacewing::RelayClient::Channel &Channel,Lacewing::RelayClient::Channel::Peer &Peer);
    typedef void (LacewingHandler * HandlerPeerDisconnect)           (Lacewing::RelayClient &Client, Lacewing::RelayClient::Channel &Channel,Lacewing::RelayClient::Channel::Peer &Peer);
    typedef void (LacewingHandler * HandlerPeerChangeName)           (Lacewing::RelayClient &Client, Lacewing::RelayClient::Channel &Channel,Lacewing::RelayClient::Channel::Peer &Peer,const char * OldName);
    typedef void (LacewingHandler * HandlerChannelListReceived)      (Lacewing::RelayClient &Client);

    LacewingFunction void onConnect               (HandlerConnect);
    LacewingFunction void onConnectionDenied      (HandlerConnectionDenied);
    LacewingFunction void onDisconnect            (HandlerDisconnect);
    LacewingFunction void onServerMessage         (HandlerServerMessage);
    LacewingFunction void onChannelMessage        (HandlerChannelMessage);
    LacewingFunction void onPeerMessage           (HandlerPeerMessage);
    LacewingFunction void onServerChannelMessage  (HandlerServerChannelMessage);
    LacewingFunction void onError                 (HandlerError);
    LacewingFunction void onJoin                  (HandlerJoin);
    LacewingFunction void onJoinDenied            (HandlerJoinDenied);
    LacewingFunction void onLeave                 (HandlerLeave);
    LacewingFunction void onLeaveDenied           (HandlerLeaveDenied);    
    LacewingFunction void onNameSet               (HandlerNameSet);
    LacewingFunction void onNameChanged           (HandlerNameChanged);
    LacewingFunction void onNameDenied            (HandlerNameDenied);
    LacewingFunction void onPeerConnect           (HandlerPeerConnect);
    LacewingFunction void onPeerDisconnect        (HandlerPeerDisconnect);
    LacewingFunction void onPeerChangeName        (HandlerPeerChangeName);
    LacewingFunction void onChannelListReceived   (HandlerChannelListReceived);

private:

    Lacewing::Client Socket;
    Lacewing::UDP    UDP;
};

struct RelayServer
{
    void * InternalTag, * Tag;

    Lacewing::Server Socket;
    Lacewing::UDP UDP;

    LacewingFunction  RelayServer(Pump &);
    LacewingFunction ~RelayServer();

    LacewingFunction void Host (int Port = 6121);
    LacewingFunction void Host (Lacewing::Filter &Filter);
    LacewingFunction void Unhost ();

    LacewingFunction bool Hosting ();
    LacewingFunction int Port ();

    LacewingFunction void SetWelcomeMessage(const char * Message);
    LacewingFunction void SetChannelListing(bool Enabled);

    struct Client;

    struct Channel
    {
        void * InternalTag, * Tag;

        LacewingFunction int ID();

        LacewingFunction Client * ChannelMaster();

        LacewingFunction const char * Name ();
        LacewingFunction void Name (const char *);

        LacewingFunction bool Hidden();
        LacewingFunction bool AutoCloseEnabled();
        LacewingFunction void Close();
        
        LacewingFunction void Send (int Subchannel, const char * Data, int Size = -1, int Variant = 0);
        LacewingFunction void Blast (int Subchannel, const char * Data, int Size = -1, int Variant = 0);

        LacewingFunction int ClientCount ();

        struct ClientIterator
        {
            void * InternalTag;
        
            LacewingFunction ClientIterator (Channel &);
            LacewingFunction Client * Next ();
        };
        
        LacewingFunction Channel * Next ();
    };

    LacewingFunction int ChannelCount();
    LacewingFunction Channel * FirstChannel ();
    
    struct Client
    {
        void * InternalTag, * Tag;

        LacewingFunction int ID();

        LacewingFunction Lacewing::Address &GetAddress();

        LacewingFunction void Disconnect();

        LacewingFunction void Send(int Subchannel, const char * Data, int Size = -1, int Variant = 0);
        LacewingFunction void Blast(int Subchannel, const char * Data, int Size = -1, int Variant = 0);

        LacewingFunction const char * Name ();
        LacewingFunction void Name (const char *);
        
        LacewingFunction int ChannelCount();
        
        struct ChannelIterator
        {
            void * InternalTag;

            LacewingFunction ChannelIterator (Client &);
            LacewingFunction Channel * Next ();
        };
        
        LacewingFunction Client * Next ();
    };

    LacewingFunction int ClientCount ();
    LacewingFunction Client * FirstClient ();

    typedef bool (LacewingHandler * HandlerConnect)     (Lacewing::RelayServer &Server, Lacewing::RelayServer::Client &Client);
    typedef void (LacewingHandler * HandlerDisconnect)  (Lacewing::RelayServer &Server, Lacewing::RelayServer::Client &Client);
    typedef void (LacewingHandler * HandlerError)       (Lacewing::RelayServer &Server, Lacewing::Error &);
   
    typedef void (LacewingHandler * HandlerServerMessage)
        (Lacewing::RelayServer &Server, Lacewing::RelayServer::Client &Client, bool Blasted, int Subchannel,
            char * Data, int Size, int Variant);
    
    typedef bool (LacewingHandler * HandlerChannelMessage)
        (Lacewing::RelayServer &Server, Lacewing::RelayServer::Client &Client, Lacewing::RelayServer::Channel &Channel,
            bool Blasted, int Subchannel, char * Data, int Size, int Variant);
    
    typedef bool (LacewingHandler * HandlerPeerMessage)
        (Lacewing::RelayServer &Server, Lacewing::RelayServer::Client &Client, Lacewing::RelayServer::Channel &Channel,
            Lacewing::RelayServer::Client &TargetClient,bool Blasted,
            int Subchannel, char * Packet, int Size, int Variant);

    typedef bool (LacewingHandler * HandlerJoinChannel)
        (Lacewing::RelayServer &Server, Lacewing::RelayServer::Client &Client, Lacewing::RelayServer::Channel &Channel);

    typedef bool (LacewingHandler * HandlerLeaveChannel)
        (Lacewing::RelayServer &Server, Lacewing::RelayServer::Client &Client, Lacewing::RelayServer::Channel &Channel);

    typedef bool (LacewingHandler * HandlerSetName)
        (Lacewing::RelayServer &Server, Lacewing::RelayServer::Client &Client, const char * Name);

    LacewingFunction void onConnect        (HandlerConnect);
    LacewingFunction void onDisconnect     (HandlerDisconnect);
    LacewingFunction void onError          (HandlerError);
    LacewingFunction void onServerMessage  (HandlerServerMessage);
    LacewingFunction void onChannelMessage (HandlerChannelMessage);
    LacewingFunction void onPeerMessage    (HandlerPeerMessage);
    LacewingFunction void onJoinChannel    (HandlerJoinChannel);
    LacewingFunction void onLeaveChannel   (HandlerLeaveChannel);
    LacewingFunction void onSetName        (HandlerSetName);
};

struct FlashPolicy
{
    void * InternalTag, * Tag;

    LacewingFunction  FlashPolicy (Pump &);
    LacewingFunction ~FlashPolicy ();

    LacewingFunction void Host (const char * Filename, int Port = 843);
    LacewingFunction void Host (const char * Filename, Lacewing::Filter &Filter);
    LacewingFunction void Unhost ();

    LacewingFunction bool Hosting ();

    typedef void (LacewingHandler * HandlerError)
        (Lacewing::FlashPolicy &FlashPolicy, Lacewing::Error &);

    LacewingFunction void onError (HandlerError);
};

}

#endif /* __cplusplus */
#endif /* LacewingIncluded */

