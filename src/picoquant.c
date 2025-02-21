/*
 * Copyright (c) 2011-2014, Thomas Bischof
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 *    this list of conditions and the following disclaimer in the documentation 
 *    and/or other materials provided with the distribution.
 * 
 * 3. Neither the name of the Massachusetts Institute of Technology nor the 
 *    names of its contributors may be used to endorse or promote products 
 *    derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <time.h>

#include "picoquant.h"
#include "error.h"
#include "options.h"

#include "picoharp.h"
#include "hydraharp.h"
#include "timeharp.h"
#include "unified.h"

#include "interactive.h"
#include "continuous.h"
#include "t2.h"
#include "t3.h"

int pq_dispatch(FILE *stream_in, FILE *stream_out, options_t *options) {
	int result;
	pu_header_t pu_header;
	pq_header_t pq_header;
	pq_dispatch_t dispatch;

	result = pq_unified_header_read(stream_in, &pq_header, &pu_header);

	if ( result < 0 ) {
		error("Could not read string header.\n");
	} else if ( result == PQ_FORMAT_CLASSIC ) {
		dispatch = pq_dispatch_get(options, &pq_header);
		if ( dispatch == NULL ) {
			error("Could not identify board %s.\n", pq_header.Ident);
		} else {
			result = dispatch(stream_in, stream_out, 
					&pq_header, options);
		}
	} else if ( result == PQ_FORMAT_UNIFIED ) {
		result = pu_dispatch(stream_in, stream_out, &pu_header, options);
	} else {
		error("Unknown result for header (code %d)\n", result);
	}

	return(result);
}       


pq_dispatch_t pq_dispatch_get(options_t *options, pq_header_t *pq_header) {
	if ( ! strcmp(pq_header->Ident, "PicoHarp 300") ) {
		return(ph_dispatch);
	} else if ( ! strcmp(pq_header->Ident, "TimeHarp 200") ) {
		return(th_dispatch);
	} else if ( ! strcmp(pq_header->Ident, "HydraHarp") ) {
		return(hh_dispatch);
	} else { 
		return(NULL);
	}
}

void pq_resolution_print(FILE *stream_out, 
		int curve, float64_t resolution, 
		options_t *options) {
	if ( options->binary_out ) {
		fwrite(&resolution, 1, sizeof(resolution), stream_out);
	} else {
		if ( curve >= 0 ) {
			fprintf(stream_out, "%d,", curve);
		}
		fprintf(stream_out, "%.2"PRIf64"\n", resolution);
	}
}
	
