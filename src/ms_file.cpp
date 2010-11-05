#include "ms_file.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>


#include "util.h"

#if defined FAST_PRINTING

static struct rudi_printf_info _pinfo;

struct rudi_printf_info*  init_print_info()
{
	_pinfo.prec = 6;
	_pinfo.width = 0;
	_pinfo.spec = 'f';
	_pinfo.space = 0;
	_pinfo.left = 0;
	_pinfo.showsign = 0;
	_pinfo.pad = ' ';
	return &_pinfo;
}

static struct rudi_printf_info *pinfo = init_print_info();


#endif


int mr_record_to_string( char *dest, const struct master_record* mr,
	unsigned char prnt_master_fields, char sep )
{
	char * cp = dest;
	int tmp;
	
	if( prnt_master_fields & M_SYM ) {
		tmp = strlen(mr->c_symbol);
		memcpy( cp, mr->c_symbol, tmp );
		cp += tmp;
		*cp++ = sep;
	}
	if( prnt_master_fields & M_NAM ) {
		tmp = strlen(mr->c_long_name);
		memcpy( cp, mr->c_long_name, tmp );
		cp += tmp;
		*cp++ = sep;
	}
	if( prnt_master_fields & M_PER ) {
		*cp++ = mr->barsize;
		*cp++ = sep;
	}
	if( prnt_master_fields & M_FNO ) {
		cp += ltoa( cp, mr->file_number );
		*cp++ = sep;
	}
	if( prnt_master_fields & M_FIL ) {
		tmp = strlen(mr->file_name);
		memcpy( cp, mr->file_name, tmp );
		cp += tmp;
		*cp++ = sep;
	}
	if( prnt_master_fields & M_FLD ) {
		cp += ltoa( cp, mr->field_bitset );
		*cp++ = sep;
	}
	if( prnt_master_fields & M_RNO ) {
		cp += ltoa( cp, mr->record_number );
		*cp++ = sep;
	}
	if( prnt_master_fields & M_KND ) {
		*cp++ = mr->kind;
		*cp++ = sep;
	}
	
	// remove last separator if exists
	if( cp != dest ) {
		*--cp = '\0';
	} else {
		*cp = '\0';
	}
	assert( (cp - dest) < MAX_SIZE_MR_STRING );
	return cp - dest;
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
	return  *( (int*)(c + offset) );
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
	int i = (int)d;
	
	// between 1900-01-01 and 2099-12-31
	assert( i>=101 && i <= 1991231 );
	// is integer
	assert( d == i );
	
	return i + 19000000;
}








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
	assert( size % record_length == 0 );
	assert( countRecords() == (size / record_length - 1) );
	
	printHeader();
	
	assert( readUnsignedChar(buf, 0) == countRecords() );
	assert( readChar(buf, 1) == '\x00' );
	assert( readUnsignedChar(buf, 2) == countRecords() );
	assert( readChar(buf, 3) == '\x00' );
	for( int i=4; i<49; i++ ) {
		assert( readChar(buf, i) == '\x00' );
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
	assert( r > 0 );
	const char *record = buf + (record_length * r);
	printRecord( record );
	assert( readUnsignedChar( record, 0) > 0 ); // F#.dat
	assert( readChar( record, 1 ) == '\x65' );
	assert( readChar( record, 2 ) == '\x00' );
	assert( readChar( record, 3 ) == '\x1c' ); // record length
	assert( readChar( record, 4 ) == '\x07' ); // record count
	assert( readChar( record, 5 ) == '\x00' );
	assert( readChar( record, 6 ) == '\x00' );
	for( int i=7; i < 23; i++ ) {
		//just a string "issue name"
	}
	assert( record[23] == '\0' );
	assert( record[24] == '\0' );
	for( int i=25; i < 29; i++ ) {
		//just a date
	}
	for( int i=29; i < 33; i++ ) {
		//just a date
	}
	assert( record[33] == 'D' ); // time frame
	for( int i=34; i < 36; i++ ) {
		//just a intraday time frame
	}
	for( int i=36; i < 52; i++ ) {
		//just a string "symbol", space padded?
	}
	assert( readChar( record, 52 ) == '\0' );
	
	
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


int MasterFile::getRecord( const master_record *mr, unsigned short rnum ) const
{
	char tmp[64];
	int tmp_len;
	
	const char *record = buf + (record_length * rnum);
	assert( mr->record_number == rnum );
	assert( mr->kind == 'E' );
	assert( mr->file_number == readUnsignedChar( record, 0 ) );
	assert( count_bits(mr->field_bitset) * 4 == readChar( record, 3 ) );
	assert( count_bits(mr->field_bitset) == readChar( record, 4 ) );
	
	trim_end( tmp, record + 36);
	assert( strcmp( mr->c_symbol, tmp ) == 0 );
	tmp_len = trim_end( tmp, record + 7);
	assert( strncmp( mr->c_long_name, tmp, tmp_len ) == 0 );
	
	return 0;
}


int MasterFile::fileNumber( int r ) const
{
	const char *record = buf + (record_length * r);
	int fileNumber = readUnsignedChar( record, 0);
	
	assert( fileNumber > 0 && fileNumber <= 255 );
	return fileNumber;
}


int MasterFile::dataLength( int r ) const
{
	const char *record = buf + (record_length * r);
	return readUnsignedChar( record, 3 );
}








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
	assert( size % record_length == 0 );
	assert( countRecords() == (size / record_length - 1) );
	
	printHeader();
	
	assert( readUnsignedChar(buf, 0) == countRecords() );
	assert( readChar(buf, 1) == '\x00' );
	assert( readUnsignedChar(buf, 2) == countRecords() );
	assert( readChar(buf, 3) == '\x00' );
	for( int i=4; i<49; i++ ) {
		assert( readChar(buf, i) == '\x00' );
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
	assert( r > 0 );
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
	
// 	assert( b_0 == '\x36' || b_0 == '\x00' );
// 	assert( b_1 == b_0 );
	assert( b_2 > 0 && b_2 <= countRecords() );
	for( int i = 3; i<=5; i++ ) {
		assert( readChar( record, i ) == '\x00' );
	}
	assert( b_6 == '\x07' );
	assert( b_7 == '\x7f' );
	assert( readChar( record, 8) == '\x00' );
	assert( readChar( record, 9) == '\x20' );
	assert( readChar( record, 10) == '\x00' );
	for( int i = 27; i<=31; i++ ) {
		assert( readChar( record, i ) == '\x00' );
	}
	for( int i = 48; i<=59; i++ ) {
		assert( readChar( record, i ) == '\x00' );
	}
	assert( b_60 == 'D' );
	for( int i = 61; i<=63; i++ ) {
		assert( readChar( record, i ) == '\x00' );
	}
	for( int i = 68; i<=71; i++ ) {
		assert( readChar( record, i ) == '\x00' );
	}
	for( int i = 76; i<=125; i++ ) {
		// some where here are start/end times
		assert( readChar( record, i ) == '\x00' );
	}
	for( int i = 130; i<=138; i++ ) {
		assert( readChar( record, i ) == '\x00' );
	}
	assert( b_191 == '\x00' );
	
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


int EMasterFile::getRecord( master_record *mr, unsigned short rnum ) const
{
	const char *record = buf + (record_length * rnum);
	mr->record_number = rnum;
	mr->kind = 'E';
	mr->file_number = readUnsignedChar( record, 2 );
	mr->field_bitset= readUnsignedChar( record, 7 );
	assert( count_bits(mr->field_bitset) == readUnsignedChar( record, 6 ) );
	mr->barsize= readChar( record, 60 );
	strcpy( mr->c_symbol, record + 11 );
	if( strlen(record + 139) > 0 ) {
		assert( strncmp( record + 32, record + 139, strlen(record + 32) ) == 0 );
		strcpy( mr->c_long_name, record + 139 );
	} else {
		strcpy( mr->c_long_name, record + 32 );
	}
	return 0;
}


int EMasterFile::fileNumber( int r ) const
{
	const char *record = buf + (record_length * r);
	int fileNumber = readUnsignedChar( record, 2);
	assert( fileNumber > 0 && fileNumber <= 255 );
	return fileNumber;
	
}


int EMasterFile::dataLength( int r ) const
{
	const char *record = buf + (record_length * r);
	return readUnsignedChar( record, 6 ) * 4;
}








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
	assert( size % record_length == 0 );
	assert( countRecords() == (size / record_length - 1) );
	
	printHeader();
	
	assert( readChar(buf, 0) == '\x5d' );
	assert( readChar(buf, 1) == '\xFE' );
	assert( readChar(buf, 2) == 'X' );
	assert( readChar(buf, 3) == 'M' );
	// char 4 - 9 unknown
	assert( readUnsignedShort(buf, 10) ==  countRecords() );
	assert( readChar( buf, 12 ) == '\x00' );
	assert( readChar( buf, 13 ) == '\x00' );
	assert( readUnsignedShort(buf, 14) ==  countRecords() );
	assert( readChar( buf, 16 ) == '\x00' );
	assert( readChar( buf, 17 ) == '\x00' );
	
	// last used + 1 !?
	assert( readUnsignedShort(buf, 18) > countRecords() );
	assert( readChar( buf, 20 ) == '\x00' );
	assert( readChar( buf, 21 ) == '\x00' );
	
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
	assert( r > 0 );
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
	
	
	assert( readChar( record, 0) == '\x01' );
	assert( readChar( record, 15) == '\x00' );
	assert( readChar( record, 61) == '\x00' );
	assert( readChar( record, 62) == 'D' );
	assert( readChar( record, 63) == '\x00' );
	assert( readChar( record, 64) == '\x00' );
	assert( readChar( record, 67) == '\x00' );
	assert( readChar( record, 68) == '\x00' );
	assert( readChar( record, 69) == '\x00' );
	assert( readChar( record, 70) == '\x7f' || readChar( record, 70) == '\x3f' );
	for( int i = 71; i<=79; i++ ) {
		assert( readChar( record, i ) == '\x00' );
	}
	assert( readChar( record, 70) == '\x7f' || readChar( record, 70) == '\x3f' );
	for( int i = 87; i<=103; i++ ) {
		assert( readChar( record, i ) == '\x00' );
	}
	for( int i = 112; i<=115; i++ ) {
		assert( readChar( record, i ) == '\x00' );
	}
	for( int i = 120; i<=149; i++ ) {
		assert( readChar( record, i ) == '\x00' );
	}
// 	assert( b_46 == '\x7f' );
	
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


int XMasterFile::getRecord( master_record *mr, unsigned short rnum ) const
{
	const char *record = buf + (record_length * rnum);
	mr->record_number = rnum;
	mr->kind = 'X';
	mr->file_number = readUnsignedShort( record, 65 );
	mr->field_bitset = readUnsignedChar( record, 70 );
	mr->barsize = readChar( record, 62 );
	strcpy( mr->c_symbol, record + 1 );
	strcpy( mr->c_long_name, record + 16 );
	return 0;
}


int XMasterFile::fileNumber( int r ) const
{
	const char *record = buf + (record_length * r);
	int fileNumber = readUnsignedShort( record, 65 );
	
	assert( fileNumber > 255 );
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








FDat::FDat( const char *_buf, int _size, unsigned char fields ) :
	field_bitset( fields ),
	record_length( count_bits(fields) * 4 ),
	buf( _buf ),
	size( _size )
{
}


char FDat::print_sep = '\t';
unsigned int FDat::print_bitset = 0xff;
#if ! defined FAST_PRINTING
char FDat::sprintf_format[64] = "%d\t%.5f\t%.5f\t%.5f\t%.5f\t%.0f\t%.0f\n";
#endif


void FDat::initPrinter( char sep, unsigned int bitset )
{
	print_sep = sep;
	print_bitset = bitset;
	
#if ! defined FAST_PRINTING
	// TODO implement constructing sprintf_format
	sprintf_format[2] = sprintf_format[7] = sprintf_format[12]
		= sprintf_format[17] = sprintf_format[22] = sprintf_format[27]
		= sep;
#endif
}


bool FDat::checkHeader() const
{
	assert( size % record_length == 0 );
	assert( countRecords() == (size / record_length) - 1 );
	
// 	assert( readChar(buf, 0) == '\x5d' );
	
	return true;
}


void FDat::print( const char* header ) const
{
	const char *record = buf + record_length;
	const char *end = buf + size;
	char buf[512];
	char *buf_p = buf;
	
	int h_size = strlen( header );
	memcpy( buf, header, h_size );
	buf_p += h_size;
	
	while( record < end ) {
		int len = record_to_string( record, buf_p );
		buf_p[len++] = '\n';
		buf_p[len] = '\0';
		
		record += record_length;
		
		fputs( buf, stdout );
	}
	fflush( stdout );
}


// to be printed when field does not exist
#define DEFAULT_FLOAT 0.0

int FDat::record_to_string( const char *record, char *s ) const
{
	int ret;
	int offset = 0;
	
	
#if defined FAST_PRINTING
#define PRINT_FIELD( _field_ ) \
	if( print_bitset & _field_) { \
		if( field_bitset & _field_ ) { \
			s += rudi_printf_fp( s, pinfo, readFloat(record, offset) ); \
			offset += 4; \
		} else { \
			s += rudi_printf_fp( s, pinfo, DEFAULT_FLOAT ); \
		} \
		*s++ = print_sep; \
	}

	char *begin = s;
	if( print_bitset & D_DAT ) {
		if( field_bitset & D_DAT ) {
			s += ltoa( s, floatToIntDate_YYY( readFloat(record, offset) ) );
			offset += 4;
		} else {
			s += ltoa( s, 10000101 );
		}
		*s++ = print_sep;
	}
	if( print_bitset & D_TIM ) {
		if( field_bitset & D_TIM ) {
			// TODO never seen a time field, we would print it here at this
			// position but probably it is the last float in the record
			assert(false);
		}
		// NOTE the only field we don't print if not exists
	}
	pinfo->prec = 5;
	PRINT_FIELD( D_OPE );
	PRINT_FIELD( D_HIG );
	PRINT_FIELD( D_LOW );
	PRINT_FIELD( D_CLO );
	pinfo->prec = 0;
	PRINT_FIELD( D_VOL );
	PRINT_FIELD( D_OPI );
	
	if( s != begin ) {
		*(--s) = '\0';
	} else {
		*s = '\0';
	}
	
	ret = s - begin;
#undef PRINT_FIELD
#else
	// TODO implement constructing sprintf_format
	ret = sprintf( s, sprintf_format,
		floatToIntDate_YYY( readFloat( record, 0 ) ),
		readFloat( record, 4 ),
		readFloat( record, 8 ),
		readFloat( record, 12 ),
		readFloat( record, 16 ),
		readFloat( record, 20 ),
		(record_length >= 28) ? readFloat( record, 24 ) : DEFAULT_FLOAT);
#endif
	
	return ret;
}

#undef DEFAULT_FLOAT


unsigned short FDat::countRecords() const
{
	return readUnsignedShort( buf, 2 ) -1;
}



