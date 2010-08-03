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
		
		bool check() const;
		unsigned char countRecords() const;
		
	private:
		bool checkRecords() const;
		bool checkRecord( unsigned char r ) const;
		
		static const unsigned int record_length = 53;
		
		const char * const buf;
		const int size;
};


MasterFile::MasterFile( const char *_buf, int _size ) :
	buf( _buf ),
	size( _size )
{
}


bool MasterFile::check() const
{
	Q_ASSERT( countRecords() == (unsigned char) buf[2] );
	Q_ASSERT( countRecords() == size / record_length - 1 && size % record_length == 0 );
	
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
	
	checkRecords();
	return true;
}


bool MasterFile::checkRecords() const
{
	for( int i = 1; i <= countRecords(); i++ ) {
		bool ok = checkRecord( i );
		if( !ok ) {
			return false;
		}
	}
	return true;
}


bool MasterFile::checkRecord( unsigned char r ) const
{
	Q_ASSERT( r > 0 );
	const char *record = buf + (record_length * r);
	Q_ASSERT( record[0] != '\0' ); // F#.dat
	Q_ASSERT( record[1] == 0x65 );
	Q_ASSERT( record[2] == '\0' );
	Q_ASSERT( record[3] == 0x1c ); // record length
	Q_ASSERT( record[4] == 0x07 ); // record count
	Q_ASSERT( record[5] == 0x00 );
	Q_ASSERT( record[6] == 0x00 );
	for( int i=7; i < 23; i++ ) {
		//just a string "issue name"
	}
	Q_ASSERT( record[23] == '\0' );
	Q_ASSERT( record[24] == '\0' );
	for( int i=25; i < 29; i++ ) {
		//just a date
	}
	for( int i=29; i < 33; i++ ) {
		//just a date
	}
	Q_ASSERT( record[33] == 'D' ); // time frame
	for( int i=34; i < 36; i++ ) {
		//just a intraday time frame
	}
	for( int i=36; i < 52; i++ ) {
		//just a string "symbol", space padded?
	}
	Q_ASSERT( record[52] == '\0' );
	
	
	return true;
}


unsigned char MasterFile::countRecords() const
{
	return buf[0];
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
	
	MasterFile mf( ba_master->constData(), ba_master->size() );
	mf.check();
	return;
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
