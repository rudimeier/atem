#include "metastock.h"

#include <QtCore/QDebug>
#include <QtCore/QStringList>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>



char readChar( const char *c, int offset )
{
	return (char)(c[offset]);
}

unsigned char readUnsignedChar( const char *c, int offset )
{
	return (unsigned char) c[offset];
}






class MasterFile
{
	public:
		MasterFile( const char *buf, int size );
		
		static bool checkHeader( const char* buf );
		static bool checkRecord( const char* buf, int record  );
		
		bool check() const;
		inline unsigned char countRecords() const;
		
	private:
		bool checkHeader() const;
		bool checkRecords() const;
		bool checkRecord( unsigned char r ) const;
		void printRecord( const char *record ) const;
		
		
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
	checkHeader();
	checkRecords();
	return true;
}


bool MasterFile::checkHeader() const
{
	Q_ASSERT( size % record_length == 0 );
	Q_ASSERT( countRecords() == (size / record_length - 1) );
	
	Q_ASSERT( readUnsignedChar(buf, 0) == countRecords() );
	Q_ASSERT( readChar(buf, 1) == '\x00' );
	Q_ASSERT( readUnsignedChar(buf, 2) == countRecords() );
	Q_ASSERT( readChar(buf, 3) == '\x00' );
	for( int i=4; i<49; i++ ) {
		Q_ASSERT( readChar(buf, i) == '\x00' );
	}
	for( int i=49; i<53; i++ ) {
		// unknown
	}
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
	printRecord( record );
	Q_ASSERT( readUnsignedChar( record, 0) > 0 ); // F#.dat
	Q_ASSERT( readChar( record, 1 ) == '\x65' );
	Q_ASSERT( readChar( record, 2 ) == '\x00' );
	Q_ASSERT( readChar( record, 3 ) == '\x1c' ); // record length
	Q_ASSERT( readChar( record, 4 ) == '\x07' ); // record count
	Q_ASSERT( readChar( record, 5 ) == '\x00' );
	Q_ASSERT( readChar( record, 6 ) == '\x00' );
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
	Q_ASSERT( readChar( record, 52 ) == '\0' );
	
	
	return true;
}


void MasterFile::printRecord( const char *record ) const
{
	fprintf( stdout, "F.dat:\t'%d'\nSymbol:\t'%s'\nName:\t'%s'\n",
		readUnsignedChar( record, 0 ),
		record + 36,
		record + 7 );
}


unsigned char MasterFile::countRecords() const
{
	return readChar( buf, 0 );
}








class EMasterFile
{
	public:
		EMasterFile( const char *buf, int size );
		
		static bool checkHeader( const char* buf );
		static bool checkRecord( const char* buf, int record  );
		
		bool check() const;
		inline unsigned char countRecords() const;
		
	private:
		bool checkHeader() const;
		bool checkRecords() const;
		bool checkRecord( unsigned char r ) const;
		void printRecord( const char *record ) const;
		
		
		static const unsigned int record_length = 192;
		
		const char * const buf;
		const int size;
};


EMasterFile::EMasterFile( const char *_buf, int _size ) :
	buf( _buf ),
	size( _size )
{
}


bool EMasterFile::check() const
{
	checkHeader();
	checkRecords();
	return true;
}


bool EMasterFile::checkHeader() const
{
	Q_ASSERT( size % record_length == 0 );
	Q_ASSERT( countRecords() == (size / record_length - 1) );
	
	Q_ASSERT( readUnsignedChar(buf, 0) == countRecords() );
	Q_ASSERT( readChar(buf, 1) == '\x00' );
	Q_ASSERT( readUnsignedChar(buf, 2) == countRecords() );
	Q_ASSERT( readChar(buf, 3) == '\x00' );
	for( int i=4; i<49; i++ ) {
		Q_ASSERT( readChar(buf, i) == '\x00' );
	}
	for( int i=49; i<53; i++ ) {
		// unknown
	}
	return true;
}


bool EMasterFile::checkRecords() const
{
	for( int i = 1; i <= countRecords(); i++ ) {
		bool ok = checkRecord( i );
		if( !ok ) {
			return false;
		}
	}
	return true;
}


bool EMasterFile::checkRecord( unsigned char r ) const
{
	Q_ASSERT( r > 0 );
	const char *record = buf + (record_length * r);
	printRecord( record );
	
	char b_0 = readChar( record, 0);
	char b_1 = readChar( record, 1);
	unsigned char b_2 = readChar( record, 2); // F#.dat
	// char 3 - 5 always zero
	char b_6 = readChar( record, 6); // record count?
	char b_7 = readChar( record, 7); // fields bit set?
	// char 8 always zero
	// char 9 always a space
	// char 10 always zero
	// char 11 - 26 symbol?
	// char 27 - 31 always zero
	// char 32 - 47 name?
	// char 48 - 59 always zero
	char b_60 = readChar( record, 60); // time frame 'D'
	// char 61 - 63 always zero
	// char 64 - 67 first date
	// char 68 - 71 always zero
	// char 72 - 75 last date
	// char 76 - 125 always zero (start/end times could be here)
	// char 126 - 129 last date in long format
	// char 130 - 138 always zero
	// char 139 - 191 long name?
	char b_191 = readChar( record, 191); // last byte always zero
	
	Q_ASSERT( b_0 == '\x36' || b_0 == '\x00' );
	Q_ASSERT( b_1 == b_0 );
	Q_ASSERT( b_2 > 0 && b_2 <= countRecords() );
	for( int i = 3; i<=5; i++ ) {
		Q_ASSERT( readChar( record, i ) == '\x00' );
	}
	Q_ASSERT( b_6 == '\x07' );
	Q_ASSERT( b_7 == '\x7f' );
	Q_ASSERT( readChar( record, 8) == '\x00' );
	Q_ASSERT( readChar( record, 9) == '\x20' );
	Q_ASSERT( readChar( record, 10) == '\x00' );
	for( int i = 27; i<=31; i++ ) {
		Q_ASSERT( readChar( record, i ) == '\x00' );
	}
	for( int i = 48; i<=59; i++ ) {
		Q_ASSERT( readChar( record, i ) == '\x00' );
	}
	Q_ASSERT( b_60 == 'D' );
	for( int i = 61; i<=63; i++ ) {
		Q_ASSERT( readChar( record, i ) == '\x00' );
	}
	for( int i = 68; i<=71; i++ ) {
		Q_ASSERT( readChar( record, i ) == '\x00' );
	}
	for( int i = 76; i<=125; i++ ) {
		// some where here are start/end times
		Q_ASSERT( readChar( record, i ) == '\x00' );
	}
	for( int i = 130; i<=138; i++ ) {
		Q_ASSERT( readChar( record, i ) == '\x00' );
	}
	Q_ASSERT( b_191 == '\x00' );
	
	return true;
}


void EMasterFile::printRecord( const char *record ) const
{
	fprintf( stdout, "F.dat:\t'%d'\nSymbol:\t'%s'\nName:\t'%s'\n",
		readUnsignedChar( record, 0 ),
		record + 36,
		record + 7 );
}


unsigned char EMasterFile::countRecords() const
{
	return readChar( buf, 0 );
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
// 	mf.check();
	EMasterFile emf( ba_emaster->constData(), ba_emaster->size() );
	emf.check();
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
