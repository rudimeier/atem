/*** atem.cpp -- convert metastock to text
 *
 * Copyright (C) 2010-2017 Ruediger Meier
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
COLUMNS may be a list of strings, e.g. 'symbol,date,close'. Prepend '+' or\n\
'-' to in/exclude, e.g. 'all,-time' (or just '-time' to get the defaults but\n\
not time). Default is symbol and all date dependent columns (resp. all date\n\
independent ones if used with --symbols).\n\
\n\
BITSET controls the output columns. Specifying octal numbers (digits 0-7 and\n\
leading 0) is recommended. The first 3 octal digits (9 bits) are used for\n\
time series columns (date dependent). They are ignored if -s is used. All\n\
higher bits are used for symbol info (date independent).\n\
\n\
Report bugs to sweet_f_a@gmx.de\n\
Homepage: https://github.com/rudimeier/atem/\n"

#define VERSION_MSG \
PACKAGE_NAME " - metastock reverse (" PACKAGE_VERSION ")\n\
Copyright (C) 2010-2017 Ruediger Meier <sweet_f_a@gmx.de>\n\
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
		printf( "\n" BITSET_HELP_MSG );
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


static int ms2csv( const char *ms_dirp );


int main(int argc, char *argv[])
{
	int ret = 0;
	const char *ms_dirp = ".";

#ifdef _WIN32
	/* never write CRLF line feeds */
	_setmode(_fileno(stderr),_O_BINARY);
	_setmode(_fileno(stdout),_O_BINARY);
#endif

	atexit( gengetopt_free );

	if( cmdline_parser(argc, argv, &args_info) != 0 ) {
		ret = 2;
		goto end;
	}

	check_display_args();

	if( args_info.inputs_num == 1 ) {
		ms_dirp = args_info.inputs[0];
	} else if( args_info.inputs_num > 1 ) {
		fprintf( stderr, "error: bad usage\n" );
		ret = 2;
		goto end;
	}

	ret = ms2csv( ms_dirp );

end:
	/* TODO teach Metastock::setError() to distinguish usage and other errors */
	if( ret == 2 ) {
		fprintf( stderr, "Try `%s --help' for more information.\n", argv[0] );
	}
	return ret; // exit
}


static int ms2csv( const char *ms_dirp )
{
	Metastock ms;
	bool dumpdata = true;

	if( args_info.output_given ) {
		if( ! ms.set_outfile( args_info.output_arg ) ) {
			goto ms_error;
		}
	}

	if( !ms.set_ignore_masters( args_info.ignore_master_given,
		  args_info.ignore_emaster_given, args_info.ignore_xmaster_given) ) {
		goto ms_error;
	}

	if( ! ms.setDir( ms_dirp ) ) {
		goto ms_error;
	}

	if( args_info.field_separator_given ) {
		if( ! ms.set_field_sep(args_info.field_separator_arg) ) {
			goto ms_error;
		}
	}

	ms.set_skip_header( args_info.skip_header_given );

	if( !ms.set_out_format(
		  args_info.format_given ? args_info.format_arg : NULL) ) {
		goto ms_error;
	}

	if( !ms.setForceFloat(
			args_info.float_openint_given, args_info.float_volume_given) ) {
		goto ms_error;
	}

	if( args_info.fdat_given ) {
		if( ! ms.incudeFile( args_info.fdat_arg ) ) {
			goto ms_error;
		}
	}

	if( args_info.date_from_given ) {
		if( !ms.setPrintDateFrom( args_info.date_from_arg ) ) {
			goto ms_error;
		}
	}

	if( args_info.exclude_older_than_given ) {
		if( !ms.excludeFiles( args_info.exclude_older_than_arg ) ) {
			goto ms_error;
		}
	}

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
		ms.dumpXMaster();
	}

	if( args_info.symbols_given ) {
		dumpdata = false;
		if( ! ms.dumpSymbolInfo() ) {
			goto ms_error;
		}
	}

	if( dumpdata ) {
		if( ! ms.dumpData() ) {
			goto ms_error;
		}
	}

	return 0;

ms_error:
	fprintf( stderr, "error: %s\n", ms.lastError() );
	return 2;
}
