#include "metastock.h"

#include <math.h>

#include <QtCore/QDebug>
#include <QtCore/QStringList>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>

#include "ms_file.h"




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
		int l_e = mf.dataLength( i );
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
