/*** metastock.h -- parsing metastock directory
 *
 * Copyright (C) 2010-2016 Ruediger Meier
 *
 * Author:  Ruediger Meier <sweet_f_a@gmx.de>
 *
 * This file is part of atem.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the author nor the names of any contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
