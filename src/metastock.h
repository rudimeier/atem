/*** metastock.h -- parsing metastock directory
 *
 * Copyright (C) 2010-2017 Ruediger Meier
 * Author:  Ruediger Meier <sweet_f_a@gmx.de>
 * License: BSD 3-Clause, see LICENSE file
 *
 ***/

#ifndef METASTOCK_H
#define METASTOCK_H

struct master_record;
class FileBuf;


#define ERROR_LENGTH 256

class Metastock
{
	public:
		Metastock();
		~Metastock();

		bool set_outfile( const char *file );
		bool setDir( const char* dir );
		bool set_field_sep( const char *sep );
		void set_skip_header( int skipheader );
		void set_out_format( int fmt_data );
		bool set_out_format( const char *columns );
		bool set_ignore_masters( bool master, bool emaster, bool xmaster );
		bool setForceFloat( bool opi, bool vol );
		bool setPrintDateFrom( const char *date );

		bool parseMasters();
		void dumpMaster() const;
		void dumpEMaster() const;
		void dumpXMaster() const;
		bool incudeFile( int f ) const;
		bool excludeFiles( const char *stamp ) const;
		bool dumpSymbolInfo() const;
		bool dumpData() const;
		const char* lastError() const;

	private:
		void printWarn( const char* e1, const char* e2 = "" ) const;
		void setError( const char* e1, const char* e2 = "" ) const;
		bool findFiles();
		bool readFile( FileBuf *file_buf ) const;
		bool readMasters();
		void resize_mr_list( int new_len );
		void add_mr_list_datfile( int datnum, const char* datname );
		void format_incl( unsigned int fmt_data );
		void format_excl( unsigned int fmt_data );
		bool columns2bitset( const char *columns );
		bool dumpData( unsigned short number, unsigned char fields,
			const char *pfx) const;

		static bool print_header;
		static char print_sep;
		static unsigned short use_master_files;
		static unsigned short prnt_master_fields;
		static unsigned char prnt_data_fields;
		static unsigned short prnt_data_mr_fields;
		int print_date_from;

		char *ms_dir;
		FileBuf *m_buf;
		FileBuf *e_buf;
		FileBuf *x_buf;
		FileBuf *fdat_buf;

		int max_dat_num;
		int mr_len;
		master_record *mr_list;
		bool *mr_skip_list;

		void *out;

		mutable char error[ERROR_LENGTH];
};



#endif
