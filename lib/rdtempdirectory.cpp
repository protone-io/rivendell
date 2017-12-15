// rdtempdirectory.cpp
//
// Securely create and then remove a temporary directory
//
//   (C) Copyright 2017 Fred Gleason <fredg@paravelsystems.com>
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License version 2 as
//   published by the Free Software Foundation.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public
//   License along with this program; if not, write to the Free Software
//   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#include <errno.h>
#include <stdlib.h>
#ifndef WIN32
#include <unistd.h>
#endif  // WIN32

#include <qdatetime.h>
#include <qstringlist.h>

#include "rdconfig.h"
#include "rdtempdirectory.h"

RDTempDirectory::RDTempDirectory(const QString &base_name)
{
  temp_base_name=base_name;
  temp_dir=NULL;
}


RDTempDirectory::~RDTempDirectory()
{
  if(temp_dir!=NULL) {
    QStringList files=temp_dir->entryList(QDir::Files);
    for(unsigned i=0;i<files.size();i++) {
      temp_dir->remove(files[i]);
    }
    temp_dir->rmdir(temp_dir->path());
    delete temp_dir;
  }
}


QString RDTempDirectory::path() const
{
  if(temp_dir==NULL) {
    return QString();
  }
  return temp_dir->path();
}


bool RDTempDirectory::create(QString *err_msg)
{
#ifdef WIN32
  QDateTime now=QDateTime::currentDateTime();
  QString tempdir=RDTempDirectory::basePath()+"/"+temp_base_name+
    QString().sprintf("%u",now.toTime_t());
  temp_dir=new QDir(tempdir);
  if(!temp_dir->mkdir(tempdir)) {
    *err_msg="unable to create temp directory";
    delete temp_dir;
    return false;
  }
#else
  char tempdir[PATH_MAX];

  strncpy(tempdir,RDTempDirectory::basePath(),PATH_MAX);
  strncat(tempdir,"/",PATH_MAX-strlen(tempdir));
  strncat(tempdir,temp_base_name,PATH_MAX-strlen(tempdir));
  strncat(tempdir,"XXXXXX",PATH_MAX-strlen(tempdir));
  if(mkdtemp(tempdir)==NULL) {
    *err_msg=strerror(errno);
    return false;
  }
  temp_dir=new QDir(tempdir);
#endif  // WIN32

  return true;
}


QString RDTempDirectory::basePath()
{
  QString conf_temp_directory = RDConfiguration()->tempDirectory();
  if (!conf_temp_directory.isEmpty()) {
    return conf_temp_directory;
  }
#ifdef WIN32
  if(getenv("TEMP")!=NULL) {
    return QString(getenv("TEMP"));
  }
  if(getenv("TMP")!=NULL) {
    return QString(getenv("TMP"));
  }
  return QString("C:\\");
#else
  if(getenv("TMPDIR")!=NULL) {
    return QString(getenv("TMPDIR"));
  }
  return QString("/tmp");
#endif  // WIN32
}
