#include <popt.h>
#include <stdlib.h>
#include <assert.h>

#include "metastock.h"


#define VERSION_MAJOR 0
#define VERSION_MINOR 1
#define VERSION_RELEASE 0




static const char *ms_dirp = ".";
static int dumpmasterp = 0;
static int dumpemasterp = 0;
static int dumpxmasterp = 0;
static int dumpsymbolsp = -1;
static int dumpdatap = -1;
static const char* sepp = "\t";
static int format_datap = 0;
static int format_symbolsp = 0;
static const char *date_fromp = "";
static const char *exclude_older_thanp = "";




static void displayArgs( poptContext con, poptCallbackReason /*foo*/,
	poptOption *key, const char */*arg*/, void */*data*/ )
{
	if (key->shortName == 'h') {
		poptPrintHelp(con, stdout, 0);
	} else if (key->shortName == 'V') {
		fprintf(stdout, "atem - metastock reverse (%d.%d.%d)\n",
			VERSION_MAJOR, VERSION_MINOR, VERSION_RELEASE);
	} else {
		poptPrintUsage(con, stdout, 0);
	}
	
	poptFreeContext(con);
	exit(0);
}




static struct poptOption flow_opts[] = {
	{"msdir", 'i', POPT_ARG_STRING, &ms_dirp, 0,
		"input metastock directory", NULL},
	{"dump-master", 'm', POPT_ARG_NONE, &dumpmasterp, 0,
		"Dump MASTER file.", NULL},
	{"dump-emaster", 'e', POPT_ARG_NONE, &dumpemasterp, 0,
		"Dump EMASTER file.", NULL},
	{"dump-xmaster", 'x', POPT_ARG_NONE, &dumpxmasterp, 0,
		"Dump XMASTER file.", NULL},
	{"dump-symbols", 's', POPT_ARG_INT | POPT_ARGFLAG_OPTIONAL, &dumpsymbolsp, 0,
		"Dump all symbol info.", NULL},
	{"dump-data", 'd', POPT_ARG_INT | POPT_ARGFLAG_OPTIONAL, &dumpdatap, 0,
		"Dump data files.", NULL},
	{"field-separator", 'F', POPT_ARG_STRING, &sepp, 0,
		"field separator", NULL},
	{"format-data", 'f', POPT_ARG_INT, &format_datap, 0,
		"data output format", NULL},
	{"format-symbols", 'g', POPT_ARG_INT, &format_symbolsp, 0,
		"symbol info output format", NULL},
	{"date-from", '\0', POPT_ARG_STRING, &date_fromp, 0,
		"Print data from specified date on.", NULL},
	{"exclude-older-than", '\0', POPT_ARG_STRING, &exclude_older_thanp, 0,
		"Don't process data files older than specified seconds.", NULL},
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
	 "Program advice", NULL},
	{NULL, '\0', POPT_ARG_INCLUDE_TABLE, help_opts, 0,
	 "Help options", NULL},
	POPT_TABLEEND
};



static const char** atem_parse_cl(size_t argc, const char *argv[])
{
	poptContext opt_ctx;
	
	opt_ctx = poptGetContext(NULL, argc, argv, atem_opts, 0);
	poptSetOtherOptionHelp( opt_ctx, "[OPTION...]");
	
	int rc;
	while( (rc = poptGetNextOpt(opt_ctx)) > 0 ) {
		// handle options when we have returning ones
		assert(false);
	}
	
	if( rc != -1 ) {
		fprintf( stderr, "error: %s '%s'\n",
			poptStrerror(rc), poptBadOption(opt_ctx, 0) );
		poptFreeContext(opt_ctx);
		exit(2);
	}
	
	const char** rest = poptGetArgs(opt_ctx);
	if( rest != NULL ) {
		fprintf( stderr, "error: bad usage\n" );
		poptFreeContext(opt_ctx);
		exit(2);
	}
	
	// we cannot free it when have a non-NULL rest
	poptFreeContext(opt_ctx);
	return rest;
}




int main(int argc, const char *argv[])
{
	atem_parse_cl(argc, argv);
	
	Metastock ms;
	if( ! ms.setDir( ms_dirp ) ) {
		fprintf( stderr, "error: %s\n", ms.lastError() );
		return 2; // exit
	}
	
	if( ! ms.setOutputFormat( *sepp, format_datap, format_symbolsp ) ) {
		fprintf( stderr, "error: %s\n", ms.lastError() );
		return 2; // exit
	}
	
	//TODO maybe parameters of -s and -d should be another one
	if( dumpsymbolsp > 0 || dumpdatap > 0 ) {
		if( dumpsymbolsp > 0 && dumpdatap > 0 && dumpsymbolsp != dumpdatap ) {
			fprintf( stderr, "error: %s\n", "parameter s != d" );
			return 2; // exit
		}
		int f = dumpsymbolsp > dumpdatap ? dumpsymbolsp : dumpdatap;
		if( ! ms.incudeFile( f ) ) {
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
	
	if( dumpmasterp == 1 ) {
		ms.dumpMaster();
	}
	if( dumpemasterp == 1 ) {
		ms.dumpEMaster();
	}
	if( dumpxmasterp == 1 ) {
		if( ms.hasXMaster() ) {
			ms.dumpXMaster();
		} else {
			fprintf( stderr, "warning: %s\n", "no XMASTER found" );
		}
	}
	
	if( dumpsymbolsp >= 0 ) {
		if( ! ms.dumpSymbolInfo() ) {
			fprintf( stderr, "error: %s\n", ms.lastError() );
			return 2; // exit
		}
	}
	
	if( dumpdatap >= 0 ) {
		if( ! ms.dumpData() ) {
			fprintf( stderr, "error: %s\n", ms.lastError() );
			return 2; // exit
		}
	}
	
	return 0;
}
