/* Copyright (C) 2003 MySQL AB

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

#include <ndb_global.h>

#include <NdbOut.hpp>
#include <NdbApi.hpp>
#include <NDBT.hpp>

#include <getarg.h>

static int g_diskbased = 0;
static const char* g_tsname = 0;

static int
g_create_hook(Ndb* ndb, NdbDictionary::Table& tab, int when, void* arg)
{
  if (when == 0) {
    if (g_diskbased) {
      for (int i = 0; i < tab.getNoOfColumns(); i++) {
        NdbDictionary::Column* col = tab.getColumn(i);
        if (! col->getPrimaryKey()) {
          col->setStorageType(NdbDictionary::Column::StorageTypeDisk);
        }
      }
    }
    if (g_tsname != NULL) {
      tab.setTablespace(g_tsname);
    }
  }
  return 0;
}

int main(int argc, const char** argv){
  ndb_init();

  int _temp = false;
  int _help = 0;
  int _all = 0;
  int _print = 0;
  const char* _connectstr = NULL;
  int _diskbased = 0;
  const char* _tsname = NULL;

  struct getargs args[] = {
    { "all", 'a', arg_flag, &_all, "Create/print all tables", 0 },
    { "print", 'p', arg_flag, &_print, "Print table(s) instead of creating it", 0},
    { "temp", 't', arg_flag, &_temp, "Temporary table", 0 },
    { "connstr", 'c', arg_string, &_connectstr, "Connect string", "cs" }, 
    { "diskbased", 0, arg_flag, &_diskbased, "Store attrs on disk if possible", 0 },
    { "tsname", 0, arg_string, &_tsname, "Tablespace name", "ts" },
    { "usage", '?', arg_flag, &_help, "Print help", "" }
  };
  int num_args = sizeof(args) / sizeof(args[0]);
  int optind = 0;
  char desc[] = 
    "tabname\n"\
    "This program will create one table in Ndb.\n"\
    "The tables may be selected from a fixed list of tables\n"\
    "defined in NDBT_Tables class\n";

  if(getarg(args, num_args, argc, argv, &optind) || _help){
    arg_printusage(args, num_args, argv[0], desc);
    return NDBT_ProgramExit(NDBT_WRONGARGS);
  }

  if(argv[optind] == NULL && !_all){
    arg_printusage(args, num_args, argv[0], desc);
    return NDBT_ProgramExit(NDBT_WRONGARGS);
  }

  g_diskbased = _diskbased;
  g_tsname = _tsname;

  int res = 0;
  if(_print){
    /**
     * Print instead of creating
     */
    if(optind < argc){
      for(int i = optind; i<argc; i++){
	NDBT_Tables::print(argv[i]);
      }
    } else {
      NDBT_Tables::printAll();
    }
  } else {
    /**
     * Creating
     */
    
    // Connect to Ndb
    Ndb_cluster_connection con(_connectstr);
    if(con.connect(12, 5, 1) != 0)
    {
      return NDBT_ProgramExit(NDBT_FAILED);
    }
    Ndb MyNdb(&con, "TEST_DB" );
    
    if(MyNdb.init() != 0){
      ERR(MyNdb.getNdbError());
      return NDBT_ProgramExit(NDBT_FAILED);
    }
    
    while(MyNdb.waitUntilReady() != 0)
      ndbout << "Waiting for ndb to become ready..." << endl;
    
    if(_all){
      res = NDBT_Tables::createAllTables(&MyNdb, _temp);
    } else {
      int tmp;
      for(int i = optind; i<argc; i++){
	ndbout << "Trying to create " <<  argv[i] << endl;
	if((tmp = NDBT_Tables::createTable(&MyNdb, argv[i], _temp, false,
                                           g_create_hook)) != 0)
	  res = tmp;
      }
    } 
  }
  
  if(res != 0)
    return NDBT_ProgramExit(NDBT_FAILED);
  else
    return NDBT_ProgramExit(NDBT_OK);
}
