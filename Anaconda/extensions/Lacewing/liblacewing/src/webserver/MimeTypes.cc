
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

const char * const MimeTypes [] =
{
    "323",            "text/h323",
    "acx",            "application/internet-property-stream",
    "ai",             "application/postscript",
    "aif",            "audio/x-aiff",
    "aifc",           "audio/x-aiff",
    "aiff",           "audio/x-aiff",
    "asf",            "video/x-ms-asf",
    "asr",            "video/x-ms-asf",
    "asx",            "video/x-ms-asf",
    "au",             "audio/basic",
    "avi",            "video/x-msvideo",
    "axs",            "application/olescript",
    "bas",            "text/plain",
    "bcpio",          "application/x-bcpio",
    "bmp",            "image/bmp",
    "c",              "text/plain",
    "cat",            "application/vnd.ms-pkiseccat",
    "cdf",            "application/x-cdf",
    "cer",            "application/x-x509-ca-cert",
    "clp",            "application/x-msclip",
    "cmx",            "image/x-cmx",
    "cod",            "image/cis-cod",
    "cpio",           "application/x-cpio",
    "crd",            "application/x-mscardfile",
    "crl",            "application/pkix-crl",
    "crt",            "application/x-x509-ca-cert",
    "csh",            "application/x-csh",
    "css",            "text/css",
    "dcr",            "application/x-director",
    "der",            "application/x-x509-ca-cert",
    "dir",            "application/x-director",
    "dll",            "application/x-msdownload",
    "doc",            "application/msword",
    "dot",            "application/msword",
    "dvi",            "application/x-dvi",
    "dxr",            "application/x-director",
    "eps",            "application/postscript",
    "etx",            "text/x-setext",
    "evy",            "application/envoy",
    "fif",            "application/fractals",
    "flr",            "x-world/x-vrml",
    "gif",            "image/gif",
    "gtar",           "application/x-gtar",
    "gz",             "application/x-gzip",
    "h",              "text/plain",
    "hdf",            "application/x-hdf",
    "hlp",            "application/winhlp",
    "hqx",            "application/mac-binhex40",
    "hta",            "application/hta",
    "htc",            "text/x-component",
    "htm",            "text/html",
    "html",           "text/html",
    "htt",            "text/webviewhtml",
    "ico",            "image/x-icon",
    "ief",            "image/ief",
    "iii",            "application/x-iphone",
    "ins",            "application/x-internet-signup",
    "isp",            "application/x-internet-signup",
    "jfif",           "image/pipeg",
    "jpe",            "image/jpeg",
    "jpeg",           "image/jpeg",
    "jpg",            "image/jpeg",
    "js",             "application/x-javascript",
    "latex",          "application/x-latex",
    "lsf",            "video/x-la-asf",
    "lsx",            "video/x-la-asf",
    "m13",            "application/x-msmediaview",
    "m14",            "application/x-msmediaview",
    "m3u",            "audio/x-mpegurl",
    "man",            "application/x-troff-man",
    "mdb",            "application/x-msaccess",
    "me",             "application/x-troff-me",
    "mht",            "message/rfc822",
    "mhtml",          "message/rfc822",
    "mid",            "audio/mid",
    "mny",            "application/x-msmoney",
    "mov",            "video/quicktime",
    "movie",          "video/x-sgi-movie",
    "mp2",            "video/mpeg",
    "mp3",            "audio/mpeg",
    "mpa",            "video/mpeg",
    "mpe",            "video/mpeg",
    "mpeg",           "video/mpeg",
    "mpg",            "video/mpeg",
    "mpp",            "application/vnd.ms-project",
    "mpv2",           "video/mpeg",
    "ms",             "application/x-troff-ms",
    "mvb",            "application/x-msmediaview",
    "nws",            "message/rfc822",
    "oda",            "application/oda",
    "p10",            "application/pkcs10",
    "p12",            "application/x-pkcs12",
    "p7b",            "application/x-pkcs7-certificates",
    "p7c",            "application/x-pkcs7-mime",
    "p7m",            "application/x-pkcs7-mime",
    "p7r",            "application/x-pkcs7-certreqresp",
    "p7s",            "application/x-pkcs7-signature",
    "pbm",            "image/x-portable-bitmap",
    "pdf",            "application/pdf",
    "pfx",            "application/x-pkcs12",
    "pgm",            "image/x-portable-graymap",
    "pko",            "application/ynd.ms-pkipko",
    "pma",            "application/x-perfmon",
    "pmc",            "application/x-perfmon",
    "pml",            "application/x-perfmon",
    "pmr",            "application/x-perfmon",
    "pmw",            "application/x-perfmon",
    "pnm",            "image/x-portable-anymap",
    "png",            "image/png",
    "pot",            "application/vnd.ms-powerpoint",
    "ppm",            "image/x-portable-pixmap",
    "pps",            "application/vnd.ms-powerpoint",
    "ppt",            "application/vnd.ms-powerpoint",
    "prf",            "application/pics-rules",
    "ps",             "application/postscript",
    "pub",            "application/x-mspublisher",
    "qt",             "video/quicktime",
    "ra",             "audio/x-pn-realaudio",
    "ram",            "audio/x-pn-realaudio",
    "ras",            "image/x-cmu-raster",
    "rgb",            "image/x-rgb",
    "rmi",            "audio/mid",
    "roff",           "application/x-troff",
    "rtf",            "application/rtf",
    "rtx",            "text/richtext",
    "scd",            "application/x-msschedule",
    "sct",            "text/scriptlet",
    "setpay",         "application/set-payment-initiation",
    "setreg",         "application/set-registration-initiation",
    "sh",             "application/x-sh",
    "shar",           "application/x-shar",
    "sit",            "application/x-stuffit",
    "snd",            "audio/basic",
    "spc",            "application/x-pkcs7-certificates",
    "spl",            "application/futuresplash",
    "src",            "application/x-wais-source",
    "sst",            "application/vnd.ms-pkicertstore",
    "stl",            "application/vnd.ms-pkistl",
    "stm",            "text/html",
    "svg",            "image/svg+xml",
    "sv4cpio",        "application/x-sv4cpio",
    "sv4crc",         "application/x-sv4crc",
    "swf",            "application/x-shockwave-flash",
    "t",              "application/x-troff",
    "tar",            "application/x-tar",
    "tcl",            "application/x-tcl",
    "tex",            "application/x-tex",
    "texi",           "application/x-texinfo",
    "texinfo",        "application/x-texinfo",
    "tgz",            "application/x-compressed",
    "tif",            "image/tiff",
    "tiff",           "image/tiff",
    "tr",             "application/x-troff",
    "trm",            "application/x-msterminal",
    "tsv",            "text/tab-separated-values",
    "txt",            "text/plain",
    "uls",            "text/iuls",
    "ustar",          "application/x-ustar",
    "vcf",            "text/x-vcard",
    "vrml",           "x-world/x-vrml",
    "wav",            "audio/x-wav",
    "wcm",            "application/vnd.ms-works",
    "wdb",            "application/vnd.ms-works",
    "wks",            "application/vnd.ms-works",
    "wmf",            "application/x-msmetafile",
    "wps",            "application/vnd.ms-works",
    "wri",            "application/x-mswrite",
    "wrl",            "x-world/x-vrml",
    "wrz",            "x-world/x-vrml",
    "xaf",            "x-world/x-vrml",
    "xbm",            "image/x-xbitmap",
    "xla",            "application/vnd.ms-excel",
    "xlc",            "application/vnd.ms-excel",
    "xlm",            "application/vnd.ms-excel",
    "xls",            "application/vnd.ms-excel",
    "xlt",            "application/vnd.ms-excel",
    "xlw",            "application/vnd.ms-excel",
    "xof",            "x-world/x-vrml",
    "xpm",            "image/x-xpixmap",
    "xwd",            "image/x-xwindowdump",
    "z",              "application/x-compress",
    "zip",            "application/zip",
0 };

const char * Lacewing::GuessMimeType (const char * Filename)
{
    if(*Filename)
    {
        const char * Extension = strrchr (Filename, '.');

        if (!Extension)
            Extension = Filename;
        else
            ++ Extension;

        DebugOut ("Guess mime type for %s", Extension);

        for(const char * const * Iterator = MimeTypes; *Iterator; Iterator += 2)
            if(!strcasecmp (*Iterator, Extension))
                return *++ Iterator;
    }

    return "application/octet-stream";
}



