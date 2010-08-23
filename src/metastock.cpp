#include "metastock.h"

#include <math.h>

#include <QtCore/QDebug>
#include <QtCore/QStringList>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>

#include "ms_file.h"
#include "util.h"




Metastock::Metastock() :
	dir(NULL),
	master(NULL),
	emaster(NULL),
	xmaster(NULL),
	files( new QHash<QString, QFileInfo>() ),
	ba_master( new QByteArray() ),
	ba_emaster( new QByteArray() ),
	ba_xmaster( new QByteArray() ),
	error("")
{
#define MAX_MR_LEN 4096
	mr_len = 0;
	mr_list = (master_record**) calloc( MAX_MR_LEN, sizeof(master_record*) );
}



#define SAFE_DELETE( _p_ ) \
	if( _p_ != NULL ) { \
		delete _p_; \
	}


Metastock::~Metastock()
{
	delete ba_xmaster;
	delete ba_emaster;
	delete ba_master;
	delete files;
	SAFE_DELETE( xmaster );
	SAFE_DELETE( emaster );
	SAFE_DELETE( master );
	SAFE_DELETE( dir );
	
	for( int i = 0; i < MAX_MR_LEN; i++ ) {
		if( mr_list[i] != NULL ) {
			mr_len--;
			delete mr_list[i];
		}
	}
	Q_ASSERT( mr_len == 0);
	free( mr_list );
}


void Metastock::findFiles()
{
	QFileInfoList fil = dir->entryInfoList();
	foreach( QFileInfo fi, fil ) {
		QString key = fi.fileName().toUpper();
		Q_ASSERT( !files->contains(key) ); // TODO handle ambiguous file error
		files->insert( key, fi );
	}
}


QFile* Metastock::findMaster( const char* name ) const
{
	if( files->contains( QString(name)) ) {
		return new QFile( files->value(QString(name)).absoluteFilePath() );
	} else {
		return NULL;
	}
}


bool Metastock::setDir( const char* d )
{
	SAFE_DELETE( xmaster );
	SAFE_DELETE( emaster );
	SAFE_DELETE( master );
	SAFE_DELETE( dir );
	
	dir = new QDir(d);
	findFiles();
	
	master = findMaster( "MASTER" );
	if( master == NULL ) {
		error = "no MASTER found";
		return false;
	}
	emaster = findMaster( "EMASTER" );
	if( emaster == NULL ) {
		error = "no EMASTER found";
		return false;
	}
	// xmaster is optional
	xmaster = findMaster( "XMASTER" );
	
	readMasters();
	parseMasters();
	
	return true;
}


void readMaster( QFile *m, QByteArray *ba )
{
	if( m != NULL ) {
		m->open( QIODevice::ReadOnly );
		*ba = m->readAll ();
		Q_ASSERT( ba->size() == m->size() ); //TODO
	} else {
		ba->clear();
	}
}


void Metastock::parseMasters()
{
	{
		MasterFile mf( ba_master->constData(), ba_master->size() );
		int cntM = mf.countRecords();
		for( int i = 1; i<=cntM; i++ ) {
			master_record *mr = new master_record;
			mr_len++;
			mf.getRecord( mr, i );
			Q_ASSERT( mr_list[mr->file_number] == NULL );
			mr_list[mr->file_number] = mr;
		}
	}
	
	{
		EMasterFile emf( ba_emaster->constData(), ba_emaster->size() );
		int cntE = emf.countRecords();
		master_record tmp;
		for( int i = 1; i<=cntE; i++ ) {
			emf.getRecord( &tmp, i );
			Q_ASSERT( mr_list[tmp.file_number] != NULL );
			*mr_list[tmp.file_number] = tmp; // TODO should be a merge E->M
		}
	}
	
	if( xmaster != NULL ) {
		XMasterFile xmf( ba_xmaster->constData(), ba_xmaster->size() );
		int cntX = xmf.countRecords();
		for( int i = 1; i<=cntX; i++ ) {
			mr_len++;
			master_record *mr = new master_record;
			xmf.getRecord( mr, i );
			Q_ASSERT( mr_list[mr->file_number] == NULL );
			mr_list[mr->file_number] = mr;
		}
	}
}


void Metastock::readMasters()
{
	readMaster( master, ba_master );
	readMaster( emaster, ba_emaster );
	readMaster( xmaster, ba_xmaster );
}


const char* Metastock::lastError() const
{
	return error;
}


void Metastock::dumpMaster() const
{
	MasterFile mf( ba_master->constData(), ba_master->size() );
	mf.check();
}


void Metastock::dumpEMaster() const
{
	EMasterFile emf( ba_emaster->constData(), ba_emaster->size() );
	emf.check();
}


void Metastock::dumpXMaster() const
{
	XMasterFile xmf( ba_xmaster->constData(), ba_xmaster->size() );
	xmf.check();
}


void Metastock::dumpData( int f ) const
{
	if( f <= 0 ) {
		dumpDataDAT();
		if( hasXMaster() ) {
			dumpDataMWD();
		}
	} else if( f <= 255 ) {
		dumpDataDAT( f );
	} else {
		dumpDataMWD( f );
	}
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
	if( tmp > 0 ) {
		memcpy( cp, mr->c_long_name, tmp );
	} else {
		tmp = strlen(mr->c_short_name);
		memcpy( cp, mr->c_short_name, tmp );
	}
	cp += tmp;
	
	*cp = '\0';
	return cp - dst;
}


void Metastock::dumpDataPlus( int f ) const
{
	char buf[256];
	char *cp = buf;
	
	if( f!=0 ) {
		if( f > 0 && f < MAX_MR_LEN && mr_list[f] != NULL ) {
			cp += itoa( buf, f );
			cp++;
			Q_ASSERT( mr_list[f]->file_number == f );
			int len = build_mr_string( buf, mr_list[f] );
			fprintf( stderr , "#%d,%s, %d\n", f, buf, len  );
		} else {
			fprintf( stderr , "error, "
				"data file #%d not referenced by master files\n", f  );
		}
		return;
	}
	
	for( int i = 1; i<MAX_MR_LEN; i++ ) {
		if( i > 0 && i < MAX_MR_LEN && mr_list[i] != NULL ) {
			Q_ASSERT( mr_list[i]->file_number == i );
			int len = build_mr_string( buf, mr_list[i] );
			fprintf( stderr , "#%d,%s, %d\n", i, buf, len  );
		} else {
			fprintf( stderr , "error, "
				"data file #%d not referenced by master files\n", i  );
		}
	}
}


void Metastock::dumpDataDAT( int f ) const
{
	MasterFile mf( ba_master->constData(), ba_master->size() );
	EMasterFile emf( ba_emaster->constData(), ba_emaster->size() );
	int cntMaster = mf.countRecords();
	Q_ASSERT( cntMaster == emf.countRecords() );
	
	if( f!=0 ) {
		int num_m = mf.fileNumber( f );
		int num_e = emf.fileNumber( f );
		Q_ASSERT( num_m == num_e );
		int l_m = mf.dataLength( f );
		int l_e = emf.dataLength( f );
		Q_ASSERT( l_m == l_e );
		dumpData( num_m, l_m );
		return;
	}
	
	for( int i = 1; i<=cntMaster; i++ ) {
		int num_m = mf.fileNumber( i );
		int num_e = emf.fileNumber( i );
		Q_ASSERT( num_m == num_e );
		int l_m = mf.dataLength( i );
		int l_e = emf.dataLength( i );
		Q_ASSERT( l_m == l_e );
		dumpData( num_m, l_m );
	}
}


void Metastock::dumpDataMWD( int f ) const
{
	XMasterFile xmf( ba_xmaster->constData(), ba_xmaster->size() );
	int cntMaster = xmf.countRecords();
	
	if( f!=0 ) {
		int num_x = xmf.fileNumber( f );
		int l = xmf.dataLength( f );
		Q_ASSERT( num_x > 255 );
		dumpData( num_x, l );
		return;
	}
	
	for( int i = 1; i<=cntMaster; i++ ) {
		int num_x = xmf.fileNumber( i );
		int l = xmf.dataLength( i );
		Q_ASSERT( num_x > 255 );
		dumpData( num_x, l );
	}
}


void Metastock::dumpData( int n, int l ) const
{
	char tmp[17];
	
	if( n <= 255 ) {
		sprintf( tmp, "F%d.DAT", n ) ;
	} else {
		sprintf( tmp, "F%d.MWD", n );
	}
	
	QFile *fdat = findMaster( tmp );
	if( fdat == NULL ) {
		Q_ASSERT(false);
		error = "no fdat found";
		return /*false*/;
	}
	
	QByteArray ba_fdat;
	readMaster( fdat, &ba_fdat );
	
	FDat datfile( ba_fdat.constData(), ba_fdat.size(), l );
	fprintf( stdout, "%s: %d x %d bytes\n",
		tmp, datfile.countRecords(), l );
	
	datfile.checkHeader();
	datfile.print( tmp );
	
	delete fdat;

}


bool Metastock::hasXMaster() const
{
	return( xmaster != NULL  );
}
