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
UPDATE TestTable SET score = 99 WHERE id = 2014011425;
SELECT id, score, name FROM TestTable WHERE id = 2014011425;
DELETE FROM TestTable WHERE id = 2014011425;
SELECT id, score, name FROM TestTable WHERE name = 'Max Duan';
SELECT id, score, name FROM TestTable WHERE name = 'Roger Cen';