#include<iostream>
#include<mysql/mysql.h>
#include<jsoncpp/json/json.h>
#include<mutex>

#define MYSQL_HOST "127.0.0.1"
#define MYSQL_USER "root"
#define MYSQL_PSWD "123456"
#define MYSQL_DB "db_blog"
namespace blog_system
{
  static std::mutex g_mutex;
  MYSQL *MysqlInit()//向外提供接口返回初始化完成的mysql句柄(连接服务器,选择数据库,设置字符集)
  {
     MYSQL *mysql;
    //初始化
     mysql=mysql_init(NULL);
     if(mysql==NULL)
     {
      printf("init mysql error\n");
      return NULL;
     }
    //连接服务器
     if( mysql_real_connect(mysql,MYSQL_HOST,MYSQL_USER,MYSQL_PSWD,NULL,0,NULL,0)==NULL)
   {
     printf("connect mysql server error:%s\n",mysql_error(mysql));
     mysql_close(mysql);
     return NULL;
   }

   //设置字符集
   if(mysql_set_character_set(mysql,"utf8")!=0)
   {
     printf("set client character error:%s\n",mysql_error(mysql));
     mysql_close(mysql);
     return NULL;
   }

   //选择数据库
   if(mysql_select_db(mysql,MYSQL_DB)!=0)
   {
     printf("select db error:%s\n",mysql_error(mysql));
     mysql_close(mysql);
     return NULL;
   }
   return mysql;

    
  }
  void MysqlRelease(MYSQL *mysql)//销毁句柄
  {
    if(mysql)
    {
      mysql_close(mysql);
    }
    return;
  }
  bool MysqlQuery(MYSQL *mysql,const char *sql)//执行语句的共有接口
  {
    int ret=mysql_query(mysql,sql);
    if(ret!=0)
    {
      printf("query sql:[%s]failed:%s\n",sql,mysql_error(mysql));
      return false;
    }
    return true;
  }

  class TableBlog
  {
    public:
      TableBlog(MYSQL *mysql)
        :_mysql(mysql)
      {}
      bool Insert(Json::Value &blog)//从blog中取出博客信息,组织sql语句,将数据插入数据库
      {
        //id tag_id title content ctime
#define INSERT_BLOG "insert tb_blog values(null,'%s','%s',now());"
        //直接定义固定长度,有可能会越界访问
        int len=blog["content"].asString().size()+4096;
      //  std::string sql;
        char* tmp=(char*)malloc(len);
        sprintf(tmp,INSERT_BLOG,blog["title"].asCString(),blog["content"].asCString());
        bool ret=MysqlQuery(_mysql,tmp);
	std::cout<<tmp<<std::endl;
        free(tmp);
        return ret;
      }
      bool Delete(int blog_id)//根据博客id删除博客
      {
#define DELETE_BLOG "delete from tb_blog where id=%d"
        char tmp[1024]={0};
        sprintf(tmp,DELETE_BLOG,blog_id);
        bool ret=MysqlQuery(_mysql,tmp);
        return ret;
      }
      bool Update(Json::Value &blog)//从blog中取出博客信息,组织sql语句,更新数据库的数据
      {
#define UPDATE_BLOG "update tb_blog set title='%s',content='%s' where id=%d;"
        int len=blog["content"].asString().size()+4096;
        char* tmp=(char*)malloc(len);
        sprintf(tmp,UPDATE_BLOG,
            blog["title"].asCString(),
            blog["content"].asCString(),
            blog["id"].asInt());
        bool ret=MysqlQuery(_mysql,tmp);
        free(tmp);
        return ret;
      }


      bool GetAll(Json::Value *blogs)//通过blog返回所有的博客信息(不包含正文)
      {
#define GETALL_BLOG "select id,title,ctime from tb_blog;"

        //执行查询语句
        g_mutex.lock();
        bool ret=MysqlQuery(_mysql,GETALL_BLOG);
        if(ret==false)
        {
          g_mutex.unlock();
          return false;
        }
        //保存结果集
        MYSQL_RES* res=mysql_store_result(_mysql);
        g_mutex.unlock();
        if(res==NULL)
        {
          printf("store all blog result failed:%s\n",mysql_error(_mysql));
          return false;
        }
        //遍历结果集
        int row_num=mysql_num_rows(res);
        for(int i=0;i<row_num;i++)
        {
          MYSQL_ROW row=mysql_fetch_row(res);
          Json::Value blog;
          blog["id"]=std::stoi(row[0]);
          blog["title"]=row[1];
          blog["ctime"]=row[2];
          blogs->append(blog);

        }
        mysql_free_result(res);
        return true;
      }
      bool GetOne(Json::Value *blog)//返回单个博客信息(包含正文)
      {
#define GETONE_BLOG "select title,content,ctime from tb_blog where id=%d;"
        char tmp[1024]={0};
        sprintf(tmp,GETONE_BLOG,(*blog)["id"].asInt());
        g_mutex.lock();
        bool ret=MysqlQuery(_mysql,tmp);
        if(ret==false)
        {
          g_mutex.unlock();
          return false;
        }
        MYSQL_RES *res=mysql_store_result(_mysql);
        g_mutex.unlock();
        if(res==NULL)
        {
          printf("store all blog result failed:%s\n",mysql_error(_mysql));
          return false;
        }
        int row_num=mysql_num_rows(res);
        if(row_num!=1)
        {
          printf("get one blog result error\n");
          mysql_free_result(res);
          return false;
        }
        MYSQL_ROW row=mysql_fetch_row(res);
        (*blog)["title"]=row[0];
        (*blog)["content"]=row[1];
        (*blog)["ctime"]=row[2];
        return true;
      }


    private:
      MYSQL *_mysql;
  };
}
