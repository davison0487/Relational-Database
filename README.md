# Relational Database

This project is to build a relational database system from scratch that follows MVC pattern.

## Supporting Commands

### Application Level

`help, version, quit`

Help command is just a place holder for future implementation, no existing helping system is implemented.

### Database Level

`CREATE DATABASE {db-name}`, `DROP DATABASE {db-name}`, `SHOW DATABASES`, `USE {db-name}`

These commands relate to creating, listing, and managing database containers.

`DUMP DATABASE {db-name}`

This command is used for internal debugging.

### Table Related

`CREATE TABLE {table-name}` : Create a new table. Below is an example,

`CREATE TABLE test1 (id int NOT NULL auto_increment primary key, first_name varchar(50) NOT NULL, last_name VARCHAR(50));`

#### Available Field Information

```
- field_name
- field_type  (bool, float, integer, timestamp, varchar)  //varchar has length
- field_length (only applies to varchar fields)
- auto_increment (determines if this (integer) field is autoincremented by DB
- primary_key  (bool indicates that field represents primary key)
- nullable (bool indicates the field can be null)
```

`DROP TABLE {table-name}` : Delete the associated table.

`DESCRIBE {table-name}` : Describe the associated schema.

`SHOW TABLES` : Show all available tables inside the current database.

`ALTER TABLE {table-name} add {field-name} {field-info};` : Add a new column.

`ALTER TABLE {table-name} drop {field-name};` : Drop an existing column.

### Data Related

`INSERT INTO...`

This command allows a user to insert (one or more) records into a given table. The command accepts a list of fields, and a collection of value lists -- one for each record you want to insert. Below is an example where we are inserting three records.

```
INSERT INTO nba_players 
('first_name', 'last_name', 'team') 
VALUES 
('Doncic','Luka', 'Dallas Mavericks'), 
('Nowitzki', 'Dirk', 'Dallas Mavericks'), 
('Bryant', 'Kobe', 'Los Angeles Lakers');
```

`UPDATE {table-name} SET {field-name} = {value} WHERE {constraint}`

The UPDATE command allows a user to select records from a given table, alter those records in memory, and save the records back out to the storage file.

`DELETE FROM {table-name} WHERE {constraint}`

The DELETE command allows a user to select records from a given table, and remove those rows from Storage. When a user issues the DELETE FROM... command, the system will find rows that match the given constraints (in the WHERE clause).

### Select

The SELECT command allows a user to retrieve (one or more) records from a given table. The command accepts one or more fields to be retrieved (or the *), along with a series of optional arguments (e.g. ORDER BY, LIMIT). Below, are examples of the SELECT statements (presumes the existence of a Users and Accounts table):

`SELECT * FROM  Users`

`SELECT first_name, last_name FROM Users ORDER BY last_name;`

`SELECT...WHERE ... LIMIT N...`

#### Available Arguments

##### ORDER BY

`ORDER BY` argument will format output data with given field.

##### LIMIT

`LIMIT` argument will limit the total number of output data.

##### Join

At this point, only `LEFT JOIN` and `RIGHT JOIN` are available.

### Index

The index system will automatically create/delete/add the associated indexes and their associated data when `CREATE Table`/`DROP Table`/`INSERT Rows` command is called. When you `SELECT` rows, the system will use the primary key index to load records for the table.

`SHOW INDEXES`

This command shows all the indexes defined in current database.

```
> show indexes
+-----------------+-----------------+
| table           | field(s)        | 
+-----------------+-----------------+
| users           | id              |  
+-----------------+-----------------+
1 rows in set (nnnn secs)
```

`SHOW INDEX {field1, field2} FROM {tablename}`

This command shows all the key/value pairs found in an index (shown below).

```
> SHOW INDEX id FROM Users; 
+-----------------+-----------------+
| key             | block#          | 
+-----------------+-----------------+
| 1               | 35              |  
+-----------------+-----------------+
| 2               | 36              |  
+-----------------+-----------------+
| 3               | 47              |  
+-----------------+-----------------+
3 rows in set (nnnn secs)
```



