#ifndef ATEM_MS_FILE_H
#define ATEM_MS_FILE_H




enum ms_master_field {
	// master fields
	M_SYM = 01,
	M_NAM = 02,
	M_PER = 04,
	M_FNO = 010,
	M_FIL = 020,
	M_FLD = 040,
	M_RNO = 0100,
	M_KND = 0200,
};

enum ms_data_field {
	// data fields
	D_DAT = 01,
	D_OPE = 02,
	D_HIG = 04,
	D_LOW = 010,
	D_CLO = 020,
	D_VOL = 040,
	D_OPI = 0100,
	D_TIM = 0200
};



struct master_record
{
	unsigned short record_number; /* position in master file */
	char kind; /* (M)aster, (E)master, (X)Master */
	unsigned short file_number; /* M, E, X */
// 	int record_length; /* M */
// 	int fields_per_record; /* M, E */
	unsigned char field_bitset; /* E, X */
	char barsize; /* E, X */
	char c_symbol[17]; /* M, E, X */
// 	char c_short_name[64]; /* M, E */
	char c_long_name[64]; /* E, X  */
	char file_name[11];
};

/* estimated maximum string length returned by mr_record_to_string()
   sizes of ints (incl. seperators) + char* lengths (+/- seperator/zero) */
#define MAX_SIZE_MR_STRING (6 + 2 + 6 + 4 + 2) + (17 + 64 + 11)


int mr_record_to_string( char *dest, const struct master_record*,
	unsigned char print_bitset, char sep );




class MasterFile
{
	public:
		MasterFile( const char *buf, int size );
		
		static bool checkHeader( const char* buf );
		static bool checkRecord( const char* buf, int record  );
		
		bool check() const;
		unsigned char countRecords() const;
		int getRecord( const master_record *, unsigned short rnum ) const;
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




class EMasterFile
{
	public:
		EMasterFile( const char *buf, int size );
		
		static bool checkHeader( const char* buf );
		static bool checkRecord( const char* buf, int record  );
		
		bool check() const;
		unsigned char countRecords() const;
		int getRecord( master_record *, unsigned short rnum ) const;
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




class XMasterFile
{
	public:
		XMasterFile( const char *buf, int size );
		
		static bool checkHeader( const char* buf );
		static bool checkRecord( const char* buf, int record  );
		
		bool check() const;
		unsigned short countRecords() const;
		int getRecord( master_record *, unsigned short rnum ) const;
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




class FDat
{
	public:
		FDat( const char *buf, int size, unsigned char fields );
		
		static bool checkHeader( const char* buf );
		static bool checkRecord( const char* buf, int record  );
		static void initPrinter( char sep, unsigned int bitset );
		
		bool checkHeader() const;
		void print( const char* header ) const;
		unsigned short countRecords() const;
		
	private:
		int record_to_string( const char *record, char *s ) const;
		
		static char print_sep;
		static unsigned int print_bitset;
#if ! defined FAST_PRINTING
		static char sprintf_format[64];
#endif
		
		const unsigned char field_bitset;
		const unsigned int record_length;
		
		const char * const buf;
		const int size;
};




#endif
