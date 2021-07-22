
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

const char * lw_version ()
    { return Lacewing::Version();
    }
lw_i64 lw_current_thread_id ()
    { return Lacewing::CurrentThreadID();
    }
lw_i64 lw_file_last_modified (const char * filename)
    { return Lacewing::LastModified(filename);
    }
lw_bool lw_file_exists (const char * filename)
    { return Lacewing::FileExists(filename);
    }
lw_bool lw_path_exists (const char * filename)
    { return Lacewing::PathExists(filename);
    }
lw_i64 lw_file_size (const char * filename)
    { return Lacewing::FileSize(filename);
    }
void lw_int64_to_string (lw_i64 int64, char * buffer)
    { Lacewing::Int64ToString(int64, buffer);
    }
void lw_temp_path (char * buffer)
    { Lacewing::TempPath(buffer);
    }
void lw_new_temp_file (char * buffer)
    { Lacewing::NewTempFile(buffer);
    }
void lw_md5 (char * output, const char * input, long length)
    { Lacewing::MD5 (output, input, length);
    }
void lw_md5_hex (char * output, const char * input, long length)
    { Lacewing::MD5_Hex (output, input, length);
    }
void lw_sha1 (char * output, const char * input, long length)
    { Lacewing::SHA1 (output, input, length);
    }
void lw_sha1_hex (char * output, const char * input, long length)
    { Lacewing::SHA1_Hex (output, input, length);
    }
