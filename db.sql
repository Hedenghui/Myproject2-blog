
create database if not exists db_blog;
use db_blog;

drop table if exists tb_blog;
create table if not exists tb_blog(
  id int primary key auto_increment comment '博客ID',
  title varchar(255) comment'博客标题',
  content text comment'博客内容',
  ctime datetime comment'博客的创建时间'
);

insert tb_blog values(null,'这是一个C++博客','##加油',now()),
                    (null,'这是一个Java博客','##加油',now()),
                    (null,'这是一个Linux博客','##加油',now()); 
