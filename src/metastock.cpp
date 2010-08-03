#include "metastock.h"

#include <QtCore/QDebug>
#include <QtCore/QStringList>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>




class MasterFile
{
	public:
		MasterFile( const char *buf, int size );
		
		static bool checkHeader( const char* buf );
		static bool checkRecord( const char* buf, int record  );
		
	private:
		const char * const buf;
		const int size;
};


MasterFile::MasterFile( const char *_buf, int _size ) :
	buf( _buf ),
	size( _size )
{
}


bool MasterFile::checkHeader( const char* buf )
{
	unsigned char countRecords = buf[0];
	Q_ASSERT( buf[1] == '\0' );
	unsigned char lastUsedRecord = buf[2];
	Q_ASSERT( buf[3] == '\0' );
	Q_ASSERT( countRecords == lastUsedRecord );
	for( int i=4; i<49; i++ ) {
		Q_ASSERT( buf[i] == '\0' );
	}
	for( int i=49; i<53; i++ ) {
		// unknown
	}
	
	return true;
}






Metastock::Metastock() :
	dir(NULL),
	master(NULL),
	emaster(NULL),
	xmaster(NULL),
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
	SAFE_DELETE( xmaster );
	SAFE_DELETE( emaster );
	SAFE_DELETE( master );
	SAFE_DELETE( dir );
}


QFile* Metastock::findMaster( const char* name )
{
	QFileInfoList fil;
	fil = dir->entryInfoList( QStringList() << name );
	if( fil.size() != 1 ) {
		return NULL;
	} else {
		return new QFile( fil.first().absoluteFilePath() );
	}
}


bool Metastock::setDir( const char* d )
{
	SAFE_DELETE( xmaster );
	SAFE_DELETE( emaster );
	SAFE_DELETE( master );
	SAFE_DELETE( dir );
	
	dir = new QDir(d);
	
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
	qDebug() << ba_master->size() << (double)ba_master->size()/53;
	qDebug() << ba_emaster->size() << (double)ba_emaster->size()/192;
	qDebug() << ba_xmaster->size() << (double)ba_xmaster->size()/150;
}


const char* Metastock::lastError() const
{
	return error;
}


void Metastock::dumpInfo() const
{
//	{
//	int i = 1;
//	while( (i*53) < ba_master->size() ) {
//		qDebug() << i;
//		const char *c = ba_master->constData() + (i*53) + 7;
//		qDebug() << c;
//		i = i + 1;
//	}
//	}
	
	MasterFile::checkHeader(ba_master->constData());
	
	{
	int i = 1;
	int count = ba_master->size() / 53;
	while( i <= count ) {
		qDebug() << i;
		const char *c;
		c = ba_master->constData() + (i*53) + 7;
		qDebug() << c;
		c = ba_emaster->constData() + (i*192) + 12;
		qDebug() << c;
		c = ba_emaster->constData() + (i*192) + 32;
		qDebug() << c;
		c = ba_emaster->constData() + (i*192) + 139;
		qDebug() << c;
		i = i + 1;
	}
	}
//	{
//	int i = 1;
//	while( (i*53) < ba_master->size() ) {
//		qDebug() << i;
//		const char *c = ba_master->constData() + (i*53) + 7;
//		qDebug() << c;
//		i = i + 1;
//	}
//	}
}
