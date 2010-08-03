#ifndef METASTOCK_H
#define METASTOCK_H



class QDir;
class QFile;
class QByteArray;


class Metastock
{
	public:
		Metastock();
		~Metastock();
		
		bool setDir( const char* dir );
		void readMasters();
		
		void dumpInfo() const;
		const char* lastError() const;
		
	private:
		QFile* findMaster( const char *name );
		
		QDir *dir;
		QFile *master;
		QFile *emaster;
		QFile *xmaster;
		
		QByteArray *ba_master;
		QByteArray *ba_emaster;
		QByteArray *ba_xmaster;
		
		const char* error;
};



#endif
