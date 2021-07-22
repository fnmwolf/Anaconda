
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

lw_ssync * lw_ssync_new ()
    { return (lw_ssync *) new Lacewing::SpinSync();
    }
void lw_ssync_delete (lw_ssync * sync)
    { delete (Lacewing::SpinSync *) sync;
    }
lw_ssync_wlock * lw_ssync_wlock_new (lw_ssync * sync)
    { return (lw_ssync_wlock *) new Lacewing::SpinSync::WriteLock(*(Lacewing::SpinSync *) sync);
    }
void lw_ssync_wlock_delete (lw_ssync_wlock * lock)
    { delete (Lacewing::SpinSync::WriteLock *) lock;
    }
void lw_ssync_wlock_release (lw_ssync_wlock * lock)
    { ((Lacewing::SpinSync::WriteLock *) lock)->Release();
    }
lw_ssync_rlock * lw_ssync_rlock_new (lw_ssync * sync)
    { return (lw_ssync_rlock *) new Lacewing::SpinSync::ReadLock(*(Lacewing::SpinSync *) sync);
    }
void lw_ssync_rlock_delete (lw_ssync_rlock * lock)
    { delete (Lacewing::SpinSync::ReadLock *) lock;
    }
void lw_ssync_rlock_release (lw_ssync_rlock * lock)
    { ((Lacewing::SpinSync::ReadLock *) lock)->Release();
    }




