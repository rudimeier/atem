/*** atem.cpp -- convert metastock to text
 *
 * Copyright (C) 2010 - 2011 Ruediger Meier
 *
 * Author:  Ruediger Meier <sweet_f_a@gmx.de>
 *
 * This file is part of atem.
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
 * 3. Neither the name of the author nor the names of any contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ***/

#include <popt.h>
#include <stdlib.h>
#include <assert.h>

#include "config.h"
#include "metastock.h"



static poptContext opt_ctx;

static const char *ms_dirp = ".";
static int dumpmasterp = 0;
static int dumpemasterp = 0;
static int dumpxmasterp = 0;
static int dumpsymbolsp = 0;
static int skipheaderp = 0;
static const char* sepp = "\t";
static int format_datap = 0;
static const char *date_fromp = "";
static const char *exclude_older_thanp = "";
static int fdatp = -1;


#define BITSET_HELP_MSG "\
BITSET controls the output columns. Specifying octal numbers (digits 0-7 and\n\
leading 0) is recommended. The first 3 octal digits (9 bits) are used for\n\
time series columns (date dependent). They are ignored if -s is used. All\n\
higher bits are used for symbol info (date independent).\n\
\n\
Report bugs to sweet_f_a@gmx.de\n"

#define VERSION_MSG \
PACKAGE_NAME " - metastock reverse (" PACKAGE_VERSION ")\n\
Copyright (C) 2010-2011 Ruediger Meier <sweet_f_a@gmx.de>\n\
License: BSD 3-Clause\n"


static void displayArgs( poptContext con, poptCallbackReason /*foo*/,
	poptOption *key, const char */*arg*/, void */*data*/ )
{
	if (key->shortName == 'h') {
		poptPrintHelp(con, stdout, 0);
		fprintf(stdout, "\n" BITSET_HELP_MSG);
	} else if (key->shortName == 'V') {
		fprintf(stdout, VERSION_MSG);
	} else {
		poptPrintUsage(con, stdout, 0);
	}
	
	exit(0);
}




static struct poptOption flow_opts[] = {
	{"symbols", 's', POPT_ARG_NONE, &dumpsymbolsp, 0,
		"Dump symbol info instead of time series data.", NULL},
	{"skip-header", 'n', POPT_ARG_NONE, &skipheaderp, 0,
		"Don't print header row.", NULL},
	{"field-separator", 'F', POPT_ARG_STRING, &sepp, 0,
		"Field separator, default: TAB (ASCII).", "CHAR"},
	{"format", 'f', POPT_ARG_INT, &format_datap, 0,
		"Set output columns, default: 01377 (resp. 01777000 if used with -s). "
		"See BITSET format below.", "BITSET"},
	{"date-from", '\0', POPT_ARG_STRING, &date_fromp, 0,
		"Print data from specified date on (YYYY-MM-DD).", "DATE"},
	{"exclude-older-than", '\0', POPT_ARG_STRING, &exclude_older_thanp, 0,
		"Don't process data files older than date time (YYYY-MM-DD hh:mm:ss). "
		"A leading '-' reverts the statement.", "DATE"},
	{"fdat", '\0', POPT_ARG_INT, &fdatp, 0,
		"Process specified dat file number only.", NULL},
	POPT_TABLEEND
};

static struct poptOption debug_opts[] = {
	{"dump-master", 'm', POPT_ARG_NONE, &dumpmasterp, 0,
		"Dump MASTER file.", NULL},
	{"dump-emaster", 'e', POPT_ARG_NONE, &dumpemasterp, 0,
		"Dump EMASTER file.", NULL},
	{"dump-xmaster", 'x', POPT_ARG_NONE, &dumpxmasterp, 0,
		"Dump XMASTER file.", NULL},
	POPT_TABLEEND
};


static struct poptOption help_opts[] = {
	{NULL, '\0', POPT_ARG_CALLBACK, (void*)displayArgs, 0, NULL, NULL},
	{"help", 'h', POPT_ARG_NONE, NULL, 0, "Show this help message.", NULL},
	{"version", 'V', POPT_ARG_NONE, NULL, 0, "Print version string and exit.",
		NULL},
	{"usage", '\0', POPT_ARG_NONE, NULL, 0, "Display brief usage message."
		, NULL},
	POPT_TABLEEND
};




static const struct poptOption atem_opts[] = {
	{NULL, '\0', POPT_ARG_INCLUDE_TABLE, flow_opts, 0,
	 "Program advice:", NULL},
	{NULL, '\0', POPT_ARG_INCLUDE_TABLE, debug_opts, 0,
	 "Debug options:", NULL},
	{NULL, '\0', POPT_ARG_INCLUDE_TABLE, help_opts, 0,
	 "Help options:", NULL},
	POPT_TABLEEND
};



void clear_popt()
{
	poptFreeContext(opt_ctx);
}



void atem_parse_cl(size_t argc, const char *argv[])
{
	opt_ctx = poptGetContext(NULL, argc, argv, atem_opts, 0);
	atexit(clear_popt);
	
	poptSetOtherOptionHelp( opt_ctx, "[OPTION]... [DATA_DIR]");
	
	int rc;
	while( (rc = poptGetNextOpt(opt_ctx)) > 0 ) {
		// handle options when we have returning ones
		assert(false);
	}
	
	if( rc != -1 ) {
		fprintf( stderr, "error: %s '%s'\n",
			poptStrerror(rc), poptBadOption(opt_ctx, 0) );
		exit(2);
	}
	
	const char** rest = poptGetArgs(opt_ctx);
	if( rest != NULL && *rest != NULL ) {
		ms_dirp = *rest;
		rest++;
		if( *rest != NULL ) {
			fprintf( stderr, "error: bad usage\n" );
			exit(2);
		}
	}
}


int main(int argc, const char *argv[])
{
	atem_parse_cl(argc, argv);
	
	Metastock ms;
	if( ! ms.setDir( ms_dirp ) ) {
		fprintf( stderr, "error: %s\n", ms.lastError() );
		return 2; // exit
	}
	
	if( ! ms.setOutputFormat( *sepp, format_datap, skipheaderp ) ) {
		fprintf( stderr, "error: %s\n", ms.lastError() );
		return 2; // exit
	}
	
	if( fdatp > 0 ) {
		if( ! ms.incudeFile( fdatp ) ) {
			fprintf( stderr, "error: %s\n", ms.lastError() );
			return 2; // exit
		}
	}
	
	if( *date_fromp != '\0' ) {
		if( !ms.setPrintDateFrom( date_fromp ) ) {
			fprintf( stderr, "error: %s\n", ms.lastError() );
			return 2; // exit
		}
	}
	
	if( *exclude_older_thanp != '\0' ) {
		if( !ms.excludeFiles( exclude_older_thanp ) ) {
			fprintf( stderr, "error: %s\n", ms.lastError() );
			return 2; // exit
		}
	}
	
	bool dumpdata = true;
	if( dumpmasterp == 1 ) {
		dumpdata = false;
		ms.dumpMaster();
	}
	if( dumpemasterp == 1 ) {
		dumpdata = false;
		ms.dumpEMaster();
	}
	if( dumpxmasterp == 1 ) {
		dumpdata = false;
		if( ms.hasXMaster() ) {
			ms.dumpXMaster();
		}
	}
	
	if( dumpsymbolsp == 1 ) {
		dumpdata = false;
		if( ! ms.dumpSymbolInfo() ) {
			fprintf( stderr, "error: %s\n", ms.lastError() );
			return 2; // exit
		}
	}
	
	if( dumpdata ) {
		if( ! ms.dumpData() ) {
			fprintf( stderr, "error: %s\n", ms.lastError() );
			return 2; // exit
		}
	}
	
	return 0;
}
