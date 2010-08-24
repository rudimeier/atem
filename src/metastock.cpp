#include "metastock.h"

#include <string.h>
#include <stdlib.h>
#include <math.h>

#ifdef USE_FTS
	#include <fts.h>
#else 
	#include <dirent.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

#include <sys/stat.h>
#include <fcntl.h>

#include "ms_file.h"
#include "util.h"



#define MAX_FILE_LENGTH (1024*1024)


Metastock::Metastock() :
	dir(NULL),
	master_name(NULL),
	emaster_name(NULL),
	xmaster_name(NULL),
	ba_master( NULL ),
	master_len(0),
	ba_emaster( NULL ),
	emaster_len(0),
	ba_xmaster( NULL ),
	xmaster_len(0),
	ba_fdat( (char*) malloc( MAX_FILE_LENGTH ) ),
	error("")
{
#define MAX_MR_LEN 4096
	mr_len = 0;
	mr_list = (master_record*) calloc( MAX_MR_LEN, sizeof(master_record) );
}



#define SAFE_DELETE( _p_ ) \
	if( _p_ != NULL ) { \
		delete _p_; \
	}


Metastock::~Metastock()
{
	free( ba_fdat );
	free( ba_xmaster );
	free( ba_emaster);
	free( ba_master);
	free(xmaster_name);
	free(emaster_name);
	free(master_name);
	
	free( mr_list );
}


#ifdef USE_FTS

#define CHECK_MASTER( _dst_, _gen_name_ ) \
	if( strcasecmp(_gen_name_, node->fts_name) == 0 ) { \
		assert( _dst_ == NULL ); \
		_dst_ = (char*) malloc( node->fts_pathlen + 1 ); \
		strcpy( _dst_, node->fts_path   ); \
	}

void Metastock::findFiles()
{
	char *path_argv[] = { (char*)dir, NULL }; // TODO conversion to char*
	FTS *tree = fts_open( path_argv,
		FTS_NOCHDIR | FTS_LOGICAL | FTS_NOSTAT, NULL );
	if (!tree) {
		perror("fts_open");
		assert(false);
	}
	FTSENT *node;
	while ((node = fts_read(tree))) {
		if( (node->fts_level > 0) && (node->fts_info == FTS_D ) ) {
			fts_set(tree, node, FTS_SKIP);
		} else if( node->fts_info == FTS_F ) {
			if( (*node->fts_name == 'F' || *node->fts_name == 'f') &&
				node->fts_name[1] >= '1' && node->fts_name[1] <= '9') {
				char *c_number = node->fts_name + 1;
				char *end;
				long int number = strtol( c_number, &end, 10 );
				assert( number > 0 && number < MAX_MR_LEN && c_number != end );
				if( strcasecmp(end, ".MWD") == 0 || strcasecmp(end, ".DAT") == 0 ) {
					strcpy( mr_list[number].file_name, node->fts_path );
				}
			} else {
				CHECK_MASTER( master_name, "MASTER" );
				CHECK_MASTER( emaster_name, "EMASTER" );
				CHECK_MASTER( xmaster_name, "XMASTER" );
			}
		}
	}
	
	//TODO error handling
	if (errno) {
		perror("fts_read");
		assert( false );
	}
	
	if (fts_close(tree)) {
		perror("fts_close");
		assert( false );
	}
}

#undef CHECK_MASTER

#else /*USE_FTS*/

#define CHECK_MASTER( _dst_, _gen_name_ ) \
	if( strcasecmp(_gen_name_, dirp->d_name) == 0 ) { \
		assert( _dst_ == NULL ); \
		_dst_ = (char*) malloc( path_len + strlen(dirp->d_name) + 1 ); \
		strcpy( fullnamep, dirp->d_name ); \
		strcpy( _dst_, fullname ); \
	}

void Metastock::findFiles()
{
	DIR *dirh;
	struct dirent *dirp;
	
	if ((dirh = opendir( dir )) == NULL) {
		perror("opendir");
		exit(1);
	}
	
	char fullname[256];
	char *fullnamep = fullname;
	int path_len = strlen(dir);
	memcpy(fullnamep, dir, path_len );
	fullnamep += path_len;
	if( *(fullnamep - 1) != '/' ) {
		path_len++;
		*(fullnamep++) = '/';
	}

	for (dirp = readdir(dirh); dirp != NULL; dirp = readdir(dirh)) {
		if( ( dirp->d_name[0] == 'F' || dirp->d_name[0] == 'f') &&
			dirp->d_name[1] >= '1' && dirp->d_name[1] <= '9') {
			char *c_number = dirp->d_name + 1;
			char *end;
			long int number = strtol( c_number, &end, 10 );
			assert( number > 0 && number < MAX_MR_LEN && c_number != end );
			if( strcasecmp(end, ".MWD") == 0 || strcasecmp(end, ".DAT") == 0 ) {
				strcpy( fullnamep, dirp->d_name );
				strcpy( mr_list[number].file_name, fullname );
			}
		} else {
			CHECK_MASTER( master_name, "MASTER" );
			CHECK_MASTER( emaster_name, "EMASTER" );
			CHECK_MASTER( xmaster_name, "XMASTER" );
		}
	}
}

#undef CHECK_MASTER

#endif /*USE_FTS*/


bool Metastock::setDir( const char* d )
{
	dir = d;
	findFiles();
	
	if( master_name == NULL ) {
		error = "no MASTER found";
		return false;
	}
	if( emaster_name == NULL ) {
		error = "no EMASTER found";
		return false;
	}
	// xmaster is optional
	
	readMasters();
	parseMasters();
	
	return true;
}


void readMaster( const char *filename , char *buf, int *len )
{
	if( filename != NULL ) {
		int fd = open( filename, O_RDWR );
		if( fd < 0 ) {
			perror("error open");
			assert(false);
		}
		*len = read( fd, buf, MAX_FILE_LENGTH );
		fprintf( stderr, "READ = %d\n", *len);
		close( fd );
//		assert( ba->size() == rb ); //TODO
	} else {
// 		ba->clear();
	}
}


void Metastock::parseMasters()
{
	{
		EMasterFile emf( ba_emaster, emaster_len );
		int cntE = emf.countRecords();
		for( int i = 1; i<=cntE; i++ ) {
			master_record *mr = &mr_list[ emf.fileNumber(i) ];
			assert( mr->record_number == 0 );
			mr_len++;
			emf.getRecord( mr, i );
		}
	}
	
	{
		// NOTE this is just a check EMaster vs. Master
		MasterFile mf( ba_master, master_len );
		int cntM = mf.countRecords();
		for( int i = 1; i<=cntM; i++ ) {
			const master_record *mr = &mr_list[ mf.fileNumber(i) ];
			assert( mr->record_number != 0 );
			mf.getRecord( mr, i );
		}
	}
	
	if( hasXMaster() ) {
		XMasterFile xmf( ba_xmaster, xmaster_len );
		int cntX = xmf.countRecords();
		for( int i = 1; i<=cntX; i++ ) {
			master_record *mr = &mr_list[ xmf.fileNumber(i) ];
			assert( mr->record_number == 0 );
			mr_len++;
			xmf.getRecord( mr, i );
		}
	}
}


void Metastock::readMasters()
{
	ba_master = (char*) malloc( MAX_FILE_LENGTH ); //TODO
	readMaster( master_name, ba_master, &master_len );
	ba_emaster = (char*) malloc( MAX_FILE_LENGTH ); //TODO
	readMaster( emaster_name, ba_emaster, &emaster_len );
	ba_xmaster = (char*) malloc( MAX_FILE_LENGTH ); //TODO
	readMaster( xmaster_name, ba_xmaster,  &xmaster_len );
}


const char* Metastock::lastError() const
{
	return error;
}


void Metastock::dumpMaster() const
{
	MasterFile mf( ba_master, master_len );
	mf.check();
}


void Metastock::dumpEMaster() const
{
	EMasterFile emf( ba_emaster, emaster_len );
	emf.check();
}


void Metastock::dumpXMaster() const
{
	XMasterFile xmf( ba_xmaster, xmaster_len );
	xmf.check();
}


int build_mr_string( char *dst, const master_record *mr )
{
	char *cp = dst;
	int tmp = 0;
	
	tmp = strlen(mr->c_symbol);
	memcpy( cp, mr->c_symbol, tmp );
	cp += tmp;
	*cp++ = '\t';
	
	tmp = strlen(mr->c_long_name);
	memcpy( cp, mr->c_long_name, tmp );
	cp += tmp;
	
	*cp = '\0';
	return cp - dst;
}


void Metastock::dumpData( int f ) const
{
	char buf[256];
	
	if( f!=0 ) {
		if( f > 0 && f < MAX_MR_LEN && mr_list[f].record_number != 0 ) {
			assert( mr_list[f].file_number == f );
			int len = build_mr_string( buf, &mr_list[f] );
			dumpData( f, mr_list[f].field_bitset, buf );
		} else {
			fprintf( stderr , "error, "
				"data file #%d not referenced by master files\n", f  );
		}
		return;
	}
	
	for( int i = 1; i<MAX_MR_LEN; i++ ) {
		if( i > 0 && i < MAX_MR_LEN && mr_list[i].record_number != 0 ) {
			assert( mr_list[i].file_number == i );
			int len = build_mr_string( buf, &mr_list[i] );
			dumpData( i, mr_list[i].field_bitset, buf );
		}
	}
}



void Metastock::dumpData( int n, unsigned int fields, const char *pfx ) const
{
	const char* fdat_name = mr_list[n].file_name;
	if( fdat_name == NULL ) {
		assert(false);
		error = "no fdat found";
		return /*false*/;
	}
	
	int fdat_len = 0;
	readMaster( fdat_name, ba_fdat, &fdat_len );
	
	FDat datfile( ba_fdat, fdat_len, fields );
	fprintf( stdout, "#%d: %d x %d bytes\n",
		n, datfile.countRecords(), count_bits(fields) * 4 );
	
	datfile.checkHeader();
	datfile.print( pfx );
}


bool Metastock::hasXMaster() const
{
	return( xmaster_name != NULL  );
}
