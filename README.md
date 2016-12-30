##数据库课程项目

###系统要求
  Linux或者OS X

###生成可执行文件
  在querylanguage目录下打开终端，然后执行make，即可编译生成可执行文件QLtest。

###运行方法说明
  在querylanguage目录下打开终端，输入./QLtest filename可以执行filename文件里的所有SQL语句。或者输入./QLtest回车之后，在终端中输入想要执行的SQL语句，然后按CONTROL-D来终止输入开始执行SQL语句。

###UI界面
  需要安装Qt，然后打开vdbms/vdbms.pro，然后点击Configure Project。打开项目之后可以在Debug或者Release模式下运行（推荐在Release模式下），需要把querylanguage目录下生成的可执行文件QLtest改名为dbms放入与vdbms相同的目录下。