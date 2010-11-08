#ifndef METASTOCK_H
#define METASTOCK_H

struct master_record;

#define ERROR_LENGTH 256

class Metastock
{
	public:
		Metastock();
		~Metastock();
		
		bool hasXMaster() const;
		
		bool setDir( const char* dir );
		bool setOutputFormat(  char sep, int fmt_data, int fmt_symbols );
		
		void parseMasters();
		void dumpMaster() const;
		void dumpEMaster() const;
		void dumpXMaster() const;
		bool dumpSymbolInfo( unsigned short f ) const;
		bool dumpData( unsigned short f ) const;
		const char* lastError() const;
		
	private:
		void setError( const char* e1, const char* e2 = "" ) const;
		bool findFiles();
		bool readFile( const char *file_name , char *buf, int *len ) const;
		bool readMasters();
		int build_mr_string( char *dst, const master_record *mr ) const;
		bool dumpData( unsigned short number, unsigned char fields,
			const char *pfx) const;
		
		static char print_sep;
		static unsigned short prnt_master_fields;
		static unsigned char prnt_data_fields;
		static unsigned short prnt_data_mr_fields;
		
		
		char *ms_dir;
		char *master_name;
		char *emaster_name;
		char *xmaster_name;
		int mr_len;
		master_record *mr_list;
		
		char *ba_master;
		int master_len;
		char *ba_emaster;
		int emaster_len;
		char *ba_xmaster;
		int xmaster_len;
		
		char *ba_fdat;
		
		mutable char error[ERROR_LENGTH];
};



#endif
