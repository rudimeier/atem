/*** atem.cpp -- convert metastock to text
 *
 * Copyright (C) 2010-2012 Ruediger Meier
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

#if defined HAVE_CONFIG_H
# include "config.h"
#endif	// HAVE_CONFIG_H
#include <stdlib.h>
#include <assert.h>

#include "atem_ggo.h"
#include "config.h"
#include "metastock.h"

#ifdef _WIN32
	#include <fcntl.h>
	#include <io.h>
#endif

#include "atem_ggo.c"




static gengetopt_args_info args_info;


#define BITSET_HELP_MSG "\
BITSET controls the output columns. Specifying octal numbers (digits 0-7 and\n\
leading 0) is recommended. The first 3 octal digits (9 bits) are used for\n\
time series columns (date dependent). They are ignored if -s is used. All\n\
higher bits are used for symbol info (date independent).\n\
\n\
"

#define UTERUS_HELP_MSG "\
This version of " PACKAGE_NAME " supports uterus output.  In addition\n\
to BITSET the option -f|--format allows the string `ute' to indicate\n\
that output should go into an ute file.\n\
\n\
"

#define BUG_MSG	"\
Report bugs to sweet_f_a@gmx.de\n\
"

#define VERSION_MSG \
PACKAGE_NAME " - metastock reverse (" PACKAGE_VERSION ")\n\
Copyright (C) 2010-2012 Ruediger Meier <sweet_f_a@gmx.de>\n\
License: BSD 3-Clause\n"


static void check_display_args()
{
	if( args_info.full_help_given || args_info.help_given ) {
		gengetopt_args_info_usage = "Usage: " PACKAGE " [OPTION]... [DATA_DIR]";
		if( args_info.full_help_given ) {
			cmdline_parser_print_full_help();
		} else {
			cmdline_parser_print_help();
		}
		puts( "\n"
		      BITSET_HELP_MSG
#if defined HAVE_UTERUS
		      UTERUS_HELP_MSG
#endif	// HAVE_UTERUS
		      BUG_MSG
		      );
	} else if( args_info.usage_given ) {
		printf( "%s\n", gengetopt_args_info_usage );
	} else if( args_info.version_given ) {
		printf( VERSION_MSG );
	} else {
		return;
	}
	
	exit(0);
}


static void gengetopt_free()
{
	cmdline_parser_free( &args_info );
}


int main(int argc, char *argv[])
{
	bool dump_csv = false;
	bool dump_ute = false;
	int fmt_data = 0;

#ifdef _WIN32
	/* never write CRLF line feeds */
	_setmode(_fileno(stderr),_O_BINARY);
	_setmode(_fileno(stdout),_O_BINARY);
#endif
	
	atexit( gengetopt_free );
	
	if( cmdline_parser(argc, argv, &args_info) != 0 ) {
		return 2; // exit
	}
	
	check_display_args();
	
	const char *ms_dirp = ".";
	if( args_info.inputs_num == 1 ) {
		ms_dirp = args_info.inputs[0];
	} else if( args_info.inputs_num > 1 ) {
		fprintf( stderr, "error: bad usage\n" );
		return 2; // exit
	}
	
	Metastock ms;
	
	if( args_info.output_given ) {
		if( ! ms.set_outfile( args_info.output_arg ) ) {
			fprintf( stderr, "error: %s\n", ms.lastError() );
			return 2; // exit
		}
	}
	
	if( ! ms.setDir( ms_dirp ) ) {
		fprintf( stderr, "error: %s\n", ms.lastError() );
		return 2; // exit
	}
	
	if( args_info.format_given &&
	    strcmp( args_info.format_arg, "ute" ) == 0 ) {
#if defined HAVE_UTERUS
		dump_ute = true;
#else  // !HAVE_UTERUS
		fprintf( stderr, "error: no ute support in this version of "
			 PACKAGE_NAME "\n" );
		return 2; // exit
#endif	// HAVE_UTERUS
	} else if( args_info.format_given &&
		   ({
			   char *on;
			   fmt_data = strtol( args_info.format_arg, &on, 0 );
			   *on;
		   }) ) {
		fprintf( stderr, "error: invalid numeric value: %s\n",
			 args_info.format_arg );
		return 2; // exit
	} else if( !ms.setOutputFormat(
		  args_info.field_separator_given ?*args_info.field_separator_arg :'\t',
		  fmt_data,
		  args_info.skip_header_given ) ) {
		fprintf( stderr, "error: %s\n", ms.lastError() );
		return 2; // exit
	} else {
		// all went well
		dump_csv = true;
	}

	if( !ms.setForceFloat(
			args_info.float_opi_given, args_info.float_vol_given) ) {
		fprintf( stderr, "error: %s\n", ms.lastError() );
		return 2; // exit
	}

	if( args_info.fdat_given ) {
		if( ! ms.incudeFile( args_info.fdat_arg ) ) {
			fprintf( stderr, "error: %s\n", ms.lastError() );
			return 2; // exit
		}
	}
	
	if( args_info.date_from_given ) {
		if( !ms.setPrintDateFrom( args_info.date_from_arg ) ) {
			fprintf( stderr, "error: %s\n", ms.lastError() );
			return 2; // exit
		}
	}
	
	if( args_info.exclude_older_than_given ) {
		if( !ms.excludeFiles( args_info.exclude_older_than_arg ) ) {
			fprintf( stderr, "error: %s\n", ms.lastError() );
			return 2; // exit
		}
	}
	
	bool dumpdata = true;
	if( args_info.dump_master_given ) {
		dumpdata = false;
		ms.dumpMaster();
	}
	if( args_info.dump_emaster_given ) {
		dumpdata = false;
		ms.dumpEMaster();
	}
	if( args_info.dump_xmaster_given ) {
		dumpdata = false;
		if( ms.hasXMaster() ) {
			ms.dumpXMaster();
		}
	}
	
	if( args_info.symbols_given ) {
		dumpdata = false;
		if( ! ms.dumpSymbolInfo() ) {
			fprintf( stderr, "error: %s\n", ms.lastError() );
			return 2; // exit
		}
	}
	
	if( dumpdata &&
	    ( dump_csv && ! ms.dumpData() ) ||
	    ( dump_ute && ! ms.dumpUte() ) ) {
		fprintf( stderr, "error: %s\n", ms.lastError() );
		return 2; // exit
	}
	
	return 0;
}
