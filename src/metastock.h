#ifndef METASTOCK_H
#define METASTOCK_H_H




class QDir;
class QFile;


class Metastock
{
	public:
		Metastock();
		~Metastock();
		
		bool setDir( const char* dir );
		
		void dumpInfo() const;
		const char* lastError() const;
		
	private:
		QFile* findMaster( const char *name );
		
		QDir *dir;
		QFile *master;
		QFile *emaster;
		QFile *xmaster;
		
		const char* error;
};



#endif
