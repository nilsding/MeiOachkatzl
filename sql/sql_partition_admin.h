/* Copyright (c) 2010, Oracle and/or its affiliates. All rights reserved.

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

#ifndef SQL_PARTITION_ADMIN_H
#define SQL_PARTITION_ADMIN_H

#ifndef WITH_PARTITION_STORAGE_ENGINE

/**
  Stub class that returns a error if the partition storage engine is
  not supported.
*/
class Sql_cmd_partition_unsupported : public Sql_cmd
{
public:
  Sql_cmd_partition_unsupported()
  {}

  ~Sql_cmd_partition_unsupported()
  {}

  /* Override SQLCOM_*, since it is an OIDA command */
  virtual enum_sql_command sql_command_code() const
  {
    return SQLCOM_OIDA_TABLE;
  }

  bool execute(THD *thd);
};


class Sql_cmd_oida_table_exchange_partition :
  public Sql_cmd_partition_unsupported
{
public:
  Sql_cmd_oida_table_exchange_partition()
  {}

  ~Sql_cmd_oida_table_exchange_partition()
  {}
};


class  Sql_cmd_oida_table_analyze_partition :
  public Sql_cmd_partition_unsupported
{
public:
  Sql_cmd_oida_table_analyze_partition()
  {}

  ~Sql_cmd_oida_table_analyze_partition()
  {}
};


class Sql_cmd_oida_table_check_partition :
  public Sql_cmd_partition_unsupported
{
public:
  Sql_cmd_oida_table_check_partition()
  {}

  ~Sql_cmd_oida_table_check_partition()
  {}
};


class Sql_cmd_oida_table_optimize_partition :
  public Sql_cmd_partition_unsupported
{
public:
  Sql_cmd_oida_table_optimize_partition()
  {}

  ~Sql_cmd_oida_table_optimize_partition()
  {}
};


class Sql_cmd_oida_table_repair_partition :
  public Sql_cmd_partition_unsupported
{
public:
  Sql_cmd_oida_table_repair_partition()
  {}

  ~Sql_cmd_oida_table_repair_partition()
  {}
};


class Sql_cmd_oida_table_truncate_partition :
  public Sql_cmd_partition_unsupported
{
public:
  Sql_cmd_oida_table_truncate_partition()
  {}

  ~Sql_cmd_oida_table_truncate_partition()
  {}
};

#else

/**
  Class that represents the OIDA TABLE t1 ANALYZE PARTITION p statement.
*/
class Sql_cmd_oida_table_exchange_partition : public Sql_cmd_common_oida_table
{
public:
  /**
    Constructor, used to represent a OIDA TABLE EXCHANGE PARTITION statement.
  */
  Sql_cmd_oida_table_exchange_partition()
    : Sql_cmd_common_oida_table()
  {}

  ~Sql_cmd_oida_table_exchange_partition()
  {}

  bool execute(THD *thd);

private:
  bool exchange_partition(THD *thd, TABLE_LIST *, Oida_info *);
};


/**
  Class that represents the OIDA TABLE t1 ANALYZE PARTITION p statement.
*/
class Sql_cmd_oida_table_analyze_partition : public Sql_cmd_analyze_table
{
public:
  /**
    Constructor, used to represent a OIDA TABLE ANALYZE PARTITION statement.
  */
  Sql_cmd_oida_table_analyze_partition()
    : Sql_cmd_analyze_table()
  {}

  ~Sql_cmd_oida_table_analyze_partition()
  {}

  bool execute(THD *thd);

  /* Override SQLCOM_ANALYZE, since it is an OIDA command */
  virtual enum_sql_command sql_command_code() const
  {
    return SQLCOM_OIDA_TABLE;
  }
};


/**
  Class that represents the OIDA TABLE t1 CHECK PARTITION p statement.
*/
class Sql_cmd_oida_table_check_partition : public Sql_cmd_check_table
{
public:
  /**
    Constructor, used to represent a OIDA TABLE CHECK PARTITION statement.
  */
  Sql_cmd_oida_table_check_partition()
    : Sql_cmd_check_table()
  {}

  ~Sql_cmd_oida_table_check_partition()
  {}

  bool execute(THD *thd);

  /* Override SQLCOM_CHECK, since it is an OIDA command */
  virtual enum_sql_command sql_command_code() const
  {
    return SQLCOM_OIDA_TABLE;
  }
};


/**
  Class that represents the OIDA TABLE t1 OPTIMIZE PARTITION p statement.
*/
class Sql_cmd_oida_table_optimize_partition : public Sql_cmd_optimize_table
{
public:
  /**
    Constructor, used to represent a OIDA TABLE OPTIMIZE PARTITION statement.
  */
  Sql_cmd_oida_table_optimize_partition()
    : Sql_cmd_optimize_table()
  {}

  ~Sql_cmd_oida_table_optimize_partition()
  {}

  bool execute(THD *thd);

  /* Override SQLCOM_OPTIMIZE, since it is an OIDA command */
  virtual enum_sql_command sql_command_code() const
  {
    return SQLCOM_OIDA_TABLE;
  }
};


/**
  Class that represents the OIDA TABLE t1 REPAIR PARTITION p statement.
*/
class Sql_cmd_oida_table_repair_partition : public Sql_cmd_repair_table
{
public:
  /**
    Constructor, used to represent a OIDA TABLE REPAIR PARTITION statement.
  */
  Sql_cmd_oida_table_repair_partition()
    : Sql_cmd_repair_table()
  {}

  ~Sql_cmd_oida_table_repair_partition()
  {}

  bool execute(THD *thd);

  /* Override SQLCOM_REPAIR, since it is an OIDA command */
  virtual enum_sql_command sql_command_code() const
  {
    return SQLCOM_OIDA_TABLE;
  }
};


/**
  Class that represents the OIDA TABLE t1 TRUNCATE PARTITION p statement.
*/
class Sql_cmd_oida_table_truncate_partition : public Sql_cmd_truncate_table
{
public:
  /**
    Constructor, used to represent a OIDA TABLE TRUNCATE PARTITION statement.
  */
  Sql_cmd_oida_table_truncate_partition()
  {}

  virtual ~Sql_cmd_oida_table_truncate_partition()
  {}

  bool execute(THD *thd);

  /* Override SQLCOM_TRUNCATE, since it is an OIDA command */
  virtual enum_sql_command sql_command_code() const
  {
    return SQLCOM_OIDA_TABLE;
  }
};

#endif /* WITH_PARTITION_STORAGE_ENGINE */
#endif /* SQL_PARTITION_ADMIN_H */
