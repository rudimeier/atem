#include "metastock.h"

#include <math.h>

#include <QtCore/QDebug>
#include <QtCore/QStringList>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>




#if ! defined I_MALC_ATOI
struct bla
{
	unsigned int div;
	unsigned int rem;
};


static inline struct bla div1000(unsigned int a, unsigned int b)
{
	register unsigned int c = a / b;
	register unsigned int d = a % b;
	struct bla ret = {c ,d };
	return ret;
}

#define MOD_500( _num, _tens ) \
	l2 = div1000( _num, _tens ); \
	*ps++ = '0' + (char)l2.div; \
	_num = l2.rem 


int itoa( long snum, char s[] )
{
	struct bla l2;
	char *ps = s;
	unsigned long num1 = snum, num2, num3, div;
	if (snum < 0) {
		*ps++ = '-';
		num1 = -snum;
	}
	if (num1 < 10000) {
		if (num1 < 10) goto L1;
		if (num1 < 100) goto L2;
		if (num1 < 1000) goto L3;
	} else {
		num2 = num1 / 10000;
		num1 -= num2 * 10000;
		if (num2 < 10000) {
			if (num2 < 10) goto L5;
			if (num2 < 100) goto L6;
			if (num2 < 1000) goto L7;
		} else {
			num3 = num2 / 10000;
			num2 -= num3 * 10000;
			if (num3 >= 10) {
				MOD_500( num3, 10);
			}
			*ps++ = '0' + (char)(num3);
		}
		MOD_500( num2, 1000);
L7:
		MOD_500( num2, 100);
L6:
		MOD_500( num2, 10);
L5:
		*ps++ = '0' + (char)(num2);
    }
	
	MOD_500( num1, 1000);
L3:
	MOD_500( num1, 100);
L2:
	MOD_500( num1, 10);
	
L1:
	*ps++ = '0' + (char)(num1);
	*ps = '\0';
	return ps - s;
}

#else

//---------------------------------------------------------------
// itoas() - iMalc version updated ver. 0.8
//---------------------------------------------------------------
int itoa( long snum, char s[] )
{
	char *ps = s;
	unsigned long num1 = snum, num2, num3, div;
	if (snum < 0) {
		*ps++ = '-';
		num1 = -snum;
	}
	if (num1 < 10000) {
		if (num1 < 10) goto L1;
		if (num1 < 100) goto L2;
		if (num1 < 1000) goto L3;
	} else {
		num2 = num1 / 10000;
		num1 -= num2 * 10000;
		if (num2 < 10000) {
			if (num2 < 10) goto L5;
			if (num2 < 100) goto L6;
			if (num2 < 1000) goto L7;
		} else {
			num3 = num2 / 10000;
			num2 -= num3 * 10000;
			if (num3 >= 10) {
				*ps++ = '0' + (char)(div = (num3*6554UL)>>16);
				num3 -= div*10;
			}
			*ps++ = '0' + (char)(num3);
		}
		*ps++ = '0' + (char)(div = (num2*8389UL)>>23);
		num2 -= div*1000;
L7:
		*ps++ = '0' + (char)(div = (num2*5243UL)>>19);
		num2 -= div*100;
L6:
		*ps++ = '0' + (char)(div = (num2*6554UL)>>16);
		num2 -= div*10;
L5:
		*ps++ = '0' + (char)(num2);
    }
	*ps++ = '0' + (char)(div = (num1*8389UL)>>23);
	num1 -= div*1000;
L3:
	*ps++ = '0' + (char)(div = (num1*5243UL)>>19);
	num1 -= div*100;
L2:
	*ps++ = '0' + (char)(div = (num1*6554UL)>>16);
	num1 -= div*10;
L1:
	*ps++ = '0' + (char)(num1);
	*ps = '\0';
	return ps - s;
}


#endif








/**
 * reverse:  reverse first l chars of string s in place
 */
void reverse( char *s, int l )
{
	int i, j;
	char c;
	
	for (i = 0, j = l-1; i<j; i++, j--) {
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}


/**
 * itoa:  convert n to characters in s
 */
int ltoa( long n, char *s )
{
	char *begin = s;
	char *rev = s;
	
	if( n < 0 ) {
		 // forget about the sign, don't reverse it later
		*s++ = '-';
		*s++ = -(n % 10) + '0';
		n /= -10;
		rev++;
	}
	
	do {       /* generate digits in reverse order */
		*s++ = (n % 10) + '0';   /* get next digit */
	} while ((n /= 10) > 0);     /* delete it */
	
	reverse( rev, s - rev);
	
	return s - begin;
}


int ftoa(float f, char *s)
{
	char *buf = s;
	if( f < 0 ) {
		f = -f;
		*buf++ = '-';
	}
	
	float a;
	float b = modff(f, &a );
	
	int b_ = (b * 1000000);
	if( b_ % 10 >= 5 ) {
		b_ = b_ / 10 + 1;
	} else {
		b_ /= 10;
	}
	buf += ltoa(a, buf);
	int d = ltoa( 100000 + b_, buf);
	*buf = '.';
	return buf + d - s;
}


char readChar( const char *c, int offset )
{
	return (char)(c[offset]);
}

unsigned char readUnsignedChar( const char *c, int offset )
{
	return (unsigned char) c[offset];
}

unsigned short readUnsignedShort( const char *c, int offset )
{
	return  *( (unsigned short*)(c + offset) );
}


/**
 Read a signed four byte int, least significant byte first
 */
int readInt( const char *c, int offset )
{
	unsigned int b0 = (unsigned char) c[offset];
	unsigned int b1 = (unsigned char) c[offset+1];
	unsigned int b2 = (unsigned char) c[offset+2];
	unsigned int b3 = (char) c[offset+3];
	
	return (b0 | b1 << 8 | b2 << 16 | b3 << 24);
}


float readFloat(const char *c, int offset)
{
	const float *src4 = (const float*) (c+offset);
	float retVal;
	float *dest4 = &retVal;
	
	unsigned char *msbin = (unsigned char *)src4;
	unsigned char *ieee = (unsigned char *)dest4;
	unsigned char sign = 0x00;
	unsigned char ieee_exp = 0x00;
	int i;

	/* MS Binary Format                         */
	/* byte order =>    m3 | m2 | m1 | exponent */
	/* m1 is most significant byte => sbbb|bbbb */
	/* m3 is the least significant byte         */
	/*      m = mantissa byte                   */
	/*      s = sign bit                        */
	/*      b = bit                             */

	sign = msbin[2] & 0x80;      /* 1000|0000b  */

	/* IEEE Single Precision Float Format       */
	/*    m3        m2        m1     exponent   */
	/* mmmm|mmmm mmmm|mmmm emmm|mmmm seee|eeee  */
	/*          s = sign bit                    */
	/*          e = exponent bit                */
	/*          m = mantissa bit                */

	for (i=0; i<4; i++) ieee[i] = 0;

	/* any msbin w/ exponent of zero = zero */
	if (msbin[3] == 0) return retVal;

	ieee[3] |= sign;

	/* MBF is bias 128 and IEEE is bias 127. ALSO, MBF places   */
	/* the decimal point before the assumed bit, while          */
	/* IEEE places the decimal point after the assumed bit.     */

	ieee_exp = msbin[3] - 2;    /* actually, msbin[3]-1-128+127 */

	/* the first 7 bits of the exponent in ieee[3] */
	ieee[3] |= ieee_exp >> 1;

	/* the one remaining bit in first bin of ieee[2] */
	ieee[2] |= ieee_exp << 7;

	/* 0111|1111b : mask out the msbin sign bit */
	ieee[2] |= msbin[2] & 0x7f;

	ieee[1] = msbin[1];
	ieee[0] = msbin[0];

	return retVal;
}







int floatToIntDate_YYY( float d )
{
	// between 1900-01-01 and 2099-12-31
	Q_ASSERT( d>=101 && d <= 1991231 );
	// is integer
	
	float tmp;
	Q_ASSERT( modff(d,&tmp) == 0.0f );
	return ((int)d) + 19000000;
}


/**
 Convert an integer YYMMDD or YYYMMDD or YYYYMMDD to java.util.Date.
 1900 is added to years less than 1000, so 1040101 is read as 20040101.
 */
// private Date toDate(int i)
// {
// 	int dateOfMonth = i % 100;
// 	i /= 100;
// 	int month = i % 100;
// 	i /= 100;
// 	int year = i;
// 	if (year < 1000)
// 		year += 1900;
// 	calendar.clear();
// 	calendar.set(year, month - 1, dateOfMonth);
// 	int parsedYear = calendar.get(Calendar.YEAR);
// 	return calendar.getTime();
// }

/**
 Read date formatted as a float YYMMDD or YYYMMDD or YYYYMMDD.
 1900 is added to years less than 1000, so 1040101 is read as 20040101.
 */
// public Date readFloatDate() throws IOException
// {
// 	float f = readFloat();
// 	return toDate((int) f);
// }


/**
 Read date formatted as an integer YYMMDD or YYYMMDD or YYYYMMDD.
 1900 is added to years less than 1000, so 1040101 is read as 20040101.
 */
// public Date readIntegerDate() throws IOException
// {
// 	int i = readInt();
// 	return toDate(i);
// }




class MasterFile
{
	public:
		MasterFile( const char *buf, int size );
		
		static bool checkHeader( const char* buf );
		static bool checkRecord( const char* buf, int record  );
		
		bool check() const;
		inline unsigned char countRecords() const;
		int fileNumber( int record ) const;
		int dataLength( int record ) const;
		
	private:
		bool checkHeader() const;
		bool checkRecords() const;
		bool checkRecord( unsigned char r ) const;
		
		void printHeader() const;
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
	
	printHeader();
	
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


void MasterFile::printHeader() const
{
	fprintf( stdout, "MASTER:\t%d\t%d\t%X\n",
		readUnsignedChar(buf, 0), // count records (stored in master?)
		readUnsignedChar(buf, 2), // count records (existing dat files?)
		readInt(buf, 49) // unknown - just print as hex
		);
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
	fprintf( stdout, "F%4d.dat\t%d\t%d\t%d\t%d\t'%s'\t'%s'\n",
		readUnsignedChar( record, 0 ), // F#.dat
		readChar( record, 3 ), // dat record length in bytes
		readChar( record, 4 ), // dat fields count per record
		floatToIntDate_YYY( readFloat( record, 25 ) ),
		floatToIntDate_YYY( readFloat( record, 29 ) ),
		record + 36, // symbol
		record + 7 // name
		);
}


unsigned char MasterFile::countRecords() const
{
	return readChar( buf, 0 );
}


int MasterFile::fileNumber( int r ) const
{
	const char *record = buf + (record_length * r);
	int fileNumber = readUnsignedChar( record, 0);
	qDebug() << r << fileNumber;
	Q_ASSERT( fileNumber > 0 && fileNumber <= 255 );
	return fileNumber;
}


int MasterFile::dataLength( int r ) const
{
	const char *record = buf + (record_length * r);
	return readUnsignedChar( record, 3 );
}







class EMasterFile
{
	public:
		EMasterFile( const char *buf, int size );
		
		static bool checkHeader( const char* buf );
		static bool checkRecord( const char* buf, int record  );
		
		bool check() const;
		inline unsigned char countRecords() const;
		int fileNumber( int record ) const;
		int dataLength( int record ) const;
		
	private:
		bool checkHeader() const;
		bool checkRecords() const;
		bool checkRecord( unsigned char r ) const;
		
		void printHeader() const;
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
	
	printHeader();
	
	Q_ASSERT( readUnsignedChar(buf, 0) == countRecords() );
	Q_ASSERT( readChar(buf, 1) == '\x00' );
	Q_ASSERT( readUnsignedChar(buf, 2) == countRecords() );
	Q_ASSERT( readChar(buf, 3) == '\x00' );
	for( int i=4; i<49; i++ ) {
		Q_ASSERT( readChar(buf, i) == '\x00' );
	}
	for( int i=49; i<52; i++ ) {
		// unknown
	}
	return true;
}


void EMasterFile::printHeader() const
{
	fprintf( stdout, "EMASTER:\t%d\t%d\t%X\t'%s'\n",
		readUnsignedChar(buf, 0), // count records (stored in master?)
		readUnsignedChar(buf, 2), // count records (existing dat files?)
		readInt(buf, 49), // unknown - just print as hex
		buf + 53 // unkown, equis sends a string
		);
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
	
// 	Q_ASSERT( b_0 == '\x36' || b_0 == '\x00' );
// 	Q_ASSERT( b_1 == b_0 );
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
// 	fprintf( stdout, "F%d.dat\t%d\t%d\t%d\t%d\t'%s'\t'%s'\n",
	fprintf( stdout, "F%4d.dat\t%d\t%d\t%c\t%X\t%X\t'%s'\t'%s'\t'%s'\n",
		readUnsignedChar( record, 2),  // F#.dat
		readChar( record, 7 ), // fields bit set
		readChar( record, 6 ), // dat fields count per record
		readChar( record, 60 ), // time frame 'D'
		// never saw these floats
// 		readFloat( record, 64 ),
// 		readFloat( record, 72 ),
// 		readFloat( record, 126 ),
// 		readFloat( record, 131 ),
// 		readFloat( record, 135 ),
		readUnsignedChar( record, 0 ), // unknown, just print hex
		readUnsignedChar( record, 1 ), // unknown, just print hex
		record + 11, // symbol
		record + 32, // name
		record + 139 ); // name when "too long"
}


unsigned char EMasterFile::countRecords() const
{
	return readChar( buf, 0 );
}


int EMasterFile::fileNumber( int r ) const
{
	const char *record = buf + (record_length * r);
	int fileNumber = readUnsignedChar( record, 2);
	Q_ASSERT( fileNumber > 0 && fileNumber <= 255 );
	return fileNumber;
	
}


int EMasterFile::dataLength( int r ) const
{
	const char *record = buf + (record_length * r);
	return readUnsignedChar( record, 6 );
}








class XMasterFile
{
	public:
		XMasterFile( const char *buf, int size );
		
		static bool checkHeader( const char* buf );
		static bool checkRecord( const char* buf, int record  );
		
		bool check() const;
		inline unsigned short countRecords() const;
		int fileNumber( int record ) const;
		int dataLength( int record ) const;
		
	private:
		bool checkHeader() const;
		bool checkRecords() const;
		bool checkRecord( int r ) const;
		
		void printHeader() const;
		void printRecord( const char *record ) const;
		
		
		static const unsigned int record_length = 150;
		
		const char * const buf;
		const int size;
};


XMasterFile::XMasterFile( const char *_buf, int _size ) :
	buf( _buf ),
	size( _size )
{
}


bool XMasterFile::check() const
{
	checkHeader();
	checkRecords();
	return true;
}


bool XMasterFile::checkHeader() const
{
	Q_ASSERT( size % record_length == 0 );
	Q_ASSERT( countRecords() == (size / record_length - 1) );
	
	printHeader();
	
	Q_ASSERT( readChar(buf, 0) == '\x5d' );
	Q_ASSERT( readChar(buf, 1) == '\xFE' );
	Q_ASSERT( readChar(buf, 2) == 'X' );
	Q_ASSERT( readChar(buf, 3) == 'M' );
	// char 4 - 9 unknown
	Q_ASSERT( readUnsignedShort(buf, 10) ==  countRecords() );
	Q_ASSERT( readChar( buf, 12 ) == '\x00' );
	Q_ASSERT( readChar( buf, 13 ) == '\x00' );
	Q_ASSERT( readUnsignedShort(buf, 14) ==  countRecords() );
	Q_ASSERT( readChar( buf, 16 ) == '\x00' );
	Q_ASSERT( readChar( buf, 17 ) == '\x00' );
	
	// last used + 1 !?
	Q_ASSERT( readUnsignedShort(buf, 18) > countRecords() );
	Q_ASSERT( readChar( buf, 20 ) == '\x00' );
	Q_ASSERT( readChar( buf, 21 ) == '\x00' );
	
	// char 22 -191 unknown
	
	return true;
}


void XMasterFile::printHeader() const
{
	fprintf( stdout, "XMASTER:\t%d\t%d\t%d\t'%s'\n",
		readUnsignedShort(buf, 10), // count records (stored in master?)
		readUnsignedShort(buf, 14), // count records (the same?)
		readUnsignedShort(buf, 18), // last used record
		buf + 22 //  // unkown, equis sends a string
		);
}


bool XMasterFile::checkRecords() const
{
	for( int i = 1; i <= countRecords(); i++ ) {
		bool ok = checkRecord( i );
		if( !ok ) {
			return false;
		}
	}
	return true;
}


bool XMasterFile::checkRecord( int r ) const
{
	Q_ASSERT( r > 0 );
	const char *record = buf + (record_length * r);
	printRecord( record );
	
	// char 0 always '\x01'?
	// char 1 - 14 symbol?
	// char 15 always zero
	// char 16 - ? name
	// char 61 always '\x00'
	// char 62 time frame
	// char 63 always '\x00'
	// char 64 always '\x00'
	// char 65 - 66 F#.mwd
	// char 67 always '\x00'
	// char 68 always '\x00'
	// char 69 always '\x00'
	// char 70 fields bit set, always '\x7f' or '\x3f'
	// char 71 - 79 always '\x00'
	// char 80 - 83 start date
	// char 84 - 86 short start date ???
	// char 87 - 103 always '\x00'
	// char 104 - 107 first date
	// char 108 - 111 last date
	// char 112 - 115 always '\x00'
	// char 116 - 119 last date
	// char 120 - 149 always '\x00'
	
	
	Q_ASSERT( readChar( record, 0) == '\x01' );
	Q_ASSERT( readChar( record, 15) == '\x00' );
	Q_ASSERT( readChar( record, 61) == '\x00' );
	Q_ASSERT( readChar( record, 62) == 'D' );
	Q_ASSERT( readChar( record, 63) == '\x00' );
	Q_ASSERT( readChar( record, 64) == '\x00' );
	Q_ASSERT( readChar( record, 67) == '\x00' );
	Q_ASSERT( readChar( record, 68) == '\x00' );
	Q_ASSERT( readChar( record, 69) == '\x00' );
	Q_ASSERT( readChar( record, 70) == '\x7f' || readChar( record, 70) == '\x3f' );
	for( int i = 71; i<=79; i++ ) {
		Q_ASSERT( readChar( record, i ) == '\x00' );
	}
	Q_ASSERT( readChar( record, 70) == '\x7f' || readChar( record, 70) == '\x3f' );
	for( int i = 87; i<=103; i++ ) {
		Q_ASSERT( readChar( record, i ) == '\x00' );
	}
	for( int i = 112; i<=115; i++ ) {
		Q_ASSERT( readChar( record, i ) == '\x00' );
	}
	for( int i = 120; i<=149; i++ ) {
		Q_ASSERT( readChar( record, i ) == '\x00' );
	}
// 	Q_ASSERT( b_46 == '\x7f' );
	
	return true;
}


void XMasterFile::printRecord( const char *record ) const
{
	fprintf( stdout, "F%4d.mwd\t%c\t%d\t%d\t%d\t%d\t%d\t'%s'\t'%s'\n",
		readUnsignedShort( record, 65 ), // F#.mwd
		readChar( record, 62 ), // time frame 'D'
		readUnsignedChar( record, 70 ), // fields bitset
		readInt( record, 80 ), // some date ?
// 		readInt( record, 84 ), // stupid date? forst 3 bytes equal 
		readInt( record, 104 ), // some date ?
		readInt( record, 108 ), // some date ?
		readInt( record, 116 ), // some date ?
		record + 1, // symbol
		record + 16 // name
		);
}


unsigned short XMasterFile::countRecords() const
{
	return readUnsignedShort( buf, 10 );
}


int XMasterFile::fileNumber( int r ) const
{
	const char *record = buf + (record_length * r);
	int fileNumber = readUnsignedShort( record, 65 );
	qDebug() << r << fileNumber;
	Q_ASSERT( fileNumber > 255 );
	return fileNumber;
}


int XMasterFile::dataLength( int r ) const
{
	const char *record = buf + (record_length * r);
	unsigned char v = readUnsignedChar( record, 70 );
	unsigned char c; // c accumulates the total bits set in v
	for (c = 0; v; v >>= 1) {
		c += v & 1;
	}
	return 4 * c;
}







class FDat
{
	public:
		FDat( const char *buf, int size, int l );
		
		static bool checkHeader( const char* buf );
		static bool checkRecord( const char* buf, int record  );
		
		bool check() const;
		inline unsigned short countRecords() const;
		
	private:
		bool checkHeader() const;
		bool checkRecords() const;
		bool checkRecord( int r ) const;
		void printRecord( const char *record ) const;
		
		
		const unsigned int record_length;
		
		const char * const buf;
		const int size;
};


FDat::FDat( const char *_buf, int _size, int l ) :
	record_length( l),
	buf( _buf ),
	size( _size )
{
}


bool FDat::check() const
{
	checkHeader();
	checkRecords();
	return true;
}


bool FDat::checkHeader() const
{
	Q_ASSERT( size % record_length == 0 );
	Q_ASSERT( countRecords() == (size / record_length) - 1 );
	
// 	Q_ASSERT( readChar(buf, 0) == '\x5d' );
	
	return true;
}


bool FDat::checkRecords() const
{
	for( int i = 1; i <= countRecords(); i++ ) {
		bool ok = checkRecord( i );
		if( !ok ) {
			return false;
		}
	}
	return true;
}


bool FDat::checkRecord( int r ) const
{
	Q_ASSERT( r > 0 );
	const char *record = buf + (record_length * r);
	printRecord( record );
	

	
	return true;
}


void FDat::printRecord( const char *record ) const
{
#if ! defined USE_FPRINTF
	char buf[512];
	char *s = buf;
	s += ltoa( floatToIntDate_YYY( readFloat( record, 0 )), s );
	*s++ = '\t';
	s += ftoa( readFloat( record, 4 ), s );
	*s++ = '\t';
	s += ftoa( readFloat( record, 8 ), s );
	*s++ = '\t';
	s += ftoa( readFloat( record, 12 ), s );
	*s++ = '\t';
	s += ftoa( readFloat( record, 16 ), s );
	*s++ = '\t';
	s += ftoa( readFloat( record, 20 ), s );
	*s++ = '\t';
	s += ftoa( record_length >= 28 ? readFloat( record, 24 ) : 0.0f, s );
	*s++ = '\n';
	*s++ = '\0';
	
	fputs( buf, stdout );
#else
	fprintf( stdout, "%d\t%.5f\t%.5f\t%.5f\t%.5f\t%g\t%g\n",
		floatToIntDate_YYY( readFloat( record, 0 ) ),
		readFloat( record, 4 ),
		readFloat( record, 8 ),
		readFloat( record, 12 ),
		readFloat( record, 16 ),
		readFloat( record, 20 ),
		(record_length >= 28) ? readFloat( record, 24 ) : NAN);
#endif
}


unsigned short FDat::countRecords() const
{
	return readUnsignedShort( buf, 2 ) -1;
}








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
		int l_e = mf.dataLength( f );
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
	datfile.check();
	delete fdat;

}


bool Metastock::hasXMaster() const
{
	return( xmaster != NULL  );
}
