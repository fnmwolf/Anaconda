
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

#include "../Common.h"

lw_ws * lw_ws_new (lw_eventpump * eventpump)
    { return (lw_ws *) new Lacewing::Webserver(*(Lacewing::EventPump *) eventpump);
    }
void lw_ws_delete (lw_ws * webserver)
    { delete (Lacewing::Webserver *) webserver;
    }
void lw_ws_host (lw_ws * webserver, long port)
    { ((Lacewing::Webserver *) webserver)->Host(port);
    }
void lw_ws_host_secure (lw_ws * webserver, long port)
    { ((Lacewing::Webserver *) webserver)->HostSecure(port);
    }
void lw_ws_host_filter (lw_ws * webserver, lw_filter * filter)
    { ((Lacewing::Webserver *) webserver)->Host(*(Lacewing::Filter *) filter);
    }
void lw_ws_host_secure_filter (lw_ws * webserver, lw_filter * filter)
    { ((Lacewing::Webserver *) webserver)->HostSecure(*(Lacewing::Filter *) filter);
    }
void lw_ws_unhost (lw_ws * webserver)
    { ((Lacewing::Webserver *) webserver)->Unhost();
    }
void lw_ws_unhost_secure (lw_ws * webserver)
    { ((Lacewing::Webserver *) webserver)->UnhostSecure();
    }
lw_bool lw_ws_hosting (lw_ws * webserver)
    { return ((Lacewing::Webserver *) webserver)->Hosting() ? 1 : 0;
    }
lw_bool lw_ws_hosting_secure (lw_ws * webserver)
    { return ((Lacewing::Webserver *) webserver)->HostingSecure() ? 1 : 0;
    }
long lw_ws_port (lw_ws * webserver)
    { return ((Lacewing::Webserver *) webserver)->Port();
    }
long lw_ws_port_secure (lw_ws * webserver)
    { return ((Lacewing::Webserver *) webserver)->SecurePort();
    }
lw_bool lw_ws_load_cert_file (lw_ws * webserver, const char * filename, const char * passphrase)
    { return ((Lacewing::Webserver *) webserver)->LoadCertificateFile(filename, passphrase);
    }
lw_bool lw_ws_load_sys_cert (lw_ws * webserver, const char * store_name, const char * common_name, const char * location)
    { return ((Lacewing::Webserver *) webserver)->LoadSystemCertificate(store_name, common_name, location);
    }
lw_bool lw_ws_cert_loaded (lw_ws * webserver)
    { return ((Lacewing::Webserver *) webserver)->CertificateLoaded();
    }
lw_i64 lw_ws_bytes_sent (lw_ws * webserver)
    { return ((Lacewing::Webserver *) webserver)->BytesSent();
    }
lw_i64 lw_ws_bytes_received (lw_ws * webserver)
    { return ((Lacewing::Webserver *) webserver)->BytesReceived();
    }
void lw_ws_close_session (lw_ws * webserver, const char * id)
    { ((Lacewing::Webserver *) webserver)->CloseSession(id);
    }
void lw_ws_enable_manual_finish (lw_ws * webserver)
    { ((Lacewing::Webserver *) webserver)->EnableManualRequestFinish();
    }
long lw_ws_idle_timeout (lw_ws * webserver)
    { return ((Lacewing::Webserver *) webserver)->IdleTimeout();
    }
void lw_ws_set_idle_timeout (lw_ws * webserver, long timeout)
    { ((Lacewing::Webserver *) webserver)->IdleTimeout(timeout);
    }
lw_addr* lw_ws_req_addr (lw_ws_req * request)
    { return (lw_addr *) &((Lacewing::Webserver::Request *) request)->GetAddress();
    }
lw_bool lw_ws_req_secure (lw_ws_req * request)
    { return ((Lacewing::Webserver::Request *) request)->Secure();
    }
const char* lw_ws_req_url (lw_ws_req * request)
    { return ((Lacewing::Webserver::Request *) request)->URL();
    }
const char* lw_ws_req_hostname (lw_ws_req * request)
    { return ((Lacewing::Webserver::Request *) request)->Hostname();
    }
void lw_ws_req_disconnect (lw_ws_req * request)
    { ((Lacewing::Webserver::Request *) request)->Disconnect();
    } 
void lw_ws_req_set_redirect (lw_ws_req * request, const char * url)
    { ((Lacewing::Webserver::Request *) request)->SetRedirect(url);
    }
void lw_ws_req_set_status (lw_ws_req * request, long code, const char * message)
    { ((Lacewing::Webserver::Request *) request)->Status(code, message);
    }
void lw_ws_req_set_mime_type (lw_ws_req * request, const char * mime_type)
    { ((Lacewing::Webserver::Request *) request)->SetMimeType(mime_type);
    }
void lw_ws_req_set_mime_type_ex (lw_ws_req * request, const char * mime_type, const char * charset)
    { ((Lacewing::Webserver::Request *) request)->SetMimeType(mime_type, charset);
    }
void lw_ws_req_guess_mime_type (lw_ws_req * request, const char * filename)
    { ((Lacewing::Webserver::Request *) request)->GuessMimeType(filename);
    }
void lw_ws_req_send_text (lw_ws_req * request, const char * data)
    { ((Lacewing::Webserver::Request *) request)->Send(data);
    }
void lw_ws_req_send_text_const (lw_ws_req * request, const char * data)
    { ((Lacewing::Webserver::Request *) request)->SendConstant(data);
    }
void lw_ws_req_send (lw_ws_req * request, const char * data, long size)
    { ((Lacewing::Webserver::Request *) request)->Send(data, size);
    }
void lw_ws_req_send_const (lw_ws_req * request, const char * data, long size)
    { ((Lacewing::Webserver::Request *) request)->SendConstant(data, size);
    }
void lw_ws_req_sendfile (lw_ws_req * request, const char * filename)
    { ((Lacewing::Webserver::Request *) request)->SendFile(filename);
    }
void lw_ws_req_sendfile_ex (lw_ws_req * request, const char * filename, lw_i64 offset, lw_i64 size)
    { ((Lacewing::Webserver::Request *) request)->SendFile(filename, offset, size);
    }
void lw_ws_req_reset (lw_ws_req * request)
    { ((Lacewing::Webserver::Request *) request)->Reset();
    }
void lw_ws_req_finish (lw_ws_req * request)
    { ((Lacewing::Webserver::Request *) request)->Finish();
    }
lw_i64 lw_ws_req_last_modified (lw_ws_req * request)
    { return ((Lacewing::Webserver::Request *) request)->LastModified();
    }
void lw_ws_req_set_last_modified (lw_ws_req * request, lw_i64 modified)
    { ((Lacewing::Webserver::Request *) request)->LastModified(modified);
    }
void lw_ws_req_set_unmodified (lw_ws_req * request)
    { ((Lacewing::Webserver::Request *) request)->SetUnmodified();
    }
void lw_ws_req_set_header (lw_ws_req * request, const char * name, const char * value)
    { ((Lacewing::Webserver::Request *) request)->Header(name, value);
    }
const char* lw_ws_req_header (lw_ws_req * request, const char * name)
    { return ((Lacewing::Webserver::Request *) request)->Header(name);
    }
lw_ws_req_hdr* lw_ws_req_first_header (lw_ws_req * request)
    { return (lw_ws_req_hdr *) ((Lacewing::Webserver::Request *) request)->FirstHeader ();
    }
lw_ws_req_hdr* lw_ws_req_hdr_next (lw_ws_req_hdr * header)
    { return (lw_ws_req_hdr *) ((struct Lacewing::Webserver::Request::Header *) header)->Next ();
    }
const char* lw_ws_req_hdr_name (lw_ws_req_hdr * header)
    { return ((struct Lacewing::Webserver::Request::Header *) header)->Name ();
    }
const char* lw_ws_req_hdr_value (lw_ws_req_hdr * header)
    { return ((struct Lacewing::Webserver::Request::Header *) header)->Value ();
    }
void lw_ws_req_set_cookie (lw_ws_req * request, const char * name, const char * value)
    { ((Lacewing::Webserver::Request *) request)->Cookie(name, value);
    }
void lw_ws_req_set_cookie_ex (lw_ws_req * request, const char * name, const char * value, const char * attributes)
    { ((Lacewing::Webserver::Request *) request)->Cookie(name, value, attributes);
    }
const char* lw_ws_req_get_cookie (lw_ws_req * request, const char * name)
    { return ((Lacewing::Webserver::Request *) request)->Cookie(name);
    }
const char* lw_ws_req_session_id (lw_ws_req * request)
    { return ((Lacewing::Webserver::Request *) request)->Session();
    }
void lw_ws_req_session_write (lw_ws_req * request, const char * name, const char * value)
    { ((Lacewing::Webserver::Request *) request)->Session(name, value);
    }
const char* lw_ws_req_session_read (lw_ws_req * request, const char * name)
    { return ((Lacewing::Webserver::Request *) request)->Session(name);
    }
lw_ws_req_ssnitem * lw_ws_req_first_session_item (lw_ws_req * request)
    { return (lw_ws_req_ssnitem *) ((Lacewing::Webserver::Request *) request)->FirstSessionItem ();
    }
const char* lw_ws_req_ssnitem_name (lw_ws_req_ssnitem * item)
    { return ((Lacewing::Webserver::Request::SessionItem *) item)->Name ();
    }
const char* lw_ws_req_ssnitem_value (lw_ws_req_ssnitem * item)
    { return ((Lacewing::Webserver::Request::SessionItem *) item)->Value ();
    }
lw_ws_req_ssnitem * lw_ws_req_ssnitem_next (lw_ws_req_ssnitem * item)
    { return (lw_ws_req_ssnitem *) ((Lacewing::Webserver::Request::SessionItem *) item)->Next ();
    }
void lw_ws_req_session_close (lw_ws_req * request)
    { ((Lacewing::Webserver::Request *) request)->CloseSession();
    }
const char* lw_ws_req_GET (lw_ws_req * request, const char * name)
    { return ((Lacewing::Webserver::Request *) request)->GET(name);
    }
const char* lw_ws_req_POST (lw_ws_req * request, const char * name)
    { return ((Lacewing::Webserver::Request *) request)->POST(name);
    }
lw_ws_req_param* lw_ws_req_first_GET (lw_ws_req * request)
    { return (lw_ws_req_param *) ((Lacewing::Webserver::Request *) request)->GET ();
    }
lw_ws_req_param* lw_ws_req_first_POST (lw_ws_req * request)
    { return (lw_ws_req_param *) ((Lacewing::Webserver::Request *) request)->POST ();
    }
const char* lw_ws_req_param_name (lw_ws_req_param * param)
    { return ((Lacewing::Webserver::Request::Parameter *) param)->Name ();
    }
const char* lw_ws_req_param_value (lw_ws_req_param * param)
    { return ((Lacewing::Webserver::Request::Parameter *) param)->Value ();
    }
lw_ws_req_param* lw_ws_req_param_next (lw_ws_req_param * param)
    { return (lw_ws_req_param *) ((Lacewing::Webserver::Request::Parameter *) param)->Next ();
    }
lw_ws_req_cookie * lw_ws_req_first_cookie (lw_ws_req * request)
    { return (lw_ws_req_cookie *) ((Lacewing::Webserver::Request *) request)->FirstCookie ();
    }
const char* lw_ws_req_cookie_name (lw_ws_req_cookie * cookie)
    { return ((struct Lacewing::Webserver::Request::Cookie *) cookie)->Name ();
    }
const char* lw_ws_req_cookie_value (lw_ws_req_cookie * cookie)
    { return ((struct Lacewing::Webserver::Request::Cookie *) cookie)->Value ();
    }
lw_ws_req_cookie * lw_ws_req_cookie_next (lw_ws_req_cookie * cookie)
    { return (lw_ws_req_cookie *) ((struct Lacewing::Webserver::Request::Cookie *) cookie)->Next ();
    }
void lw_ws_req_disable_cache (lw_ws_req * request)
    { ((Lacewing::Webserver::Request *) request)->DisableCache();
    }
long lw_ws_req_idle_timeout (lw_ws_req * request)
    { return ((Lacewing::Webserver::Request *) request)->IdleTimeout();
    }
void lw_ws_req_set_idle_timeout (lw_ws_req * request, long timeout)
    { ((Lacewing::Webserver::Request *) request)->IdleTimeout(timeout);
    }
/*void lw_ws_req_enable_dl_resuming (lw_ws_req * request)
    { ((Lacewing::Webserver::Request *) request)->EnableDownloadResuming();
    }
lw_i64 lw_ws_req_reqrange_begin (lw_ws_req * request)
    { return ((Lacewing::Webserver::Request *) request)->RequestedRangeBegin();
    }
lw_i64 lw_ws_req_reqrange_end (lw_ws_req * request)
    { return ((Lacewing::Webserver::Request *) request)->RequestedRangeEnd();
    }
void lw_ws_req_set_outgoing_range (lw_ws_req * request, lw_i64 begin, lw_i64 end)
    { ((Lacewing::Webserver::Request *) request)->SetOutgoingRange(begin, end);
    }*/
const char* lw_ws_upload_form_el_name (lw_ws_upload * upload)
    { return ((Lacewing::Webserver::Upload *) upload)->FormElementName();
    }
const char* lw_ws_upload_filename (lw_ws_upload * upload)
    { return ((Lacewing::Webserver::Upload *) upload)->Filename();
    }
const char* lw_ws_upload_header (lw_ws_upload * upload, const char * name)
    { return ((Lacewing::Webserver::Upload *) upload)->Header(name);
    }
lw_ws_upload_hdr* lw_ws_upload_first_header (lw_ws_req * upload)
    { return (lw_ws_upload_hdr *) ((Lacewing::Webserver::Upload *) upload)->FirstHeader ();
    }
lw_ws_upload_hdr* lw_ws_upload_hdr_next (lw_ws_upload_hdr * header)
    { return (lw_ws_upload_hdr *) ((struct Lacewing::Webserver::Upload::Header *) header)->Next ();
    }
const char* lw_ws_upload_hdr_name (lw_ws_upload_hdr * header)
    { return ((struct Lacewing::Webserver::Upload::Header *) header)->Name ();
    }
const char* lw_ws_upload_hdr_value (lw_ws_upload_hdr * header)
    { return ((struct Lacewing::Webserver::Upload::Header *) header)->Value ();
    }
void lw_ws_upload_set_autosave (lw_ws_upload * upload)
    { ((Lacewing::Webserver::Upload *) upload)->SetAutoSave();
    }
const char* lw_ws_upload_autosave_fname (lw_ws_upload * upload)
    { return ((Lacewing::Webserver::Upload *) upload)->GetAutoSaveFilename();
    }

AutoHandlerFlat(Lacewing::Webserver, lw_ws, Get, get)
AutoHandlerFlat(Lacewing::Webserver, lw_ws, Post, post)
AutoHandlerFlat(Lacewing::Webserver, lw_ws, Head, head)
AutoHandlerFlat(Lacewing::Webserver, lw_ws, Error, error)
AutoHandlerFlat(Lacewing::Webserver, lw_ws, UploadStart, upload_start)
AutoHandlerFlat(Lacewing::Webserver, lw_ws, UploadChunk, upload_chunk)
AutoHandlerFlat(Lacewing::Webserver, lw_ws, UploadDone, upload_done)
AutoHandlerFlat(Lacewing::Webserver, lw_ws, UploadPost, upload_post)

void lw_ws_req_sendf (lw_ws_req * request, const char * format, ...)
{
    va_list args;
    va_start (args, format);
    
    char * data;
    int size = LacewingFormat(data, format, args);
    
    if(size > 0)
        ((Lacewing::Webserver::Request *) request)->Send(data, size);

    va_end (args);
}
