#include "ms_file.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>


#include "util.h"

#if defined FAST_PRINTING

static struct rudi_printf_info _pinfo;

struct rudi_printf_info*  init_print_info()
{
	_pinfo.prec = 5;
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
	unsigned short prnt_master_fields, char sep )
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
	if( prnt_master_fields & M_DT1 ) {
		cp += ltoa( cp, mr->from_date );
		*cp++ = sep;
	}
	if( prnt_master_fields & M_DT2 ) {
		cp += ltoa( cp, mr->to_date );
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


float readFloat_IEEE(const char *c, int offset)
{
	float ret = * (const float*) (c+offset);
	return ret;
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
	
	//  #0,  1b, unsigned char, count records (dat files)
	//           must be >0 (error #1002)
	//  #1,  1b, char, always '\0' (~ error #1002)
	//  #2,  1b, unsigned char, max record number (dat file number)
	//           must be >0 (error #1003)
	//  #3,  1b, char, always '\0' (~ error #1003)
	//  #4, 45b, char*, always '\0' ?
	// #49,  4b, int, serial number
	
	unsigned char cntRec = readUnsignedChar(buf, 0);
	assert( cntRec == countRecords() && cntRec > 0 );
	assert( buf[1] == '\0' );
	unsigned char maxRec = readUnsignedChar(buf, 2);
	assert( maxRec >= cntRec && maxRec > 0 );
	assert( buf[3] == '\0' );
	for( int i=4; i<49; i++ ) {
		assert( buf[i] == '\0' );
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
	
	//  #0,  1b, unsigned char, dat file number
	//  #1,  2b, short, file type, always 101 (error #1005)
	//  #3,  1b, unsigned char, record length
	//           must be 4 times record count (error #1006)
	//  #4,  1b, unsigned char, record count
	//           must be 4, 5, 6, 7 or 8 (error #1007)
	//  #5,  1b: char, always '\0' (error #1008)
	//  #6,  1b: char, always '\0' (error #1009)
	//  #7, 16b: char*, security name
	//           only alphanumeric characters (error #1010)
	// #23,  2b: short, always 0 (error #1011)
	// #25,  4b: float(ms basic), first date, valid (error #1012)
	// #29,  4b: float(ms basic), last date, valid (error #1013)
	// #33,  1b: char, periodicity, must be 'I', 'D', 'W', 'M' (error #1014)
	// #34,  2b: unsigned short, intraday time frame between 0 and 60 minutes
	//           (error #1015)
	// #36, 14b: char*, symbol, space padded,
	//           not always (or never?) zero terminated
	//           only alphanumeric characters (error #1016)
	// #50,  1b: char, always a space ' ' (error #1017)
	//           note, premium data sets '\0'
	// #51,  1b: char, chart flag, always ' ' or '*' (error #1018)
	//           note, premium data sets '\0'
	// #52,  1b: char, always '\0' (error #1019)
	
	assert( readUnsignedShort( record, 1 ) == 101 );
	assert( record[3] == 4 * record[4] );
	assert( record[4] >= 5 && record[4] <= 8 );
	assert( record[5] == '\0' );
	assert( record[6] == '\0' );
	
	assert( readUnsignedShort( record, 23 ) == 0 );
	int date1 = floatToIntDate_YYY( readFloat( record, 25 ) );
	int date2 = floatToIntDate_YYY( readFloat( record, 29 ) );
	assert( date1 <= date2 );
	assert( record[33] == 'D' || record[33] == 'I' );
	unsigned short intrTimeFrame = readUnsignedShort( record, 34 );
	assert( intrTimeFrame == 0
		|| (record[33] == 'I' && intrTimeFrame > 0 && intrTimeFrame <= 60) );
	
	assert( record[50] == ' ' || record[50] == '\0' );
	assert( record[51] == ' ' || record[51] == '*' || record[51] == '\0' );
	assert( record[52] == '\0' );
	
	
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
	
	trim_end( tmp, record + 36, 14);
	assert( strcmp( mr->c_symbol, tmp ) == 0 );
	tmp_len = trim_end( tmp, record + 7, 16);
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
	
	// note, first 53 bytes seems to be identical to MASTER
	//  #0,  1b, unsigned char, count records (dat files)
	//           must be >0 (error #1023)
	//  #1,  1b, char, always '\0' (~ error #1023)
	//  #2,  1b, unsigned char, max record number (dat file number)
	//           must be >0 (error #1024)
	//  #3,  1b, char, always '\0' (~ error #1024)
	//  #4, 45b, char*, always '\0' ?
	// #49,  4b, int, serial number
	// #53,139b, char*, seems to be vendor specific
	
	unsigned char cntRec = readUnsignedChar(buf, 0);
	assert( cntRec == countRecords() && cntRec > 0 );
	assert( buf[1] == '\0' );
	unsigned char maxRec = readUnsignedChar(buf, 2);
	assert( maxRec >= cntRec && maxRec > 0 );
	assert( buf[3] == '\0' );
	for( int i=4; i<49; i++ ) {
		assert( buf[i] == '\0' );
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
	
	//   #0,  2b: unsigned short, version number, valid (error #1026)
	//   #2,  1b: unsigned char, dat file number, valid (error #1027)
	//   #3,  1b: char, security type always '\0' (error #1028)
	//   #4,  1b: char, security type always '\0' (~error #1028)
	//   #5,  1b: char, security type always '\0' (~error #1028)
	//   #6,  1b: unsigned char, data field count
	//            must be 4, 5, 6, 7 or 8 (error #1029)
	//   #7,  1b: unsigned char, data field bitset
	//   #8,  1b: char, data fields always '\0' (error #1030)
	//   #9,  1b: char, chart flag, always ' ' or '*' (error #1031)
	//  #10,  1b: char, vendor code always '\0' (error #1032)
	//  #11, 14b: char*, symbol, always zero terminated?
	//            only alphanumeric characters (error #1033)
	//  #25,  6b: char*, always zero? (error #1034)
	//  #32, 16b: char*, short name, always zero terminated? (error #1034)
	//  #48, 12b: char*, always zero? (error #1036 and error #1037)
	//  #60,  1b: char, periodicity, must be 'I', 'D', 'W', 'M' (error #1038)
	//  #61,  1b: char, always zero (error #1039)
	//  #62,  2b: short, intraday time frame between 0 and 60 minutes (error #1040)
	// note, we have only times when intraday time == 'I'
	//  #64,  4b: float(ieee), first date, valid (error #1041)
	//  #68,  4b: float(ieee), first time, valid (error #1042)
	//  #72,  4b: float(ieee), last date, valid (error #1043)
	//  #76,  4b: float(ieee), last time, valid (error #1044)
	//  #80,  4b: float(ieee), start trade time, not always set,
	//            valid (error #1045)
	//  #84,  4b: float(ieee), end trade time, not always set,
	//            valid (error #1046)
	//  #88, 38b: char*, assume always zero but should something about composite
	//            ticker stuff, see error #1047 - #1054
	// #126,  4b: int, first date again as integer, equal to #64 or invalid 19000101
	// #130,  9b: char* assume always zero
	// #139, 52b: char*, long name
	// #191,  1b: char, last byte zero
	
	unsigned short version = readUnsignedShort( record, 0 );
	assert( version == 0 || version == 0x3636 );
	assert( record[3]== '\0' && record[4]== '\0' && record[5]== '\0' );
	assert( record[6] >=5 && record[6] <= 8);
	assert( record[6] == count_bits(readUnsignedChar(record, 7)) );
	assert( record[8] == '\0' );
	assert( record[9] == ' ' || record[9] == '*' );
	assert( record[10] == '\0' );
	// if byte 25 is always zero then strlen(symbol) <= 14
	for( int i = 25; i<32; i++ ) {
		assert( record[i] == '\0' );
	}
	// if byte 48 is always zero then strlen(name) <= 16
	for( int i = 48; i<60; i++ ) {
		assert( record[i] == '\0' );
	}
	assert( record[60] == 'D' || record[60] == 'I' );
	assert( record[61] == '\0' );
	unsigned short intrTimeFrame = readUnsignedShort( record, 62 );
	assert( intrTimeFrame == 0
		|| (record[60] == 'I' && intrTimeFrame > 0 && intrTimeFrame <= 60) );
	
	int date1 = floatToIntDate_YYY( readFloat_IEEE( record, 64 ) );
	int time1 = readFloat_IEEE( record, 68 );
	int date2 = floatToIntDate_YYY( readFloat_IEEE( record, 72 ) );
	int time2 = readFloat_IEEE( record, 76 );
	int timeA = readFloat_IEEE( record, 80 );
	int timeB = readFloat_IEEE( record, 84 );
	for( int i = 88; i<126; i++ ) {
		assert( record[i] == '\0' );
	}
	int dateL = readInt( record, 126 );
	
	
	if( date1 > date2 ) {
		//HACK premium data have year like 128 (+1900) but should be 28 (+1900)
		date1 -= 1000000;
	}
	if( dateL > date2 ) {
		//HACK premium data have year like 128 (+1900) but should be 28 (+1900)
		dateL -= 1000000;
	}
	assert( date1 >0 && date2>0 );
	assert( (record[60] != 'I' && time1 == 0 && time2 == 0 && timeA == 0 && timeB == 0)
		|| ( record[60] == 'I' && time1 > 0 && time2 > 0 && timeA >= 0 && timeB >= 0) );
	assert( (long) date1*1000000+time1 <= (long) date2*1000000+time2 );
	
	assert( date1 == dateL || dateL == 19000101 );
	
	for( int i = 130; i<=138; i++ ) {
		assert( record[i] == '\0' );
	}
	assert( record[191] == '\0' );
	
	return true;
}


void EMasterFile::printRecord( const char *record ) const
{
// 	fprintf( stdout, "F%d.dat\t%d\t%d\t%d\t%d\t'%s'\t'%s'\n",
	fprintf( stdout, "F%4d.dat\t%d\t%d\t%c\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%X\t%X\t'%s'\t'%s'\t'%s'\n",
		readUnsignedChar( record, 2),  // F#.dat
		readChar( record, 7 ), // fields bit set
		readChar( record, 6 ), // dat fields count per record
		readChar( record, 60 ), // time frame 'D'
		// never saw these floats
		floatToIntDate_YYY(readFloat_IEEE( record, 64 )), // first date YYY but seems to be wrong
		(int)readFloat_IEEE( record, 68 ),
		floatToIntDate_YYY(readFloat_IEEE( record, 72 )), // last date YYY
		(int)readFloat_IEEE( record, 76 ),
		(int)readFloat_IEEE( record, 80 ),
		(int)readFloat_IEEE( record, 84 ),
		readInt( record, 126 ), // first date YYYY
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
	
	mr->from_date = floatToIntDate_YYY(readFloat_IEEE(record, 64));
	// equis got this record invalid sometimes
	int from_date_2 = readInt(record, 126);
	assert( mr->from_date == from_date_2 || from_date_2 == 19000101 );
	
	mr->to_date = floatToIntDate_YYY(readFloat_IEEE(record, 72));
	if( mr->from_date > mr->to_date ) {
		//HACK premium data have year like 128 (+1900) but should be 28 (+1900)
		mr->from_date -= 1000000;
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
		readInt( record, 108 ), // looks like first date ?
		readInt( record, 116 ), // looks like last date ?
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
	mr->from_date = readInt( record, 108 );
	mr->to_date = readInt( record, 116 );
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
#define DEFAULT_FLOAT -0.0

#define READ_FIELD( _dst_, _field_) \
	if( field_bitset & _field_ ) { \
		 _dst_ = readFloat(record, offset); \
		offset += 4; \
	}

#define PRINT_FIELD( _func_, _field_, _var_ ) \
	if( print_bitset & _field_) { \
		s += _func_( s, _var_ ); \
		*s++ = print_sep; \
	}


int FDat::record_to_string( const char *record, char *s ) const
{
	int offset = 0;
	char *begin = s;
	
	float date, time, open, high , low, close, volume, openint;
	date = time = open = high = low = close = volume = openint = DEFAULT_FLOAT;
	
	READ_FIELD( date, D_DAT );
	READ_FIELD( time, D_TIM );
	READ_FIELD( open, D_OPE );
	READ_FIELD( high, D_HIG );
	READ_FIELD( low, D_LOW );
	READ_FIELD( close, D_CLO );
	READ_FIELD( volume, D_VOL );
	READ_FIELD( openint, D_OPI );
	
	PRINT_FIELD( itodatestr, D_DAT, floatToIntDate_YYY(date) );
	PRINT_FIELD( itotimestr, D_TIM, (int) time );
	PRINT_FIELD( ftoa, D_OPE, open );
	PRINT_FIELD( ftoa, D_HIG, high );
	PRINT_FIELD( ftoa, D_LOW, low );
	PRINT_FIELD( ftoa, D_CLO, close );
	PRINT_FIELD( ftoa_prec_f0, D_VOL, volume );
	PRINT_FIELD( ftoa_prec_f0, D_OPI, openint );
	
	if( s != begin ) {
		*(--s) = '\0';
	} else {
		*s = '\0';
	}
	
	return s - begin;
}

#undef DEFAULT_FLOAT
#undef READ_FIELD


unsigned short FDat::countRecords() const
{
	return readUnsignedShort( buf, 2 ) -1;
}



