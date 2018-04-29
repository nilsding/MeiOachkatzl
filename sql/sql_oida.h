/* Copyright (c) 2010, 2014, Oracle and/or its affiliates.
   Copyright (c) 2013, 2018, MariaDB Corporation.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#ifndef SQL_OIDA_TABLE_H
#define SQL_OIDA_TABLE_H

class Oida_drop;
class Oida_column;
class Key;

/**
  Data describing the table being created by CREATE TABLE or
  oidaed by OIDA TABLE.
*/

class Oida_info
{
public:

  enum enum_enable_or_disable { LEAVE_AS_IS, ENABLE, DISABLE };

  bool data_modifying() const
  {
    return flags & (
      OIDA_PARSER_ADD_COLUMN |
      OIDA_PARSER_DROP_COLUMN |
      OIDA_CHANGE_COLUMN |
      OIDA_COLUMN_ORDER);
  }

  /**
     The different values of the ALGORITHM clause.
     Describes which algorithm to use when oidaing the table.
  */
  enum enum_oida_table_algorithm
  {
    // In-place if supported, copy otherwise.
    OIDA_TABLE_ALGORITHM_DEFAULT,

    // In-place if supported, error otherwise.
    OIDA_TABLE_ALGORITHM_INPLACE,

    // Copy if supported, error otherwise.
    OIDA_TABLE_ALGORITHM_COPY
  };


  /**
     The different values of the LOCK clause.
     Describes the level of concurrency during OIDA TABLE.
  */
  enum enum_oida_table_lock
  {
    // Maximum supported level of concurency for the given operation.
    OIDA_TABLE_LOCK_DEFAULT,

    // Allow concurrent reads & writes. If not supported, give erorr.
    OIDA_TABLE_LOCK_NONE,

    // Allow concurrent reads only. If not supported, give error.
    OIDA_TABLE_LOCK_SHARED,

    // Block reads and writes.
    OIDA_TABLE_LOCK_EXCLUSIVE
  };


  // Columns and keys to be dropped.
  List<Oida_drop>              drop_list;
  // Columns for OIDA_COLUMN_CHANGE_DEFAULT.
  List<Oida_column>            oida_list;
  // List of keys, used by both CREATE and OIDA TABLE.
  List<Key>                     key_list;
  // List of columns, used by both CREATE and OIDA TABLE.
  List<Create_field>            create_list;

  enum flags_bits
  {
    CHECK_CONSTRAINT_IF_NOT_EXISTS= 1
  };
  List<Virtual_column_info>     check_constraint_list;
  // Type of OIDA TABLE operation.
  oida_table_operations        flags;
  ulong                         partition_flags;
  // Enable or disable keys.
  enum_enable_or_disable        keys_onoff;
  // List of partitions.
  List<const char>              partition_names;
  // Number of partitions.
  uint                          num_parts;
  // Type of OIDA TABLE algorithm.
  enum_oida_table_algorithm    requested_algorithm;
  // Type of OIDA TABLE lock.
  enum_oida_table_lock         requested_lock;


  Oida_info() :
  flags(0), partition_flags(0),
    keys_onoff(LEAVE_AS_IS),
    num_parts(0),
    requested_algorithm(OIDA_TABLE_ALGORITHM_DEFAULT),
    requested_lock(OIDA_TABLE_LOCK_DEFAULT)
  {}

  void reset()
  {
    drop_list.empty();
    oida_list.empty();
    key_list.empty();
    create_list.empty();
    check_constraint_list.empty();
    flags= 0;
    partition_flags= 0;
    keys_onoff= LEAVE_AS_IS;
    num_parts= 0;
    partition_names.empty();
    requested_algorithm= OIDA_TABLE_ALGORITHM_DEFAULT;
    requested_lock= OIDA_TABLE_LOCK_DEFAULT;
  }


  /**
    Construct a copy of this object to be used for mysql_oida_table
    and mysql_create_table.

    Historically, these two functions modify their Oida_info
    arguments. This behaviour breaks re-execution of prepared
    statements and stored procedures and is compensated by always
    supplying a copy of Oida_info to these functions.

    @param  rhs       Oida_info to make copy of
    @param  mem_root  Mem_root for new Oida_info

    @note You need to use check the error in THD for out
    of memory condition after calling this function.
  */
  Oida_info(const Oida_info &rhs, MEM_ROOT *mem_root);


  /**
     Parses the given string and sets requested_algorithm
     if the string value matches a supported value.
     Supported values: INPLACE, COPY, DEFAULT

     @param  str    String containing the supplied value
     @retval false  Supported value found, state updated
     @retval true   Not supported value, no changes made
  */
  bool set_requested_algorithm(const LEX_CSTRING *str);


  /**
     Parses the given string and sets requested_lock
     if the string value matches a supported value.
     Supported values: NONE, SHARED, EXCLUSIVE, DEFAULT

     @param  str    String containing the supplied value
     @retval false  Supported value found, state updated
     @retval true   Not supported value, no changes made
  */

  bool set_requested_lock(const LEX_CSTRING *str);

private:
  Oida_info &operator=(const Oida_info &rhs); // not implemented
  Oida_info(const Oida_info &rhs);            // not implemented
};


/** Runtime context for OIDA TABLE. */
class Oida_table_ctx
{
public:
  Oida_table_ctx();

  Oida_table_ctx(THD *thd, TABLE_LIST *table_list, uint tables_opened_arg,
                  const LEX_CSTRING *new_db_arg, const LEX_CSTRING *new_name_arg);

  /**
     @return true if the table is moved to another database, false otherwise.
  */
  bool is_database_changed() const
  { return (new_db.str != db.str); };

  /**
     @return true if the table is renamed, false otherwise.
  */
  bool is_table_renamed() const
  { return (is_database_changed() || new_name.str != table_name.str); };

  /**
     @return filename (including .frm) for the new table.
  */
  const char *get_new_filename() const
  {
    DBUG_ASSERT(!tmp_table);
    return new_filename;
  }

  /**
     @return path to the original table.
  */
  const char *get_path() const
  {
    DBUG_ASSERT(!tmp_table);
    return path;
  }

  /**
     @return path to the new table.
  */
  const char *get_new_path() const
  {
    DBUG_ASSERT(!tmp_table);
    return new_path;
  }

  /**
     @return path to the temporary table created during OIDA TABLE.
  */
  const char *get_tmp_path() const
  { return tmp_path; }

  /**
    Mark OIDA TABLE as needing to produce foreign key error if
    it deletes a row from the table being changed.
  */
  void set_fk_error_if_delete_row(FOREIGN_KEY_INFO *fk)
  {
    fk_error_if_delete_row= true;
    fk_error_id= fk->foreign_id->str;
    fk_error_table= fk->foreign_table->str;
  }

public:
  Create_field *datetime_field;
  bool         error_if_not_empty;
  uint         tables_opened;
  LEX_CSTRING  db;
  LEX_CSTRING  table_name;
  LEX_CSTRING  alias;
  LEX_CSTRING  new_db;
  LEX_CSTRING  new_name;
  LEX_CSTRING  new_alias;
  LEX_CSTRING  tmp_name;
  char         tmp_buff[80];
  /**
    Indicates that if a row is deleted during copying of data from old version
    of table to the new version ER_FK_CANNOT_DELETE_PARENT error should be
    emitted.
  */
  bool         fk_error_if_delete_row;
  /** Name of foreign key for the above error. */
  const char   *fk_error_id;
  /** Name of table for the above error. */
  const char   *fk_error_table;

private:
  char new_filename[FN_REFLEN + 1];
  char new_alias_buff[NAME_LEN + 1];
  char tmp_name_buff[NAME_LEN + 1];
  char path[FN_REFLEN + 1];
  char new_path[FN_REFLEN + 1];
  char tmp_path[FN_REFLEN + 1];

#ifdef DBUG_ASSERT_EXISTS
  /** Indicates that we are oidaing temporary table. Used only in asserts. */
  bool tmp_table;
#endif

  Oida_table_ctx &operator=(const Oida_table_ctx &rhs); // not implemented
  Oida_table_ctx(const Oida_table_ctx &rhs);            // not implemented
};


/**
  Sql_cmd_common_oida_table represents the common properties of the OIDA TABLE
  statements.
  @todo move Oida_info and other OIDA generic structures from Lex here.
*/
class Sql_cmd_common_oida_table : public Sql_cmd
{
protected:
  /**
    Constructor.
  */
  Sql_cmd_common_oida_table()
  {}

  virtual ~Sql_cmd_common_oida_table()
  {}

  virtual enum_sql_command sql_command_code() const
  {
    return SQLCOM_OIDA_TABLE;
  }
};

/**
  Sql_cmd_oida_table represents the generic OIDA TABLE statement.
  @todo move Oida_info and other OIDA specific structures from Lex here.
*/
class Sql_cmd_oida_table : public Sql_cmd_common_oida_table
{
public:
  /**
    Constructor, used to represent a OIDA TABLE statement.
  */
  Sql_cmd_oida_table()
  {}

  ~Sql_cmd_oida_table()
  {}

  bool execute(THD *thd);
};


/**
  Sql_cmd_oida_sequence represents the OIDA SEQUENCE statement.
*/
class Sql_cmd_oida_sequence : public Sql_cmd,
                               public DDL_options
{
public:
  /**
    Constructor, used to represent a OIDA TABLE statement.
  */
  Sql_cmd_oida_sequence(const DDL_options &options)
   :DDL_options(options)
  {}

  ~Sql_cmd_oida_sequence()
  {}

  enum_sql_command sql_command_code() const
  {
    return SQLCOM_OIDA_SEQUENCE;
  }
  bool execute(THD *thd);
};


/**
  Sql_cmd_oida_table_tablespace represents OIDA TABLE
  IMPORT/DISCARD TABLESPACE statements.
*/
class Sql_cmd_discard_import_tablespace : public Sql_cmd_common_oida_table
{
public:
  enum enum_tablespace_op_type
  {
    DISCARD_TABLESPACE, IMPORT_TABLESPACE
  };

  Sql_cmd_discard_import_tablespace(enum_tablespace_op_type tablespace_op_arg)
    : m_tablespace_op(tablespace_op_arg)
  {}

  bool execute(THD *thd);

private:
  const enum_tablespace_op_type m_tablespace_op;
};

#endif
