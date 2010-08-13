#ifndef ATEM_MS_FILE_H
#define ATEM_MS_FILE_H


class MasterFile
{
	public:
		MasterFile( const char *buf, int size );
		
		static bool checkHeader( const char* buf );
		static bool checkRecord( const char* buf, int record  );
		
		bool check() const;
		unsigned char countRecords() const;
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
		FDat( const char *buf, int size, int l );
		
		static bool checkHeader( const char* buf );
		static bool checkRecord( const char* buf, int record  );
		
		bool checkHeader() const;
		void print( const char* header ) const;
		unsigned short countRecords() const;
		
	private:
		int record_to_string( const char *record, char *s ) const;
		
		
		const unsigned int record_length;
		
		const char * const buf;
		const int size;
};




#endif
