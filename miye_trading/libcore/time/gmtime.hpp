/*
 * gmtime.hpp
 *
 * Purpose: implementation from newlib of gmtime
 * without doing a systemcall
 *
 * Author: https://sourceware.org/newlib/ adapted for libcore
 * License: (c) 1981-2000 The Regents of the University of California
 *
 * (2) University of California, Berkeley
 *
 *   Copyright (c) 1981-2000 The Regents of the University of California.
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without modification,
 *   are permitted provided that the following conditions are met:
 *
 *       * Redistributions of source code must retain the above copyright notice,
 *         this list of conditions and the following disclaimer.
 *       * Redistributions in binary form must reproduce the above copyright notice,
 *         this list of conditions and the following disclaimer in the documentation
 *         and/or other materials provided with the distribution.
 *       * Neither the name of the University nor the names of its contributors
 *         may be used to endorse or promote products derived from this software
 *         without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 *   IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 *   INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 *   OF SUCH DAMAGE.
 *
 */

#pragma once
#include <time.h>

#ifdef __cplusplus
extern "C"
{
#endif
    namespace miye ::gmtime
    {

    struct tm* gmtime(const time_t* timer)
#ifdef __cplusplus
        noexcept
#endif
        ;
    struct tm* gmtime_r(const time_t* __restrict timer, struct tm* __restrict out)
#ifdef __cplusplus
        noexcept
#endif
        ;

    } // namespace miye::gmtime

#ifdef __cplusplus
}
#endif
