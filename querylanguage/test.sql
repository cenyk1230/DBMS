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
(2014011425, 73, 'Max Duan'),
(2015011687, , 'Long Guo');
SELECT id, score, name FROM TestTable WHERE id = 2014011415;
DELETE FROM TestTable WHERE name = TestTable.id AND TestTable.id <> TestTable.score AND id IS NOT NULL;
DROP DATABASE TestDB;