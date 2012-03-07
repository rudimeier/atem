/*** metastock.cpp -- parsing metastock directory
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
#include "metastock.h"

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <dirent.h>

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <limits.h>

#include "ms_file.h"
#include "util.h"



#define READ_BLCKSZ 16384


class FileBuf
{
	public:
		FileBuf();
		~FileBuf();
		
		bool hasName() const;
		const char* constName() const;
		const char* constBuf() const;
		int len() const;
		
		void setName( const char* file_name );
		
		int readFile( int fildes );
		
	private:
		void resize( int size );
		
		char name[MAX_LEN_MR_FILENAME + 1];
		char *buf;
		int buf_len;
		int buf_size;
};

FileBuf::FileBuf() :
	buf( NULL ),
	buf_len(0),
	buf_size(0)
{
	*name = 0;
}

FileBuf::~FileBuf()
{
	free(buf);
}

bool FileBuf::hasName() const
{
	return (*name != 0);
}

const char* FileBuf::constName() const
{
	return name;
}

const char* FileBuf::constBuf() const
{
	return buf;
}

int FileBuf::len() const
{
	return buf_len;
}

void FileBuf::setName( const char* file_name )
{
	buf_len = 0;
	strcpy( name, file_name );
}

int FileBuf::readFile( int fildes )
{
	char *cp = buf;
	buf_len = 0;
	int tmp_len;
	do {
		if( buf_len + READ_BLCKSZ > buf_size ) {
			resize( buf_size + READ_BLCKSZ );
			cp = buf + buf_len;
		}
		tmp_len = read( fildes, cp, READ_BLCKSZ );
		buf_len += tmp_len;
		cp += tmp_len;
	} while( tmp_len > 0 );
	
	// tmp_len < 0 is an error with errno set
	return tmp_len;
}


void FileBuf::resize( int size )
{
	buf = (char*) realloc( buf, size );
	buf_size = size;
}




bool Metastock::print_header = true;
char Metastock::print_sep = '\t';
unsigned short Metastock::prnt_master_fields = 0xFFFF;
unsigned char Metastock::prnt_data_fields = 0xFF;
unsigned short Metastock::prnt_data_mr_fields = M_SYM;


Metastock::Metastock() :
	print_date_from(0),
	ms_dir(NULL),
	m_buf( new FileBuf() ),
	e_buf( new FileBuf() ),
	x_buf( new FileBuf() ),
	fdat_buf( new FileBuf() ),
	out( stdout )
{
	error[0] = '\0';
/* dat file numbers are unsigned short only */
#define MAX_DAT_NUM 0xFFFF
	max_dat_num = 0;
	mr_len = 0;
	mr_list = NULL;
	mr_skip_list = NULL;
}



#define SAFE_DELETE( _p_ ) \
	if( _p_ != NULL ) { \
		delete _p_; \
	}


Metastock::~Metastock()
{
	free( mr_skip_list );
	free( mr_list );
	
	delete( fdat_buf );
	delete( x_buf );
	delete( e_buf );
	delete( m_buf );
	free( ms_dir );
	
	/* out is either stdout or a real file which was opened in set_outfile() */
	if( out != stdout && out != NULL ) {
		fclose( (FILE*)out );
	}
}


#define CHECK_MASTER( _file_buf_, _gen_name_ ) \
	if( strcasecmp(_gen_name_, dirp->d_name) == 0 ) { \
		assert( !_file_buf_->hasName() ); \
		_file_buf_->setName( dirp->d_name ); \
	}

bool Metastock::findFiles()
{
	DIR *dirh;
	struct dirent *dirp;
	
	if ((dirh = opendir( ms_dir )) == NULL) {
		setError( ms_dir, strerror(errno) );
		return false;
	}
	
	for (dirp = readdir(dirh); dirp != NULL; dirp = readdir(dirh)) {
		if( ( dirp->d_name[0] == 'F' || dirp->d_name[0] == 'f') &&
			dirp->d_name[1] >= '1' && dirp->d_name[1] <= '9') {
			char *c_number = dirp->d_name + 1;
			char *end;
			long int number = strtol( c_number, &end, 10 );
			assert( number > 0 && c_number != end );
			if( (strcasecmp(end, ".MWD") == 0 || strcasecmp(end, ".DAT") == 0)
					&& number <= MAX_DAT_NUM ) {
				add_mr_list_datfile( number, dirp->d_name );
			}
		} else {
			CHECK_MASTER( m_buf, "MASTER" );
			CHECK_MASTER( e_buf, "EMASTER" );
			CHECK_MASTER( x_buf, "XMASTER" );
		}
	}
	
	closedir( dirh );
	return true;
}

#undef CHECK_MASTER


bool Metastock::set_outfile( const char *file )
{
	int fd = open( file,
#if defined _WIN32
		_O_WRONLY | _O_CREAT |O_TRUNC | _O_BINARY );
#else
		O_WRONLY | O_CREAT | O_TRUNC , 0666 );
#endif
	if( fd < 0 ) {
		setError( file, strerror(errno) );
		return false;
	}
	
	out = fdopen( fd, "wb");
	if( out == NULL ) {
		setError( file, strerror(errno) );
		return false;
	}
	
	return true;
}


bool Metastock::setDir( const char* d )
{
	// set member ms_dir inclusive trailing '/'
	int dir_len = strlen(d);
	ms_dir = (char*) realloc( ms_dir, dir_len + 2 );
	strcpy( ms_dir, d );
	if( ms_dir[ dir_len - 1] != '/' ) {
		ms_dir[dir_len] = '/';
		ms_dir[dir_len + 1] = '\0';
	}
	
	if( !findFiles() ) {
		return false;
	}
	
	if( !readMasters() ){
		return false;
	}
	if( !parseMasters() ) {
		return false;
	}
	
	FDat::set_outfile( out );
	return true;
}


bool Metastock::setOutputFormat( char sep, int fmt_data, int skipheader )
{
	print_sep = sep;
	print_header = !skipheader;
	
	if( fmt_data < 0 ) {
		setError( "wrong output format" );
		return false;
	}
	
	if( fmt_data ) {
		prnt_master_fields = fmt_data >> 9;
		prnt_data_fields = fmt_data;
		prnt_data_mr_fields = prnt_master_fields;
	}
	
	FDat::initPrinter( sep, prnt_data_fields );
	return true;
}


bool Metastock::setForceFloat( bool opi, bool vol )
{
	if( opi ) {
		FDat::setForceFloat(D_OPI);
	}
	if( vol ) {
		FDat::setForceFloat(D_VOL);
	}
	return true;
}


bool Metastock::readFile( FileBuf *file_buf ) const
{
	// build file name with full path
	char puff[strlen(ms_dir) + strlen(file_buf->constName()) + 1];
	char *file_path = puff;
	strcpy( file_path, ms_dir );
	strcpy( file_path + strlen(ms_dir), file_buf->constName() );
	
#if defined _WIN32
	int fd = open( file_path, _O_RDONLY | _O_BINARY );
#else
	int fd = open( file_path, O_RDONLY );
#endif
	if( fd < 0 ) {
		setError( file_path, strerror(errno) );
		return false;
	}
	int err = file_buf->readFile( fd );
	if( err < 0 ) {
		setError( file_path, strerror(errno) );
	}
	
	close( fd );
	
	return (err >= 0);
}


#define DEBUG_MASTER( _buf_, _cnt_ ) \
	if( _cnt_ <= 0 && _buf_->hasName() ) { \
		printWarn( _buf_->constName(), "not usable"); \
	}

#define SELECT_MR( _master_ ) \
	do { \
		int datnum = _master_.fileNumber(i); \
		if( mr_len <= datnum ) { \
			resize_mr_list(datnum + 128); \
		} \
		mr = &mr_list[ datnum ]; \
	} while( false )


bool Metastock::parseMasters()
{
	MasterFile mf( m_buf->constBuf(), m_buf->len() );
	EMasterFile emf( e_buf->constBuf(), e_buf->len() );
	XMasterFile xmf( x_buf->constBuf(), x_buf->len() );
	int cntM = mf.countRecords();
	int cntE = emf.countRecords();
	int cntX = xmf.countRecords();
	
	if( cntM <= 0 && cntE <= 0 && cntX <= 0 ) {
		setError( "all *Master files invalid" );
		return false;
	}
	
	DEBUG_MASTER( m_buf, cntM );
	DEBUG_MASTER( e_buf, cntE );
	DEBUG_MASTER( x_buf, cntX );
	
	master_record *mr;
	
	if( cntM > 0 ) {
		/* we prefer to use Master because EMaster is often broken */
		for( int i = 1; i<=cntM; i++ ) {
			SELECT_MR( mf );
			assert( mr->record_number == 0 );
			mf.getRecord( mr, i );
		}
		if( cntE == cntM ) {
			/* EMaster seems to be usable - fill up long names */
			for( int i = 1; i<=cntE; i++ ) {
				SELECT_MR( emf );
				assert( mr->record_number != 0 );
				emf.getLongName( mr, i );
			}
		}
	} else if ( cntE > 0 ) {
		/* Master is broken - use EMaster */
		for( int i = 1; i<=cntE; i++ ) {
			SELECT_MR( emf );
			assert( mr->record_number == 0 );
			emf.getRecord( mr, i );
		}
	} /* else neither Master or EMaster is valid */
	
	if( cntX > 0 ) {
		/* XMaster is optional */
		for( int i = 1; i<=cntX; i++ ) {
			SELECT_MR( xmf );
			assert( mr->record_number == 0 );
			xmf.getRecord( mr, i );
		}
	}
	
	return true;
}

#undef DEBUG_MASTER
#undef SELECT_MR


bool Metastock::readMasters()
{
	if( !m_buf->hasName() && !e_buf->hasName() && !x_buf->hasName() ) {
		setError( "no *Master files found" );
		return false;
	}
	
	if( m_buf->hasName() ) {
		if( !readFile( m_buf ) ) {
			return false;
		}
	} else {
		printWarn("Master file not found");
	}
	
	if( e_buf->hasName() ) {
		if( !readFile( e_buf ) ) {
			return false;
		}
	}else {
		printWarn("EMaster file not found");
	}
	
	if( x_buf->hasName() ) {
		if( !readFile( x_buf ) ) {
			return false;
		}
	} else if( max_dat_num > 255 ) {
		printWarn("XMaster file not found");
	}
	
	return true;
}


const char* Metastock::lastError() const
{
	return error;
}


void Metastock::printWarn( const char* e1, const char* e2 ) const
{
	if( e2 == NULL || *e2 == '\0' ) {
		fprintf( stderr, "warning: %s\n", e1);
	} else {
		fprintf( stderr, "warning: %s: %s\n", e1, e2 );
	}
}


void Metastock::setError( const char* e1, const char* e2 ) const
{
	if( e2 == NULL || *e2 == '\0' ) {
		snprintf( error, ERROR_LENGTH, "%s", e1);
	} else {
		snprintf( error, ERROR_LENGTH, "%s: %s", e1, e2 );
	}
}


void Metastock::dumpMaster() const
{
	MasterFile mf( m_buf->constBuf(), m_buf->len() );
	mf.check();
}


void Metastock::dumpEMaster() const
{
	EMasterFile emf( e_buf->constBuf(), e_buf->len() );
	emf.check();
}


void Metastock::dumpXMaster() const
{
	XMasterFile xmf( x_buf->constBuf(), x_buf->len() );
	xmf.check();
}


bool Metastock::incudeFile( int f ) const
{
	for( int i = 1; i< mr_len; i++ ) {
			mr_skip_list[i] = true;
	}
	
	if( f > 0 && f < mr_len && mr_list[f].record_number != 0 ) {
		mr_skip_list[f] = false;
		return true;
	} else {
		setError("data file not referenced by master files");
		return false;
	}
}


time_t str2time( const char* s)
{
	struct tm dt;
	time_t dt_t;
	memset( &dt, 0, sizeof(tm) );
	
	int ret = sscanf( s, "%d-%d-%d %d:%d:%d", &dt.tm_year,
		&dt.tm_mon, &dt.tm_mday, &dt.tm_hour, &dt.tm_min, &dt.tm_sec );
	
	if( ret < 0 ) {
		return -1;
	} else if( ret != 6  && ret != 3 ) {
		return -1;
	}
	
	
	dt.tm_year -= 1900;
	dt.tm_mon -= 1;
	dt.tm_isdst = -1;
	
	dt_t = mktime( &dt );
	
	return dt_t;
}


int str2date( const char* s)
{
	int y, m, d;
	y = m = d = 0;
	
	int ret = sscanf( s, "%d-%d-%d", &y, &m, &d );
	
	if( ret != 3 ) {
		return -1;
	}
	
	if( !(y>=0 && y<=9999) ||  !(m>=1 && m<=12 ) || !(d>=1 && d<=31) ) {
		return -1;
	}
	
	return 10000 * y + 100 * m + d;
}


bool Metastock::setPrintDateFrom( const char *date )
{
	int dt = str2date( date );
	if( dt < 0 ) {
		setError("parsing date time");
		return false;
	}
	print_date_from = dt;
	FDat::setPrintDateFrom( dt );
	return true;
}


bool Metastock::excludeFiles( const char *stamp ) const
{
	bool revert = false;
	if( *stamp == '-' ) {
		stamp++;
		revert = true;
	}
	time_t oldest_t = str2time( stamp );
	if( oldest_t < 0 ) {
		setError("parsing date time");
		return false;
	}
	
	for( int i = 1; i<mr_len; i++ ) {
		if( *mr_list[i].file_name == '\0' || mr_skip_list[i] ) {
			continue;
		}
		assert( mr_list[i].file_number == i );
		
		char puff[strlen(ms_dir) + strlen( mr_list[i].file_name) + 1];
		char *file_path = puff;
		strcpy( file_path, ms_dir );
		strcpy( file_path + strlen(ms_dir), mr_list[i].file_name );
		
		struct stat s;
		int tmp = stat( file_path, &s );
		if( tmp < 0 ) {
			setError( file_path,  strerror(errno) );
			return false;
		}
		if( !revert ) {
			if( oldest_t > s.st_mtime ) {
				mr_skip_list[i] = true;
			}
		} else {
			if( oldest_t <= s.st_mtime ) {
				mr_skip_list[i] = true;
			}
		}
	}
	return true;
}


bool Metastock::dumpSymbolInfo() const
{
	char buf[MAX_SIZE_MR_STRING + 1];
	int len;
	
	if( print_header ) {
		len = mr_header_to_string( buf, prnt_master_fields, print_sep );
		buf[len++] = '\n';
		buf[len] = '\0';
		fputs( buf, (FILE*)out );
	}
	
	for( int i = 1; i<mr_len; i++ ) {
		if( mr_list[i].record_number != 0 && !mr_skip_list[i] ) {
			assert( mr_list[i].file_number == i );
			len = mr_record_to_string( buf, &mr_list[i],
				prnt_master_fields, print_sep );
			buf[len++] = '\n';
			buf[len] = '\0';
			fputs( buf, (FILE*)out );
		}
	}
	return true;
}


void Metastock::resize_mr_list( int new_len )
{
	mr_list = (master_record*) realloc( mr_list,
		new_len * sizeof(master_record) );
	mr_skip_list = (bool*) realloc( mr_skip_list,
		new_len * sizeof(bool) );
	
	memset( mr_list + mr_len, '\0',
		(new_len - mr_len) * sizeof(master_record) );
	memset( mr_skip_list + mr_len, '\0',
		(new_len - mr_len) * sizeof(bool) );
	
	mr_len = new_len;
}


void Metastock::add_mr_list_datfile(  int datnum, const char* datname )
{
	if( datnum > max_dat_num ) {
		max_dat_num = datnum;
	}
	if( mr_len <= datnum ) {
		/* increase by 128 instead of 1 to avoid some reallocs */
		resize_mr_list(datnum + 128);
	}
	strcpy( mr_list[datnum].file_name, datname );
}



bool Metastock::dumpData() const
{
	char buf[256];
	int len;
	
	if( print_header ) {
		len = mr_header_to_string( buf, prnt_data_mr_fields, print_sep );
		if( len > 0 ) {
			buf[len++] = print_sep;
			buf[len] = '\0';
		}
		FDat::print_header( buf );
	}
	
	for( int i = 1; i<mr_len; i++ ) {
		if( mr_list[i].record_number != 0 && !mr_skip_list[i] ) {
			assert( mr_list[i].file_number == i );
			len = mr_record_to_string( buf, &mr_list[i],
				prnt_data_mr_fields, print_sep );
			if( len > 0 ) {
				buf[len++] = print_sep;
				buf[len] = '\0';
			}
			if( !dumpData( i, mr_list[i].field_bitset, buf ) ) {
				return false;
			}
		}
	}
	return true;
}



bool Metastock::dumpData( unsigned short n, unsigned char fields, const char *pfx ) const
{
	fdat_buf->setName( mr_list[n].file_name );
	
	if( !fdat_buf->hasName() ) {
		setError( "no fdat found" );
		return false;
	}
	
	if( ! readFile( fdat_buf ) ) {
		return false;
	}
	
	FDat datfile( fdat_buf->constBuf(), fdat_buf->len(), fields );
// 	fprintf( stderr, "#%d: %d x %d bytes\n",
// 		n, datfile.countRecords(), count_bits(fields) * 4 );
	
	datfile.checkHeader();
	if( datfile.print( pfx ) < 0) {
		/* This is should only happen on WIN32 instead of SIGPIPE */
		setError( "writing interrupted" );
		return false;
	}
	
	return true;
}


bool Metastock::hasXMaster() const
{
	return( x_buf->hasName() );
}

#if defined HAVE_UTERUS
#include <uterus.h>
#include <m30.h>

struct rec_clo_s {
	utectx_t hdl;
	uint16_t idx;
	uint16_t ttf;
	uint32_t len;
};

bool Metastock::dumpUte() const
{
	struct rec_clo_s clo;
	bool res = true;
	int len;
		
	if ((clo.hdl = ute_mktemp(UO_RDWR)) == NULL) {
		setError( "ute_mktemp() b0rked" );
		return false;
	}

	// hand out the name of the file
	puts(ute_fn(clo.hdl));

	for( int i = 1; i<mr_len; i++ ) {
		if( mr_list[i].record_number != 0 && !mr_skip_list[i] ) {
			char buf[256];

			assert( mr_list[i].file_number == i );
			len = mr_record_to_string( buf, &mr_list[i],
				prnt_data_mr_fields, print_sep );
			if (len >= SLUT_SYMLEN) {
				// cut off after SLUT_SYMLEN
				buf[SLUT_SYMLEN] = '\0';
			} else if (len > 0) {
				buf[len] = '\0';
			} else {
				// no symbol, this is somewhat alarming :|
			}

			// get us an ute index for this one
			clo.idx = ute_sym2idx(clo.hdl, buf);
			// also it's always candles of trades or something
			clo.ttf = SCDL_FLAVOUR | SL1T_TTF_FIX;
			// candles are assumed to be daily
			clo.len = 86400;

			if( !dumpUte( i, &clo ) ) {
				res = false;
				goto out;
			}
		}
	}
out:
	ute_close(clo.hdl);
	return res;
}

static int
rec_cb(struct glue_s g, void *clo)
{
// add data in G to ute handle in CLO
// at the moment, we assume candles in G and CLO
	struct rec_clo_s *rec_clo = (struct rec_clo_s*)clo;
	struct scdl_s sn[1];
	time_t unx;

	// date fiddling, metastock uses bcd-dates
	{
		struct tm tm;

		tm.tm_year = (g.date / 10000) - 1900;
		tm.tm_mon = (g.date / 100) % 100 - 1;
		tm.tm_mday = (g.date % 100);
		tm.tm_hour = (g.time / 10000);
		tm.tm_min = (g.time / 100) % 100;
		tm.tm_sec = (g.time % 100);

		tm.tm_wday = /*bugger*/0;
		tm.tm_yday = /*bugger*/0;
		tm.tm_isdst = /*assume UTC*/0;
		unx = timegm(&tm);

		// candles store the end stamp
		scdl_set_stmp_sec(sn, unx + rec_clo->len);
		// no millisecond resolution in metastock
		scdl_set_stmp_msec(sn, 0);
	}

	// finish candle header set up
	scdl_set_tblidx(sn, rec_clo->idx);
	scdl_set_ttf(sn, rec_clo->ttf);

	// fill in the rest of the candle, payload
	sn->o = ffff_m30_get_f(g.open).u;
	sn->h = ffff_m30_get_f(g.high).u;
	sn->l = ffff_m30_get_f(g.low).u;
	sn->c = ffff_m30_get_f(g.close).u;
	sn->cnt = ffff_m30_get_f(g.volume).u;
	// start stamp
	sn->sta_ts = unx;

	// and off we dump this to ute
	ute_add_tick(rec_clo->hdl, AS_SCOM(sn));
	return 0;
}

bool Metastock::dumpUte( unsigned short n, void *clo ) const
{
	fdat_buf->setName( mr_list[n].file_name );
	
	if( !fdat_buf->hasName() ) {
		setError( "no fdat found" );
		return false;
	}
	
	if( ! readFile( fdat_buf ) ) {
		return false;
	}
	
	{
		unsigned char flds = mr_list[n].field_bitset;
		FDat datfile( fdat_buf->constBuf(), fdat_buf->len(), flds );
		
		datfile.checkHeader();
		datfile.iter( rec_cb, clo );
	}
	return true;
}

#else  // !HAVE_UTERUS

bool Metastock::dumpUte() const
{
	// trivial response in this case
	setError( "no ute support in this version of atem" );
	return false;
}

bool Metastock::dumpUte( unsigned short n, unsigned char flds, const char *pfx ) const
{
	// trivial response in this case
	setError( "no ute support in this version of atem" );
	return false;
}

#endif	// HAVE_UTERUS
