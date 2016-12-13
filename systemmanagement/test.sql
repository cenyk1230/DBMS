CREATE DATABASE TestDB;
USE TestDB;
CREATE TABLE TestTable
(
	id int(4) NOT NULL,
	score float(4),
	name varchar(20) NOT NULL,
	PRIMARY KEY (id)
);
SHOW DATABASE TestDB;
SHOW TABLE TestTable;
DROP TABLE TestTable;
DROP DATABASE TestDB;