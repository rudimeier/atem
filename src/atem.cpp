#include <QtCore/QCoreApplication>
#include <popt.h>
#include <stdlib.h>

#include "metastock.h"


#define VERSION_MAJOR 0
#define VERSION_MINOR 1
#define VERSION_RELEASE 0




static const char *ms_dirp = ".";
static int dumpmasterp = 0;
static int dumpemasterp = 0;
static int dumpxmasterp = 0;
static int dumpdatap = 0;




static void displayArgs( poptContext con, poptCallbackReason /*foo*/,
	poptOption *key, const char */*arg*/, void */*data*/ )
{
	if (key->shortName == 'h') {
		poptPrintHelp(con, stdout, 0);
	} else if (key->shortName == 'V') {
		fprintf(stdout, "GA Index Calcuator %d.%d.%d\n",
			VERSION_MAJOR, VERSION_MINOR, VERSION_RELEASE);
	} else {
		poptPrintUsage(con, stdout, 0);
	}
	
#if !defined(__LCLINT__)
	// XXX keep both splint & valgrind happy
	con = poptFreeContext(con);
#endif
	exit(0); // TODO we shouldn't exit here
	return;
}




static struct poptOption flow_opts[] = {
	{"msdir", 'i', POPT_ARG_STRING, &ms_dirp, 0,
		"input metastock directory", NULL},
	{"dump-masterp", 'm', POPT_ARG_NONE, &dumpmasterp, 0,
		"Dump MASTER file.", NULL},
	{"dump-emasterp", 'e', POPT_ARG_NONE, &dumpemasterp, 0,
		"Dump EMASTER file.", NULL},
	{"dump-xmasterp", 'x', POPT_ARG_NONE, &dumpxmasterp, 0,
		"Dump XMASTER file.", NULL},
	{"dump-data", 'd', POPT_ARG_NONE, &dumpdatap, 0,
		"Dump data files.", NULL},
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
	poptSetOtherOptionHelp(
		opt_ctx, "[options] [configuration file]");
	
	int rc;
	while( (rc = poptGetNextOpt(opt_ctx)) > 0 ) {
		// handle options when we have returning ones
		Q_ASSERT(false);
	}
	
	if( rc != -1 ) {
		fprintf( stderr, "error: %s '%s'\n",
			poptStrerror(rc), poptBadOption(opt_ctx, 0) );
		exit(2); // TODO we shouldn't exit here
	}
	
	const char** rest = poptGetArgs(opt_ctx);
	if( rest != NULL ) {
		fprintf( stderr, "error: bad usage\n" );
		exit(2);
	}
	
	return rest;
}




int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);
	
	const char **rest = atem_parse_cl(argc, (const char **)argv);
	Q_ASSERT( rest == NULL );
	
	Metastock ms;
	if( ! ms.setDir( ms_dirp ) ) {
		fprintf( stderr, "error: %s\n",
			ms.lastError() );
			exit(2);
	}
	
	if( dumpmasterp == 1 ) {
		ms.dumpMaster();
	}
	if( dumpemasterp == 1 ) {
		ms.dumpEMaster();
	}
	if( dumpxmasterp == 1 ) {
		ms.dumpXMaster();
	}
	if( dumpdatap == 1 ) {
		ms.dumpData();
	}
}
