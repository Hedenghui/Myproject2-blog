#include"db.hpp"
#include"httplib.h"
using namespace httplib;
blog_system::TableBlog *table_blog;

//数据模块测试代码
//void test()
//{
//  MYSQL* mysql=blog_system::MysqlInit();
 // blog_system::TableBlog table_blog(mysql);
//  Json::Value blog;
//  blog["tag_id"]=2;
 // blog["title"]= "这是一个PHP博客";
//  blog["content"]="php是最好的语言";
 // table_blog.Insert(blog);
// table_blog.Delete(4);
 // blog["id"]=1;
 // table_blog.Update(blog);
//    table_blog.GetAll(&blog);
 //   Json::StyledWriter writer;
  //  std::cout<<writer.write(blog)<<std::endl;
 // table_blog.GetOne(&blog);
//  Json::StyledWriter writer1;
//  std::cout<<writer1.write(blog)<<std::endl;

//}


void InsertBlog(const Request &req,Response &rsp)
{
  //插入博客的业务处理
  //从请求中取出正文----正文就是提交的博客信息,以json格式的字符串形式组织的
  //将Json格式的字符串进行反序列化,得到各个博客信息
  Json::Reader reader;
  Json::Value blog;
  Json::FastWriter writer;
  Json::Value errmsg;
  bool ret=reader.parse(req.body,blog);
  if(ret==false)
  {
    printf("InsertBlog parse blog json failed!\n");
    rsp.status=400;
    errmsg["ok"]=false;
    errmsg["reason"]="parse blog json failed!";
    rsp.set_content(writer.write(errmsg),"application/json");//添加正文到rsp.body中
    return;
  }
  //将得到的博客信息插入到数据库
  ret=table_blog->Insert(blog);
  if(ret==false)
  {
    printf("InsertBlog insert into databases failed!\n");
    rsp.status=500;
    return;
  }
  rsp.status=200;
  return;
}

void DeleteBlog(const Request &req,Response &rsp)
{
  //删除博客的业务处理
  // /blog/123   /blog/(\d+)  req.matches[0]=/blog/1223 req.matches[1]=123;
  int blog_id=std::stoi(req.matches[1]);
  bool ret=table_blog->Delete(blog_id);
  if(ret==false)
  {
    printf("DeleteBlog delete from databases failed!\n");
    rsp.status=500;
    return;
  }
  rsp.status=200;
  return;
}
void UpdateBlog(const Request &req,Response &rsp)
{
  //更改博客的业务处理
  int blog_id =std::stoi(req.matches[1]);
  Json::Value blog;
  Json::Reader reader;
  bool ret =reader.parse(req.body,blog);
  if(ret==false)
  {
    printf("UpdateBlog parse json failed!\n");
    rsp.status=400;
    return;
  }
  blog["id"]=blog_id;
  ret=table_blog->Update(blog);
  if(ret==false)
  {
    printf("UpdateBlog update database failed!\n");
    rsp.status=500;
    return;
  }
  rsp.status=200;
  return;
}
void GetAllBlog(const Request &req,Response &rsp)
{
  //获取全部博客的业务处理
  Json::Value blogs;
  bool ret=table_blog->GetAll(&blogs);
  if(ret==false)
  {
    printf("GetAllBlog select from databases failed!\n");
    rsp.status=500;
    return;
  }
  //将数据进行Json序列化,添加到rsp正文中
  Json::FastWriter writer;
  rsp.set_content(writer.write(blogs),"application/json");
  rsp.status=200;
  return;
}
void GetOneBlog(const Request &req,Response &rsp)
{
  //获取单个博客的业务处理
  int blog_id=std::stoi(req.matches[1]);
  Json::Value blog;
  blog["id"]=blog_id;
  bool ret=table_blog->GetOne(&blog);
  if(ret==false)
  {
    printf("GetoneBlog select from databases failed!\n");
    rsp.status=500;
    return;
  }
  //将数据进行json序列化,添加到rsp正文中
  Json::FastWriter writer;
  rsp.set_content(writer.write(blog),"application/json");
  rsp.status=200;
  return;
}

int main()
{
  MYSQL *mysql=blog_system::MysqlInit();
  table_blog=new blog_system::TableBlog(mysql);
  //业务处理模块
  Server server;
  //设置相对根目录的目的:当客户端请求静态文件资源时,httplib会直接根据路径读取文件数据进行响应
  server.set_base_dir("./www");//设url中的资源相对根目录,并且1请求/时候,自动添加index.html
  //博客信息的增删改查
  server.Post("/blog",InsertBlog);
  // 正则表达式
  //(\d+) 表示匹配多个数字,括号表示临时保存匹配的数字
  server.Delete(R"(/blog/(\d+))",DeleteBlog);//R"()"去除括号中所有字符的特殊含义
  server.Put(R"(/blog/(\d+))",UpdateBlog);
  server.Get("/blog",GetAllBlog);
  server.Get(R"(/blog/(\d+))",GetOneBlog);

  server.listen("0.0.0.0",9000);
  return 0;  
}
