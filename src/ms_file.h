/*** ms_file.h -- parsing metastock files
 *
 * Copyright (C) 2010-2012 Ruediger Meier
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

#ifndef ATEM_MS_FILE_H
#define ATEM_MS_FILE_H




enum ms_master_field {
	// master fields
	M_SYM = 01,
	M_NAM = 02,
	M_PER = 04,
	M_DT1 = 010,
	M_DT2 = 020,
	M_FNO = 040,
	M_FIL = 0100,
	M_FLD = 0200,
	M_RNO = 0400,
	M_KND = 01000,
};

enum ms_data_field {
	// data fields
	D_DAT = 01,
	D_HIG = 02,
	D_LOW = 04,
	D_CLO = 010,
	D_VOL = 020,
	D_OPE = 040,
	D_OPI = 0100,
	D_TIM = 0200
};


#define MAX_LEN_MR_SYMBOL 14
#define MAX_LEN_MR_LNAME 45
#define MAX_LEN_MR_FILENAME 10


struct master_record
{
	unsigned short record_number; /* position in master file */
	char kind; /* (M)aster, (E)master, (X)Master */
	unsigned short file_number; /* M, E, X */
// 	int record_length; /* M */
// 	int fields_per_record; /* M, E */
	unsigned char field_bitset; /* E, X */
	char barsize; /* E, X */
	char c_symbol[MAX_LEN_MR_SYMBOL + 1]; /* M, E, X */
// 	char c_short_name[64]; /* M, E */
	char c_long_name[MAX_LEN_MR_LNAME + 1]; /* E, X  */
	char file_name[MAX_LEN_MR_FILENAME + 1];
	int from_date;
	int to_date;
};

/* estimated maximum string length returned by mr_record_to_string()
   sizes of ints (incl. seperators) + char* lengths (+/- seperator/zero) */
#define MAX_SIZE_MR_STRING ( 6 + 2 + 6 + 4 + 2 \
	+ MAX_LEN_MR_SYMBOL + 1 + MAX_LEN_MR_LNAME + 1 + MAX_LEN_MR_FILENAME + 1 \
	+ 9 + 9 )


int mr_record_to_string( char *dest, const struct master_record*,
	unsigned short print_bitset, char sep );

int mr_header_to_string( char *dest, unsigned short print_bitset, char sep );



class MasterFile
{
	public:
		MasterFile( const char *buf, int size );
		
		static bool checkHeader( const char* buf );
		static bool checkRecord( const char* buf, int record  );
		
		bool check() const;
		int countRecords() const;
		int getRecord( master_record *, unsigned short rnum ) const;
		int fileNumber( int record ) const;
		int dataLength( int record ) const;
		
	private:
		bool checkHeader() const;
		bool checkRecords() const;
		bool checkRecord( unsigned char r ) const;
		
		void printHeader() const;
		void printRecord( const char *record ) const;
		
		
		static const int record_length = 53;
		
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
		int countRecords() const;
		int getLongName( master_record *, unsigned short rnum ) const;
		int getRecord( master_record *, unsigned short rnum ) const;
		int fileNumber( int record ) const;
		int dataLength( int record ) const;
		
	private:
		bool checkHeader() const;
		bool checkRecords() const;
		bool checkRecord( unsigned char r ) const;
		
		void printHeader() const;
		void printRecord( const char *record ) const;
		
		
		static const int record_length = 192;
		
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
		int countRecords() const;
		int getRecord( master_record *, unsigned short rnum ) const;
		int fileNumber( int record ) const;
		int dataLength( int record ) const;
		
	private:
		bool checkHeader() const;
		bool checkRecords() const;
		bool checkRecord( int r ) const;
		
		void printHeader() const;
		void printRecord( const char *record ) const;
		
		
		static const int record_length = 150;
		
		const char * const buf;
		const int size;
};



typedef int (*ftoa_func)(char*, float);

struct glue_s {
	int date, time;
	float open, high, low, close, volume, openint;
};

class FDat
{
	public:
		FDat( const char *buf, int size, unsigned char fields );
		
		static bool checkHeader( const char* buf );
		static bool checkRecord( const char* buf, int record  );
		static void set_outfile( void *file );
		static void initPrinter( char sep, unsigned int bitset );
		static void setPrintDateFrom( int date );
		static void setForceFloat( ms_data_field );
		static void print_header( const char* symbol_header );
		
		bool checkHeader() const;
		int print( const char* header ) const;
		unsigned short countRecords() const;

		/** iterator */
		int iter( int(*cb)(struct glue_s, void *clo), void *clo ) const;

	private:
		static int header_to_string( char *s );
		int record_to_string( const char *record, char *s ) const;
		struct glue_s record_to_glue( const char *record ) const;
		
		static void *out;
		static char print_sep;
		static unsigned int print_bitset;
		static int print_date_from;
		static ftoa_func prc_ftoa;
		static ftoa_func vol_ftoa;
		static ftoa_func opi_ftoa;
		
		const unsigned char field_bitset;
		const unsigned int record_length;
		
		const char * const buf;
		const int size;
};




#endif
