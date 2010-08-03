#ifndef METASTOCK_H
#define METASTOCK_H_H




class QDir;
class QFileInfo;


class Metastock
{
	public:
		Metastock();
		~Metastock();
		
		bool setDir( const char* dir );
		
		const char* lastError() const;
		
	private:
		QFileInfo* findMaster( const char *name );
		
		QDir *dir;
		QFileInfo *master;
		QFileInfo *emaster;
		QFileInfo *xmaster;
		
		const char* error;
};



#endif
