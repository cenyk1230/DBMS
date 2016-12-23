CREATE DATABASE TestDB;
USE TestDB;
CREATE TABLE TestTable
(
	id int(4) NOT NULL,
	score int(4),
	name string(20) NOT NULL,
	PRIMARY KEY (id)
);
SHOW DATABASES;
SHOW DATABASE TestDB;
SHOW TABLE TestTable;
INSERT INTO TestTable VALUES
(2014011415, 100, 'Roger Cen'),
(2014011425, 73, 'Max Duan');
SELECT id, score, name FROM TestTable WHERE id = 2014011415;
